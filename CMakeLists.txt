##---------------------------------------------------------------------------
## Author:      Douwe Fokkema, following a model of Sean D'Epagnier
## Copyright:   
## License:     GPLv3+
##---------------------------------------------------------------------------
# CI-CMake: v1.0.238.0 11/06/2023  # DD/MM/YYYY format  CI-CMake Author: Jon Gough
# ---------------------------------------------------------------------------

## ----- When changing this file do NOT change the order in which sections occur        ----- ##
## ----- Changes should only be made between the section blocks that identify where     ----- ##
## ----- these changes should be. The whole configuration process relies on this        ----- ##
## ----- sequence to be successful                                                      ----- ##
##

##
## ----- Modify section below to include all the details for your plugin ----- ##
##

set(CMLOC "CMakeLists: ")

# define minimum cmake version
cmake_minimum_required(VERSION 3.5.1)
if(COMMAND cmake_policy)
    if(POLICY CMP0043)
        cmake_policy(SET CMP0043 NEW)
    endif(POLICY CMP0043)
    cmake_policy(SET CMP0048 NEW)
# Need cmake 3.13 to implement this, but raspbian builds are at 3.7.2 and bionic at 3.10 and xenial builds at 3.5.1
#    cmake_policy(SET CMP0076 NEW)
    if(POLICY CMP0077)
        cmake_policy(SET CMP0077 NEW)
    endif(POLICY CMP0077)
endif(COMMAND cmake_policy)

# define plugin name, owner and versions
set(VERBOSE_NAME "AutoTrackRaymarine")
set(COMMON_NAME "AutoTrackRaymarine")
set(TITLE_NAME "AutoTrackRaymarine")
set(PACKAGE_CONTACT "Douwe Fokkema")
set(PACKAGE "AutoTrackRaymarine")
set(SHORT_DESCRIPTION "AutoTrackRaymarine PlugIn for OpenCPN")
set(LONG_DESCRIPTION "Route tracking and remote control for Raymarine Evolution pilots")

SET(VERSION_MAJOR "1")
SET(VERSION_MINOR "1")
SET(VERSION_PATCH "4")
set(VERSION_TWEAK "0")
<<<<<<< HEAD
SET(VERSION_DATE "25/06/2023") # DD/MM/YYYY format
SET(OCPN_MIN_VERSION "ov58")
set(OCPN_API_VERSION_MAJOR "1")
set(OCPN_API_VERSION_MINOR "18")
=======
SET(VERSION_DATE "17/06/2023") # DD/MM/YYYY format
SET(OCPN_MIN_VERSION "ov58")
set(OCPN_API_VERSION_MAJOR "1")
set(OCPN_API_VERSION_MINOR "17")
>>>>>>> 86d9bb3771f5f6b1b90b5a110dcc3ada34526130
set(TP_COMMENT "  * Release for O58 using CI")

set(PARENT "opencpn")

# The next line allows for setting up your own local webserver with git on it for testing purposes, the default is github.com
#set(GIT_REPOSITORY_SERVER "github.com")

# Specifies cloudsmith repository sufficies
set(PROD "prod")    # Standard Repos
set(BETA "beta")    # Standard Repos
set(ALPHA "alpha")  # Standard Repos

# set this if your cloudsmith repository name does not match your git repository name
#set(CLOUDSMITH_BASE_REPOSITORY "AutoTrackRaymarine")
set(CLOUDSMITH_BASE_REPOSITORY "autotrackraymarine")
# Set the following to use OpenCPN cloudsmith repositories instead of own repositories
set(CLOUDSMITH_USER "opencpn")
#set(CLOUDSMITH_USER "rick-gleason")

set(XML_INFO_URL "https://opencpn-manuals.github.io/main/autotrackraymarine/index.html")
set(XML_SUMMARY ${SHORT_DESCRIPTION})
set(XML_DESCRIPTION ${LONG_DESCRIPTION})

##
## ----- Modify section above to include all the details for your plugin ----- ##
##

##
## ----- Modify section below if there are special requirements for the plugin ----- ##
##

option(PLUGIN_USE_SVG "Use SVG graphics" ON)

set(CMAKE_CXX_STANDARD 11)

# Use local version of GLU library
# requires libs/glu directory
set(USE_LOCAL_GLU TRUE)
# Prefer libGL.so to libOpenGL.so, see CMP0072
set(OpenGL_GL_PREFERENCE "LEGACY")

option(USE_GL "Enable OpenGL support" ON)
message(STATUS "${CMLOC}USE_GL: ${USE_GL}")

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
endif("${CMAKE_BUILD_TYPE}" STREQUAL "")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "" "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
message(STATUS "${CMLOC}Build type: ${CMAKE_BUILD_TYPE}")

##
## ----- The statements below are used to setup standard variables that are required by the CMAKE process - do not remove ----- ##
##

project(${PACKAGE})

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include("PluginSetup")

##
## ----- The above is used to setup standard variables that are required by the CMAKE process - do not remove ----- ##
##

##
## ----- Modify section below if there are special requirements for the plugin ----- ##
##

set(CMAKE_VERBOSE_MAKEFILE "Activate verbose mode for make files" ON)

option(Plugin_CXX11 "Use c++11" OFF)

##
## ----- Modify section above if there are special requirements for the plugin ----- ##
##

##
## ----- do not change next section - needed to configure build process ----- ##
##

include("PluginConfigure")

##
## ----- do not change section above - needed to configure build process ----- ##
##

#
# ----- Change below to match project requirements for source, headers, etc. ----- ##
#

SET(SRCS
      src/AutoTrackRaymarine_pi.cpp
      src/PreferencesDialog.cpp
      src/AutotrackInfoUI.cpp
      src/AutotrackPrefsUI.cpp
      src/Info.cpp
      src/icons.cpp
      src/TexFont.cpp
	)
	
SET (HDRS
      src/AutoTrackRaymarine_pi.h
      src/PreferencesDialog.h
      src/AutotrackInfoUI.h
      src/AutotrackPrefsUI.h
      src/Info.h
#      src/ODAPI.h
)	
	
	
#set(OCPNSRC
#    ocpnsrc/cutil.cpp
#    ocpnsrc/TexFont.cpp
#)

#set(OCPNHDRS
#    ocpninclude/cutil.h
#    ocpninclude/pluginmanager.h
#    ocpninclude/shaders.h
#    ocpninclude/TexFont.h
#    ocpninclude/vector2D.h
#    ocpninclude/ocpn_plugin.h
#    libs/ocpn-api/ocpn_plugin.h
#)

#set(EXTSRC
#)

set(EXTINCLUDE
    extinclude/ODAPI.h
    extinclude/ODJSONSchemas.h
)

#set(LIBSSRC libs/wxJSON/src/jsonreader.cpp libs/wxJSON/src/jsonval.cpp libs/wxJSON/src/jsonwriter.cpp)

#set(LIBHDRS
#    libs/GL/gl.h
#    libs/GL/gl_private.h
#    libs/GL/glext.h
#    libs/GL/glu.h
#    libs/wxJSON/include/json_defs.h
#    libs/wxJSON/include/jsonreader.h
#    libs/wxJSON/include/jsonval.h
#    libs/wxJSON/include/jsonwriter.h)

add_definitions(-DPLUGIN_USE_SVG)

set(EXTINCLUDE_DIR ${EXTINCLUDE_DIR} extinclude libs/ocpn-api/)

#
# ----- If using JSON validation in plugin section below is needed ----- ##
#

# Not using JSON validation

#
# ----- If using JSON validation in plugin section above is needed ----- ##
#

include_directories(BEFORE ${PROJECT_SOURCE_DIR}/include)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/ocpninclude)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/${EXTINCLUDE_DIR})
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/serial)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/GL)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/glu/include)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/ocpn-api)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/wxJSON/include)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/jsoncpp)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/jsoncpp/include)

#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/plugingl/include)

#
# ----- Change below to match project requirements for android build ----- ##
#

if(QT_ANDROID)
    include_directories( ${PROJECT_SOURCE_DIR}/extinclude/GLES2)
endif(QT_ANDROID)

##
## ----- Change above to match project requirements for android build ----- ##
##


# ------------------------------------------------------------------------------------------
target_sources(${PACKAGE_NAME} PUBLIC ${SRCS} ${HDRS} ${OCPNSRC} ${EXTSRC} ${LIBSSRC})
# ------------------------------------------------------------------------------------------


if(NOT OCPN_FLATPAK_CONFIG)
    # Build environment not available when flatpak is being configured so following statements will not work
    message(STATUS "${CMLOC}Adding target link libraries to ${PACKAGE_NAME}")

	add_subdirectory(libs/jsoncpp)
    add_subdirectory(libs/nmea0183)
    add_subdirectory(libs/serial)	
    add_subdirectory(libs/tinyxml)
#	add_subdirectory(libs/plugingl)

    target_link_libraries(${PACKAGE_NAME} ocpn::jsoncpp)
    target_link_libraries(${PACKAGE_NAME} ocpn::nmea0183)
    target_link_libraries(${PACKAGE_NAME} ocpn::serial)
    target_link_libraries(${PACKAGE_NAME} ocpn::tinyxml)
#	target_link_libraries(${PACKAGE_NAME} ocpn::plugingl)
	
endif(NOT OCPN_FLATPAK_CONFIG)

#
# ----- If using JSON validation in plugin section below is needed ----- ##
#

# Not using JSON Validation

add_definitions(-DTIXML_USE_STL)

##------- Change below to match project requirements --------------


##
## ----- do not change next section - needed to configure build process ----- ##
##

# Needed for android builds
if(QT_ANDROID)
    include_directories(BEFORE ${qt_android_include})
endif(QT_ANDROID)

# Needed for all builds
include("PluginInstall")
include("PluginLocalization")
include("PluginPackage")

##
## ----- do not change section above - needed to configure build process ----- ##
##
message(STATUS "${CMLOC}include directories: ")
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
    message(STATUS "${CMLOC}dir='${dir}'")
endforeach()
