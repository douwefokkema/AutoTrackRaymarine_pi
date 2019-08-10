
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


#include "SerialPort.h"
#include "AutoTrackRaymarine_pi.h"
#include "AutoTrackRaymarineUI.h"
#include "PreferencesDialog.h"
#include "conio.h"
#include "serial/serial.h"

//#define NGT1_port 6  // later in preferences, todo

class AutoTrackRaymarine_pi;
class PreferencesDialog;
class PreferencesDialogBase;

SerialPort::SerialPort(AutoTrackRaymarine_pi *pi) {
  m_pi = pi;

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

  //sprintf_s(s, "\\\\.\\COM%d", NGT1_port);
  mbstowcs(pcCommPort, m_pi->prefs.com_port, strlen(m_pi->prefs.com_port) + 1); //Plus null
  sprintf_s(Port, "%s", m_pi->prefs.com_port);
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
      wxLogMessage(wxT("AutoTrackRaymarine_pi: serial port COM%s not found, can not open NGT-1."), m_pi->prefs.com_port);
  }
  else
  {
    CloseHandle(hComm);
    wxLogMessage(wxT("AutoTrackRaymarine_pi: Found serial port COM%s and it's ready to use"), m_pi->prefs.com_port);
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



};

SerialPort::~SerialPort() {
};

bool SerialPort::OpenSerialPort(wchar_t* pcCommPort, HANDLE* handle) {
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



