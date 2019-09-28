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
#include "jsoncpp/json/json.h"
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
#define TURNRATE 10.   // turnrate per second

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
  : opencpn_plugin_110(ppimgr)
{
  // Create the PlugIn icons
  initialize_images();
  m_InfoDialog = NULL;
  m_PreferencesDialog = NULL;
  m_initialized = false;
}

//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

int AutoTrackRaymarine_pi::Init(void)
{
  AddLocaleCatalog(PLUGIN_CATALOG_NAME);
  
  // Read Config
  wxFileConfig *pConf = GetOCPNConfigObject();
  pConf->SetPath(_T("/Settings/AutoTrackRaymarine"));

  m_heading_set = false;
  m_pilot_state = STANDBY;

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

  ShowConsoleCanvas();
  m_XTE_refreshed = false;
  m_initialized = true;
  m_route_active = false;
  m_pilot_heading = -1.;     // undefined
  m_vessel_heading = -1.;       // current heading of vessel according to pilot, undefined
  m_XTE = 100000.;      // undefined

  m_Timer.Connect(wxEVT_TIMER, wxTimerEventHandler(AutoTrackRaymarine_pi::OnTimer), NULL, this);
  m_Timer.Start(1000);

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

bool AutoTrackRaymarine_pi::DeInit(void)
{
  //PlugInHandleAutoTrackRaymarine(false);
  delete m_PreferencesDialog;

  RemovePlugInTool(m_leftclick_tool_id);

  // save config
  wxFileConfig *pConf = GetOCPNConfigObject();
  pConf->SetPath(_T("/Settings/AutoTrackRaymarine"));

  if (m_InfoDialog) {
    wxPoint p = GetFrameAuiManager()->GetPane(m_InfoDialog).floating_pos;
    pConf->Write("PosX", p.x);
    pConf->Write("PosY", p.y);
  }
  delete m_InfoDialog;

  preferences &p = prefs;
  
  pConf->Write("MaxAngle", p.max_angle);
  pConf->Write("ActisenseComPort", p.com_port);
  
  // Waypoint Arrival
  pConf->Write("ConfirmBearingChange", p.confirm_bearing_change);
  pConf->Write("InterceptRoute", p.intercept_route);

  //  Close serial port and stop reader thread NGT-1
  delete (m_serial_comms);
  m_Timer.Stop();
  m_Timer.Disconnect(wxEVT_TIMER, wxTimerEventHandler(AutoTrackRaymarine_pi::OnTimer), NULL, this);

  return true;
}

int AutoTrackRaymarine_pi::GetAPIVersionMajor()
{
  return MY_API_VERSION_MAJOR;
}

int AutoTrackRaymarine_pi::GetAPIVersionMinor()
{
  return MY_API_VERSION_MINOR;
}

int AutoTrackRaymarine_pi::GetPlugInVersionMajor()
{
  return PLUGIN_VERSION_MAJOR;
}

int AutoTrackRaymarine_pi::GetPlugInVersionMinor()
{
  return PLUGIN_VERSION_MINOR;
}

wxBitmap *AutoTrackRaymarine_pi::GetPlugInBitmap()
{
  return new wxBitmap(_img_AutoTrackRaymarine->ConvertToImage().Copy());
}

wxString AutoTrackRaymarine_pi::GetCommonName()
{
  return _("AutoTrackRaymarine");
}

wxString AutoTrackRaymarine_pi::GetShortDescription()
{
  return _("AutoTrackRaymarine PlugIn for OpenCPN");
}

wxString AutoTrackRaymarine_pi::GetLongDescription()
{
  return _("AutoTrackRaymarine PlugIn for OpenCPN\n\
Route tracking and remote control for Raymarine Evolution pilots");
}

//void AutoTrackRaymarine_pi::SetColorScheme(PI_ColorScheme cs) // $$$
//{
//  m_colorscheme = cs;
//  if (m_InfoDialog)
//    m_InfoDialog->SetColorScheme(cs);
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
    wxLogMessage(wxString("$$$$ canvas refreshed"));
  }
}

bool AutoTrackRaymarine_pi::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp) {
  if (!m_initialized) {
    return true;
  }
  if (m_InfoDialog) {
    wxLogMessage(wxString("$$$ update  i nfo called, m_pilot_state= %i"), m_pilot_state);
    m_InfoDialog->UpdateInfo();
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
  if (m_InfoDialog) {
    m_InfoDialog->UpdateInfo();
    wxLogMessage(wxString("$$$ update  info called, m_pilot_state= %i"), m_pilot_state);
  }
  if (m_XTE_refreshed) {
    m_XTE_refreshed = false;
    Compute();
  }
  return true;
}


void AutoTrackRaymarine_pi::ShowConsoleCanvas()
{
  wxLogMessage(wxString("AutoTrackRaymarine_pi: $$$ show1"));
  if (!m_InfoDialog) {
    wxLogMessage(wxString("AutoTrackRaymarine_pi: $$$ show2"));
    wxFileConfig *pConf = GetOCPNConfigObject();
    pConf->SetPath(_T("/Settings/AutoTrackRaymarine"));
    wxPoint pos(pConf->Read("PosX", 0L), pConf->Read("PosY", 50));

    m_InfoDialog = new InfoDialog(GetOCPNCanvasWindow(), this);
    //wxLogMessage(wxString("AutoTrackRaymarine_pi: $$$ show3"));
    wxSize sz = m_InfoDialog->GetSize();
    m_InfoDialog->Show();
    
  }
  wxLogMessage(wxString("AutoTrackRaymarine_pi: $$$ show3"));
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

static bool ParseMessage(wxString &message_body, Json::Value &root)
{
  Json::Reader reader;
  if (reader.parse(std::string(message_body), root))
    return true;

  wxLogMessage(wxString("AutoTrackRaymarine_pi: Error parsing JSON message: ") + reader.getFormattedErrorMessages());
  return false;
}


void AutoTrackRaymarine_pi::SetPluginMessage(wxString &message_id, wxString &message_body)
{
  // construct the JSON root object
  Json::Value  root;
  // construct a JSON parser
  wxString    out;

  if (message_id == wxS("AutoTrackRaymarine_pi")) {
    return; // nothing yet
  }
  else if (message_id == wxS("AIS")) {
  }
  else if (message_id == "OCPN_RTE_ACTIVATED") {
    if (ParseMessage(message_body, root)) {
      ResetXTE();
      ShowConsoleCanvas();
      if (m_pilot_state == TRACKING) {
        m_pilot_state = STANDBY;
      }
      m_route_active = true;
    }
  }
  else if (message_id == "OCPN_WPT_ACTIVATED") {
    wxString guid = root["GUID"].asString();
    m_last_wpt_activated_guid = guid;
    //ShowConsoleCanvas();
  }
  else if (message_id == "OCPN_WPT_ARRIVED") {
  }
  else if (message_id == "OCPN_RTE_DEACTIVATED" || message_id == "OCPN_RTE_ENDED") {
    m_active_guid = "";
    m_route_active = false;
    m_XTE = 100000.;  // undefined
    wxCommandEvent event;
    m_InfoDialog->OnStandby(event);
  }
}


void AutoTrackRaymarine_pi::SetNMEASentence(wxString &sentence) {
  m_NMEA0183 << sentence;
  time_t now = time(0);
  
  float xte;

  //wxLogMessage(wxT("AutoTrackRaymarine: $$$SetNMEASentence %s"), sentence.c_str());
  wxString nm = sentence;
  

  if (m_NMEA0183.PreParse()) {
    // it seems that we can't parse XTE, so do it ourselves
    if (m_NMEA0183.LastSentenceIDReceived == _T("XTE")) {
      //if (m_NMEA0183.Parse()) {
      //  //m_NMEA0183.Xte.CrossTrackErrorDistance;
      //}
      //else {
      //  wxLogMessage(wxT("AutoTrackRaymarine: $$$ no parse"));
      //}
    }
    if (sentence.c_str()[3] == 'X' && sentence.c_str()[4] == 'T' && sentence.c_str()[5] == 'E') {
      wxLogMessage(wxT("AutoTrackRaymarine: $$$SetNMEASentence XTE = %s"), sentence);
      for (int i = 0; i < 3; i++) {
        nm = nm.AfterFirst(',');
      }
      
      sscanf(nm, "%f", &xte);
      nm = nm.AfterFirst(',');
      if (nm[0] == 'L') {
        xte = - xte;
      }
      m_XTE = xte;
      if (m_XTE > -0.00001 && m_XTE < 0.) m_XTE = 0.;
      m_XTE_refreshed = true;
      m_route_active = true;  // when XTE messages arrive a route must be active
    }
    else if (m_NMEA0183.LastSentenceIDReceived == _T("APB") && m_NMEA0183.Parse()) {
      m_BTW = m_NMEA0183.Apb.BearingPresentPositionToDestination;
    }
  }

}

void AutoTrackRaymarine_pi::Compute(){
  double dist;
  double XTE_for_correction;

  if (m_pilot_state != TRACKING || !m_route_active) {
    return;
  }
  wxLogMessage(wxT("AutoTrackRaymarine: $$$compute m_XTE= %f"), m_XTE);
  dist = 50; // in meters  // change into waypoint arrival distance  $$$
  double dist_nm = dist / 1852.;

  // integration of XTE, but prevent increase of m_XTE_I when XTE is large
  if (m_XTE > -0.25 * dist_nm && m_XTE < 0.25 * dist_nm) {
    m_XTE_I += m_XTE;
  }
  else if (m_XTE > -0.5 * dist_nm && m_XTE < 0.5 * dist_nm) {
    m_XTE_I += 0.5 * m_XTE;
   // wxLogMessage(wxT("AutoTrackRaymarine: $$$4 added .5"));
  }
  else if (m_XTE > - dist_nm && m_XTE < dist_nm) {
    m_XTE_I += 0.2 * m_XTE;
   // wxLogMessage(wxT(AutoTrackRaymarine: $$$4 added .2"));
  }
  else {
   // wxLogMessage(wxT("AutoTrackRaymarine: $$$4 added nothing"));
  };            // do nothing for now

  m_XTE_D = m_XTE - m_XTE_P;  // difference
  m_XTE_P = m_XTE;            // proportional used as previous xte next timw

  if (m_XTE_I > 0.5 * dist_nm / I_FACTOR) { // in NM
    wxLogMessage(wxT("AutoTrackRaymarine: $$$4 limited1"));
    m_XTE_I = 0.5 * dist_nm / I_FACTOR;
  }
  if (m_XTE_I < -0.5 * dist_nm / I_FACTOR) { // in NM
    m_XTE_I = -0.5 * dist_nm / I_FACTOR;
    wxLogMessage(wxT("AutoTrackRaymarine: $$$4 limited2"));
  }

  XTE_for_correction = m_XTE + I_FACTOR * m_XTE_I + D_FACTOR * m_XTE_D;
  wxLogMessage(wxT("AutoTrackRaymarine: $$$ m_XTE_P=%f, m_XTE_I=%f, m_XTE_D=%f, XTE_for_correction=%f"),
    m_XTE_P, m_XTE_I, m_XTE_D, XTE_for_correction);

  double gamma, new_bearing;  // angle for correction of heading relative to BTW
  if (dist > 1.) {
    gamma = atan(XTE_for_correction * 1852. / dist) / (2. * 3.14) * 360.;
  }
  double max_angle = prefs.max_angle;
  wxLogMessage(wxT("AutoTrackRaymarine: $$$$ initial gamma=%f, btw=%f, dist=%f, max_angle= %f"), gamma, m_BTW, dist, max_angle);
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
  wxLogMessage(wxT("AutoTrackRaymarine $$$ state == tracking, heading send to raymarine"));
  m_serial_comms->SetAutopilotHeading(m_current_bearing - m_var);  // the commands used expect magnetic heading
  m_pilot_heading = m_current_bearing; // $$$ this should not be needed, pilot heading will come from pilot. For testing only.
  SendHSC(m_current_bearing);
}

void AutoTrackRaymarine_pi::ChangePilotHeading(int degrees) {
  if (m_pilot_state == STANDBY) {
    return;
  }
  if (m_pilot_state == TRACKING) {  // N.B.: for the pilot AUTO and TRACKING is the same
    m_pilot_state = AUTO;
  }
  double new_pilot_heading = m_pilot_heading + (double) degrees;
  if (new_pilot_heading >= 360.) m_pilot_heading -= 360.;
  if (new_pilot_heading < 0.) m_pilot_heading += 360.;
  m_serial_comms->SetAutopilotHeading(new_pilot_heading - m_var); //$$$  and display one decimal
  m_pilot_heading = new_pilot_heading; // this should not be needed, pilot heading will come from pilot. For testing only.
  SendHSC(new_pilot_heading);
  
}


NMEA0183    NMEA0183;



void AutoTrackRaymarine_pi::SendHSC(double course) {
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
