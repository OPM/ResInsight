
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicLaunchUnitTestsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicShowPlotWindowFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicShowMainWindowFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicTileWindowsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicOpenProjectFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicOpenLastUsedFileFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicSaveProjectFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicSaveProjectAsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicExitApplicationFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicCloseProjectFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicHelpFeatures.h
${CMAKE_CURRENT_LIST_DIR}/RicEditPreferencesFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicShowPlotDataFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicLaunchRegressionTestsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicRunCommandFileFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicLaunchUnitTestsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicShowPlotWindowFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicShowMainWindowFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicTileWindowsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicOpenProjectFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicOpenLastUsedFileFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSaveProjectFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSaveProjectAsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExitApplicationFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCloseProjectFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicHelpFeatures.cpp
${CMAKE_CURRENT_LIST_DIR}/RicEditPreferencesFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicShowPlotDataFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicLaunchRegressionTestsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicRunCommandFileFeature.cpp
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


source_group( "CommandFeature\\Application" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
