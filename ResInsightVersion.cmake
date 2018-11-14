
set(RESINSIGHT_MAJOR_VERSION 2018)
set(RESINSIGHT_MINOR_VERSION 05)
set(RESINSIGHT_PATCH_VERSION 1)

# Opional text with no restrictions
set(RESINSIGHT_VERSION_TEXT "-dev")

# Optional text
# Must be unique and increasing within one combination of major/minor/patch version 
# The uniqueness of this text is independent of RESINSIGHT_VERSION_TEXT 
# Format of text must be ".xx"
set(RESINSIGHT_DEV_VERSION ".12")

# https://github.com/CRAVA/crava/tree/master/libs/nrlib
set(NRLIB_GITHUB_SHA "ba35d4359882f1c6f5e9dc30eb95fe52af50fd6f") 

# https://github.com/Statoil/libecl
# Note:
# Apply patches fix-synthetic-odb-cases.patch and install-ert.patch after update
set(ECL_GITHUB_SHA "7f93730c08a4d981a4b738b42146d099977572ce")

# https://github.com/OPM/opm-flowdiagnostics
set(OPM_FLOWDIAGNOSTICS_SHA "f8af0914f8b1ddcda41f040f539c945a6057f5e4")

# https://github.com/OPM/opm-flowdiagnostics-applications
set(OPM_FLOWDIAGNOSTICS_APPLICATIONS_SHA "24ff768dc509b6c6bbd0121ef46a5932fae92961")

# https://github.com/OPM/opm-parser/blob/master/opm/parser/eclipse/Units/Units.hpp
# This file was moved from opm-core to opm-parser october 2016
# sha for Units.hpp 9a679071dd0066236154852c39a9e0b3c3ac4873

set(STRPRODUCTVER ${RESINSIGHT_MAJOR_VERSION}.${RESINSIGHT_MINOR_VERSION}.${RESINSIGHT_PATCH_VERSION}${RESINSIGHT_VERSION_TEXT}${RESINSIGHT_DEV_VERSION})

