
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
    ${CEE_CURRENT_LIST_DIR}RigGeoMechWellLogExtractor.h
    ${CEE_CURRENT_LIST_DIR}RigCaseToCaseCellMapper.h
    ${CEE_CURRENT_LIST_DIR}RigCaseToCaseCellMapperTools.h
    ${CEE_CURRENT_LIST_DIR}RigCaseToCaseRangeFilterMapper.h
    ${CEE_CURRENT_LIST_DIR}RigSimulationWellCenterLineCalculator.h
    ${CEE_CURRENT_LIST_DIR}RigWellLogFile.h
    ${CEE_CURRENT_LIST_DIR}RigReservoirGridTools.h
)

set (SOURCE_GROUP_SOURCE_FILES
    ${CEE_CURRENT_LIST_DIR}RigGeoMechWellLogExtractor.cpp
    ${CEE_CURRENT_LIST_DIR}RigCaseToCaseCellMapper.cpp
    ${CEE_CURRENT_LIST_DIR}RigCaseToCaseCellMapperTools.cpp
    ${CEE_CURRENT_LIST_DIR}RigCaseToCaseRangeFilterMapper.cpp
    ${CEE_CURRENT_LIST_DIR}RigSimulationWellCenterLineCalculator.cpp
    ${CEE_CURRENT_LIST_DIR}RigWellLogFile.cpp
    ${CEE_CURRENT_LIST_DIR}RigReservoirGridTools.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ReservoirDataModel2" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_filesNotToUnitTest.cmake )
