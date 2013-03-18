
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()


list(APPEND CODE_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RigActiveCellInfo.h
${CEE_CURRENT_LIST_DIR}RigCell.h
${CEE_CURRENT_LIST_DIR}RigEclipseCase.h
${CEE_CURRENT_LIST_DIR}RigGridBase.h
${CEE_CURRENT_LIST_DIR}RigGridCollection.h
${CEE_CURRENT_LIST_DIR}RigGridScalarDataAccess.h
${CEE_CURRENT_LIST_DIR}RigLocalGrid.h
${CEE_CURRENT_LIST_DIR}RigMainGrid.h
${CEE_CURRENT_LIST_DIR}RigReservoirBuilderMock.h
${CEE_CURRENT_LIST_DIR}RigReservoirCellResults.h
${CEE_CURRENT_LIST_DIR}RigWellResults.h
)

list(APPEND CODE_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RigActiveCellInfo.cpp
${CEE_CURRENT_LIST_DIR}RigCell.cpp
${CEE_CURRENT_LIST_DIR}RigEclipseCase.cpp
${CEE_CURRENT_LIST_DIR}RigGridBase.cpp
${CEE_CURRENT_LIST_DIR}RigGridCollection.cpp
${CEE_CURRENT_LIST_DIR}RigGridScalarDataAccess.cpp
${CEE_CURRENT_LIST_DIR}RigLocalGrid.cpp
${CEE_CURRENT_LIST_DIR}RigMainGrid.cpp
${CEE_CURRENT_LIST_DIR}RigReservoirBuilderMock.cpp
${CEE_CURRENT_LIST_DIR}RigReservoirCellResults.cpp
${CEE_CURRENT_LIST_DIR}RigWellResults.cpp
)


