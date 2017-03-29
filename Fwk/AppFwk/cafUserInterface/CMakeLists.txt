cmake_minimum_required (VERSION 2.8)

# Qt
find_package ( Qt4 COMPONENTS QtCore QtGui QtMain )
include (${QT_USE_FILE})

project (cafUserInterface)

include_directories(
    ${cafProjectDataModel_SOURCE_DIR}
)

include_directories (
    ${cafPdmCore_SOURCE_DIR}
    ${cafPdmUiCore_SOURCE_DIR}
    ${cafPdmXml_SOURCE_DIR}
)

# These headers need to go through Qt's MOC compiler
set( QOBJECT_HEADERS    
    cafPdmUiCheckBoxDelegate.h
    cafPdmUiCheckBoxEditor.h
    cafPdmUiCheckBoxTristateEditor.h
    cafPdmUiColorEditor.h
    cafPdmUiComboBoxEditor.h
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
    cafPdmUiTableViewModel.h
    cafPdmUiTextEditor.h
    cafPdmUiToolButtonEditor.h
    cafPdmUiTreeView.h
    cafPdmUiTreeViewModel.h
    cafPdmUiTreeViewEditor.h
    cafUiProcess.h
    QMinimizePanel.h
)

if ( (${CMAKE_VERSION} VERSION_LESS 2.8.6) OR (NOT CMAKE_AUTOMOC) )
    qt4_wrap_cpp( MOC_FILES_CPP ${QOBJECT_HEADERS} )
endif()


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

    # object editors
    cafPdmUiDefaultObjectEditor.cpp
    cafPdmUiDefaultObjectEditor.h
    cafPdmUiListView.cpp
    cafPdmUiListView.h
    cafPdmUiListViewEditor.cpp
    cafPdmUiListViewEditor.h
    cafPdmUiTableItemEditor.cpp
    cafPdmUiTableItemEditor.h
    cafPdmUiTableView.cpp
    cafPdmUiTableView.h
    cafPdmUiTableViewDelegate.cpp
    cafPdmUiTableViewDelegate.h
    cafPdmUiTableViewEditor.cpp
    cafPdmUiTableViewEditor.h
    cafPdmUiTableViewModel.cpp
    cafPdmUiTableViewModel.h
    cafPdmUiTreeEditorHandle.cpp
    cafPdmUiTreeEditorHandle.h
    cafPdmUiTreeItemEditor.cpp
    cafPdmUiTreeItemEditor.h
    cafPdmUiTreeView.cpp
    cafPdmUiTreeView.h
    cafPdmUiTreeViewEditor.cpp
    cafPdmUiTreeViewEditor.h
    cafPdmUiTreeViewModel.cpp
    cafPdmUiTreeViewModel.h
    cafPdmUiPropertyView.cpp
    cafPdmUiPropertyView.h
    cafPdmUiPropertyViewDialog.cpp
    cafPdmUiPropertyViewDialog.h

    # div
    cafAboutDialog.cpp
    cafAboutDialog.h
    cafProgressInfo.cpp
    cafProgressInfo.h
    cafUiProcess.cpp
    cafUiProcess.h
    QMinimizePanel.cpp
    QMinimizePanel.h
)

add_library( ${PROJECT_NAME}
    
 ${PROJECT_FILES}

    ${MOC_FILES_CPP}
)

target_link_libraries ( ${PROJECT_NAME}
    cafProjectDataModel
    cafPdmUiCore
    cafPdmCore
    ${QT_LIBRARIES}
)

if (COMMAND cotire)
    cotire(${PROJECT_NAME})
  
    # make sure the unity target is included in the active builds to trigger rebuild before debug
    get_target_property(_unityTargetName ${PROJECT_NAME} COTIRE_UNITY_TARGET_NAME)
    set_target_properties(${_unityTargetName} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD FALSE)
    set_target_properties(${PROJECT_NAME} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)

endif()

source_group("" FILES ${PROJECT_FILES})
