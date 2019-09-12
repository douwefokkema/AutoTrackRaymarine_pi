
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



#include "NGT1Read.h"
#include "AutoTrackRaymarine_pi.h"
#include "SerialPort.h"
#include "actisense.h"


enum MSG_State
{
  MSG_START,
  MSG_ESCAPE,
  MSG_MESSAGE
};


  /*
  * Entry
  *
  * Called by wxThread when the new thread is running.
  * It should remain running until Shutdown is called.
  */

NGT1Input::NGT1Input(AutoTrackRaymarine_pi *pi) : wxThread(wxTHREAD_JOINABLE) {
  Create(64 * 1024);  // Stack size
  m_pi = pi;          // This allows you to access the main plugin stuff
  m_shutdown = false;
  m_is_shutdown = true;

  wxLogMessage(wxT("AutoTrackRaymarine_pi: NGT1Input thread created, prio= %i"), GetPriority());
}



void *NGT1Input::Entry(void) {
  int wake_timeout = 0;
  wxLogMessage(wxT("AutoTrackRaymarine_pi: NGTInput1 thread starting"));
  m_is_shutdown = false;
  while (!m_shutdown) {
    if (!ReadNGT1(m_pi->m_serial_comms->m_hSerialin)) {
      break;
    }
  }


  // endless loop until thread destroy
  wxLogMessage(wxT("AutoTrackRaymarine_pi: NGT1Read thread stopping"));
  m_is_shutdown = true;
  return 0;
}

int NGT1Input::ReadNGT1(HANDLE handle)
{
  size_t i;
  //   ssize_t r;
  int r;
  bool printed = 0;
  unsigned char c;
  unsigned char buf[500];
  DWORD dwBytesRead;

  r = ReadFile(handle, buf, sizeof(buf), &dwBytesRead, NULL);
  //	r = read(handle, buf, sizeof(buf));
  if (r <= 0) /* No char read, abort message read */
  {
    wxLogMessage(wxT("AutoTrackRaymarine_pi: Unable to read from NGT1 device"));
  }

  //  fprintf_s(stderr, "Read %d bytes from device \n", (int)dwBytesRead);

  for (i = 0; i < dwBytesRead; i++)
  {
    c = buf[i];
    readNGT1Byte(c);
  }

  return r;
}

void NGT1Input::readNGT1Byte(unsigned char c)
{
  static enum MSG_State state = MSG_START;
  static bool startEscape = false;
  static bool noEscape = false;
  static unsigned char buf[500];
  static unsigned char * head = buf;

  if (state == MSG_ESCAPE)
  {
    if (c == ETX)
    {
      messageReceived(buf, head - buf);
      head = buf;
      state = MSG_START;
    }
    else if (c == STX)
    {
      head = buf;
      state = MSG_MESSAGE;
    }
    else if ((c == DLE) || noEscape)
    {
      *head++ = c;
      state = MSG_MESSAGE;
    }
    else
    {
      //wxLogMessage(wxT("AutoTrackRaymarine_pi: $$$ DLE followed by unexpected char %02X, ignore message"), c);
      state = MSG_START;
    }
  }
  else if (state == MSG_MESSAGE)
  {
    if (c == DLE)
    {
      state = MSG_ESCAPE;
    }
    else {
      *head++ = c;
    }
    //  }
  }
  else
  {
    if (c == DLE)
    {
      state = MSG_ESCAPE;
    }
  }
}



void NGT1Input::messageReceived(const unsigned char * msg, size_t msgLen)
{
  unsigned char command;
  unsigned char checksum = 0;
  unsigned char payloadLen;
  size_t i;
  //wxLogMessage(wxT("AutoTrackRaymarine_pi: $$$ messageReceived len = %i"), msgLen);
  if (msgLen < 3)
  {
    //wxLogMessage(wxT("AutoTrackRaymarine_pi: $$$ Ignore short command len = %i"), msgLen);
    return;
  }

  for (i = 0; i < msgLen; i++)
  {
    checksum += msg[i];
  }

  if (checksum)
  {
    wxLogMessage(wxT("AutoTrackRaymarine_pi: $$$ Ignoring message with invalid checksum"));
    return;
  }

  command = msg[0];
  payloadLen = msg[1];

  //wxLogMessage(wxT("AutoTrackRaymarine_pi: $$$ X message command = %02x len = %u"), command, payloadLen);

  if (command == N2K_MSG_RECEIVED)
  {
    n2kMessageReceived(msg + 2, payloadLen);
  }
  else if (command == NGT_MSG_RECEIVED)
  {
    //  ngtMessageReceived(msg + 2, payloadLen);  // not needed
  }
}


void NGT1Input::n2kMessageReceived(const unsigned char * msg, size_t msgLen)
{
  unsigned int prio, src, dst;
  unsigned int pgn;
  unsigned int len;
  double new_heading, p_h;
  static int test = 3;

  //wxLogMessage(wxT("AutoTrackRaymarine_pi: $$$ n2kMessage reseived  msgLen=%i"), msgLen);
  if (msgLen < 11)
  {
    //wxLogMessage(wxT("AutoTrackRaymarine_pi: $$$Ignoring N2K message - too short"));
    return;
  }
  prio = msg[0];
  pgn = (unsigned int)msg[1] + 256 * ((unsigned int)msg[2] + 256 * (unsigned int)msg[3]);
  dst = msg[4];
  src = msg[5];
  /* Skip the timestamp logged by the NGT-1-A in bytes 6-9 */
  len = msg[10];
  //wxLogMessage(wxT("AutoTrackRaymarine_pi: $$$ n2kMessage received  msgLen=%i, pgn=%i"), msgLen, pgn);
  if (len > 223)
  {
    wxLogMessage(wxT("AutoTrackRaymarine_pi: $$$Ignoring N2K message - too long (%u)"), len);
    return;
  }
  //wxLogMessage(wxT("AutoTrackRaymarine_pi: $$$ message received %i"), pgn);
  double heading = -1000.;

  switch (pgn) {
  case 65360:  // autopilot heading only when pilot is auto. From pilot
    wxLogMessage(wxT("AutoTrackRaymarine_pi: $$$###receiving 65360"));
    if (m_pi->m_pilot_state == 0) {   // standby
      printf("new state = AUTO");
      m_pi->m_pilot_state = 1;  // auto
    }
    p_h = ((unsigned int)msg[16] + 256 * (unsigned int)msg[17]) * 360. / 3.141 / 20000;
    if (m_pi->m_pilot_heading != (int)p_h) {
      m_pi->m_pilot_heading = (int)p_h;
      printf("auto = %i \n", m_pi->m_pilot_heading);  // only print when heading is changed, take out later $$$
    }
    break;

    case 126208:  // set to standby or auto if length is 28
    	//heading = ((unsigned int)msg[12] + 256 * (unsigned int)msg[13]) * 360. / 3.141 / 20000;
    	if (msgLen != 28) { // we only want the messages that originate from a keystroke auto / standly
        break;
    	}

    /*  wxLogMessage(wxT("              $$$  #received message  len= %i \n"), msgLen);
      wxLogMessage(wxT("%0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x, %0x,%0x, %0x, %0x, %0x \n"),
        msg[0], msg[1], msg[2], msg[3], msg[4], msg[5], msg[6], msg[7], msg[8], msg[9], msg[10], msg[11], msg[12], msg[13], msg[14], msg[15],
        msg[16], msg[17], msg[18], msg[19], msg[20], msg[21], msg[22], msg[23], msg[24]);
*/


      //wxLogMessage(wxT("AutoTrackRaymarine_pi:  $$$ lengt 23, fiels22=%0x, f19=%0x, f20=%0x, len=%i"), msg[22], msg[23], msg[24], msgLen);
      if (msg[23] == 0x00 && m_pi->m_pilot_state != STANDBY) {   
        m_pi->m_pilot_state = STANDBY;  // standly
        wxLogMessage(wxT("AutoTrackRaymarine_pi:  $$$###New pilot state = STANDBY "));
        m_pi->m_pilot_heading = -1; // undefined
      }
      if (msg[23] == 0x40) {  // AUTO
        if (m_pi->m_pilot_state == STANDBY) {
          m_pi->m_pilot_state = AUTO;
          wxLogMessage(wxT("AutoTrackRaymarine_pi: $$$###New pilot state = AUTO"));
        }
        else {
          m_pi->ResetXTE();
          m_pi->m_pilot_state = TRACKING;
        }
      }
      break;

  case 126720:
    //wxLogMessage(wxT("AutoTrackRaymarine_pi: 126720 len= %i"), len);
    if (len != 13) {
      break;
    }
    if (msg[19] == 0x40 && m_pi->m_pilot_state != STANDBY) {
      m_pi->m_pilot_state = STANDBY;
      wxLogMessage(wxT("AutoTrackRaymarine_pi:  $$$###1New pilot state = STANDBY "));
    }
    if (msg[19] == 0x42) {  // AUTO
      if (m_pi->m_pilot_state == STANDBY) {
        m_pi->m_pilot_state = AUTO;                       // 
        wxLogMessage(wxT("AutoTrackRaymarine_pi: $$$###1New pilot state = AUTO "));
      }
    }
    break;


  case 65359:
    //  heading all the time
     p_h = (((unsigned int)msg[16] + 256 * (unsigned int)msg[17]) * 360. / 3.141 / 20000);
     m_pi->m_heading = (int)p_h;
     wxLogMessage(wxT("AutoTrackRaymarine_pi: $$$heading updated by 65359 %i "), m_pi->m_heading);
    break;

    //case 65379:
    //	if (len != 8) {
    //		break;
    //	}
    //	if (msg[13] == 0 && state != STANDB) {  // state == STANDBYE
    //		state = STANDB;
    //		printf("New pilot state = STANDB \n");
    //	}
    //	if (msg[13] == 0x42 && state != AUTO) {  // state == AUTO
    //				state = AUTO;
    //				printf("New pilot state = AUTO \n");
    //	}
    //	break;

  case 127250:
    if (len != 8) {
      break;
    }
    if (test >= 0) {

      test--;
      printf("%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x\n", msg[0], msg[1], msg[2], msg[3], msg[4], msg[5], msg[6], msg[7], msg[8], msg[9], msg[10], msg[11], msg[12], msg[13], msg[14]);
    }
    new_heading = ((unsigned int)msg[12] + 256 * (unsigned int)msg[13]) * 360. / 3.141 / 20000;
    if ((int)new_heading != heading) {
      heading = (int)new_heading;
    }

    break;
  default:
    break;
  }
}


void NGT1Input::ngtMessageReceived(const unsigned char * msg, size_t msgLen)
{
  size_t i;
  char line[1000];
  char * p;
  //   char dateStr[DATE_LENGTH];

  if (msgLen < 12)
  {
    fprintf(stderr, "Ignore short msg len = %zu\n", msgLen);
    return;
  }

  sprintf(line, "%s,%u,%u,%u,%u,%u", "date ", 0, 0x40000 + msg[0], 0, 0, (unsigned int)msgLen - 1);
  p = line + strlen(line);
  for (i = 1; i < msgLen && p < line + sizeof(line) - 5; i++)
  {
    sprintf(p, ",%02x", msg[i]);
    p += 3;
  }
  *p++ = 0;

  puts(line);
  fflush(stdout);
}





