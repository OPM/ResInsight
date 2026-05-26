set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimFileWellPath.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPointBasedWellPath.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimModeledWellPath.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellPath.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellPathAttribute.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellPathAttributeCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellPathCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellPathGeometryDef.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellPathGeometryDefTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellPathGroup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellPathTarget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellPathTieIn.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellIASettings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellIASettingsCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellIAModelBox.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellIAModelData.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellIADataAccess.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimOsduWellPath.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimOsduWellPathDataLoader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFileWellPathDataLoader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimModeledWellPathDataLoader.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
