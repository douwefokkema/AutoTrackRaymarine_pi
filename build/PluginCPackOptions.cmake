# ---------------------------------------------------------------------------
# Author:      Jon Gough Copyright:   2020 License:     GPLv3+
# ---------------------------------------------------------------------------

# This file contains changes needed during the make package process depending on the type of package being created

if(CPACK_GENERATOR MATCHES "DEB")
    set(CPACK_PACKAGE_FILE_NAME "AutoTrackRaymarine_pi-1.1.1.2-msvc-10.0.22000")
    if(CPACK_DEBIAN_PACKAGE_ARCHITECTURE MATCHES "x86_64")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE )
    endif()
else()
    set(CPACK_PACKAGE_FILE_NAME "AutoTrackRaymarine_pi-1.1.1.2-msvc-x86_64-10.0.22000")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE x86_64)
endif()
