
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicWellPathDeleteFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicImportWellPaths.h
${CMAKE_CURRENT_LIST_DIR}/RicNewEditableWellPathFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicShowWellPlanFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewWellPathListTargetFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewWellPathAttributeFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicDeleteWellPathTargetFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicDeleteWellPathAttributeFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicWellPathsUnitSystemSettingsImpl.h
${CMAKE_CURRENT_LIST_DIR}/RicWellPathsUnitSystemSettingsUi.h
${CMAKE_CURRENT_LIST_DIR}/RicWellPathPickEventHandler.h 
${CMAKE_CURRENT_LIST_DIR}/RicCreateWellTargetsPickEventHandler.h
${CMAKE_CURRENT_LIST_DIR}/RicIntersectionPickEventHandler.h
${CMAKE_CURRENT_LIST_DIR}/RicWellPathFormationsImportFileFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicPolylineTargetsPickEventHandler.h
${CMAKE_CURRENT_LIST_DIR}/RicNewPolylineTargetFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicDeletePolylineTargetFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicImportWellMeasurementsFeature.h
${CMAKE_CURRENT_LIST_DIR}/PointTangentManipulator/Ric3dObjectEditorHandle.h
${CMAKE_CURRENT_LIST_DIR}/PointTangentManipulator/RicPointTangentManipulator.h
${CMAKE_CURRENT_LIST_DIR}/PointTangentManipulator/RicWellTarget3dEditor.h
${CMAKE_CURRENT_LIST_DIR}/PointTangentManipulator/RicWellPathGeometry3dEditor.h
${CMAKE_CURRENT_LIST_DIR}/PointTangentManipulator/RicPointTangentManipulatorPartMgr.h
${CMAKE_CURRENT_LIST_DIR}/PointTangentManipulator/RicPolyline3dEditor.h
${CMAKE_CURRENT_LIST_DIR}/PointTangentManipulator/RicPolylineTarget3dEditor.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicWellPathDeleteFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicImportWellPaths.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewEditableWellPathFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicShowWellPlanFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewWellPathListTargetFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewWellPathAttributeFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicDeleteWellPathTargetFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicDeleteWellPathAttributeFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellPathsUnitSystemSettingsImpl.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellPathsUnitSystemSettingsUi.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellPathPickEventHandler.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCreateWellTargetsPickEventHandler.cpp
${CMAKE_CURRENT_LIST_DIR}/RicIntersectionPickEventHandler.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellPathFormationsImportFileFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicPolylineTargetsPickEventHandler.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewPolylineTargetFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicDeletePolylineTargetFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicImportWellMeasurementsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/PointTangentManipulator/Ric3dObjectEditorHandle.cpp
${CMAKE_CURRENT_LIST_DIR}/PointTangentManipulator/RicPointTangentManipulator.cpp
${CMAKE_CURRENT_LIST_DIR}/PointTangentManipulator/RicWellTarget3dEditor.cpp
${CMAKE_CURRENT_LIST_DIR}/PointTangentManipulator/RicWellPathGeometry3dEditor.cpp
${CMAKE_CURRENT_LIST_DIR}/PointTangentManipulator/RicPointTangentManipulatorPartMgr.cpp
${CMAKE_CURRENT_LIST_DIR}/PointTangentManipulator/RicPolyline3dEditor.cpp
${CMAKE_CURRENT_LIST_DIR}/PointTangentManipulator/RicPolylineTarget3dEditor.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

list(APPEND QT_MOC_HEADERS
${CMAKE_CURRENT_LIST_DIR}/PointTangentManipulator/RicPointTangentManipulator.h
${CMAKE_CURRENT_LIST_DIR}/PointTangentManipulator/RicWellTarget3dEditor.h
${CMAKE_CURRENT_LIST_DIR}/PointTangentManipulator/RicWellPathGeometry3dEditor.h
${CMAKE_CURRENT_LIST_DIR}/PointTangentManipulator/RicPolyline3dEditor.h
${CMAKE_CURRENT_LIST_DIR}/PointTangentManipulator/RicPolylineTarget3dEditor.h
)

source_group( "CommandFeature\\WellPath" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
