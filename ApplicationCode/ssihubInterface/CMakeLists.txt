cmake_minimum_required (VERSION 2.8)

project (ssihubInterface)

# These headers need to go through Qt's MOC compiler
set( QOBJECT_HEADERS   
    ssihubWebServiceInterface.h
    ssihubDialog.h 
    httpwindow.h

    RiuWellImportWizard.h

)


set( QT_UI_FILES   
    authenticationdialog.ui 
)

if ( (${CMAKE_VERSION} VERSION_LESS 2.8.6) OR (NOT CMAKE_AUTOMOC) )
    qt4_wrap_cpp( MOC_FILES_CPP ${QOBJECT_HEADERS} )
    qt4_wrap_ui( FORM_FILES_CPP ${QT_UI_FILES} )
endif()

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}

    ${CMAKE_CURRENT_SOURCE_DIR}/../FileInterface
	${CMAKE_CURRENT_SOURCE_DIR}/../../cafProjectDataModel 
	${CMAKE_CURRENT_SOURCE_DIR}/../../cafUserInterface
)


add_library( ${PROJECT_NAME}
    ssihubInterface.cpp
    ssihubWebServiceInterface.cpp
    ssihubDialog.cpp
    httpwindow.cpp

    RimWellPathImport.h
    RimWellPathImport.cpp
    RimOilRegionEntry.h
    RimOilRegionEntry.cpp
    RimOilFieldEntry.h
    RimOilFieldEntry.cpp
    RimWellsEntry.h
    RimWellsEntry.cpp
    RimWellCollection.h
    RimWellCollection.cpp
    RiuWellImportWizard.h
    RiuWellImportWizard.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/../FileInterface/RifJsonEncodeDecode.cpp

    ${MOC_FILES_CPP}
    ${FORM_FILES_CPP}
    ${HEADER_FILES}
)
