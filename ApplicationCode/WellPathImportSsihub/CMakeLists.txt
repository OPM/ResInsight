cmake_minimum_required (VERSION 2.8.12)

project (WellPathImportSsihub)

# These headers need to go through Qt's MOC compiler
set( MOC_HEADER_FILES   
    RiuWellImportWizard.h
)

if (RESINSIGHT_BUILD_WITH_QT5)
	find_package(Qt5 COMPONENTS REQUIRED Core Network)
	set(QT_LIBRARIES Qt5::Core Qt5::Network)
	qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES})	
else()
    find_package(Qt4 COMPONENTS QtCore QtNetwork REQUIRED)
    include(${QT_USE_FILE})
	qt4_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES})
endif(RESINSIGHT_BUILD_WITH_QT5)

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

target_link_libraries ( ${PROJECT_NAME}
    cafUserInterface cafPdmCvf CommonCode ${QT_LIBRARIES}
)

source_group("" FILES ${PROJECT_FILES})
