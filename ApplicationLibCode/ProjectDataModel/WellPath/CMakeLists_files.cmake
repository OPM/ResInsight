set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimWellPath.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellPathGroup.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFileWellPath.h
    ${CMAKE_CURRENT_LIST_DIR}/RimModeledWellPath.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellPathGeometryDef.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellPathGeometryDefTools.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellPathAttribute.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellPathAttributeCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellPathCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellPathTarget.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellPathTieIn.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellIASettings.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellIASettingsCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellIAModelBox.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellIAModelData.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellIADataAccess.h
    ${CMAKE_CURRENT_LIST_DIR}/RimOsduWellPath.h
    ${CMAKE_CURRENT_LIST_DIR}/RimOsduWellPathDataLoader.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFileWellPathDataLoader.h
    ${CMAKE_CURRENT_LIST_DIR}/RimModeledWellPathDataLoader.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimFileWellPath.cpp
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

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

list(APPEND QT_MOC_HEADERS ${CMAKE_CURRENT_LIST_DIR}/RimOsduWellPathDataLoader.h)


source_group(
  "ProjectDataModel\\WellPath"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake
)
