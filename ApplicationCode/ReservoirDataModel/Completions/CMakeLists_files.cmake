
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RigCompletionData.h
${CMAKE_CURRENT_LIST_DIR}/RigCompletionDataGridCell.h
${CMAKE_CURRENT_LIST_DIR}/RigEclipseToStimPlanCellTransmissibilityCalculator.h
${CMAKE_CURRENT_LIST_DIR}/RigTransmissibilityCondenser.h
${CMAKE_CURRENT_LIST_DIR}/RigFractureTransmissibilityEquations.h
${CMAKE_CURRENT_LIST_DIR}/RigWellPathStimplanIntersector.h
${CMAKE_CURRENT_LIST_DIR}/RigVirtualPerforationTransmissibilities.h
${CMAKE_CURRENT_LIST_DIR}/RigEclipseToStimPlanCalculator.h
)


set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RigCompletionData.cpp
${CMAKE_CURRENT_LIST_DIR}/RigCompletionDataGridCell.cpp
${CMAKE_CURRENT_LIST_DIR}/RigEclipseToStimPlanCellTransmissibilityCalculator.cpp
${CMAKE_CURRENT_LIST_DIR}/RigTransmissibilityCondenser.cpp
${CMAKE_CURRENT_LIST_DIR}/RigFractureTransmissibilityEquations.cpp
${CMAKE_CURRENT_LIST_DIR}/RigWellPathStimplanIntersector.cpp
${CMAKE_CURRENT_LIST_DIR}/RigVirtualPerforationTransmissibilities.cpp
${CMAKE_CURRENT_LIST_DIR}/RigEclipseToStimPlanCalculator.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ReservoirDataModel\\Completions" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
