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
    wxLogMessage(wxT("AutoTrackRaymarine_pi: $$$x comport value =%s"), p.com_port);
    Hide();
}

void PreferencesDialog::FindSerialPorts() {
  // find and locate serial ports
  m_pSerialArray = NULL;
  m_pSerialArray = EnumerateSerialPorts();

  if (m_pSerialArray) {
    if (m_comboBox1)
      m_comboBox1->Clear();
    for (size_t i = 0; i < m_pSerialArray->Count(); i++) {
      m_comboBox1->Append(m_pSerialArray->Item(i));
    }
  }
}

wxArrayString *PreferencesDialog::EnumerateSerialPorts(void) {
  wxArrayString *preturn = new wxArrayString;
#ifdef OCPN_USE_NEWSERIAL
  std::vector<serial::PortInfo> ports = serial::list_ports();
  for (std::vector<serial::PortInfo>::iterator it = ports.begin(); it != ports.end(); ++it) {
    wxString port((*it).port);
    if ((*it).description.length() > 0 && (*it).description != "n/a") {
      port.Append(_T(" - "));
      wxString s_description = wxString::FromUTF8(((*it).description).c_str());
      port.Append(s_description);
    }
    preturn->Add(port);
  }
#ifdef __WXMSW__
  //    Search for Garmin device driver on Windows platforms
  HDEVINFO hdeviceinfo = INVALID_HANDLE_VALUE;
  hdeviceinfo = SetupDiGetClassDevs((GUID *)&GARMIN_DETECT_GUID, NULL, NULL,
    DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
  if (hdeviceinfo != INVALID_HANDLE_VALUE) {

    if (GarminProtocolHandler::IsGarminPlugged()) {
      wxLogMessage(_T("EnumerateSerialPorts() Found Garmin USB Device."));
      preturn->Add(_T("Garmin-USB"));         // Add generic Garmin selectable device
    }
  }
#endif // __WXMSW__
#else
#if defined(__UNIX__) && !defined(__OCPN__ANDROID__) && !defined(__WXOSX__)

  //Initialize the pattern table
  if (devPatern[0] == NULL) {
    paternAdd("ttyUSB");
    paternAdd("ttyACM");
    paternAdd("ttyGPS");
    paternAdd("refcom");
  }

  //  Looking for user privilege openable devices in /dev
  //  Fulup use scandir to improve user experience and support new generation of AIS devices.

  wxString sdev;
  int ind, fcount;
  struct dirent **filelist = { 0 };

  // scan directory filter is applied automatically by this call
  fcount = scandir("/dev", &filelist, paternFilter, alphasort);

  for (ind = 0; ind < fcount; ind++) {
    wxString sdev(filelist[ind]->d_name, wxConvUTF8);
    sdev.Prepend(_T("/dev/"));

    preturn->Add(sdev);
    free(filelist[ind]);
  }

  free(filelist);

  //        We try to add a few more, arbitrarily, for those systems that have fixed, traditional COM ports

#ifdef __linux__
  if (isTTYreal("/dev/ttyS0"))
    preturn->Add(_T("/dev/ttyS0"));

  if (isTTYreal("/dev/ttyS1"))
    preturn->Add(_T("/dev/ttyS1"));
#endif /* linux */


#endif

#ifdef PROBE_PORTS__WITH_HELPER

  /*
  *     For modern Linux/(Posix??) systems, we may use
  *     the system files /proc/tty/driver/serial
  *     and /proc/tty/driver/usbserial to identify
  *     available serial ports.
  *     A complicating factor is that most (all??) linux
  *     systems require root privileges to access these files.
  *     We will use a helper program method here, despite implied vulnerability.
  */

  char buf[256]; // enough to hold one line from serial devices list
  char left_digit;
  char right_digit;
  int port_num;
  FILE *f;

  pid_t pID = vfork();

  if (pID == 0)// child
  {
    //    Temporarily gain root privileges
    seteuid(file_user_id);

    //  Execute the helper program
    execlp("ocpnhelper", "ocpnhelper", "-SB", NULL);

    //  Return to user privileges
    seteuid(user_user_id);

    wxLogMessage(_T("Warning: ocpnhelper failed...."));
    _exit(0);// If exec fails then exit forked process.
  }

  wait(NULL);                  // for the child to quit

                               //    Read and parse the files

                               /*
                               * see if we have any traditional ttySx ports available
                               */
  f = fopen("/var/tmp/serial", "r");

  if (f != NULL)
  {
    wxLogMessage(_T("Parsing copy of /proc/tty/driver/serial..."));

    /* read in each line of the file */
    while (fgets(buf, sizeof(buf), f) != NULL)
    {
      wxString sm(buf, wxConvUTF8);
      sm.Prepend(_T("   "));
      sm.Replace(_T("\n"), _T(" "));
      wxLogMessage(sm);

      /* if the line doesn't start with a number get the next line */
      if (buf[0] < '0' || buf[0] > '9')
        continue;

      /*
      * convert digits to an int
      */
      left_digit = buf[0];
      right_digit = buf[1];
      if (right_digit < '0' || right_digit > '9')
        port_num = left_digit - '0';
      else
        port_num = (left_digit - '0') * 10 + right_digit - '0';

      /* skip if "unknown" in the string */
      if (strstr(buf, "unknown") != NULL)
        continue;

      /* upper limit of 15 */
      if (port_num > 15)
        continue;

      /* create string from port_num  */

      wxString s;
      s.Printf(_T("/dev/ttyS%d"), port_num);

      /*  add to the output array  */
      preturn->Add(wxString(s));

    }

    fclose(f);
  }

  /*
  * Same for USB ports
  */
  f = fopen("/var/tmp/usbserial", "r");

  if (f != NULL)
  {
    wxLogMessage(_T("Parsing copy of /proc/tty/driver/usbserial..."));

    /* read in each line of the file */
    while (fgets(buf, sizeof(buf), f) != NULL)
    {

      wxString sm(buf, wxConvUTF8);
      sm.Prepend(_T("   "));
      sm.Replace(_T("\n"), _T(" "));
      wxLogMessage(sm);

      /* if the line doesn't start with a number get the next line */
      if (buf[0] < '0' || buf[0] > '9')
        continue;

      /*
      * convert digits to an int
      */
      left_digit = buf[0];
      right_digit = buf[1];
      if (right_digit < '0' || right_digit > '9')
        port_num = left_digit - '0';
      else
        port_num = (left_digit - '0') * 10 + right_digit - '0';

      /* skip if "unknown" in the string */
      if (strstr(buf, "unknown") != NULL)
        continue;

      /* upper limit of 15 */
      if (port_num > 15)
        continue;

      /* create string from port_num  */

      wxString s;
      s.Printf(_T("/dev/ttyUSB%d"), port_num);

      /*  add to the output array  */
      preturn->Add(wxString(s));

    }

    fclose(f);
  }

  //    As a fallback, in case seteuid doesn't work....
  //    provide some defaults
  //    This is currently the case for GTK+, which
  //    refuses to run suid.  sigh...

  if (preturn->IsEmpty())
  {
    preturn->Add(_T("/dev/ttyS0"));
    preturn->Add(_T("/dev/ttyS1"));
    preturn->Add(_T("/dev/ttyUSB0"));
    preturn->Add(_T("/dev/ttyUSB1"));
    preturn->Add(_T("/dev/ttyACM0"));
    preturn->Add(_T("/dev/ttyACM1"));
  }

  //    Clean up the temporary files created by helper.
  pid_t cpID = vfork();

  if (cpID == 0)// child
  {
    //    Temporarily gain root privileges
    seteuid(file_user_id);

    //  Execute the helper program
    execlp("ocpnhelper", "ocpnhelper", "-U", NULL);

    //  Return to user privileges
    seteuid(user_user_id);
    _exit(0);// If exec fails then exit forked process.
  }

#endif      // __WXGTK__
#ifdef __WXOSX__
#include "macutils.h"
  char* paPortNames[MAX_SERIAL_PORTS];
  int iPortNameCount;

  memset(paPortNames, 0x00, sizeof(paPortNames));
  iPortNameCount = FindSerialPortNames(&paPortNames[0], MAX_SERIAL_PORTS);
  for (int iPortIndex = 0; iPortIndex<iPortNameCount; iPortIndex++)
  {
    wxString sm(paPortNames[iPortIndex], wxConvUTF8);
    preturn->Add(sm);
    free(paPortNames[iPortIndex]);
  }
#endif      //__WXOSX__
#ifdef __WXMSW__
  /*************************************************************************
  * Windows provides no system level enumeration of available serial ports
  * There are several ways of doing this.
  *
  *************************************************************************/

#include <windows.h>

  //    Method 1:  Use GetDefaultCommConfig()
  // Try first {g_nCOMPortCheck} possible COM ports, check for a default configuration
  //  This method will not find some Bluetooth SPP ports
  for (int i = 1; i < g_nCOMPortCheck; i++) {
    wxString s;
    s.Printf(_T("COM%d"), i);

    COMMCONFIG cc;
    DWORD dwSize = sizeof(COMMCONFIG);
    if (GetDefaultCommConfig(s.fn_str(), &cc, &dwSize))
      preturn->Add(wxString(s));
  }

#if 0
  // Method 2:  Use FileOpen()
  // Try all 255 possible COM ports, check to see if it can be opened, or if
  // not, that an expected error is returned.

  BOOL bFound;
  for (int j = 1; j<256; j++)
  {
    char s[20];
    sprintf(s, "\\\\.\\COM%d", j);

    // Open the port tentatively
    BOOL bSuccess = FALSE;
    HANDLE hComm = ::CreateFile(s, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);

    //  Check for the error returns that indicate a port is there, but not currently useable
    if (hComm == INVALID_HANDLE_VALUE)
    {
      DWORD dwError = GetLastError();

      if (dwError == ERROR_ACCESS_DENIED ||
        dwError == ERROR_GEN_FAILURE ||
        dwError == ERROR_SHARING_VIOLATION ||
        dwError == ERROR_SEM_TIMEOUT)
        bFound = TRUE;
    }
    else
    {
      bFound = TRUE;
      CloseHandle(hComm);
    }

    if (bFound)
      preturn->Add(wxString(s));
  }
#endif

  //// Method 3:  WDM-Setupapi
  ////  This method may not find XPort virtual ports,
  ////  but does find Bluetooth SPP ports

  //GUID *guidDev = (GUID*)&GUID_CLASS_COMPORT;

  //HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;

  //hDevInfo = SetupDiGetClassDevs(guidDev,
  //  NULL,
  //  NULL,
  //  DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

  //if (hDevInfo != INVALID_HANDLE_VALUE) {

  //  BOOL bOk = TRUE;
  //  SP_DEVICE_INTERFACE_DATA ifcData;

  //  ifcData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
  //  for (DWORD ii = 0; bOk; ii++) {
  //    bOk = SetupDiEnumDeviceInterfaces(hDevInfo, NULL, guidDev, ii, &ifcData);
  //    if (bOk) {
  //      // Got a device. Get the details.

  //      SP_DEVINFO_DATA devdata = { sizeof(SP_DEVINFO_DATA) };
  //      bOk = SetupDiGetDeviceInterfaceDetail(hDevInfo,
  //        &ifcData, NULL, 0, NULL, &devdata);

  //      //      We really only need devdata
  //      if (!bOk) {
  //        if (GetLastError() == 122)  //ERROR_INSUFFICIENT_BUFFER, OK in this case
  //          bOk = true;
  //      }

  //      //      We could get friendly name and/or description here
  //      TCHAR fname[256] = { 0 };
  //      TCHAR desc[256] = { 0 };
  //      if (bOk) {
  //        BOOL bSuccess = SetupDiGetDeviceRegistryProperty(
  //          hDevInfo, &devdata, SPDRP_FRIENDLYNAME, NULL,
  //          (PBYTE)fname, sizeof(fname), NULL);

  //        bSuccess = bSuccess && SetupDiGetDeviceRegistryProperty(
  //          hDevInfo, &devdata, SPDRP_DEVICEDESC, NULL,
  //          (PBYTE)desc, sizeof(desc), NULL);
  //      }

  //      //  Get the "COMn string from the registry key
  //      if (bOk) {
  //        bool bFoundCom = false;
  //        TCHAR dname[256];
  //        HKEY hDeviceRegistryKey = SetupDiOpenDevRegKey(hDevInfo, &devdata,
  //          DICS_FLAG_GLOBAL, 0,
  //          DIREG_DEV, KEY_QUERY_VALUE);
  //        if (INVALID_HANDLE_VALUE != hDeviceRegistryKey) {
  //          DWORD RegKeyType;
  //          wchar_t    wport[80];
  //          LPCWSTR cstr = wport;
  //          MultiByteToWideChar(0, 0, "PortName", -1, wport, 80);
  //          DWORD len = sizeof(dname);

  //          int result = RegQueryValueEx(hDeviceRegistryKey, cstr,
  //            0, &RegKeyType, (PBYTE)dname, &len);
  //          if (result == 0)
  //            bFoundCom = true;
  //        }

  //        if (bFoundCom) {
  //          wxString port(dname, wxConvUTF8);

  //          //      If the port has already been found, remove the prior entry
  //          //      in favor of this entry, which will have descriptive information appended
  //          for (unsigned int n = 0; n < preturn->GetCount(); n++) {
  //            if ((preturn->Item(n)).IsSameAs(port)) {
  //              preturn->RemoveAt(n);
  //              break;
  //            }
  //          }
  //          wxString desc_name(desc, wxConvUTF8);         // append "description"
  //          port += _T(" ");
  //          port += desc_name;

  //          preturn->Add(port);
  //        }
  //      }
  //    }
  //  }//for
  //}// if


  // //    Search for Garmin device driver on Windows platforms

  //HDEVINFO hdeviceinfo = INVALID_HANDLE_VALUE;

  //hdeviceinfo = SetupDiGetClassDevs((GUID *)&GARMIN_DETECT_GUID, NULL, NULL,
  //  DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

  //if (hdeviceinfo != INVALID_HANDLE_VALUE) {

  //  if (GarminProtocolHandler::IsGarminPlugged()) {
  //    wxLogMessage(_T("EnumerateSerialPorts() Found Garmin USB Device."));
  //    preturn->Add(_T("Garmin-USB"));         // Add generic Garmin selectable device
  //  }
  //}

#if 0
  SP_DEVICE_INTERFACE_DATA deviceinterface;
  deviceinterface.cbSize = sizeof(deviceinterface);

  if (SetupDiEnumDeviceInterfaces(hdeviceinfo,
    NULL,
    (GUID *)&GARMIN_DETECT_GUID,
    0,
    &deviceinterface))
  {
    wxLogMessage(_T("Found Garmin Device."));

    preturn->Add(_T("GARMIN"));         // Add generic Garmin selectable device
  }
#endif

#endif      //__WXMSW__
#endif //OCPN_USE_NEWSERIAL
  return preturn;
}

