REM Remove the current opencpn-libs submodule linkage and add new linkage
git submodule deinit -f opencpn-libs
git rm --cached opencpn-libs
rmdir /S .\.git\modules\opencpn-libs 
rmdir /S .\opencpn-libs
git config -f .gitmodules --remove-section submodule.opencpn-libs  
git add .gitmodules
git commit -m "Remove opencpn-libs submodule."
REM opencpn-libs linkage and files removed and  commit made

REM Add the new linkage
git submodule add https://github.com/OpenCPN/opencpn-libs.git
git commit -m "Adding revised opencpn-libs submodule main"
REM Added new linkage to module opencpn-libs main and made commit.
git log --oneline

