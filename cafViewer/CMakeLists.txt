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
	cafCadNavigation.cpp
	cafCadNavigation.h
	cafCeetronNavigation.cpp
	cafCeetronNavigation.h
	cafNavigationPolicy.cpp
	cafNavigationPolicy.h
	cafOpenGLWidget.cpp
	cafOpenGLWidget.h
	cafViewer.cpp
	cafViewer.h

    ${MOC_FILES_CPP}
)
