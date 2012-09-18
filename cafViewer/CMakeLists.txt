cmake_minimum_required (VERSION 2.8)

project (cafViewer)

include_directories(
    ${cafAnimControl_SOURCE_DIR}
)

# These headers need to go through Qt's MOC compiler
set( QOBJECT_HEADERS    
    cafViewer.h
)

if ( (${CMAKE_VERSION} VERSION_LESS 2.8.6) OR (NOT CMAKE_AUTOMOC) )
    qt4_wrap_cpp( MOC_FILES_CPP ${QOBJECT_HEADERS} )
endif()


add_library( ${PROJECT_NAME}
	cafViewer.cpp
	cafNavigationPolicy.cpp
	cafCadNavigation.cpp
	cafCeetronNavigation.cpp
	cafOpenGLWidget.cpp
    ${MOC_FILES_CPP}
)
