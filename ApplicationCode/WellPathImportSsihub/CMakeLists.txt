cmake_minimum_required (VERSION 2.8.12)

project (WellPathImportSsihub)

# These headers need to go through Qt's MOC compiler
set( QOBJECT_HEADERS   
    RiuWellImportWizard.h
)


set( QT_UI_FILES   
)

if ( NOT CMAKE_AUTOMOC )
    qt4_wrap_cpp( MOC_FILES_CPP ${QOBJECT_HEADERS} )
    qt4_wrap_ui( FORM_FILES_CPP ${QT_UI_FILES} )
endif()

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}

    ${CMAKE_CURRENT_SOURCE_DIR}/../FileInterface
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

target_link_libraries ( ${PROJECT_NAME}
    cafUserInterface cafPdmCvf CommonCode
)

source_group("" FILES ${PROJECT_FILES})
