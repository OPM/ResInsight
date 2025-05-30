project(cafUserInterface)

# Unity Build
if(CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

if(MSVC)
  # Define this one to tell windows.h to not define min() and max() as macros
  add_definitions(-DNOMINMAX)
endif(MSVC)

find_package(
  Qt6
  COMPONENTS
  REQUIRED Core Gui Widgets Svg
)
set(QT_LIBRARIES Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Svg)
qt_standard_project_setup()

set(PROJECT_FILES
    # field editors
    cafPdmUiCheckBoxDelegate.cpp
    cafPdmUiCheckBoxDelegate.h
    cafPdmUiCheckBoxEditor.cpp
    cafPdmUiCheckBoxEditor.h
    cafPdmUiCheckBoxTristateEditor.cpp
    cafPdmUiCheckBoxTristateEditor.h
    cafPdmUiColorEditor.cpp
    cafPdmUiColorEditor.h
    cafPdmUiComboBoxEditor.cpp
    cafPdmUiComboBoxEditor.h
    cafPdmUiDateEditor.cpp
    cafPdmUiDateEditor.h
    cafPdmUiTimeEditor.cpp
    cafPdmUiTimeEditor.h
    cafPdmUiDoubleSliderEditor.cpp
    cafPdmUiDoubleSliderEditor.h
    cafPdmUiDragDropInterface.h
    cafPdmUiFilePathEditor.cpp
    cafPdmUiFilePathEditor.h
    cafPdmUiLineEditor.cpp
    cafPdmUiLineEditor.h
    cafPdmUiListEditor.cpp
    cafPdmUiListEditor.h
    cafPdmUiPushButtonEditor.cpp
    cafPdmUiPushButtonEditor.h
    cafPdmUiActionPushButtonEditor.cpp
    cafPdmUiActionPushButtonEditor.h
    cafPdmUiSliderEditor.cpp
    cafPdmUiSliderEditor.h
    cafPdmUiTextEditor.cpp
    cafPdmUiTextEditor.h
    cafPdmUiToolBarEditor.cpp
    cafPdmUiToolBarEditor.h
    cafPdmUiToolButtonEditor.cpp
    cafPdmUiToolButtonEditor.h
    cafPdmUiToolButtonCallbackEditor.cpp
    cafPdmUiToolButtonCallbackEditor.h
    cafPdmUiTreeSelectionEditor.h
    cafPdmUiTreeSelectionEditor.cpp
    cafPdmUiTreeSelectionQModel.h
    cafPdmUiTreeSelectionQModel.cpp
    cafPdmUiFieldEditorHelper.h
    cafPdmUiFieldEditorHelper.cpp
    cafPdmUiLabelEditor.cpp
    cafPdmUiCheckBoxAndTextEditor.h
    cafPdmUiCheckBoxAndTextEditor.cpp
    cafPdmUiValueRangeEditor.h
    cafPdmUiValueRangeEditor.cpp
    # object editors
    cafPdmUiDefaultObjectEditor.cpp
    cafPdmUiDefaultObjectEditor.h
    cafPdmUiListView.cpp
    cafPdmUiListView.h
    cafPdmUiListViewEditor.cpp
    cafPdmUiListViewEditor.h
    cafPdmUiTableRowEditor.cpp
    cafPdmUiTableRowEditor.h
    cafPdmUiTableView.cpp
    cafPdmUiTableView.h
    cafPdmUiTableViewDelegate.cpp
    cafPdmUiTableViewDelegate.h
    cafPdmUiTableViewEditor.cpp
    cafPdmUiTableViewEditor.h
    cafPdmUiTableViewQModel.cpp
    cafPdmUiTableViewQModel.h
    cafPdmUiTreeEditorHandle.cpp
    cafPdmUiTreeEditorHandle.h
    cafPdmUiTreeItemEditor.cpp
    cafPdmUiTreeItemEditor.h
    cafPdmUiTreeView.cpp
    cafPdmUiTreeView.h
    cafPdmUiTreeViewEditor.cpp
    cafPdmUiTreeViewEditor.h
    cafPdmUiTreeViewQModel.cpp
    cafPdmUiTreeViewQModel.h
    cafPdmUiPropertyView.cpp
    cafPdmUiPropertyView.h
    cafPdmUiPropertyViewDialog.cpp
    cafPdmUiPropertyViewDialog.h
    cafPdmUiFormLayoutObjectEditor.cpp
    cafPdmUiFormLayoutObjectEditor.h
    cafPdmUiDoubleValueEditor.cpp
    cafPdmUiDoubleValueEditor.h
    # div
    cafAboutDialog.cpp
    cafAboutDialog.h
    cafProgressInfo.cpp
    cafProgressInfo.h
    cafUiProcess.cpp
    cafUiProcess.h
    QMinimizePanel.cpp
    QMinimizePanel.h
    cafQStyledProgressBar.cpp
    cafQStyledProgressBar.h
    cafQTreeViewStateSerializer.h
    cafQTreeViewStateSerializer.cpp
    cafMemoryInspector.h
    cafMemoryInspector.cpp
    cafPdmUniqueIdValidator.cpp
    cafPdmDoubleStringValidator.cpp
    cafPickEventHandler.h
    cafPdmUiPickableLineEditor.cpp
    cafStyleSheetTools.h
    cafStyleSheetTools.cpp
    cafPdmUiTabbedPropertyViewDialog.h
    cafPdmUiTabbedPropertyViewDialog.cpp
    cafPdmUiTreeViewItemDelegate.h
    cafPdmUiTreeViewItemDelegate.cpp
    cafPdmUiTreeAttributes.h
    cafPdmUiTreeAttributes.cpp
    cafUiAppearanceSettings.cpp
    cafUiIconFactory.cpp
    cafPdmUiSliderTools.h
    cafPdmUiSliderTools.cpp
    cafPdmUiFieldLabelEditorHandle.h
    cafPdmUiFieldLabelEditorHandle.cpp
)

add_library(${PROJECT_NAME} ${PROJECT_FILES} ${QRC_FILES_CPP})

if(MSVC)
  set_target_properties(
    ${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127 /wd4996"
  )
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  ${PROJECT_NAME} cafProjectDataModel cafPdmScripting ${QT_LIBRARIES}
)
