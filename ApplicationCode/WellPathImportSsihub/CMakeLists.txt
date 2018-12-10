cmake_minimum_required (VERSION 2.8.12)

project (WellPathImportSsihub)

if (RESINSIGHT_BUILD_WITH_QT5)
	find_package(Qt5 COMPONENTS Core QUIET)
endif(RESINSIGHT_BUILD_WITH_QT5)

if (Qt5Core_FOUND)
	find_package(Qt5 CONFIG REQUIRED Core Network)
else()
	find_package(Qt4 COMPONENTS QtCore QtNetwork REQUIRED)
    include(${QT_USE_FILE})
endif(Qt5Core_FOUND)

# These headers need to go through Qt's MOC compiler
set( QT_MOC_HEADERS   
    RiuWellImportWizard.h
)


set( QT_UI_FILES   
)

if ( NOT CMAKE_AUTOMOC )
	if (Qt5Core_FOUND)
		qt5_wrap_cpp(MOC_SOURCE_FILES ${QT_MOC_HEADERS} )
        qt5_wrap_ui( FORM_FILES_CPP ${QT_UI_FILES} )
	else()
		qt4_wrap_cpp(MOC_SOURCE_FILES ${QT_MOC_HEADERS} )
        qt4_wrap_ui( FORM_FILES_CPP ${QT_UI_FILES} )
	endif()
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

    ${MOC_SOURCE_FILES}
    ${FORM_FILES_CPP}
    ${HEADER_FILES}
)

if (Qt5Core_FOUND)
	set(QT_LIBRARIES Qt5::Core Qt5::Network)
endif()

target_link_libraries ( ${PROJECT_NAME}
    cafUserInterface cafPdmCvf CommonCode ${QT_LIBRARIES}
)

source_group("" FILES ${PROJECT_FILES})
