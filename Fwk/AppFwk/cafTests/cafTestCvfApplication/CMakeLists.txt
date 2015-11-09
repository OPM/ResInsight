cmake_minimum_required (VERSION 2.8)

# Qt
find_package ( Qt4 COMPONENTS QtCore QtGui QtMain QtOpenGl )
include (${QT_USE_FILE})

project ( cafTestCvfApplication )

# Qt MOC
set ( QT_MOC_HEADERS 
	MainWindow.h 
    WidgetLayoutTest.h
)

qt4_wrap_cpp( MOC_FILES_CPP
    ${QT_MOC_HEADERS}
)

# Resource file
set( QRC_FILES  
    textedit.qrc
)

# Runs RCC on specified files
qt4_add_resources( QRC_FILES_CPP
    ${QRC_FILES}
)

include_directories (
	${LibCore_SOURCE_DIR}
    ${LibGeometry_SOURCE_DIR}
    ${LibGuiQt_SOURCE_DIR}
    ${LibRender_SOURCE_DIR}
    ${LibViewing_SOURCE_DIR}

	${cafPdmCore_SOURCE_DIR}
    ${cafPdmUiCore_SOURCE_DIR}
    ${cafPdmXml_SOURCE_DIR}
	${cafProjectDataModel_SOURCE_DIR}
	
	${cafCommand_SOURCE_DIR}
#	${cafViewer_SOURCE_DIR}
	${cafUserInterface_SOURCE_DIR}
	${cafPdmCvf_SOURCE_DIR}
#	${CommonCode_SOURCE_DIR}

)

set( PROJECT_FILES
    Main.cpp
    MainWindow.cpp
    WidgetLayoutTest.cpp

	TapCvfSpecialization.cpp
	TapProject.cpp
)


# add the executable 
add_executable ( ${PROJECT_NAME}
	${PROJECT_FILES}
    ${MOC_FILES_CPP}
    ${QRC_FILES_CPP}
)

target_link_libraries ( ${PROJECT_NAME}

	LibCore

    cafPdmCore
    cafPdmUiCore
    cafPdmXml
    cafProjectDataModel

	cafCommand
	cafUserInterface

	cafPdmCvf

    ${QT_LIBRARIES}
)

source_group("" FILES ${PROJECT_FILES})

# Copy Qt Dlls
if (MSVC)
    set (QTLIBLIST QtCore QtGui QtOpenGl)
    foreach (qtlib ${QTLIBLIST})
        
        # Debug
        execute_process(COMMAND cmake -E copy_if_different ${QT_BINARY_DIR}/${qtlib}d4.dll ${CMAKE_CURRENT_BINARY_DIR}/Debug/${qtlib}d4.dll)
        
        # Release
        execute_process(COMMAND cmake -E copy_if_different ${QT_BINARY_DIR}/${qtlib}4.dll ${CMAKE_CURRENT_BINARY_DIR}/Release/${qtlib}4.dll)
    endforeach( qtlib )
endif(MSVC)
