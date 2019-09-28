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
#include "nmea0183/nmea0183.h"

#define     MY_API_VERSION_MAJOR    1
#define     MY_API_VERSION_MINOR    10

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

#ifdef __MSVC__
#include "msvcdefs.h"
#endif

//----------------------------------
//    The PlugIn Class Definition
//----------------------------------

class apDC;
class ConsoleCanvas;
class PreferencesDialog;
//class NGT1Input;
class SerialPort;
//class m_dialog;
class InfoDialog;

class AutoTrackRaymarine_pi : public wxEvtHandler, public opencpn_plugin_110
{
  //friend InfoDialog;
public:
  AutoTrackRaymarine_pi(void *ppimgr);
  int Init(void);
  bool DeInit(void);

  int GetAPIVersionMajor();
  int GetAPIVersionMinor();
  int GetPlugInVersionMajor();
  int GetPlugInVersionMinor();
  wxBitmap *GetPlugInBitmap();
  wxString GetCommonName();
  wxString GetShortDescription();
  wxString GetLongDescription();

 // void SetColorScheme(PI_ColorScheme cs);  $$$
  void ShowPreferencesDialog(wxWindow* parent);

  bool RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp);
  bool RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp);

  void ShowConsoleCanvas();
  void ShowPreferences();
  void ChangePilotHeading(int degrees);

  SerialPort *m_serial_comms;

  static wxString StandardPath();

  PlugIn_Position_Fix_Ex &LastFix() { return m_lastfix; }

  NMEA0183 m_NMEA0183;

  double m_XTE, m_BTW;
  bool m_XTE_refreshed;
  bool m_heading_set;
  double m_var;
  bool m_initialized;
  bool m_route_active;
  double m_pilot_heading;  // target heading of pilot in auto mode
  double m_vessel_heading;        // current heading of vessel according to pilot
  
  double m_XTE_P, m_XTE_I, m_XTE_D;   // proportional, integral and differential factors
  //enum PilotState { UNKNOWN, STANDB, TRACKING, AUTO, test} m_pilot_state;  does not function in some classes 
  uint16_t m_pilot_state; // 0 standby, 1 auto, 2 tracking
#define STANDBY 0
#define AUTO 1
#define TRACKING 2
#define I_FACTOR 0.0075
#define D_FACTOR 5.0

public:
  void ResetXTE() {
    m_XTE_P = 0.;  m_XTE_I = 0.; m_XTE_D = 0.; m_heading_set = false;
  }
    // these are stored to the config
    struct preferences {
        double max_angle;
        wxString com_port;

        // Waypoint Arrival
        bool confirm_bearing_change;
        bool intercept_route;

        // Boundary
        wxString boundary_guid;
        int boundary_width;

    } prefs;
    
    PreferencesDialog *m_PreferencesDialog;

protected:   
    wxPoint m_cursor_position;
    PlugIn_Position_Fix_Ex m_lastfix;

private:
    void SetNMEASentence(wxString &sentence);
    void SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix);
    void SetPluginMessage(wxString &message_id, wxString &message_body);

    
    void Compute();
    void SendHSC(double course);

    int m_leftclick_tool_id;

    InfoDialog *m_InfoDialog;
    

   // PI_ColorScheme m_colorscheme; $$$

    wxString m_active_guid, m_active_request_guid;
    wxDateTime m_active_request_time;
    
    wxString m_last_wp_name, m_last_wpt_activated_guid;

    bool m_bArrival;

    double m_current_bearing;

};

#endif
