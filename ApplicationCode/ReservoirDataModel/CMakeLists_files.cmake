
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RigActiveCellInfo.h
${CEE_CURRENT_LIST_DIR}RigCell.h
${CEE_CURRENT_LIST_DIR}RigCaseData.h
${CEE_CURRENT_LIST_DIR}RigGridBase.h
${CEE_CURRENT_LIST_DIR}RigGridManager.h
${CEE_CURRENT_LIST_DIR}RigGridScalarDataAccess.h
${CEE_CURRENT_LIST_DIR}RigLocalGrid.h
${CEE_CURRENT_LIST_DIR}RigMainGrid.h
${CEE_CURRENT_LIST_DIR}RigReservoirBuilderMock.h
${CEE_CURRENT_LIST_DIR}RigCaseCellResultsData.h
${CEE_CURRENT_LIST_DIR}RigSingleWellResultsData.h
${CEE_CURRENT_LIST_DIR}RigStatisticsMath.h
${CEE_CURRENT_LIST_DIR}RigWellPath.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RigActiveCellInfo.cpp
${CEE_CURRENT_LIST_DIR}RigCell.cpp
${CEE_CURRENT_LIST_DIR}RigCaseData.cpp
${CEE_CURRENT_LIST_DIR}RigGridBase.cpp
${CEE_CURRENT_LIST_DIR}RigGridManager.cpp
${CEE_CURRENT_LIST_DIR}RigGridScalarDataAccess.cpp
${CEE_CURRENT_LIST_DIR}RigLocalGrid.cpp
${CEE_CURRENT_LIST_DIR}RigMainGrid.cpp
${CEE_CURRENT_LIST_DIR}RigReservoirBuilderMock.cpp
${CEE_CURRENT_LIST_DIR}RigCaseCellResultsData.cpp
${CEE_CURRENT_LIST_DIR}RigSingleWellResultsData.cpp
${CEE_CURRENT_LIST_DIR}RigStatisticsMath.cpp
${CEE_CURRENT_LIST_DIR}RigWellPath.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ReservoirDataModel" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} )
