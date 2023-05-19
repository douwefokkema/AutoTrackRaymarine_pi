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

int                       g_nCOMPortCheck = 32;

bool PreferencesDialog::Show( bool show )
{
    if(show) {

        // load preferences
        AutoTrackRaymarine_pi::preferences &p = m_pi.prefs;
        m_sMaxAngle1->SetValue(p.max_angle);
        m_comboBox1->SetValue(p.com_port);

        // Waypoint Arrival
        m_cbConfirmBearingChange->SetValue(p.confirm_bearing_change);
        m_cbInterceptRoute->SetValue(p.intercept_route);
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
    AutoTrackRaymarine_pi::preferences &p = m_pi.prefs;
    p.max_angle = m_sMaxAngle1->GetValue();
    // Waypoint Arrival
    p.confirm_bearing_change = m_cbConfirmBearingChange->GetValue();
    p.intercept_route = m_cbInterceptRoute->GetValue();
    p.com_port = m_comboBox1->GetValue();
    // make com port now
    Hide();
}

PreferencesDialog::~PreferencesDialog() {
};



