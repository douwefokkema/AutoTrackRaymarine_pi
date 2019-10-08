
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
#include "conio.h"
#include "serial/serial.h"
#include "actisense.h"


using namespace std;

//#define NGT1_port 6  // later in preferences, todo

class AutoTrackRaymarine_pi;
class PreferencesDialog;
class PreferencesDialogBase;

SerialPort::SerialPort(AutoTrackRaymarine_pi *pi) {
  m_pi = pi;
  wxLogMessage(wxT("AutoTrackRaymarine_pi: $$$ entering SerialPort"));
  // initialise NGT-1 com port
  //char s[20];
  wchar_t pcCommPort[20];
 

  /* The following startup command reverse engineered from Actisense NMEAreader.
  * It instructs the NGT1 to clear its PGN message TX list, thus it starts
  * sending all PGNs.
  */
  static unsigned char NGT_STARTUP_SEQ[] =
  { 0x11   /* msg byte 1, meaning ? */
    , 0x02   /* msg byte 2, meaning ? */
    , 0x00   /* msg byte 3, meaning ? */
  };


  string r = "\\\\.\\" + m_pi->prefs.com_port;
  
  mbstowcs(pcCommPort, &r[0], strlen(m_pi->prefs.com_port) + 5); //Plus null
  // Open the port tentatively
  HANDLE hComm = ::CreateFile(pcCommPort, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
  wxLogMessage(wxT("AutoTrackRaymarine_pi: pcCommPort2=%s"), pcCommPort);

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
    wxLogMessage(wxT("AutoTrackRaymarine_pi: Found serial port %s and it's ready to use"), m_pi->prefs.com_port);
  }
  // now really open com port
  wxLogMessage(wxT("AutoTrackRaymarine_pi $$$ going to call oopenserial port"));
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


  m_NGT1_read = new NGT1Input(m_pi);
  if (m_NGT1_read->Run() != wxTHREAD_NO_ERROR) {
    wxLogMessage(wxT("AutoTrackRaymarine_pi: unable to start NGT1Input thread"));
  }
};

SerialPort::~SerialPort() {
  if (m_NGT1_read) {
    m_NGT1_read->Shutdown();
    wxLogMessage(wxT("AutoTrackRaymarine_pi: shut down NGT1read thread"));
    delete m_NGT1_read;
  }
  if (m_hSerialin) {
    if (CloseHandle(m_hSerialin) == 0) {
      wxLogMessage(wxT("AutoTrackRaymarine_pi Error closing serial port NGT-1"));
    }
    else {
      wxLogMessage(wxT("AutoTrackRaymarine_pi: closed serial port"), m_pi->prefs.com_port);
    }
    if (m_hSerialin) {
      /*wxLogMessage(wxT("AutoTrackRaymarine_pi: m_hSerialin deleted0"));
      delete m_hSerialin;
      wxLogMessage(wxT("AutoTrackRaymarine_pi: m_hSerialin deleted1"));*/
    }
    else {
      wxLogMessage(wxT("AutoTrackRaymarine_pi: m_hSerialin not deleted"));
    }
  }

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

  // Set device parameters (115200 baud, 1 start bit,
  // 1 stop bit, no parity)
  dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
  if (GetCommState(*handle, &dcbSerialParams) == 0)
  {
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

void SerialPort::writeMessage(HANDLE handle, unsigned char command, const unsigned char * cmd, const size_t len)
{
  unsigned char bst[255];
  unsigned char *b = bst;
  unsigned char *lenPtr;
  unsigned char crc;
  DWORD bytes_written;
  int length = len;

  *b++ = DLE;
  *b++ = STX;
  *b++ = command;
  crc = command;
  lenPtr = b++;

  for (size_t i = 0; i < len; i++)
  {
    if (cmd[i] == DLE)
    {
      *b++ = DLE;
    }
    *b++ = cmd[i];
    crc += (unsigned char)cmd[i];
  }

  *lenPtr = length;
  crc += length;

  *b++ = (unsigned char)(256 - (int)crc);
  *b++ = DLE;
  *b++ = ETX;
  if (!WriteFile(handle, bst, b - bst, &bytes_written, NULL)) {
  //  wxLogMessage(wxT("AutoTrackRaymarine_pi Error. Writing to serial port"));
    CloseHandle(m_hSerialin);
    int Dummy = toupper(_getch());
  }
  // wxLogMessage(wxT("AutoTrackRaymarine_pi $$$length=%i, written %i "), length, bytes_written);
  //  if (write(handle, bst, b - bst) != b - bst)
}

void SerialPort::SetAutopilotHeading(double heading) {
   wxLogMessage(wxT("$$$AutoTrackRaymarine_pi SetAutopilotHeading = %f"), heading);
  
  // commands for NGT-1 in Canboat format
  string standby_command = "Z,3,126208,7,204,17,01,63,ff,00,f8,04,01,3b,07,03,04,04,00,00,05,ff,ff";  //set standby
  // string auto_command = "Z,3,126208,7,204,17,01,63,ff,00,f8,04,01,3b,07,03,04,04,40,00,05,ff,ff";  // set auto
  string msg2 = "Z,3,126208,7,204,14,01,50,ff,00,f8,03,01,3b,07,03,04,06,00,00";  //set 0 magnetic
  // string msg3 = "Z,3,126208,7,204,14,01,50,ff,00,f8,03,01,3b,07,03,04,06,9f,3e";  //set 92 magnetic
  // string msg4 = "Z,3,126208,7,204,14,01,50,ff,00,f8,03,01,3b,07,03,04,06,4e,3f";  //set 93 example only, magnetic

  double heading_normal = heading;
  while (heading_normal < 0) heading_normal += 360;
  while (heading_normal >= 360) heading_normal -= 360;
  uint16_t heading_radials1000 = (uint16_t)(heading_normal * 174.53); // heading to be set in thousands of radials
                                                                      //wxLogMessage(wxT("$$$AutoTrackRaymarine_pi SetAutopilotHeading2 radials = %i %000x"), heading_radials1000, heading_radials1000);
  uint8_t byte0, byte1;
  byte0 = heading_radials1000 & 0xff;
  byte1 = heading_radials1000 >> 8;
  //wxLogMessage(wxT("AutoTrackRaymarine_pi SetAutopilotHeading byte0 = %0x, byte1 = %0x"), byte0, byte1);
  char s[4];
  sprintf_s(s, "%0x", byte0);
  if (byte0 > 15) {
    msg2[56] = s[0];
    msg2[57] = s[1];
  }
  else {
    msg2[56] = '0';
    msg2[57] = s[0];
  }
  sprintf_s(s, "%0x", byte1);
  if (byte1 > 15) {
    msg2[59] = s[0];
    msg2[60] = s[1];
  }
  else {
    msg2[59] = '0';
    msg2[60] = s[0];
  }
  unsigned char msg[500];
  for (unsigned int i = 0; i <= msg2.length(); i++) {
    msg[i] = msg2[i];
  }
  parseAndWriteIn(m_hSerialin, msg);
}

void SerialPort::SetAuto() {
  wxLogMessage(wxT("$$$# set pilot auto"));
  string auto_command = "Z,3,126208,7,204,17,01,63,ff,00,f8,04,01,3b,07,03,04,04,40,00,05,ff,ff";  // set auto
  unsigned char msg[500];
  for (unsigned int i = 0; i <= auto_command.length(); i++) {
    msg[i] = auto_command[i];
  }
  parseAndWriteIn(m_hSerialin, msg);
}

void SerialPort::SetStandby() {
  wxLogMessage(wxT("$$$# set pilot standby"));
  string standby_command = "Z,3,126208,7,204,17,01,63,ff,00,f8,04,01,3b,07,03,04,04,00,00,05,ff,ff";  //set standby
  unsigned char msg[500];
  for (unsigned int i = 0; i <= standby_command.length(); i++) {
    msg[i] = standby_command[i];
  }
  parseAndWriteIn(m_hSerialin, msg);
}

void SerialPort::SetP70Tracking() {
  //string msg3 = "Z,3,126208,7,204,14,01,50,ff,00,f8,03,01,3b,07,03,04,06,9f,3e";
  //string auto_command = "Z,3,126208,7,204,17,01,63,ff,00,f8,04,01,3b,07,03,04,04,40,00,05,ff,ff";  // set auto
  string track =          "Z,3,126208,7,204,0d,3b,9f,f0,81,84,46,27,9d,4a,00,00,02,08,4e";  // status message for track
  unsigned char msg[500];
  for (unsigned int i = 0; i <= track.length(); i++) {
    msg[i] = track[i];
  }
  parseAndWriteIn(m_hSerialin, msg);
}

void SerialPort::parseAndWriteIn(HANDLE handle, const unsigned char * cmd)
{
  unsigned char msg[500];
  unsigned char * m;

  unsigned int prio;
  unsigned int pgn;
  unsigned int src;
  unsigned int dst;
  unsigned int bytes;

  char * p;
  int i;
  unsigned int byt, b;
  int r;

  if (!cmd || !*cmd || *cmd == '\n')
  {
    return;
  }
  wxLogMessage(wxT("$$$# parse and writein"));
  p = strchr((char *)cmd, ',');
  if (!p)
  {
    return;
  }

  r = sscanf(p, ",%u,%u,%u,%u,%u,%n", &prio, &pgn, &src, &dst, &bytes, &i);
  if (r == 5)
  {
    p += i - 1;
    m = msg;
    *m++ = (unsigned char)prio;
    *m++ = (unsigned char)pgn;
    *m++ = (unsigned char)(pgn >> 8);
    *m++ = (unsigned char)(pgn >> 16);
    *m++ = (unsigned char)dst;
    //*m++ = (unsigned char) 0;
    *m++ = (unsigned char)bytes;
    //wxLogMessage(wxT("AutoTrackRaymarine_pi $$$write message prio %i, pgn %i, src %i, dst %i, bytes %i, i %i "), prio, pgn, src, dst, bytes, i);

    for (b = 0; m < msg + sizeof(msg) && b < bytes; b++)
    {
      if ((sscanf(p, ",%x%n", &byt, &i) == 1) && (byt < 256))
      {
        *m++ = byt;
        //wxLogMessage(wxT("AutoTrackRaymarine_pi byt=%0x "), byt);   // $$$
      }
      else
      {
        wxLogMessage(wxT("AutoTrackRaymarine_pi: Unable to parse incoming message '%s' at offset %u %i"), cmd, b, byt);
        Sleep(1000);
        return;
      }
      p += i;
    }
  }
  else
  {
    wxLogMessage(wxT("AutoTrackRaymarine_pi: Unable to parse incoming message '%s', r = %d"), cmd, r);
    return;
  }
  /*wxLogMessage(wxT("AutoTrackRaymarine_pi                  write message called len= %i "), m - msg)*/;

  /* wxLogMessage(wxT("              $$$  write message called len= %i \n"), m - msg);
  wxLogMessage(wxT("%0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x \n"),
  msg[0], msg[1], msg[2], msg[3], msg[4], msg[5], msg[6], msg[7], msg[8], msg[9], msg[10], msg[11], msg[12], msg[13], msg[14], msg[15],
  msg[16], msg[17], msg[18], msg[19], msg[20]);*/


  writeMessage(handle, N2K_MSG_SEND, msg, m - msg);
  /*wxLogMessage(wxT("AutoTrackRaymarine_pi: $$$message written to Actisense"));*/
}



