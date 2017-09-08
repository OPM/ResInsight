cmake_minimum_required (VERSION 2.8)

# Qt
find_package ( Qt4 COMPONENTS QtCore QtGui QtMain QtOpenGl )
include (${QT_USE_FILE})

project ( cafTestApplication )


option(USE_COMMAND_FRAMEWORK "Use Caf Command Framework" ON)

# Qt MOC
set ( QT_MOC_HEADERS 
	MainWindow.h 
    WidgetLayoutTest.h
    ManyGroups.h
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
    ${cafProjectDataModel_SOURCE_DIR}
    ${cafUserInterface_SOURCE_DIR}
)

if (USE_COMMAND_FRAMEWORK)
    include_directories (
        ${cafCommand_SOURCE_DIR}
    )
    ADD_DEFINITIONS(-DTAP_USE_COMMAND_FRAMEWORK)
endif(USE_COMMAND_FRAMEWORK)


include_directories (
    ${cafPdmCore_SOURCE_DIR}
    ${cafPdmUiCore_SOURCE_DIR}
    ${cafPdmXml_SOURCE_DIR}
)

set( PROJECT_FILES
    Main.cpp
    MainWindow.cpp
    WidgetLayoutTest.cpp
    ManyGroups.cpp
)


# add the executable 
add_executable ( ${PROJECT_NAME}
	${PROJECT_FILES}
    ${MOC_FILES_CPP}
    ${QRC_FILES_CPP}
)

set (TAP_LINK_LIBRARIES
    cafUserInterface
    cafPdmXml
    ${QT_LIBRARIES}
    )

if (USE_COMMAND_FRAMEWORK)
    set (TAP_LINK_LIBRARIES
        ${TAP_LINK_LIBRARIES}
        cafCommand
    )
endif(USE_COMMAND_FRAMEWORK)


target_link_libraries ( ${PROJECT_NAME}
    ${TAP_LINK_LIBRARIES}
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
