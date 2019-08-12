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

#ifndef INC_NGT1Read_h
#define INC_NGT1Read_h
#include "AutoTrackRaymarine_pi.h"

//
// Will start a thread to read te incoming messages on the NGT-1
//


class AutoTrackRaymarine_pi;

class NGT1Input : public wxThread {
public:

  NGT1Input(AutoTrackRaymarine_pi *pi) ;

  int ReadNGT1(HANDLE handle);
  void messageReceived(const unsigned char * msg, size_t msgLen);
  void n2kMessageReceived(const unsigned char * msg, size_t msgLen);
  void readNGT1Byte(unsigned char c);
  void ngtMessageReceived(const unsigned char * msg, size_t msgLen);

  /*
  * Shutdown
  *
  * Called when the thread should stop.
  * It should stop running.
  */
  void Shutdown(void) { m_shutdown = true; }

  ~NGT1Input() {
    while (!m_is_shutdown) {
      wxMilliSleep(50);
    }
  }

  volatile bool m_is_shutdown;

protected:
  void *Entry(void);

private:
  //bool ProcessReport(const NetworkAddress &radar_address, const uint8_t *data, size_t len);

  AutoTrackRaymarine_pi *m_pi;
  volatile bool m_shutdown;

  wxCriticalSection m_exclusive;
};

#endif // INC_NGT1Read_h