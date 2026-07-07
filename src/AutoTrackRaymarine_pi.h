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

#ifndef _AUTOTRACKRAYMARINEPI_H_
#define _AUTOTRACKRAYMARINEPI_H_

#include "wx/wx.h"

#include <wx/fileconf.h>

#include "version.h"
#include "wxWTranslateCatalog.h"

#define ABOUT_AUTHOR_URL 
#define OPC wxS("opencpn-AutoTrackRaymarine_pi")

#include <list>
#include <map>


#ifndef WXINTL_NO_GETTEXT_MACRO
#ifdef OPC
#ifdef _
#undef _
#endif // _
#define _(s) wxGetTranslation((s),OPC)
#endif // OPC
#else 
#define _(s) wxGetTranslation((s))
#endif // WXINTL_NO_GETTEXT_MACRO


#include "ocpn_plugin.h"

#include "nmea0183.h"


#ifdef __MSVC__
//#include <string>
#include "msvcdefs.h"
#endif

//----------------------------------
//    The PlugIn Class Definition
//----------------------------------



extern int g_verbose;

class apDC;
class ConsoleCanvas;
class PreferencesDialog;
class InfoDialog;
class ErrorDialog;

#ifndef PI
#define PI (3.1415926535897931160E0)
#endif

#define LOGLEVEL_VERBOSE 1
#define IF_LOG_AT_LEVEL(x) if ((g_verbose & (x)) != 0)

#define IF_LOG_AT(x, y)       \
  do {                        \
    IF_LOG_AT_LEVEL(x) { y; } \
  } while (0)
#define LOG_INFO wxLogMessage
#define LOG_VERBOSE IF_LOG_AT_LEVEL(LOGLEVEL_VERBOSE) wxLogMessage


class AutoTrackRaymarine_pi : public wxEvtHandler, public opencpn_plugin_118
{
  //friend InfoDialog;
private:
    //std::shared_ptr<ObservableListener> listener;

    void HandleN2K_65360 (ObservedEvt ev); // Pilot heading
    void HandleN2K_126208(ObservedEvt ev); // Set Set pilot heading or set auto/standby
    void HandleN2K_126720(ObservedEvt ev); // From EV1 (204) indicating auto or standby state
    void HandleN2K_65359 (ObservedEvt ev); // Vessel heading, proprietary
    void HandleN2K_127250(ObservedEvt ev); // Vessel heading, standerd NMEA2000

    std::shared_ptr<ObservableListener> listener_65360;  // Autopilot heading if auto
    std::shared_ptr<ObservableListener> listener_126208; // Set pilot heading or set auto/standby
    std::shared_ptr<ObservableListener> listener_126720; // From EV1 (204) indicating auto or standby state
    std::shared_ptr<ObservableListener> listener_65359;  // Vessel heading
    std::shared_ptr<ObservableListener> listener_127250; // Vessel heading

    /*std::string prio127245;
    std::string prio127257;
    std::string prio128259;
    std::string prio128267;
    std::string prio129029;
    std::string prioN2kPGNsat;
    std::string prio130306;*/

    wxString m_self;


public:
  AutoTrackRaymarine_pi(void *ppimgr);
  int Init(void);
  bool DeInit(void);
  DriverHandle m_handleN2k;
  NMEA0183 m_NMEA0183;
  double m_origin_to_dest;
  wxString m_next_wp;

  int GetAPIVersionMajor();
  int GetAPIVersionMinor();
  int GetPlugInVersionMajor();
  int GetPlugInVersionMinor();
  int GetPlugInVersionPatch();
  int GetPlugInVersionTweak();
  wxBitmap *GetPlugInBitmap();
  wxString GetCommonName();
  wxString GetShortDescription();
  wxString GetLongDescription();
  wxBitmap m_panelBitmap;
  bool m_pilot_seen;
  time_t m_pilot_seen_timeout;
  bool m_variation_seen;
  long m_var_timeout;

  void ShowPreferencesDialog(wxWindow* parent);

  //wxBitmap* GetPlugInBitmap();
 

  bool RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp);
  bool RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp);

  void ShowInfoDialog();
  void ShowPreferences();
  void ChangePilotHeading(int degrees);
  void SetStandby();
  void SetAuto();
  void SetTracking();
  void SetPilotHeading(double heading);
  void SetPilotAuto();
  void SetPilotStandby();
  void SetP70Tracking();
  void ShowErrorDialog();
  void HideErrorDialog();
  void SetPilotSeen(bool seen);
  void DisplayErrorText(wxString xx);

  static wxString StandardPath();

  PlugIn_Position_Fix_Ex &LastFix() { return m_lastfix; }

  DriverHandle m_N2khandle;
  bool m_arrival;
  uint16_t m_wp_loop, m_wp_loop_max;
  double m_arrival_radius;
  double m_XTE, m_BTW, m_DTW;
  bool m_XTE_refreshed;
  bool m_heading_set;
  double m_var, m_sog;
  bool m_initialized;
  bool m_route_active;
  double m_pilot_heading;         // target heading of pilot in auto mode
  double m_vessel_heading;        // current heading of vessel according to pilot
  double m_delayed_heading;       // delayed heading for simulation only
  double m_heading_change;
  double m_XTE_P, m_XTE_I, m_XTE_D;   // proportional, integral and differential factors
  //enum PilotState { UNKNOWN, STANDB, TRACKING, AUTO, test} m_pilot_state;  does not function in some classes 
  uint16_t m_pilot_state; // 0 standby, 1 auto, 2 tracking
  int m_context_menu_show_id;
#define STANDBY 0
#define AUTO 1
#define TRACKING 2
  // default values for PID parameters
#define P_FACTOR _("1.")
#define I_FACTOR _("0.0075")
  // was 0.3, instable occillations
#define D_FACTOR _("5.")
  // back to 5 13-06-2025

  wxCriticalSection m_exclusive;

public:
    void ResetXTE(); 
    // these are stored to the config
    struct preferences {
      double max_angle;
      double sensitivity;
      double p_factor;  // proportional factor for PID
      double d_factor;  // differential factor for PID
      double i_factor;  // integration factor for PID
    } m_prefs;
    
    PreferencesDialog *m_PreferencesDialog;
    ErrorDialog* m_ErrorDialog;
    InfoDialog *m_info_dialog;
    
    void OnToolbarToolCallback(int id);

    // Icons
    wxString m_shareLocn;
    int m_tool_id;
    wxBitmap *m_pdeficon;
#define TRACKING_TOOL_POSITION -1  // Request default positioning of toolbar tool

protected:   
    wxPoint m_cursor_position;
    PlugIn_Position_Fix_Ex m_lastfix;
   

private:
    void SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix);
    void SetPluginMessage(wxString &message_id, wxString &message_body);
    void SetActiveLegInfo(Plugin_Active_Leg_Info &leg_info);
    void OnContextMenuItemCallback(int id);
    void Compute();
    void SendHSC(double course);
    int m_leftclick_tool_id;
};

#endif
