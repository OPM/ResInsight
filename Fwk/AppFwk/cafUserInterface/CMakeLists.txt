cmake_minimum_required (VERSION 2.8)

project (cafUserInterface)

include_directories(
    ${cafProjectDataModel_SOURCE_DIR}
)

# These headers need to go through Qt's MOC compiler
set( QOBJECT_HEADERS    
    cafUiTreeModelPdm.h
    cafUiProcess.h
    
    cafPdmSettings.h
    cafPdmUiLineEditor.h
    cafPdmUiCheckBoxEditor.h
    cafPdmUiComboBoxEditor.h
    cafPdmUiPushButtonEditor.h
    cafPdmUiFilePathEditor.h
    cafPdmUiListEditor.h
    cafPdmUiSliderEditor.h
    cafPdmUiDoubleSliderEditor.h
    cafPdmUiTextEditor.h

    cafPdmUiColorEditor.h
    
    cafPdmUiPropertyView.h
    cafPdmUiPropertyDialog.h
    cafPdmUiTreeView.h
    cafPdmUiTreeViewModel.h
    cafPdmUiListView.h
    cafPdmUiListViewEditor.h
)

if ( (${CMAKE_VERSION} VERSION_LESS 2.8.6) OR (NOT CMAKE_AUTOMOC) )
    qt4_wrap_cpp( MOC_FILES_CPP ${QOBJECT_HEADERS} )
endif()


add_library( ${PROJECT_NAME}
	cafAboutDialog.cpp
	cafAboutDialog.h
	cafPdmSettings.cpp
	cafPdmUiCheckBoxEditor.cpp
	cafPdmUiCheckBoxEditor.h
	cafPdmUiColorEditor.cpp
	cafPdmUiColorEditor.h
	cafPdmUiComboBoxEditor.cpp
	cafPdmUiComboBoxEditor.h
	cafPdmUiDefaultObjectEditor.cpp
	cafPdmUiDefaultObjectEditor.h
	cafPdmUiDoubleSliderEditor.cpp
	cafPdmUiDoubleSliderEditor.h
	cafPdmUiFilePathEditor.cpp
	cafPdmUiFilePathEditor.h
	cafPdmUiLineEditor.cpp
	cafPdmUiLineEditor.h
	cafPdmUiListEditor.cpp
	cafPdmUiListEditor.h
    cafPdmUiListViewEditor.cpp
    cafPdmUiListViewEditor.h
    cafPdmUiListView.cpp
	cafPdmUiPropertyDialog.cpp
	cafPdmUiPropertyDialog.h
	cafPdmUiPropertyView.cpp
	cafPdmUiPropertyView.h
	cafPdmUiPushButtonEditor.cpp
	cafPdmUiPushButtonEditor.h
	cafPdmUiSliderEditor.cpp
	cafPdmUiSliderEditor.h
	cafPdmUiTextEditor.cpp
	cafPdmUiTextEditor.h
    cafPdmUiTreeViewEditor.cpp
    cafPdmUiTreeViewEditor.h
    cafPdmUiTreeViewModel.cpp
    cafPdmUiTreeViewModel.h
    cafPdmUiTreeView.cpp
	cafProgressInfo.cpp
	cafProgressInfo.h
	cafUiProcess.cpp
	cafUiProcess.h
	cafUiTreeModelPdm.cpp
	cafUiTreeModelPdm.h

    ${MOC_FILES_CPP}
)
