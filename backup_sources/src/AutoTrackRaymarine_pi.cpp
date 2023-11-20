/******************************************************************************
*
* Project:  OpenCPN
* Purpose:  AutoTrackRaymarine Plugin
* Author:   Douwe Fokkema
*
***************************************************************************
*   Copyright (C) 2019 by Douwe Fokkema                                   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
***************************************************************************
*/

#include <wx/wx.h>
#include <wx/stdpaths.h>
#include <wx/aui/aui.h>
#include "AutoTrackRaymarine_pi.h"
#include "PreferencesDialog.h"
#include "icons.h"
#include "GL/gl.h"
#include "actisense.h"
#include "actisense.h"
#include "SerialPort.h"
#include "AutotrackInfoUI.h"
#include "Info.h"


//using namespace std;
#define TURNRATE 20.   // turnrate per second

double heading_resolve(double degrees, double offset = 0)
{
  while (degrees < offset - 180)
    degrees += 360;
  while (degrees >= offset + 180)
    degrees -= 360;
  return degrees;
}

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr)
{
  return new AutoTrackRaymarine_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p)
{
  delete p;
}



//-----------------------------------------------------------------------------
//
//    AutoTrackRaymarine PlugIn Implementation
//
//-----------------------------------------------------------------------------

AutoTrackRaymarine_pi::AutoTrackRaymarine_pi(void *ppimgr)
  :opencpn_plugin_117(ppimgr)
{
  // Create the PlugIn icons
  initialize_images();
  
// Create the PlugIn icons  -from shipdriver
// loads png file for the listing panel icon
    wxFileName fn;
    auto path = GetPluginDataDir("AutoTrackRaymarine_pi");
    fn.SetPath(path);
    fn.AppendDir("data");
    fn.SetFullName("tracking_panel.png");

    path = fn.GetFullPath();

    wxInitAllImageHandlers();

    wxLogDebug(wxString("Using icon path: ") + path);
    if (!wxImage::CanRead(path)) {
        wxLogDebug("Initiating image handlers.");
        wxInitAllImageHandlers();
    }
    wxImage panelIcon(path);
    if (panelIcon.IsOk())
        m_panelBitmap = wxBitmap(panelIcon);
    else
        wxLogWarning("AutoTrackRaymarine panel icon has NOT been loaded");
// End of from Shipdrive 
  
  
  m_info_dialog = NULL;
  m_PreferencesDialog = NULL;
  m_pdeficon = new wxBitmap(*_img_AutoTrackRaymarine);
  m_initialized = false;
}

//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

int AutoTrackRaymarine_pi::Init(void)
{
  //AddLocaleCatalog(PLUGIN_CATALOG_NAME);
  
  // Read Config
  wxFileConfig *pConf = GetOCPNConfigObject();
  pConf->SetPath(_T("/Settings/AutoTrackRaymarine"));

  m_heading_set = false;
  m_current_bearing = 0.;
  m_XTE = 0.;
  m_BTW = 0.;
  m_var = 0.;
  m_XTE_P = 0.;
  m_XTE_I = 0.;
  m_XTE_D = 0.;
  m_pilot_heading = -1.;  // target heading of pilot in auto mode, -1 means undefined
  m_vessel_heading = -1.;
  SetStandby();

  // Mode
  preferences &p = prefs;
  
  p.max_angle = pConf->Read("MaxAngle", 30);
  p.com_port = pConf->Read("ActisenseComPort", "");
  
  // options
  p.confirm_bearing_change = (bool)pConf->Read("ConfirmBearingChange", 0L);
  p.intercept_route = (bool)pConf->Read("InterceptRoute", 1L);
  
  // Boundary
  p.boundary_guid = pConf->Read("Boundary", "");
  p.boundary_width = pConf->Read("BoundaryWidth", 30);

  m_serial_comms = new SerialPort(this);

  ShowInfoDialog();
  m_XTE_refreshed = false;
  m_route_active = false;
  m_pilot_heading = -1.;     // undefined
  m_vessel_heading = -1.;       // current heading of vessel according to pilot, undefined
  m_XTE = 100000.;      // undefined

  m_Timer.Connect(wxEVT_TIMER, wxTimerEventHandler(AutoTrackRaymarine_pi::OnTimer), NULL, this);
  m_Timer.Start(1000);

  //    This PlugIn needs a toolbar icon

#ifdef PLUGIN_USE_SVG
   m_tool_id = InsertPlugInToolSVG(_T( "AutoTrackRaymarine" ),
       _svg_tracking, _svg_tracking_toggled, _svg_tracking_toggled,
       wxITEM_NORMAL, _("Tracking"), _T( "Track Following for Raymarine Evolution Pilots" ), NULL, TRACKING_TOOL_POSITION, 0, this);
#else
   m_tool_id  = InsertPlugInTool( _T(""), _img_AutoTrackRaymarine,
      _img_AutoTrackRaymarine, wxITEM_NORMAL, _("AutoTrackRaymarine"), _T(""), NULL,
	  TRACKING_TOOL_POSITION, 0, this);
#endif 

  SetStandby();
  m_initialized = true;

//  m_shareLocn = *GetpSharedDataLocation() + _T("plugins") + wxFileName::GetPathSeparator() + _T("AutoTrackRaymarine_pi") +
//   wxFileName::GetPathSeparator() + _T("data") + wxFileName::GetPathSeparator();
//  wxString svg_normal = m_shareLocn + wxT("tracking.svg");
//  m_tool_id = InsertPlugInToolSVG(wxT("Tracking"), svg_normal, svg_normal, svg_normal, wxITEM_NORMAL, wxT("Tracking"),
//    _("Track following for Raymarine Evolution pilots"), NULL, TRACKING_TOOL_POSITION, 0, this);
//  SetStandby();
//  m_initialized = true;

    // initialize NavMsg listeners
 //-----------------------------

 // Heading PGN 127250
  wxDEFINE_EVENT(EVT_N2K_127250, ObservedEvt);
  NMEA2000Id id_127250 = NMEA2000Id(127250);
  listener_127250 = std::move(GetListener(id_127250, EVT_N2K_127250, this));
  Bind(EVT_N2K_127250, [&](ObservedEvt ev) {
      HandleN2K_127250(ev);
      });

  /*std::vector<DriverHandle> drivers = GetActiveDrivers();*/
  for (const auto& handle : GetActiveDrivers()) {
      const auto& attributes = GetAttributes(handle);
      if (attributes.find("protocol") == attributes.end()) continue;
      wxLogMessage(wxT("$$$ handle proto %s"), attributes.at("protocol"));
      if (attributes.at("protocol") == "nmea2000") {
          m_handleN2k = handle;
          wxLogMessage(wxT("$$$ handle  found"));
          break; }
      /*if (attributes.find("commPort") == attributes.end()) continue;
      if (attributes.at("commPort").find(my_port) == string::npos) continue;*/
      wxLogMessage(wxT("$$$ handle not found"));
  }

  std::vector<int> pgn_list = { 127250 };
  CommDriverResult xx = RegisterTXPGNs(m_handleN2k, pgn_list);


  return (WANTS_OVERLAY_CALLBACK |
    WANTS_OPENGL_OVERLAY_CALLBACK |
    WANTS_CURSOR_LATLON |
    WANTS_NMEA_SENTENCES |
    WANTS_NMEA_EVENTS |
    WANTS_AIS_SENTENCES |
    WANTS_PLUGIN_MESSAGING |
    WANTS_PREFERENCES |
    WANTS_CONFIG);



}

//wxBitmap* AutoTrackRaymarine_pi::GetPlugInBitmap() { return m_pdeficon; }

wxString AutoTrackRaymarine_pi::GetCommonName() { return wxT("XX"); }

wxString AutoTrackRaymarine_pi::GetShortDescription() { return _("XX"); }

int AutoTrackRaymarine_pi::GetAPIVersionMajor() { return MY_API_VERSION_MAJOR; }

int AutoTrackRaymarine_pi::GetAPIVersionMinor() { return MY_API_VERSION_MINOR; }

//int AutoTrackRaymarine_pi::GetPlugInVersionMajor() { return PLUGIN_VERSION_MAJOR; }

wxString AutoTrackRaymarine_pi::GetLongDescription() {
    return _("Route following for Raymarine EV pilots") + wxT("\n") /*+ wxT(PLUGIN_VERSION_WITH_DATE)*/;
}

// wxBitmap *AutoTrackRaymarine_pi::GetPlugInBitmap() { return m_pdeficon; }

void AutoTrackRaymarine_pi::OnToolbarToolCallback(int id) {
  if (!m_initialized) {
    return;
  }
  if (m_info_dialog->IsShown()) {
    m_info_dialog->Hide();
  } else {
    m_info_dialog->Show();
  }
}

bool AutoTrackRaymarine_pi::DeInit(void) {
    // No logging here, will crash OpenCPN in DoLogRecord because of illegal pointer to file
  if (!m_initialized) return true;
  delete m_PreferencesDialog;
  RemovePlugInTool(m_leftclick_tool_id);

  // save config
  wxFileConfig *pConf = GetOCPNConfigObject();
  pConf->SetPath(_T("/Settings/AutoTrackRaymarine"));

  if (m_info_dialog) {
    wxPoint p = m_info_dialog->GetPosition();
    pConf->Write("PosX", p.x);
    pConf->Write("PosY", p.y);
    }
  delete m_info_dialog;

  preferences &p = prefs;
  
  pConf->Write("MaxAngle", p.max_angle);
  pConf->Write("ActisenseComPort", p.com_port);
  
  // Waypoint Arrival
  pConf->Write("ConfirmBearingChange", p.confirm_bearing_change);
  pConf->Write("InterceptRoute", p.intercept_route);
  m_Timer.Stop();
  m_Timer.Disconnect(wxEVT_TIMER, wxTimerEventHandler(AutoTrackRaymarine_pi::OnTimer), NULL, this);

  //  Close serial port and stop reader thread NGT-1
  if (m_serial_comms) {
      delete(m_serial_comms);
      m_serial_comms = NULL;
   }
  m_initialized = false;
  return true;
}


//void HandleN2K_65360(ObservedEvt ev); // Pilot heading
//void HandleN2K_126208(ObservedEvt ev); // Set Set pilot heading or set auto/standby
//void HandleN2K_126720(ObservedEvt ev); // From EV1 (204) indicating auto or standby state
//void HandleN2K_65359(ObservedEvt ev); // Vessel heading, proprietary



void AutoTrackRaymarine_pi::HandleN2K_127250(ObservedEvt ev) {  // Vessel heading, standerd NMEA2000
    NMEA2000Id id_127250(127250);

    std::vector<uint8_t>msg = GetN2000Payload(id_127250, ev);
    double p_h = ((unsigned int)msg[14] + 256 * (unsigned int)msg[15]) * 360. / 3.141 / 20000;
    m_vessel_heading = p_h + m_var;
    //wxLogMessage(wxT("$$$ 127250 heading=%f"));
}

//int AutoTrackRaymarine_pi::GetAPIVersionMajor()
//{
//      //return OCPN_API_VERSION_MAJOR;
//}

//int AutoTrackRaymarine_pi::GetAPIVersionMinor()
//{
//     //return OCPN_API_VERSION_MINOR;
//}

//int AutoTrackRaymarine_pi::GetPlugInVersionMajor()
//{
//       //return PLUGIN_VERSION_MAJOR;
//}
//
//int AutoTrackRaymarine_pi::GetPlugInVersionMinor()
//{
//      //return PLUGIN_VERSION_MINOR;
//}
//
//int AutoTrackRaymarine_pi::GetPlugInVersionPatch()
//{
//      //return PLUGIN_VERSION_PATCH;
//}


//wxBitmap *AutoTrackRaymarine_pi::GetPlugInBitmap()
//{
//  return new wxBitmap(_img_AutoTrackRaymarine->ConvertToImage().Copy());
//}

// Uses the tracking_panel.png file to make the bitmap.
//wxBitmap *AutoTrackRaymarine_pi::GetPlugInBitmap()  { return &m_panelBitmap; }
// End of  process  (from Shipdriver)



//wxString AutoTrackRaymarine_pi::GetCommonName()
//{
//   return _T(PLUGIN_COMMON_NAME);
//}
//
//wxString AutoTrackRaymarine_pi::GetShortDescription()
//{
//   return _(PLUGIN_SHORT_DESCRIPTION);
//}
//
//wxString AutoTrackRaymarine_pi::GetLongDescription()
//{
//    return _(PLUGIN_LONG_DESCRIPTION);
//}



void AutoTrackRaymarine_pi::ShowPreferencesDialog(wxWindow* parent)
{
  if (NULL == m_PreferencesDialog)
    m_PreferencesDialog = new PreferencesDialog(parent, *this);

  m_PreferencesDialog->ShowModal();

  delete m_PreferencesDialog;
  m_PreferencesDialog = NULL;
}

void AutoTrackRaymarine_pi::OnTimer(wxTimerEvent &)
{
  wxWindow *canvas = GetCanvasByIndex(0);
  if (canvas) {
    canvas->Refresh(false);
  }
}

bool AutoTrackRaymarine_pi::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp) {
  if (!m_initialized) {
    return true;
  }
  if (m_info_dialog) {
     m_info_dialog->UpdateInfo();
  }
  if (m_XTE_refreshed) {
    m_XTE_refreshed = false;
    Compute();
  }
  return true;
}

bool AutoTrackRaymarine_pi::RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp){
  if (!m_initialized) {
    return true;
  }
  if (m_info_dialog) {
    m_info_dialog->UpdateInfo();
    }
  if (m_XTE_refreshed) {
    m_XTE_refreshed = false;
    Compute();
  }
  return true;
}


void AutoTrackRaymarine_pi::ShowInfoDialog()
{
  if (!m_info_dialog) {
    wxFileConfig *pConf = GetOCPNConfigObject();
    pConf->SetPath(_T("/Settings/AutoTrackRaymarine"));
    wxPoint pos(pConf->Read("PosX", 0L), pConf->Read("PosY", 50));

    m_info_dialog = new InfoDialog(GetOCPNCanvasWindow(), this);
    m_info_dialog->SetPosition(pos);
    m_info_dialog->EnableHeadingButtons(false);
    m_info_dialog->EnableTrackButton(false);
    wxSize sz = m_info_dialog->GetSize();
    m_info_dialog->Show();
  }
}

void AutoTrackRaymarine_pi::ShowPreferences()
{
  if (!m_PreferencesDialog) {
    m_PreferencesDialog = new PreferencesDialog(GetOCPNCanvasWindow(), *this);
    wxIcon icon;
    icon.CopyFromBitmap(*_img_AutoTrackRaymarine);
    m_PreferencesDialog->SetIcon(icon);
  }
  m_PreferencesDialog->Show();
}

wxString AutoTrackRaymarine_pi::StandardPath()
{
  wxStandardPathsBase& std_path = wxStandardPathsBase::Get();
  wxString s = wxFileName::GetPathSeparator();

#if defined(__WXMSW__)
  wxString stdPath = std_path.GetConfigDir();
#elif defined(__WXGTK__) || defined(__WXQT__)
  wxString stdPath = std_path.GetUserDataDir();
#elif defined(__WXOSX__)
  wxString stdPath = (std_path.GetUserConfigDir() + s + _T("opencpn"));
#endif

  stdPath += s + _T("plugins");
  if (!wxDirExists(stdPath))
    wxMkdir(stdPath);

  stdPath += s + _T("AutoTrackRaymarine");

#ifdef __WXOSX__
  // Compatibility with pre-OCPN-4.2; move config dir to
  // ~/Library/Preferences/opencpn if it exists
  wxString oldPath = (std_path.GetUserConfigDir() + s + _T("plugins") + s + _T("weatherfax"));
  if (wxDirExists(oldPath) && !wxDirExists(stdPath)) {
    wxLogMessage("weatherfax_pi: moving config dir %s to %s", oldPath, stdPath);
    wxRenameFile(oldPath, stdPath);
  }
#endif

  if (!wxDirExists(stdPath))
    wxMkdir(stdPath);

  stdPath += s; // is this necessary?
  return stdPath;
}


void AutoTrackRaymarine_pi::SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix)
{
  m_var = pfix.Var;
}

void AutoTrackRaymarine_pi::SetActiveLegInfo(Plugin_Active_Leg_Info &leg_info) {
 // wxLogMessage(wxString("AutoTrackRaymarine_pi: SetActiveLegInfo called xte=%f, BTW= %f, DTW= %f, name= %s"), leg_info.xte, leg_info.btw, leg_info.dtw, leg_info.wp_name);
  m_XTE = leg_info.Xte;
  if (isnan(m_XTE)) {
      m_XTE = 0.;
      wxLogMessage(wxString("AutoTrackRaymarine_pi: m_XTE is NaN"));
  }
  if (m_XTE > -0.000001 && m_XTE < 0.) m_XTE = 0.;
  m_XTE_refreshed = true;
  m_route_active = true;  // when SetActiveLegInfo is called a route must be active
  if (!isnan(leg_info.Btw)) {
      m_BTW = leg_info.Btw;
  }
}

void AutoTrackRaymarine_pi::SetPluginMessage(wxString &message_id, wxString &message_body)
{
  if (message_id == wxS("AutoTrackRaymarine_pi")) {
    return; // nothing yet
  }
  else if (message_id == wxS("AIS")) {
  }
  else if (message_id == "OCPN_RTE_ACTIVATED") {
    ResetXTE();
    if (m_pilot_state == TRACKING) {
      SetStandby();
    }
    m_route_active = true;
    m_info_dialog->EnableTrackButton(true);
  }
  else if (message_id == "OCPN_WPT_ARRIVED") {
  }
  else if (message_id == "OCPN_RTE_DEACTIVATED" || message_id == "OCPN_RTE_ENDED") {
    m_route_active = false;
    m_XTE = 100000.;  // undefined
    wxCommandEvent event;
    if (m_info_dialog) {
      m_info_dialog->OnStandby(event);
      m_info_dialog->EnableTrackButton(false);
    }
  }
}

void AutoTrackRaymarine_pi::SetStandby() {
  m_pilot_state = STANDBY;
  if (m_info_dialog) {
    m_info_dialog->EnableHeadingButtons(false);
    m_info_dialog->EnableTrackButton(true);
  }
}

void AutoTrackRaymarine_pi::SetAuto() {
  m_pilot_state = AUTO;
  if (m_info_dialog) {
    m_info_dialog->EnableHeadingButtons(true);
  }
}

void AutoTrackRaymarine_pi::SetTracking() {
  m_pilot_state = TRACKING;
  if (m_info_dialog) {
    m_info_dialog->EnableHeadingButtons(true);
  }
  ResetXTE();  // reset local XTE calculations
  ZeroXTE();   // zero XTE on OpenCPN
}

void AutoTrackRaymarine_pi::Compute(){
  double dist;
  double XTE_for_correction;
  if (isnan(m_BTW)) return;
  if (isnan(m_XTE) || m_XTE == 100000.) return;
  if (m_pilot_state != TRACKING || !m_route_active) {
    return;
  }
  dist = 50; // in meters
  double dist_nm = dist / 1852.;

  // integration of XTE, but prevent increase of m_XTE_I when XTE is large
  if (m_XTE > -0.25 * dist_nm && m_XTE < 0.25 * dist_nm) {
    m_XTE_I += m_XTE;
  }
  else if (m_XTE > -0.5 * dist_nm && m_XTE < 0.5 * dist_nm) {
    m_XTE_I += 0.5 * m_XTE;
  }
  else if (m_XTE > - dist_nm && m_XTE < dist_nm) {
    m_XTE_I += 0.2 * m_XTE;
  }
  else {
  };            // do nothing for now

  m_XTE_D = m_XTE - m_XTE_P;  // difference
  m_XTE_P = m_XTE;            // proportional used as previous xte next timw

  if (m_XTE_I > 0.5 * dist_nm / I_FACTOR) { // in NM
    m_XTE_I = 0.5 * dist_nm / I_FACTOR;
  }
  if (m_XTE_I < -0.5 * dist_nm / I_FACTOR) { // in NM
    m_XTE_I = -0.5 * dist_nm / I_FACTOR;
  }

  XTE_for_correction = m_XTE + I_FACTOR * m_XTE_I + D_FACTOR * m_XTE_D;

  //wxLogMessage(wxT(" XTE_for_correction=%f, 5 * m_XTE=%f,  I_FACTOR * m_XTE_I=%f, D_FACTOR * m_XTE_D=%f"),
  // XTE_for_correction, 5 * m_XTE, I_FACTOR * m_XTE_I, D_FACTOR * m_XTE_D);

  double gamma, new_bearing;  // angle for correction of heading relative to BTW
  if (dist > 1.) {
    gamma = atan( XTE_for_correction * 1852. / dist) / (2. * 3.1416) * 360.;
  }
  double max_angle = prefs.max_angle;
  //wxLogMessage(wxT("AutoTrackRaymarine initial gamma=%f, btw=%f, dist=%f, max_angle= %f, XTE_for_correction=%f"), gamma, m_BTW, dist, max_angle, XTE_for_correction);
  new_bearing = m_BTW + gamma;                          // bearing of next wp

  if (gamma > max_angle) {
    new_bearing = m_BTW + max_angle;
  }
  else
    if (gamma < -max_angle) {
      new_bearing = m_BTW - max_angle;
    }
  // don't turn too fast....

  if (!m_heading_set) {    // after reset accept any turn
    m_current_bearing = new_bearing;
    m_heading_set = true;
  }
  else {
    while (new_bearing >= 360.) new_bearing -= 360.;
    while (new_bearing < 0.) new_bearing += 360.;
    double turnrate = TURNRATE;

    // turn left or right?
    double turn = new_bearing - m_current_bearing;

    if (turn < -180.) turn += 360;
    if (turn > 80. || turn < -80.) turnrate = 2 * TURNRATE;
    if (turn < -turnrate || (turn > 180. && turn < 360 - turnrate)) {
      // turn left
      m_current_bearing -= turnrate;
    }
    else if (turn > turnrate && turn <= 180.) {
      // turn right
      m_current_bearing += turnrate;
    }
    else {
      // go almost straight, correction < TURNRATE
      m_current_bearing = new_bearing;
    }
  }
  while (m_current_bearing >= 360.) m_current_bearing -= 360.;
  while (m_current_bearing < 0.) m_current_bearing += 360.;
  m_serial_comms->SetAutopilotHeading(m_current_bearing - m_var);  // the commands used expect magnetic heading
  m_pilot_heading = m_current_bearing; // This should not be needed, pilot heading will come from pilot. For testing only.
  SendHSC(m_current_bearing);
}

void AutoTrackRaymarine_pi::ChangePilotHeading(int degrees) {
  if (m_pilot_state == STANDBY) {
    return;
  }
  if (m_pilot_state == TRACKING) {  // N.B.: for the pilot AUTO and TRACKING is the same
    SetAuto();
  }
  double new_pilot_heading = m_pilot_heading + (double) degrees;
  if (new_pilot_heading >= 360.) new_pilot_heading -= 360.;
  if (new_pilot_heading < 0.) new_pilot_heading += 360.;
  m_serial_comms->SetAutopilotHeading(new_pilot_heading - m_var); //send magnitic heading to Raymarine
  m_pilot_heading = new_pilot_heading; // this should not be needed, pilot heading will come from pilot. For testing only.
  SendHSC(new_pilot_heading);
}


//NMEA0183    NMEA0183;



void AutoTrackRaymarine_pi::SendHSC(double course) {

  // For autopilots that aaccept this message, I do not know if they exist
  // Used for testing with a modified shipdriver_pi

  /*
  HSC - Heading Steering Command

  1   2 3   4  5
  |   | |   |  |
  $--HSC, x.x, T, x.x, M, *hh<CR><LF>

  Field Number :

  1 Heading Degrees, True

  2. T = True

  3. Heading Degrees, Magnetic

  4. M = Magnetic

  Checksum */

  wxString nmea;
  char sentence[40];
  char checksum = 0;
  char *p;

  snprintf(sentence, sizeof(sentence), "AUHSC,%.1f,T", course);

  for (p = sentence; *p; p++) {
    checksum ^= *p;
  }
  nmea.Printf(wxT("$%s*%02X\r\n"), sentence, (unsigned)checksum);
  PushNMEABuffer(nmea);
}
