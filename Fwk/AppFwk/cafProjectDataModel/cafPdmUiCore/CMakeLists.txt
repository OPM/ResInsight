project(cafPdmUiCore)

# Unity Build
if(CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

find_package(
  Qt6
  COMPONENTS
  REQUIRED Core Gui Widgets
)
set(QT_LIBRARIES Qt6::Core Qt6::Gui Qt6::Widgets)
qt_standard_project_setup()

set(PROJECT_FILES
    cafInternalPdmFieldTypeSpecializations.h
    cafInternalPdmUiFieldCapability.h
    cafInternalPdmUiFieldCapability.inl
    cafInternalPdmUiCommandSystemInterface.h
    cafPdmUiEditorHandle.cpp
    cafPdmUiEditorHandle.h
    cafPdmUiFieldEditorHandle.cpp
    cafPdmUiFieldEditorHandle.h
    cafPdmUiFieldHandle.cpp
    cafPdmUiFieldHandle.h
    cafPdmUiFieldSpecialization.h
    cafPdmUiItem.cpp
    cafPdmUiItem.h
    cafPdmUiModelChangeDetector.cpp
    cafPdmUiModelChangeDetector.h
    cafPdmUiObjectEditorHandle.cpp
    cafPdmUiObjectEditorHandle.h
    cafPdmUiWidgetObjectEditorHandle.cpp
    cafPdmUiWidgetObjectEditorHandle.h
    cafPdmUi3dObjectEditorHandle.cpp
    cafPdmUi3dObjectEditorHandle.h
    cafPdmUiObjectHandle.cpp
    cafPdmUiObjectHandle.h
    cafPdmUiOrdering.cpp
    cafPdmUiOrdering.h
    cafPdmUiCommandSystemProxy.cpp
    cafPdmUiCommandSystemProxy.h
    cafPdmUiTreeOrdering.cpp
    cafPdmUiTreeOrdering.h
    cafUiTreeItem.h
    cafPdmUiGroup.cpp
    cafPdmUiGroup.h
    cafPdmUiLabel.cpp
    cafPdmUiLabel.h
    cafPdmUiButton.cpp
    cafPdmUiButton.h
    cafSelectionManager.cpp
    cafSelectionManager.h
    cafSelectionChangedReceiver.h
    cafSelectionChangedReceiver.cpp
    cafSelectionManagerTools.h
    cafPdmUiSelection3dEditorVisualizer.h
    cafPdmUiSelection3dEditorVisualizer.cpp
    cafQShortenedLabel.cpp
    cafQShortenedLabel.h
    cafIconProvider.cpp
    cafIconProvider.h
    cafFontTools.cpp
    cafFontTools.h
    caf.h
    caf.cpp
    cafScheduler.h
    cafScheduler.cpp
    cafUpdateEditorsScheduler.h
    cafUpdateEditorsScheduler.cpp
)

# NOTE! Resources in this subfolder appends to the variable QRC_FILES in parent
# scope CMakeList.txt in the application folder (parent scope) must use the
# following syntax to make sure the QRC_FILES variable contains appended files
# in subfolders

# set( QRC_FILES ${QRC_FILES} Resources/MyaApplication.qrc )

set(QRC_FILES
    ${QRC_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/Resources/caf.qrc
    PARENT_SCOPE
)

add_library(${PROJECT_NAME} ${PROJECT_FILES} ${MOC_SOURCE_FILES})

target_link_libraries(
  ${PROJECT_NAME} cafPdmCore cafProjectDataModel ${QT_LIBRARIES}
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
  set_target_properties(
    ${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127"
  )
endif()
