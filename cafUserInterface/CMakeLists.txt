cmake_minimum_required (VERSION 2.8)

project (cafUserInterface)

include_directories(
    ${cafProjectDataModel_SOURCE_DIR}
    ${QtPropertyBrowser_SOURCE_DIR}
)

# These headers need to go through Qt's MOC compiler
set( QOBJECT_HEADERS    
    cafBasicAboutDialog.h
    cafUiTreeModelPdm.h
    cafUiPropertyCreatorPdm.h
    cafUiProcess.h
)

qt4_wrap_cpp( MOC_FILES_CPP ${QOBJECT_HEADERS} )


add_library( ${PROJECT_NAME}
    cafBasicAboutDialog.cpp 
    cafUiTreeModelPdm.cpp
    cafUiPropertyCreatorPdm.cpp
    cafUiProcess.cpp
    ${MOC_FILES_CPP}
)
