

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

#ifndef INC_INFO_H
#define INC_INFO_H

#include "AutoTrackRaymarine_pi.h"
#include "AutotrackInfoUI.h"

#define MOD_ANGLE(angle) if (angle < 0.) angle += 360.; if (angle >= 360.) angle -= 360.;

class InfoDialog : public m_dialog
{
public:
  InfoDialog(wxWindow *parent, AutoTrackRaymarine_pi *pi)
    : m_dialog(parent), m_pi(pi) {
  }
  ~InfoDialog() {};
  void UpdateInfo();
  void OnAuto(wxCommandEvent & event);
  void OnStandby(wxCommandEvent & event);
  void OnTracking(wxCommandEvent & event);
  void OnMinusTen(wxCommandEvent & event);
  void OnPlusTen(wxCommandEvent & event);
  void OnMinusOne(wxCommandEvent & event);
  void OnPlusOne(wxCommandEvent & event);
  void EnableHeadingButtons(bool enable);
  void EnableTrackButton(bool enable);

private:
  /*void OnCancel(wxCommandEvent& event);
  void OnOk(wxCommandEvent& event);*/

  AutoTrackRaymarine_pi *m_pi;

};




#endif
