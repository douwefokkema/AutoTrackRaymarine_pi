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
//#include "apdc.h"
#include "jsoncpp/json/json.h"
//#include "json/json.h"
#include "AutoTrackRaymarine_pi.h"
#include "concanv.h"
#include "PreferencesDialog.h"
#include "icons.h"
#include "GL/gl.h"
#include "conio.h"
#include "actisense.h"
#include "NGT1Read.h"
#include "actisense.h"
#include "serial/serial.h"
#include "SerialPort.h"

using namespace std;
#define TURNRATE 10.   // turnrate per second

#define NGT1_port 6  // later in preferences, todo

// commands for NGT-1 in Canboat format
// string msg0 = "Z,3,126208,7,204,17,01,63,ff,00,f8,04,01,3b,07,03,04,04,00,00,05,ff,ff";  //set standbye
// string msg1 = "Z,3,126208,7,204,17,01,63,ff,00,f8,04,01,3b,07,03,04,04,40,00,05,ff,ff"; // set auto
string msg2    = "Z,3,126208,7,204,14,01,50,ff,00,f8,03,01,3b,07,03,04,06,00,00";  //set 0 magnetic
// string msg3 = "Z,3,126208,7,204,14,01,50,ff,00,f8,03,01,3b,07,03,04,06,9f,3e";  //set 92 magnetic
// string msg4 = "Z,3,126208,7,204,14,01,50,ff,00,f8,03,01,3b,07,03,04,06,4e,3f";  //set 93 example only, magnetic

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
  m_ConsoleCanvas = NULL;
  m_PreferencesDialog = NULL;
  m_avg_sog = 0;
  m_declination = NAN;
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
  m_pilot_state = 0;   // standby

  // Mode
  preferences &p = prefs;
  
  p.max_angle = pConf->Read("MaxAngle", 30);
  p.com_port = pConf->Read("ActisenseComPort", "");

 // InitSerial();

 
  if (m_ConsoleCanvas) {
    m_ConsoleCanvas->pDeactivate->SetLabel(_("Standby"));
  }
  
  // options
  p.confirm_bearing_change = (bool)pConf->Read("ConfirmBearingChange", 0L);
  p.intercept_route = (bool)pConf->Read("InterceptRoute", 1L);
  
  // Boundary
  p.boundary_guid = pConf->Read("Boundary", "");
  p.boundary_width = pConf->Read("BoundaryWidth", 30);

  // NMEA output
  p.rate = pConf->Read("NMEARate", 1L);
  p.magnetic = (bool)pConf->Read("NMEAMagnetic", 0L);
  wxString sentences = pConf->Read("NMEASentences", "APB;");
  while (sentences.size()) {
    p.nmea_sentences[sentences.BeforeFirst(';')] = true;
    sentences = sentences.AfterFirst(';');
  }

 
  m_Timer.Connect(wxEVT_TIMER, wxTimerEventHandler
  (AutoTrackRaymarine_pi::OnTimer), NULL, this);


  // initialise NGT-1 com port
  char s[20];
  wchar_t pcCommPort[20];
  char Port[10];

  /* The following startup command reverse engineered from Actisense NMEAreader.
  * It instructs the NGT1 to clear its PGN message TX list, thus it starts
  * sending all PGNs.
  */
  static unsigned char NGT_STARTUP_SEQ[] =
  { 0x11   /* msg byte 1, meaning ? */
    , 0x02   /* msg byte 2, meaning ? */
    , 0x00   /* msg byte 3, meaning ? */
  };

  sprintf_s(s, "\\\\.\\COM%d", NGT1_port);
  mbstowcs(pcCommPort, s, strlen(s) + 1); //Plus null
  sprintf_s(Port, "COM%d", NGT1_port);
  // Open the port tentatively
  HANDLE hComm = ::CreateFile(pcCommPort, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);

  //  Check for the error returns that indicate a port is there, but not currently useable
  if (hComm == INVALID_HANDLE_VALUE)
  {
    DWORD dwError = GetLastError();

    if (dwError == ERROR_ACCESS_DENIED ||
      dwError == ERROR_GEN_FAILURE ||
      dwError == ERROR_SHARING_VIOLATION ||
      dwError == ERROR_SEM_TIMEOUT)
      wxLogMessage(wxT("AutoTrackRaymarine_pi: serial port COM%d not found, can not open NGT-1."), NGT1_port);
  }
  else
  {
    CloseHandle(hComm);
    wxLogMessage(wxT("AutoTrackRaymarine_pi: Found serial port COM%d and it's ready to use"), NGT1_port);
  }
  // now really open com port
  if (!OpenSerialPort(pcCommPort, &m_hSerialin)) {            // open serial port for NGT-1
    wxLogMessage(wxT("AutoTrackRaymarine_pi Error making serial port for NGT-1"));
  }
  else {
    wxLogMessage(wxT("AutoTrackRaymarine_pi serial port for NGT-1 opened"));
  }

  COMMTIMEOUTS timeouts;
  if ((GetCommTimeouts(m_hSerialin, &timeouts) == 0))
  {
    wxLogMessage(wxT("AutoTrackRaymarine_pi: error getting timeouts"));
  }
  timeouts.ReadIntervalTimeout = 1;
  timeouts.ReadTotalTimeoutMultiplier = 1;
  timeouts.ReadTotalTimeoutConstant = 1;
  timeouts.WriteTotalTimeoutMultiplier = 1;
  timeouts.WriteTotalTimeoutConstant = 1;
  if (SetCommTimeouts(m_hSerialin, &timeouts) == 0)
  {
    wxLogMessage(wxT("AutoTrackRaymarine_pi error setting timeouts"));
  }

  wxLogMessage(wxT("AutoTrackRaymarine_pi Device is a serial port, send the startup sequence."));

  writeMessage(m_hSerialin, NGT_MSG_SEND, NGT_STARTUP_SEQ, sizeof(NGT_STARTUP_SEQ));
  Sleep(100);  // $$$ this was 2000 earlier, why so long? for startup only.

  m_serial_comms = new SerialPort(this);
  m_NGT1_read = new NGT1Input(this);
  if (m_NGT1_read->Run() != wxTHREAD_NO_ERROR) {
    wxLogMessage(wxT("AutoTrackRaymarine_pi: unable to start NGT1Input thread"));
    return 0;
  }  

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

  m_Timer.Disconnect(wxEVT_TIMER, wxTimerEventHandler(AutoTrackRaymarine_pi::OnTimer), NULL, this);

  RemovePlugInTool(m_leftclick_tool_id);

  // save config
  wxFileConfig *pConf = GetOCPNConfigObject();
  pConf->SetPath(_T("/Settings/AutoTrackRaymarine"));

  if (m_ConsoleCanvas) {
    wxPoint p = GetFrameAuiManager()->GetPane(m_ConsoleCanvas).floating_pos;
    pConf->Write("PosX", p.x);
    pConf->Write("PosY", p.y);
  }
  delete m_ConsoleCanvas;

  preferences &p = prefs;
  
  pConf->Write("MaxAngle", p.max_angle);
  pConf->Write("ActisenseComPort", p.com_port);
  
  // Waypoint Arrival
  pConf->Write("ConfirmBearingChange", p.confirm_bearing_change);
  pConf->Write("InterceptRoute", p.intercept_route);

  //  Stop read thread NGT-1
  m_NGT1_read->Shutdown();
  delete m_NGT1_read;
  // Close NGT-1 serial port
  if (CloseHandle(m_hSerialin) == 0) {
    wxLogMessage(wxT("AutoTrackRaymarine_pi Error closing serial port NGT-1"));
  }
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

void AutoTrackRaymarine_pi::SetColorScheme(PI_ColorScheme cs)
{
  m_colorscheme = cs;
  if (m_ConsoleCanvas)
    m_ConsoleCanvas->SetColorScheme(cs);
}

void AutoTrackRaymarine_pi::ShowPreferencesDialog(wxWindow* parent)
{
  if (NULL == m_PreferencesDialog)
    m_PreferencesDialog = new PreferencesDialog(parent, *this);

  m_PreferencesDialog->ShowModal();

  delete m_PreferencesDialog;
  m_PreferencesDialog = NULL;
}


void AutoTrackRaymarine_pi::ShowConsoleCanvas()
{
  wxLogMessage(wxString("AutoTrackRaymarine_pi: $$$ show1"));
  if (!m_ConsoleCanvas) {
    wxLogMessage(wxString("AutoTrackRaymarine_pi: $$$ show2"));
    wxFileConfig *pConf = GetOCPNConfigObject();
    pConf->SetPath(_T("/Settings/AutoTrackRaymarine"));
    wxPoint pos(pConf->Read("PosX", 0L), pConf->Read("PosY", 50));

    m_ConsoleCanvas = new ConsoleCanvas(GetOCPNCanvasWindow(), *this);
    wxSize sz = m_ConsoleCanvas->GetSize();
    wxAuiPaneInfo p = wxAuiPaneInfo().
      Name("Console Canvas").
      Caption("Console Canvas").
      CaptionVisible(false).
      TopDockable(false).
      BottomDockable(false).
      LeftDockable(true).
      RightDockable(true).
      MinSize(wxSize(20, 20)).
      BestSize(sz).
      FloatingSize(sz).
      FloatingPosition(pos).
      Float().
      Gripper(false);
    wxAuiManager *pauimgr = GetFrameAuiManager();
    pauimgr->AddPane(m_ConsoleCanvas, p);
    pauimgr->Update();
    SetColorScheme(m_colorscheme);
    m_ConsoleCanvas->Show();
  }
  wxLogMessage(wxString("AutoTrackRaymarine_pi: $$$ show3"));
  m_ConsoleCanvas->ShowWithFreshFonts();
  GetFrameAuiManager()->GetPane(m_ConsoleCanvas).Show(true);
  GetFrameAuiManager()->Update();
  wxSize sz = GetFrameAuiManager()->GetPane(m_ConsoleCanvas).floating_size;
  sz.y++; // horrible stupid hack to make things look right
  GetFrameAuiManager()->GetPane(m_ConsoleCanvas).FloatingSize(sz);
  GetFrameAuiManager()->Update();
  wxLogMessage(wxString("AutoTrackRaymarine_pi: $$$ show4"));
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

double AutoTrackRaymarine_pi::Declination()
{
  if (prefs.magnetic &&
    (!m_declinationTime.IsValid() || (wxDateTime::Now() - m_declinationTime).GetSeconds() > 1200)) {
    m_declination = NAN;
    SendPluginMessage("WMM_VARIATION_BOAT_REQUEST", "");
  }
  return m_declination;
}

bool AutoTrackRaymarine_pi::GetConsoleInfo(
  double &bearing, double &xte)
{
  bearing = m_current_bearing;
  xte = m_current_xte;
  return true;
}

void AutoTrackRaymarine_pi::DeactivateRoute()
{
  //SendPluginMessage("OCPN_RTE_DEACTIVATED", "");
  m_heading_set = false;
  if (m_pilot_state == STANDBY || m_pilot_state == AUTO) {
    ResetXTE();
    m_pilot_state = TRACKING; // tracking
    if (m_ConsoleCanvas) {
      m_ConsoleCanvas->pDeactivate->SetLabel(_("Tracking"));
    }
  }
  else {
    m_pilot_state = STANDBY;
    if (m_ConsoleCanvas) {
      m_ConsoleCanvas->pDeactivate->SetLabel(_("Standby"));
    }
  }
}


void AutoTrackRaymarine_pi::OnTimer(wxTimerEvent &)
{
  if (!m_ConsoleCanvas) return;
  Compute();
  m_ConsoleCanvas->UpdateRouteData();
}


void AutoTrackRaymarine_pi::SetCursorLatLon(double lat, double lon)
{
  wxPoint pos = wxGetMouseState().GetPosition();
  if (pos == m_cursor_position)
    return;
  m_cursor_position = pos;
}


void AutoTrackRaymarine_pi::SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix)
{
  m_lastfix = pfix;
  if (pfix.nSats > 3) {
    m_avg_sog = m_avg_sog * .9 + pfix.Sog*.1;
  }
  m_var = pfix.Var;
  //wxLogMessage(wxString("AutoTrackRaymarine_pi: $$$ variation read %f"), m_var);
}

static bool ParseMessage(wxString &message_body, Json::Value &root)
{
  Json::Reader reader;
  if (reader.parse(std::string(message_body), root))
    return true;   //$$$

  wxLogMessage(wxString("AutoTrackRaymarine_pi: Error parsing JSON message: ") + reader.getFormattedErrorMessages());
  return false;
}


void AutoTrackRaymarine_pi::SetPluginMessage(wxString &message_id, wxString &message_body)
{
  //// construct the JSON root object
  //Json::Value  root;
  //// construct a JSON parser
  //wxString    out;

  //if (message_id == wxS("AutoTrackRaymarine_pi")) {
  //  return; // nothing yet
  //}
  //else if (message_id == wxS("AIS")) {
  //}
  //else if (message_id == _T("WMM_VARIATION_BOAT")) {
  //  if (ParseMessage(message_body, root)) {
  //    wxString(root["Decl"].asString()).ToDouble(&m_declination);
  //    m_declinationTime = wxDateTime::Now();
  //  }
  //}
  //else if (message_id == "OCPN_RTE_ACTIVATED") {
  //  if (ParseMessage(message_body, root)) {
  //    ResetXTE();
  //    ShowConsoleCanvas();
  //    m_pilot_state = 0;    // standby
  //    if (m_ConsoleCanvas) {
  //      m_ConsoleCanvas->pDeactivate->SetLabel(_("Standby"));
  //    }
  //  }
  //  m_Timer.Start(1000);
  //}
  //else if (message_id == "OCPN_WPT_ACTIVATED") {
  //  wxString guid = root["GUID"].asString();
  //  m_last_wpt_activated_guid = guid;
  //  //ShowConsoleCanvas();
  //}
  //else if (message_id == "OCPN_WPT_ARRIVED") {
  //}
  //else if (message_id == "OCPN_RTE_DEACTIVATED" || message_id == "OCPN_RTE_ENDED") {
  //  m_Timer.Stop();
  //  m_active_guid = "";
  //  m_active_request_guid = "";
  //  if (m_ConsoleCanvas) {
  //    GetFrameAuiManager()->GetPane(m_ConsoleCanvas).Float();
  //    GetFrameAuiManager()->GetPane(m_ConsoleCanvas).Show(false);
  //    GetFrameAuiManager()->Update();
  //  }
  //}
}


void AutoTrackRaymarine_pi::SetNMEASentence(wxString &sentence) {
  m_NMEA0183 << sentence;
  time_t now = time(0);
  
  float xte;

  wxLogMessage(wxT("AutoTrackRaymarine: $$$SetNMEASentence %s"), sentence.c_str());
  wxString nm = sentence;
  

  if (m_NMEA0183.PreParse()) {
    // it seems that we can't parse XTE, so do it ourselves
    if (m_NMEA0183.LastSentenceIDReceived == _T("XTE")) {
      if (m_NMEA0183.Parse()) {
        wxLogMessage(wxT("AutoTrackRaymarine: $$$ XTE received %f"), m_NMEA0183.Xte.CrossTrackErrorDistance);
        //m_NMEA0183.Xte.CrossTrackErrorDistance;
      }
      else {
        wxLogMessage(wxT("AutoTrackRaymarine: $$$ no parse"));
      }
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
    }
    else if (m_NMEA0183.LastSentenceIDReceived == _T("APB") && m_NMEA0183.Parse()) {
      m_BTW = m_NMEA0183.Apb.BearingPresentPositionToDestination;
    }
  }

}


void AutoTrackRaymarine_pi::Compute()
{
  double dist;
  double XTE_for_correction;

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
  wxLogMessage(wxT("AutoTrackRaymarine: $$$$ initial gamma=%f, btw=%f, dist=%f, max_angle"), gamma, m_BTW, dist, max_angle);
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
  if (m_pilot_state == 2) {   // tracking
    wxLogMessage(wxT("AutoTrackRaymarine $$$ tracking send to raymarine"));
    SetAutopilotHeading(m_current_bearing - m_var);  // the commands used expect magnetic heading
    SendHSC(m_current_bearing);  
  }
  m_current_xte = m_XTE;
  wxLogMessage(wxT("AutoTrackRaymarine $$$1 m_XTE=%f"), m_XTE);
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
  wxLogMessage(wxT("AutoTrackRaymarine: $$$sending HSC %s"), nmea);
  PushNMEABuffer(nmea);
}

bool AutoTrackRaymarine_pi::OpenSerialPort(wchar_t* pcCommPort, HANDLE* handle) {
  // Open serial port number
  DCB dcbSerialParams = { 0 };
  COMMTIMEOUTS timeouts = { 0 };

  *handle = CreateFile(
    pcCommPort, GENERIC_READ | GENERIC_WRITE, 0, NULL,
    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (*handle == INVALID_HANDLE_VALUE)
  {
    wxLogMessage(wxT("AutoTrackRaymarine_pi Error com port"));
    int Dummy = toupper(_getch());
    return false;
  }
  else wxLogMessage(wxT("AutoTrackRaymarine_pi  OK"));

  // Set device parameters (115200 baud, 1 start bit,
  // 1 stop bit, no parity)
  dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
  if (GetCommState(*handle, &dcbSerialParams) == 0)
  {
    wxLogMessage(wxT("AutoTrackRaymarine_pi Error getting device state. Press a key to exit"));
    CloseHandle(*handle);
    int Dummy = toupper(_getch());
    return false;
  }

  dcbSerialParams.BaudRate = CBR_115200;
  dcbSerialParams.ByteSize = 8;
  dcbSerialParams.StopBits = ONESTOPBIT;
  dcbSerialParams.Parity = NOPARITY;
  if (SetCommState(*handle, &dcbSerialParams) == 0)
  {
    wxLogMessage(wxT("AutoTrackRaymarine_pi Error setting device parameters"));
    CloseHandle(*handle);
    int Dummy = toupper(_getch());
    return false;
  }

  // Set COM port timeout settings
  timeouts.ReadIntervalTimeout = 50;
  timeouts.ReadTotalTimeoutConstant = 50;
  timeouts.ReadTotalTimeoutMultiplier = 10;
  timeouts.WriteTotalTimeoutConstant = 50;
  timeouts.WriteTotalTimeoutMultiplier = 10;
  if (SetCommTimeouts(*handle, &timeouts) == 0)
  {
    wxLogMessage(wxT("AutoTrackRaymarine_pi Error setting timeouts"));
    CloseHandle(*handle);
    int Dummy = toupper(_getch());
    return false;
  }
  return true;
}

void AutoTrackRaymarine_pi::writeMessage(HANDLE handle, unsigned char command, const unsigned char * cmd, const size_t len)
{
  unsigned char bst[255];
  unsigned char *b = bst;
  unsigned char *lenPtr;
  unsigned char crc;
  DWORD bytes_written;
  int length = len;

  *b++ = DLE;
  *b++ = STX;
  *b++ = command;
  crc = command;
  lenPtr = b++;

  for (size_t i = 0; i < len; i++)
  {
    if (cmd[i] == DLE)
    {
      *b++ = DLE;
    }
    *b++ = cmd[i];
    crc += (unsigned char)cmd[i];
  }
  
  *lenPtr = length;
  crc += length;

  *b++ = (unsigned char)(256 - (int)crc);
  *b++ = DLE;
  *b++ = ETX;
  if (!WriteFile(handle, bst, b - bst, &bytes_written, NULL)) {
    wxLogMessage(wxT("AutoTrackRaymarine_pi Error. Writing to serial port"));
    CloseHandle(m_hSerialin);
    int Dummy = toupper(_getch());
  }
 // wxLogMessage(wxT("AutoTrackRaymarine_pi $$$length=%i, written %i "), length, bytes_written);
  //  if (write(handle, bst, b - bst) != b - bst)
}

void AutoTrackRaymarine_pi::SetAutopilotHeading(double heading) {
 // wxLogMessage(wxT("$$$AutoTrackRaymarine_pi SetAutopilotHeading = %f"), heading);
  double heading_normal = heading;
  while (heading_normal < 0) heading_normal += 360;
  while (heading_normal >= 360) heading_normal -= 360;
  uint16_t heading_radials1000 = (uint16_t)(heading_normal * 174.53); // heading to be set in thousands of radials
  //wxLogMessage(wxT("$$$AutoTrackRaymarine_pi SetAutopilotHeading2 radials = %i %000x"), heading_radials1000, heading_radials1000);
  uint8_t byte0, byte1;
  byte0 = heading_radials1000 & 0xff;
  byte1 = heading_radials1000 >> 8;
  //wxLogMessage(wxT("AutoTrackRaymarine_pi SetAutopilotHeading byte0 = %0x, byte1 = %0x"), byte0, byte1);
  char s[4];
  sprintf_s(s, "%0x", byte0);
  if (byte0 > 15) {
    msg2[56] = s[0];
    msg2[57] = s[1];
  }
  else {
    msg2[56] = '0';
    msg2[57] = s[0];
  }
  sprintf_s(s, "%0x", byte1);
  if (byte1 > 15) {
    msg2[59] = s[0];
    msg2[60] = s[1];
  }
  else {
    msg2[59] = '0';
    msg2[60] = s[0];
  }
  unsigned char msg[500];
  for (unsigned int i = 0; i <= msg2.length(); i++) {
    msg[i] = msg2[i];
  }
  parseAndWriteIn(m_hSerialin, msg);
}

void AutoTrackRaymarine_pi::parseAndWriteIn(HANDLE handle, const unsigned char * cmd)
{
  unsigned char msg[500];
  unsigned char * m;

  unsigned int prio;
  unsigned int pgn;
  unsigned int src;
  unsigned int dst;
  unsigned int bytes;

  char * p;
  int i;
  int b;
  unsigned int byt;
  int r;

  if (!cmd || !*cmd || *cmd == '\n')
  {
    return;
  }

  p = strchr((char *)cmd, ',');
  if (!p)
  {
    return;
  }

  r = sscanf(p, ",%u,%u,%u,%u,%u,%n", &prio, &pgn, &src, &dst, &bytes, &i);
  if (r == 5)
  {
    p += i - 1;
    m = msg;
    *m++ = (unsigned char)prio;
    *m++ = (unsigned char)pgn;
    *m++ = (unsigned char)(pgn >> 8);
    *m++ = (unsigned char)(pgn >> 16);
    *m++ = (unsigned char)dst;
    //*m++ = (unsigned char) 0;
    *m++ = (unsigned char)bytes;
    //wxLogMessage(wxT("AutoTrackRaymarine_pi $$$write message prio %i, pgn %i, src %i, dst %i, bytes %i, i %i "), prio, pgn, src, dst, bytes, i);

    for (b = 0; m < msg + sizeof(msg) && b < bytes; b++)
    {
      if ((sscanf(p, ",%x%n", &byt, &i) == 1) && (byt < 256))
      {
        *m++ = byt;
        //wxLogMessage(wxT("AutoTrackRaymarine_pi byt=%0x "), byt);   // $$$
      }
      else
      {
        wxLogMessage(wxT("AutoTrackRaymarine_pi: Unable to parse incoming message '%s' at offset %u %i"), cmd, b, byt);
        Sleep(1000);
        return;
      }
      p += i;
    }
  }
  else
  {
    wxLogMessage(wxT("AutoTrackRaymarine_pi: Unable to parse incoming message '%s', r = %d"), cmd, r);
    return;
  }
  /*wxLogMessage(wxT("AutoTrackRaymarine_pi                  write message called len= %i "), m - msg)*/;

 /* wxLogMessage(wxT("              $$$  write message called len= %i \n"), m - msg);
  wxLogMessage(wxT("%0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x \n"),
    msg[0], msg[1], msg[2], msg[3], msg[4], msg[5], msg[6], msg[7], msg[8], msg[9], msg[10], msg[11], msg[12], msg[13], msg[14], msg[15],
    msg[16], msg[17], msg[18], msg[19], msg[20]);*/


  writeMessage(handle, N2K_MSG_SEND, msg, m - msg);
  /*wxLogMessage(wxT("AutoTrackRaymarine_pi: $$$message written to Actisense"));*/
}
