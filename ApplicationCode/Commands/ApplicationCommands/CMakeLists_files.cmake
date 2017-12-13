
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RicLaunchUnitTestsFeature.h
${CEE_CURRENT_LIST_DIR}RicShowPlotWindowFeature.h
${CEE_CURRENT_LIST_DIR}RicShowMainWindowFeature.h
${CEE_CURRENT_LIST_DIR}RicTileWindowsFeature.h
${CEE_CURRENT_LIST_DIR}RicOpenProjectFeature.h
${CEE_CURRENT_LIST_DIR}RicOpenLastUsedFileFeature.h
${CEE_CURRENT_LIST_DIR}RicSaveProjectFeature.h
${CEE_CURRENT_LIST_DIR}RicSaveProjectAsFeature.h
${CEE_CURRENT_LIST_DIR}RicExitApplicationFeature.h
${CEE_CURRENT_LIST_DIR}RicCloseProjectFeature.h
${CEE_CURRENT_LIST_DIR}RicHelpFeatures.h
${CEE_CURRENT_LIST_DIR}RicEditPreferencesFeature.h
${CEE_CURRENT_LIST_DIR}RicShowPlotDataFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RicLaunchUnitTestsFeature.cpp
${CEE_CURRENT_LIST_DIR}RicShowPlotWindowFeature.cpp
${CEE_CURRENT_LIST_DIR}RicShowMainWindowFeature.cpp
${CEE_CURRENT_LIST_DIR}RicTileWindowsFeature.cpp
${CEE_CURRENT_LIST_DIR}RicOpenProjectFeature.cpp
${CEE_CURRENT_LIST_DIR}RicOpenLastUsedFileFeature.cpp
${CEE_CURRENT_LIST_DIR}RicSaveProjectFeature.cpp
${CEE_CURRENT_LIST_DIR}RicSaveProjectAsFeature.cpp
${CEE_CURRENT_LIST_DIR}RicExitApplicationFeature.cpp
${CEE_CURRENT_LIST_DIR}RicCloseProjectFeature.cpp
${CEE_CURRENT_LIST_DIR}RicHelpFeatures.cpp
${CEE_CURRENT_LIST_DIR}RicEditPreferencesFeature.cpp
${CEE_CURRENT_LIST_DIR}RicShowPlotDataFeature.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

set (QT_MOC_HEADERS
${QT_MOC_HEADERS}
)


source_group( "CommandFeature\\Application" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
