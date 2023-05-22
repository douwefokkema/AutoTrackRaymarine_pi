
    execute_process(
      COMMAND
        flatpak-builder --force-clean --keep-build-dirs
          D:/Radar/AutoTrackRaymarine_pi/app D:/Radar/AutoTrackRaymarine_pi/org.opencpn.OpenCPN.Plugin.radar.yaml
    )
    # Copy the data out of the sandbox to installation directory
    execute_process(
      WORKING_DIRECTORY D:/Radar/AutoTrackRaymarine_pi
      COMMAND
        flatpak-builder --run app D:/Radar/AutoTrackRaymarine_pi/org.opencpn.OpenCPN.Plugin.radar.yaml
           bash copy_out libautotrackraymarine_pi.so D:/Radar/AutoTrackRaymarine_pi
    )
    if (NOT EXISTS app/files/lib/opencpn/libautotrackraymarine_pi.so)
      message(FATAL_ERROR "Cannot find generated file libautotrackraymarine_pi.so")
    endif ()
    execute_process(
      COMMAND bash -c "sed -e '/@checksum@/d'           < Autotrackraymarine-5.5-msvc-wx32-10.0.22621.xml.in > app/files/metadata.xml"
    )
    if (Debug MATCHES Release|MinSizeRel)
      message(STATUS "Stripping app/files/lib/opencpn/libautotrackraymarine_pi.so")
      execute_process(
        COMMAND strip app/files/lib/opencpn/libautotrackraymarine_pi.so
      )
    endif ()
    execute_process(
      WORKING_DIRECTORY D:/Radar/AutoTrackRaymarine_pi/app
      COMMAND mv -fT files Autotrackraymarine-5.5-msvc-wx32-10.0.22621
    )
    execute_process(
      WORKING_DIRECTORY  D:/Radar/AutoTrackRaymarine_pi/app
      COMMAND
        cmake -E
        tar -czf ../Autotrackraymarine-5.5.1-beta+2305221233.b7f6a02_msvc-wx32-10.0.22621-win32.tar.gz --format=gnutar Autotrackraymarine-5.5-msvc-wx32-10.0.22621
    )
    message(STATUS "Building Autotrackraymarine-5.5.1-beta+2305221233.b7f6a02_msvc-wx32-10.0.22621-win32.tar.gz")
    execute_process(
      COMMAND cmake -P D:/Radar/AutoTrackRaymarine_pi/checksum.cmake
    )
    message(STATUS "Computing checksum in Autotrackraymarine-5.5-msvc-wx32-10.0.22621.xml")
  