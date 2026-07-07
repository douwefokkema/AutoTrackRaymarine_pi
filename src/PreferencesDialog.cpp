/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  AutoTrackRaymarine plugin
 * Author:   Douwe Fokkema
 *          
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

#include "PreferencesDialog.h"
//#include "AutoTrackRaymarine_pi.h"


bool PreferencesDialog::Show( bool show )

{
    if(show) {
        // load preferences
        AutoTrackRaymarine_pi::preferences &p = m_pi.m_prefs;
        m_sMaxAngle1->SetValue(p.max_angle);
        m_sensitivity->SetValue(p.sensitivity);
        m_checkBox1->SetValue(g_verbose);
        m_textCtrl1->SetValue(wxString::Format(wxT("%3.4f"), p.p_factor));
        m_textCtrl2->SetValue(wxString::Format(wxT("%3.4f"), p.i_factor));
        m_textCtrl3->SetValue(wxString::Format(wxT("%3.4f"), p.d_factor));
    }
    return PreferencesDialogBase::Show(show);
}


void PreferencesDialog::OnCancel( wxCommandEvent& event )
{
    if(IsModal())
        EndModal(wxID_CANCEL);
    Hide();
}

void PreferencesDialog::OnOk( wxCommandEvent& event )
{
    AutoTrackRaymarine_pi::preferences &p = m_pi.m_prefs;
    p.max_angle = m_sMaxAngle1->GetValue();
    p.sensitivity = m_sensitivity->GetValue();
    g_verbose = m_checkBox1->GetValue();
    wxString temp = m_textCtrl1->GetValue();  // Proportional
    temp.ToDouble(&p.p_factor);
    temp = m_textCtrl2->GetValue(); // Integral
    temp.ToDouble(&p.i_factor);
    temp = m_textCtrl3->GetValue();  // Differential
    temp.ToDouble(&p.d_factor);
    Hide();
}

void PreferencesDialog::OnDefault(wxCommandEvent& event) {
  wxString temp = P_FACTOR;
  m_textCtrl1->SetValue(temp);
  temp = I_FACTOR;
  m_textCtrl2->SetValue(temp);
  temp = D_FACTOR;
  m_textCtrl3->SetValue(temp);
}

PreferencesDialog::~PreferencesDialog() {
};



