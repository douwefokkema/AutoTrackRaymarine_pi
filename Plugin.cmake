# ~~~
# Summary:      Plugin-specific build setup
# Summary:      Plugin-specific build setup
# Copyright (c) 2020-2021 Mike Rossiter
#               2021 Alec Leamas
# License:      GPLv3+
# ~~~

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.


# -------- Options ----------

set(OCPN_TEST_REPO
    "opencpn-radar-pi/opencpn-radar-pi-unstable"
    CACHE STRING "Default repository for untagged builds"
)
set(OCPN_BETA_REPO
    "opencpn-radar-pi/opencpn-radar-pi-beta"
    CACHE STRING 
    "Default repository for tagged builds matching 'beta'"
)
set(OCPN_RELEASE_REPO
    "opencpn-radar-pi/opencpn-radar-pi-prod"
    CACHE STRING 
    "Default repository for tagged builds not matching 'beta'"
)

#
# -------  Plugin setup --------
#
set(PKG_NAME autotrackraymarine_pi)
set(PKG_VERSION 5.5.1)
set(PKG_PRERELEASE "-beta")  # Empty, or a tag like 'beta'

set(DISPLAY_NAME autotrackraymarine)    # Dialogs, installer artifacts, ...
set(PLUGIN_API_NAME Autotrackraymarine) # As of GetCommonName() in plugin API
set(CPACK_PACKAGE_CONTACT "df@percussion.nl")   ##  Not used
set(PKG_SUMMARY "Tracking with Raymarine pilots on OpenCPN")
set(PKG_DESCRIPTION [=[
Will follow the active route with a Raymarine Evolution autopilot.
Also provides remote control for Evolution pilot.
]=])

set(PKG_AUTHOR "Douwe Fokkema")
set(PKG_IS_OPEN_SOURCE "yes")
set(PKG_HOMEPAGE https://github.com/douwefokkema/AutoTrackRaymarine_pi)
set(PKG_INFO_URL https://opencpn.org/OpenCPN/plugins/)


set(SRC
      include/AutoTrackRaymarine_pi.h
      include/PreferencesDialog.h
      include/AutotrackInfoUI.h
      include/AutotrackPrefsUI.h
      include/Info.h

      src/AutoTrackRaymarine_pi.cpp
      src/PreferencesDialog.cpp
      src/AutotrackInfoUI.cpp
      src/AutotrackPrefsUI.cpp
      src/Info.cpp
      src/icons.cpp
      src/TexFont.cpp
)

set(PKG_API_LIB api-18)  #  A directory in opencpn-libs

macro(late_init)
  # Perform initialization after the PACKAGE_NAME library, compilers
  # and ocpn::api is available.

  # Fix OpenGL deprecated warnings in Xcode
  target_compile_definitions(${PACKAGE_NAME} PRIVATE GL_SILENCE_DEPRECATION)

  target_include_directories(${PACKAGE_NAME} PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/include 
  )
endmacro ()

macro(add_plugin_libraries)
  # Add libraries required by this plugin

  add_subdirectory("opencpn-libs/nmea0183")
  target_link_libraries(${PACKAGE_NAME} ocpn::nmea0183)

  add_subdirectory("opencpn-libs/wxJSON")
  target_link_libraries(${PACKAGE_NAME} ocpn::wxjson)

  add_subdirectory("opencpn-libs/N2KParser")
  target_link_libraries(${PACKAGE_NAME} ocpn::N2KParser)
endmacro ()
