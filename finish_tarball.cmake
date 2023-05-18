
    execute_process(
      COMMAND cmake -E rm -rf app/Radar-v5.5.0-beta-msvc-wx32-10
    )
    execute_process(
      COMMAND cmake -E rename app/files app/Radar-v5.5.0-beta-msvc-wx32-10
    )
    execute_process(
      WORKING_DIRECTORY C:/radar/radar_pi/app
      COMMAND
        cmake -E
        tar -czf ../Radar-v5.5.0-beta_msvc-wx32-10-win32.tar.gz --format=gnutar Radar-v5.5.0-beta-msvc-wx32-10
    )
    message(STATUS "Creating tarball Radar-v5.5.0-beta_msvc-wx32-10-win32.tar.gz")

    execute_process(COMMAND cmake -P C:/radar/radar_pi/checksum.cmake)
    message(STATUS "Computing checksum in Radar-v5.5.0-beta-msvc-wx32-10.xml")
  