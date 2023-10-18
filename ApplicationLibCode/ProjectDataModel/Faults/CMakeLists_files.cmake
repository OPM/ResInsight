set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultInView.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultInViewCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationModel.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationModelCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationTools.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationDataAccess.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationEnums.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultInView.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultInViewCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationModelCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationDataAccess.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

source_group(
  "ProjectDataModel\\Faults"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake
)
