
    execute_process(
      COMMAND cmake -E rm -rf app/Autotrackraymarine-5.5-msvc-wx32-10.0.22621
    )
    execute_process(
      COMMAND cmake -E rename app/files app/Autotrackraymarine-5.5-msvc-wx32-10.0.22621
    )
    execute_process(
      WORKING_DIRECTORY D:/Radar/AutoTrackRaymarine_pi/app
      COMMAND
        cmake -E
        tar -czf ../Autotrackraymarine-5.5.1-beta+2305221233.b7f6a02_msvc-wx32-10.0.22621-win32.tar.gz --format=gnutar Autotrackraymarine-5.5-msvc-wx32-10.0.22621
    )
    message(STATUS "Creating tarball Autotrackraymarine-5.5.1-beta+2305221233.b7f6a02_msvc-wx32-10.0.22621-win32.tar.gz")

    execute_process(COMMAND cmake -P D:/Radar/AutoTrackRaymarine_pi/checksum.cmake)
    message(STATUS "Computing checksum in Autotrackraymarine-5.5-msvc-wx32-10.0.22621.xml")
  