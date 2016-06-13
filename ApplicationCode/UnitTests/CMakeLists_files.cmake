
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}cvfGeometryTools-Test.cpp
${CEE_CURRENT_LIST_DIR}Ert-Test.cpp
${CEE_CURRENT_LIST_DIR}RifEclipseInputFileTools-Test.cpp
${CEE_CURRENT_LIST_DIR}RifReaderEclipseOutput-Test.cpp
${CEE_CURRENT_LIST_DIR}RifReaderEclipseSummary-Test.cpp
${CEE_CURRENT_LIST_DIR}RigActiveCellInfo-Test.cpp
${CEE_CURRENT_LIST_DIR}RigReservoir-Test.cpp
${CEE_CURRENT_LIST_DIR}RigStatisticsMath-Test.cpp
${CEE_CURRENT_LIST_DIR}RimWellLogExtractionCurveImpl-Test.cpp
${CEE_CURRENT_LIST_DIR}RivPipeGeometryGenerator-Test.cpp
${CEE_CURRENT_LIST_DIR}RivTernaryScalarMapper-Test.cpp
${CEE_CURRENT_LIST_DIR}ScalarMapper-Test.cpp
${CEE_CURRENT_LIST_DIR}WellPathAsciiFileReader-Test.cpp
${CEE_CURRENT_LIST_DIR}opm-parser-Test.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "UnitTests" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
