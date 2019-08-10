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



#pragma once

#include "wx/wx.h"

#include <wx/fileconf.h>

#include "version.h"
#include "wxWTranslateCatalog.h"

#include "SerialPort.h"

class AutoTrackRaymarine_pi;

// Handles serial port selection and opening

class SerialPort {

public:
  SerialPort(AutoTrackRaymarine_pi *pi);
  ~SerialPort();
  AutoTrackRaymarine_pi* m_pi;
  HANDLE m_hSerialin; //COM port handler
  NGT1Input *m_NGT1_read;
  bool OpenSerialPort(wchar_t* pcCommPort, HANDLE* handle);
  void writeMessage(HANDLE handle, unsigned char command, const unsigned char * cmd, const size_t len);
  void parseAndWriteIn(HANDLE handle, const unsigned char * cmd);

};