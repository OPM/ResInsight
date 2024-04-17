
set(RESINSIGHT_MAJOR_VERSION 2024)
set(RESINSIGHT_MINOR_VERSION 03)
set(RESINSIGHT_PATCH_VERSION 1)

# Opional text with no restrictions
set(RESINSIGHT_VERSION_TEXT "-dev")
#set(RESINSIGHT_VERSION_TEXT "-RC_05")

# Optional text
# Must be unique and increasing within one combination of major/minor/patch version 
# The uniqueness of this text is independent of RESINSIGHT_VERSION_TEXT 
# Format of text must be ".xx"
set(RESINSIGHT_DEV_VERSION ".05")

# https://github.com/CRAVA/crava/tree/master/libs/nrlib
set(NRLIB_GITHUB_SHA "ba35d4359882f1c6f5e9dc30eb95fe52af50fd6f") 

# https://github.com/Statoil/libecl
# Note:
# Apply patches fix-synthetic-odb-cases.patch and install-ert.patch after update
set(ECL_GITHUB_SHA "0e1e780fd6f18ce93119061e36a4fca9711bc020")

# https://github.com/OPM/opm-flowdiagnostics
set(OPM_FLOWDIAGNOSTICS_SHA "8bb60d6111063f2b7557502ecaa329a2d5c13b41")

# https://github.com/OPM/opm-flowdiagnostics-applications
set(OPM_FLOWDIAGNOSTICS_APPLICATIONS_SHA "f57942a8cdf57422fabf3a4423d02a3e46e0be4e")

# https://github.com/OPM/opm-parser/blob/master/opm/parser/eclipse/Units/Units.hpp
# This file was moved from opm-core to opm-parser october 2016
# sha for Units.hpp 9a679071dd0066236154852c39a9e0b3c3ac4873

set(STRPRODUCTVER ${RESINSIGHT_MAJOR_VERSION}.${RESINSIGHT_MINOR_VERSION}.${RESINSIGHT_PATCH_VERSION}${RESINSIGHT_VERSION_TEXT}${RESINSIGHT_DEV_VERSION})

