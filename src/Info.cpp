
/******************************************************************************
*
* Project:  OpenCPN
* Purpose:  AutoTrackRaymarine plugin
* Author:   Douwe Fokkema
*           David Register (The serial ports enumeration, copied from OpenCPN)
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
#include "SerialPort.h"


void InfoDialog::UpdateInfo() {
    if (m_pi->m_pilot_state == TRACKING)
      TextStatus11->SetLabel(_("Tracking"));
    if (m_pi->m_pilot_state == AUTO)
      TextStatus11->SetLabel(_("Auto"));
    if (m_pi->m_pilot_state == STANDBY)
      TextStatus11->SetLabel(_("Standby"));
    wxString pilot_heading;
    if (m_pi->m_pilot_heading == -1.) {
      pilot_heading = _("----");
    }
    else {
      pilot_heading << wxString::Format(wxString("%4.1f", wxConvUTF8), m_pi->m_pilot_heading);
    }
    TextStatus14->SetLabel(pilot_heading);
    wxString heading;
    if (m_pi->m_vessel_heading == -1.) {
      heading = _("----");
    }
    else {
      heading << wxString::Format(wxString("%4.1f", wxConvUTF8), m_pi->m_vessel_heading);
    }
    TextStatus12->SetLabel(heading);
    wxString xte;
    if (m_pi->m_XTE == 100000.) {
      xte = _("----");
    }
    else {
      xte << wxString::Format(wxString("%6.1f", wxConvUTF8), m_pi->m_XTE * 1852.);
    }
    TextStatus121->SetLabel(xte);
}

void InfoDialog::OnAuto(wxCommandEvent& event) {
  m_pi->m_serial_comms->SetAuto();
  m_pi->m_pilot_state = AUTO;
  wxLogMessage(wxT("$$$ On Auto clicked"));
  EnableHeadingButtons(true);
}

void InfoDialog::OnStandby(wxCommandEvent& event) {
  m_pi->m_serial_comms->SetStandby();
  m_pi->m_pilot_state = STANDBY;
  EnableHeadingButtons(false);
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
  if (enable) {
    buttonTrack->Enable();
    wxLogMessage(wxString("AutoTrackRaymarine_pi: $$$ trackbutton enabled"));
  }
  else {
    buttonTrack->Disable();
    wxLogMessage(wxString("AutoTrackRaymarine_pi: $$$ trackbutton disabled"));
  }
}

void InfoDialog::OnTracking(wxCommandEvent& event) {
  if (m_pi->m_route_active) {
    if (m_pi->m_pilot_state == STANDBY) {
      m_pi->m_serial_comms->SetAuto();
    }
    m_pi->ResetXTE();
    ZeroXTE();  // Zero XTE in OpenCPN;  to be added in new plugin API! $$$
    m_pi->m_pilot_state = TRACKING;
    EnableHeadingButtons(true);
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
  m_pi->ChangePilotHeading(10);
}