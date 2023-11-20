REM ..\..\..\OpenCPN\cache\wx-config.bat
del C:\ProgramData\opencpn\opencpn.log
set "wxWidgets_ROOT_DIR=C:\OpenCPN\buildwin\..\cache\wxWidgets-3.2.2" 
set "wxWidgets_LIB_DIR=C:\OpenCPN\buildwin\..\cache\wxWidgets-3.2.2\lib\vc14x_dll" 
set "WXWIN=%wxWidgets_ROOT_DIR%"

if %1%==c cmake -A Win32 -G "Visual Studio 17 2022" -DCMAKE_GENERATOR_PLATFORM=Win32 ..

cmake --build . --config Release
copy C:\radar\AutoTrackRaymarine_pi\build\Release\autotrackraymarine_pi.dll C:\Users\"Douwe Fokkema"\AppData\Local\opencpn\plugins

