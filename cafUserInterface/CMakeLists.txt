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
    cafPdmUiFilePathEditor.h
    cafPdmUiListEditor.h
    cafPdmUiSliderEditor.h
    cafPdmUiColorEditor.h
    
    cafPdmUiPropertyView.h
)

qt4_wrap_cpp( MOC_FILES_CPP ${QOBJECT_HEADERS} )


add_library( ${PROJECT_NAME}
    cafBasicAboutDialog.cpp 
    cafUiTreeModelPdm.cpp
    cafUiProcess.cpp
    cafPdmUiDefaultObjectEditor.cpp
	cafProgressInfo.cpp

    cafPdmUiLineEditor.cpp
    cafPdmUiCheckBoxEditor.cpp
    cafPdmUiComboBoxEditor.cpp
    cafPdmUiFilePathEditor.cpp
    cafPdmUiListEditor.cpp
    cafPdmUiSliderEditor.cpp
    cafPdmUiColorEditor.cpp

    cafPdmUiPropertyView.cpp
    ${MOC_FILES_CPP}
)
