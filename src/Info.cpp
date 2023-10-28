
/******************************************************************************
*
* Project:  OpenCPN
* Purpose:  AutoTrackRaymarine plugin
* Author:   Douwe Fokkema
*           David Register 
*
***************************************************************************
*   Copyright (C) 2019 by Douwe Fokkema                                   *
*                         David Register                                  *
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

#include "Info.h"
#include "AutotrackInfoUI.h"


void InfoDialog::UpdateInfo() {
    if (m_pi->m_pilot_state == TRACKING)
      TextStatus11->SetValue(_("Tracking"));
    if (m_pi->m_pilot_state == AUTO)
      TextStatus11->SetValue(_("Auto"));
    if (m_pi->m_pilot_state == STANDBY)
      TextStatus11->SetValue(_("Standby"));
    wxString pilot_heading;
    if (m_pi->m_pilot_heading == -1.) {
      pilot_heading = _("----");
    }
    else {
      pilot_heading << wxString::Format(wxString("%4.1f", wxConvUTF8), m_pi->m_pilot_heading);
    }
    TextStatus14->SetValue(pilot_heading);
    wxString heading;
    if (m_pi->m_vessel_heading == -1.) {
      heading = _("----");
    }
    else if (isnan(m_pi->m_var)) {
        heading = _("no variation");
    }
    else {
      heading << wxString::Format(wxString("%4.1f", wxConvUTF8), m_pi->m_vessel_heading);
    }
    TextStatus12->SetValue(heading);
    wxString xte;
    if (m_pi->m_XTE == 100000.) {
      xte = _("----");
    }
    else {
      xte << wxString::Format(wxString("%6.1f", wxConvUTF8), m_pi->m_XTE * 1852.);
    }
    TextStatus121->SetValue(xte);
}

void InfoDialog::OnAuto(wxCommandEvent& event) {
  m_pi->SetPilotAuto();   // sent auto command to pilot
  m_pi->SetAuto();
}


void InfoDialog::OnStandby(wxCommandEvent& event) {
  
  m_pi->SetPilotStandby();
  m_pi->SetStandby();
  
     /* std::shared_ptr <std::vector<uint8_t>> payload(new std::vector<uint8_t> ({ 1, 0xa9, 0x5d, 0xff, 0x7f, 0xff, 0x7f, 0xfd }));
      wxLogMessage(wxT("$$$ payload , %0x, %0x, %0x"), payload->at(0), payload->at(1), payload->at(2));
  int PGN = 127250;
  WriteCommDriverN2K(m_pi->m_handleN2k, PGN,
      255, 6, payload);*/
}

void InfoDialog::EnableHeadingButtons(bool show) {
  if (show) {
    buttonDecOne->Enable();
    buttonIncOne->Enable();
    buttonDecTen->Enable();
    buttonIncTen->Enable();
  }
  else {
    buttonDecOne->Disable();
    buttonIncOne->Disable();
    buttonDecTen->Disable();
    buttonIncTen->Disable();
  }
}

void InfoDialog::EnableTrackButton(bool enable) {
  if (enable && m_pi->m_route_active) {
    buttonTrack->Enable();
  }
  else {
    buttonTrack->Disable();
  }
}

void InfoDialog::OnTracking(wxCommandEvent& event) {
  if (m_pi->m_route_active) {
    if (m_pi->m_pilot_state == STANDBY) {
      m_pi->SetAuto();
      m_pi->SetPilotAuto();
    }
    m_pi->SetTracking();
  }
}

void InfoDialog::OnMinusTen(wxCommandEvent& event) {
m_pi->ChangePilotHeading(-10);
}

void InfoDialog::OnPlusTen(wxCommandEvent& event) {
  m_pi->ChangePilotHeading(10);
}

void InfoDialog::OnMinusOne(wxCommandEvent& event) {
  m_pi->ChangePilotHeading(-1);
}

void InfoDialog::OnPlusOne(wxCommandEvent& event) {
  m_pi->ChangePilotHeading(1);
}
