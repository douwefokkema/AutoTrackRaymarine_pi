
  execute_process(
    COMMAND  cmake -E sha256sum D:/Radar/AutoTrackRaymarine_pi/Autotrackraymarine-5.5.1-beta+2305221233.b7f6a02_msvc-wx32-10.0.22621-win32.tar.gz
    OUTPUT_FILE D:/Radar/AutoTrackRaymarine_pi/Autotrackraymarine-5.5.1-beta+2305221233.b7f6a02_msvc-wx32-10.0.22621-win32.sha256
  )
  file(READ D:/Radar/AutoTrackRaymarine_pi/Autotrackraymarine-5.5.1-beta+2305221233.b7f6a02_msvc-wx32-10.0.22621-win32.sha256 _SHA256)
  string(REGEX MATCH "^[^ ]*" checksum ${_SHA256} )
  configure_file(
    D:/Radar/AutoTrackRaymarine_pi/Autotrackraymarine-5.5-msvc-wx32-10.0.22621.xml.in
    D:/Radar/AutoTrackRaymarine_pi/Autotrackraymarine-5.5-msvc-wx32-10.0.22621.xml
    @ONLY
  )
