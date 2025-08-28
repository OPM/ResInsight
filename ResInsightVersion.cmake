
set(RESINSIGHT_MAJOR_VERSION 2025)
set(RESINSIGHT_MINOR_VERSION 09)
set(RESINSIGHT_PATCH_VERSION 0)

# Opional text with no restrictions
#set(RESINSIGHT_VERSION_TEXT "-dev")
set(RESINSIGHT_VERSION_TEXT "-RC_1")

# Optional text
# Must be unique and increasing within one combination of major/minor/patch version 
# The uniqueness of this text is independent of RESINSIGHT_VERSION_TEXT 
# Format of text must be ".xx"
#set(RESINSIGHT_DEV_VERSION ".10")

set(STRPRODUCTVER ${RESINSIGHT_MAJOR_VERSION}.${RESINSIGHT_MINOR_VERSION}.${RESINSIGHT_PATCH_VERSION}${RESINSIGHT_VERSION_TEXT}${RESINSIGHT_DEV_VERSION})

