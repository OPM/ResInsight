cmake_minimum_required (VERSION 2.8)

project (cafViewer)

include_directories(
    ${cafAnimControl_SOURCE_DIR}
)

# These headers need to go through Qt's MOC compiler
set( QOBJECT_HEADERS    
    cafViewer.h
)

qt4_wrap_cpp( MOC_FILES_CPP ${QOBJECT_HEADERS} )


add_library( ${PROJECT_NAME}
	cafViewer.cpp
	cafNavigationPolicy.cpp
	cafCadNavigation.cpp
	cafCeetronNavigation.cpp
	cafOpenGLWidget.cpp
    ${MOC_FILES_CPP}
)
