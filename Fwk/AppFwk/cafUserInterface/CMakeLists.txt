cmake_minimum_required (VERSION 2.8.12)

project (cafUserInterface)

if (MSVC)
    # Define this one to tell windows.h to not define min() and max() as macros
    add_definitions(-DNOMINMAX)
endif (MSVC)

# These headers need to go through Qt's MOC compiler
set (MOC_HEADER_FILES    
    cafPdmUiCheckBoxDelegate.h
    cafPdmUiCheckBoxEditor.h
    cafPdmUiCheckBoxTristateEditor.h
    cafPdmUiColorEditor.h
    cafPdmUiComboBoxEditor.h
    cafPdmUiDateEditor.h
	cafPdmUiTimeEditor.h
    cafPdmUiDefaultObjectEditor.h
    cafPdmUiDoubleSliderEditor.h
    cafPdmUiFilePathEditor.h
    cafPdmUiLineEditor.h
    cafPdmUiListEditor.h
    cafPdmUiListView.h
    cafPdmUiListViewEditor.h
    cafPdmUiPropertyView.h
    cafPdmUiPushButtonEditor.h
    cafPdmUiSliderEditor.h
    cafPdmUiTableView.h
    cafPdmUiTableViewDelegate.h
    cafPdmUiTableViewEditor.h
    cafPdmUiTableViewQModel.h
    cafPdmUiTextEditor.h
    cafPdmUiToolButtonEditor.h
    cafPdmUiTreeView.h
    cafPdmUiTreeViewQModel.h
    cafPdmUiTreeViewEditor.h
    cafUiProcess.h
    QMinimizePanel.h
	cafQStyledProgressBar.h
    cafPdmUiTreeSelectionEditor.h
    cafPdmUiTreeSelectionQModel.h
    cafPdmUiFormLayoutObjectEditor.h
    cafPdmUiDoubleValueEditor.h
    cafPdmUniqueIdValidator.h
    cafPdmDoubleStringValidator.h
    cafPdmUiPickableLineEditor.h
    cafPdmUiLabelEditor.h
)

if (CEE_USE_QT5)
	find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets)
	set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)
	qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )	
else()
    find_package(Qt4 COMPONENTS QtCore QtGui QtMain REQUIRED)
    include(${QT_USE_FILE})
	qt4_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )
endif(CEE_USE_QT5)

set( PROJECT_FILES
    
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
    cafPdmUiSliderEditor.cpp
    cafPdmUiSliderEditor.h
    cafPdmUiTextEditor.cpp
    cafPdmUiTextEditor.h
    cafPdmUiToolBarEditor.cpp
    cafPdmUiToolBarEditor.h
    cafPdmUiToolButtonEditor.cpp
    cafPdmUiToolButtonEditor.h
    cafPdmUiTreeSelectionEditor.h
    cafPdmUiTreeSelectionEditor.cpp
    cafPdmUiTreeSelectionQModel.h
    cafPdmUiTreeSelectionQModel.cpp
    cafPdmUiFieldEditorHelper.h
    cafPdmUiFieldEditorHelper.cpp
    cafPdmUiLabelEditor.cpp


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
)

add_library( ${PROJECT_NAME}
    
 ${PROJECT_FILES}

    ${MOC_SOURCE_FILES}
)

if (MSVC) 
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
endif() 

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries ( ${PROJECT_NAME}
    cafProjectDataModel
    ${QT_LIBRARIES}
)

source_group("" FILES ${PROJECT_FILES})

# cotire
if (COMMAND caf_apply_cotire)
  caf_apply_cotire("${PROJECT_NAME}")
endif()
