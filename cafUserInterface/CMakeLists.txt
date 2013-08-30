cmake_minimum_required (VERSION 2.8)

project (cafUserInterface)

include_directories(
    ${cafProjectDataModel_SOURCE_DIR}
)

# These headers need to go through Qt's MOC compiler
set( QOBJECT_HEADERS    
    cafBasicAboutDialog.h
    cafUiTreeModelPdm.h
    cafUiProcess.h
    
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
)

if ( (${CMAKE_VERSION} VERSION_LESS 2.8.6) OR (NOT CMAKE_AUTOMOC) )
    qt4_wrap_cpp( MOC_FILES_CPP ${QOBJECT_HEADERS} )
endif()


add_library( ${PROJECT_NAME}
	cafBasicAboutDialog.cpp
	cafBasicAboutDialog.h
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
	cafPdmUiPropertyView.cpp
	cafPdmUiPropertyView.h
	cafPdmUiPushButtonEditor.cpp
	cafPdmUiPushButtonEditor.h
	cafPdmUiSliderEditor.cpp
	cafPdmUiSliderEditor.h
	cafPdmUiTextEditor.cpp
	cafPdmUiTextEditor.h
	cafProgressInfo.cpp
	cafProgressInfo.h
	cafUiProcess.cpp
	cafUiProcess.h
	cafUiTreeItem.h
	cafUiTreeModelPdm.cpp
	cafUiTreeModelPdm.h

    ${MOC_FILES_CPP}
)
