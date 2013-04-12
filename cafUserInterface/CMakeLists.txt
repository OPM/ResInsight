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
    cafUiTreeModelPdm.cpp
    cafUiProcess.cpp
    cafPdmUiDefaultObjectEditor.cpp
	cafProgressInfo.cpp

    cafPdmUiLineEditor.cpp
    cafPdmUiCheckBoxEditor.cpp
    cafPdmUiComboBoxEditor.cpp
    cafPdmUiPushButtonEditor.cpp
    cafPdmUiFilePathEditor.cpp
    cafPdmUiListEditor.cpp
    cafPdmUiSliderEditor.cpp
    cafPdmUiDoubleSliderEditor.cpp
    cafPdmUiColorEditor.cpp
    cafPdmUiTextEditor.cpp

    cafPdmUiPropertyView.cpp
    ${MOC_FILES_CPP}
)
