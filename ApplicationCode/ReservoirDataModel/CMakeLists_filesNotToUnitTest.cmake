
set (SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RigGeoMechWellLogExtractor.h
    ${CMAKE_CURRENT_LIST_DIR}/RigCaseToCaseCellMapper.h
    ${CMAKE_CURRENT_LIST_DIR}/RigCaseToCaseCellMapperTools.h
    ${CMAKE_CURRENT_LIST_DIR}/RigCaseToCaseRangeFilterMapper.h
    ${CMAKE_CURRENT_LIST_DIR}/RigSimulationWellCenterLineCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RigWellLogFile.h
    ${CMAKE_CURRENT_LIST_DIR}/RigReservoirGridTools.h
)

set (SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RigGeoMechWellLogExtractor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigCaseToCaseCellMapper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigCaseToCaseCellMapperTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigCaseToCaseRangeFilterMapper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigSimulationWellCenterLineCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigWellLogFile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigReservoirGridTools.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ReservoirDataModel2" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_filesNotToUnitTest.cmake )
