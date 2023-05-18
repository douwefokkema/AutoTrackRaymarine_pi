
  execute_process(
    COMMAND  cmake -E sha256sum C:/radar/radar_pi/Radar-v5.5.0-beta_msvc-wx32-10-win32.tar.gz
    OUTPUT_FILE C:/radar/radar_pi/Radar-v5.5.0-beta_msvc-wx32-10-win32.sha256
  )
  file(READ C:/radar/radar_pi/Radar-v5.5.0-beta_msvc-wx32-10-win32.sha256 _SHA256)
  string(REGEX MATCH "^[^ ]*" checksum ${_SHA256} )
  configure_file(
    C:/radar/radar_pi/Radar-v5.5.0-beta-msvc-wx32-10.xml.in
    C:/radar/radar_pi/Radar-v5.5.0-beta-msvc-wx32-10.xml
    @ONLY
  )
