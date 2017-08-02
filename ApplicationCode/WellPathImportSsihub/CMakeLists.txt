cmake_minimum_required (VERSION 2.8)

project (WellPathImportSsihub)
find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork)
include(${QT_USE_FILE})

# These headers need to go through Qt's MOC compiler
set( QOBJECT_HEADERS   
    RiuWellImportWizard.h
)


set( QT_UI_FILES   
)

if ( (${CMAKE_VERSION} VERSION_LESS 2.8.6) OR (NOT CMAKE_AUTOMOC) )
    qt4_wrap_cpp( MOC_FILES_CPP ${QOBJECT_HEADERS} )
    qt4_wrap_ui( FORM_FILES_CPP ${QT_UI_FILES} )
endif()

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}

    ${CMAKE_CURRENT_SOURCE_DIR}/../FileInterface
	${CMAKE_CURRENT_SOURCE_DIR}/../../Fwk/AppFwk/cafProjectDataModel 
	${CMAKE_CURRENT_SOURCE_DIR}/../../Fwk/AppFwk/cafUserInterface
)


add_library( ${PROJECT_NAME}
    RimWellPathImport.h
    RimWellPathImport.cpp
    RimOilRegionEntry.h
    RimOilRegionEntry.cpp
    RimOilFieldEntry.h
    RimOilFieldEntry.cpp
    RimWellsEntry.h
    RimWellsEntry.cpp
    RiuWellImportWizard.h
    RiuWellImportWizard.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/../FileInterface/RifJsonEncodeDecode.cpp

    ${MOC_FILES_CPP}
    ${FORM_FILES_CPP}
    ${HEADER_FILES}
)
