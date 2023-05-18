
    execute_process(
      COMMAND
        flatpak-builder --force-clean --keep-build-dirs
          C:/radar/radar_pi/app C:/radar/radar_pi/org.opencpn.OpenCPN.Plugin.radar.yaml
    )
    # Copy the data out of the sandbox to installation directory
    execute_process(
      WORKING_DIRECTORY C:/radar/radar_pi
      COMMAND
        flatpak-builder --run app C:/radar/radar_pi/org.opencpn.OpenCPN.Plugin.radar.yaml
           bash copy_out libradar_pi.so C:/radar/radar_pi
    )
    if (NOT EXISTS app/files/lib/opencpn/libradar_pi.so)
      message(FATAL_ERROR "Cannot find generated file libradar_pi.so")
    endif ()
    execute_process(
      COMMAND bash -c "sed -e '/@checksum@/d'           < Radar-v5.5.0-beta-msvc-wx32-10.xml.in > app/files/metadata.xml"
    )
    if (RelWithDebInfo MATCHES Release|MinSizeRel)
      message(STATUS "Stripping app/files/lib/opencpn/libradar_pi.so")
      execute_process(
        COMMAND strip app/files/lib/opencpn/libradar_pi.so
      )
    endif ()
    execute_process(
      WORKING_DIRECTORY C:/radar/radar_pi/app
      COMMAND mv -fT files Radar-v5.5.0-beta-msvc-wx32-10
    )
    execute_process(
      WORKING_DIRECTORY  C:/radar/radar_pi/app
      COMMAND
        cmake -E
        tar -czf ../Radar-v5.5.0-beta_msvc-wx32-10-win32.tar.gz --format=gnutar Radar-v5.5.0-beta-msvc-wx32-10
    )
    message(STATUS "Building Radar-v5.5.0-beta_msvc-wx32-10-win32.tar.gz")
    execute_process(
      COMMAND cmake -P C:/radar/radar_pi/checksum.cmake
    )
    message(STATUS "Computing checksum in Radar-v5.5.0-beta-msvc-wx32-10.xml")
  