
set(RESINSIGHT_MAJOR_VERSION 2018)
set(RESINSIGHT_MINOR_VERSION 01)
set(RESINSIGHT_PATCH_VERSION 00)

# Opional text with no restrictions
#set(RESINSIGHT_VERSION_TEXT "-dev")

# Optional text
# Must be unique and increasing within one combination of major/minor/patch version 
# The uniqueness of this text is independent of RESINSIGHT_VERSION_TEXT 
# Format of text must be ".xx"
#set(RESINSIGHT_DEV_VERSION ".16")

# https://github.com/CRAVA/crava/tree/master/libs/nrlib
set(NRLIB_GITHUB_SHA "ba35d4359882f1c6f5e9dc30eb95fe52af50fd6f") 

# https://github.com/Statoil/libecl
set(ERT_GITHUB_SHA "2e36798b43daf18c112b91aa3febbf2fccd4a95f") 

# https://github.com/OPM/opm-flowdiagnostics
set(OPM_FLOWDIAGNOSTICS_SHA "7e2be931d430796ed42efcfb5c6b67a8d5962f7f")

# https://github.com/OPM/opm-flowdiagnostics-applications
set(OPM_FLOWDIAGNOSTICS_APPLICATIONS_SHA "44f7e47ecdc87ba566ab4146629de49039a73b2e")

# https://github.com/OPM/opm-parser/blob/master/opm/parser/eclipse/Units/Units.hpp
# This file was moved from opm-core to opm-parser october 2016
# sha for Units.hpp 9a679071dd0066236154852c39a9e0b3c3ac4873

set(STRPRODUCTVER ${RESINSIGHT_MAJOR_VERSION}.${RESINSIGHT_MINOR_VERSION}.${RESINSIGHT_PATCH_VERSION}${RESINSIGHT_VERSION_TEXT}${RESINSIGHT_DEV_VERSION})

