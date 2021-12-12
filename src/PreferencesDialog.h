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

#ifndef INC_PREFERENCESDIALOG_H
#define INC_PREFERENCESDIALOG_H

#include "AutoTrackRaymarine_pi.h"
#include "AutotrackPrefsUI.h"

class PreferencesDialog : public PreferencesDialogBase
{
public:
    PreferencesDialog(wxWindow *parent, AutoTrackRaymarine_pi &pi )
        : PreferencesDialogBase(parent), m_pi(pi) {
      
    }
    ~PreferencesDialog();

    bool Show( bool show = true );
    void FindSerialPorts();
    wxArrayString *m_pSerialArray;
    wxArrayString *EnumerateSerialPorts(void);

private:
    void OnCancel( wxCommandEvent& event );
    void OnOk( wxCommandEvent& event );


    AutoTrackRaymarine_pi &m_pi;
};

#endif // !INC_PREFERENCESDIALOG_H