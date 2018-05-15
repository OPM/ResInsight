
set(RESINSIGHT_MAJOR_VERSION 2018)
set(RESINSIGHT_MINOR_VERSION 01)
set(RESINSIGHT_PATCH_VERSION 1)

# Opional text with no restrictions
set(RESINSIGHT_VERSION_TEXT "-devRC")

# Optional text
# Must be unique and increasing within one combination of major/minor/patch version 
# The uniqueness of this text is independent of RESINSIGHT_VERSION_TEXT 
# Format of text must be ".xx"
set(RESINSIGHT_DEV_VERSION ".110")

# https://github.com/CRAVA/crava/tree/master/libs/nrlib
set(NRLIB_GITHUB_SHA "ba35d4359882f1c6f5e9dc30eb95fe52af50fd6f") 

# https://github.com/Statoil/libecl
set(ECL_GITHUB_SHA "0188b08081eb1ac4ade89ac224b8128b4c9b0481")

# https://github.com/OPM/opm-flowdiagnostics
set(OPM_FLOWDIAGNOSTICS_SHA "f8af0914f8b1ddcda41f040f539c945a6057f5e4")

# https://github.com/OPM/opm-flowdiagnostics-applications
set(OPM_FLOWDIAGNOSTICS_APPLICATIONS_SHA "e769c492ccd3fc4e1f834ed60f4f9279ba8524bc")

# https://github.com/OPM/opm-parser/blob/master/opm/parser/eclipse/Units/Units.hpp
# This file was moved from opm-core to opm-parser october 2016
# sha for Units.hpp 9a679071dd0066236154852c39a9e0b3c3ac4873

set(STRPRODUCTVER ${RESINSIGHT_MAJOR_VERSION}.${RESINSIGHT_MINOR_VERSION}.${RESINSIGHT_PATCH_VERSION}${RESINSIGHT_VERSION_TEXT}${RESINSIGHT_DEV_VERSION})

