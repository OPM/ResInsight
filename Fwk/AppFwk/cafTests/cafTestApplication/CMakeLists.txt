cmake_minimum_required (VERSION 2.8)

project ( cafTestApplication )

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
    ${CMAKE_SOURCE_DIR}/cafProjectDataModel
    ${CMAKE_SOURCE_DIR}/cafUserInterface
)

# add the executable 
add_executable ( ${PROJECT_NAME}
    Main.cpp
    MainWindow.cpp
    WidgetLayoutTest.cpp
    ${MOC_FILES_CPP}
    ${QRC_FILES_CPP}
)

target_link_libraries ( ${PROJECT_NAME}
    cafUserInterface
    cafProjectDataModel
    ${QT_LIBRARIES}
)

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
