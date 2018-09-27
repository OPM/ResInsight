cmake_minimum_required (VERSION 2.8.12)

project (cafViewer)

# Qt
find_package ( Qt4 COMPONENTS QtCore QtGui QtMain )
include (${QT_USE_FILE})

# These headers need to go through Qt's MOC compiler
set( QOBJECT_HEADERS    
    cafViewer.h
)

if ( NOT CMAKE_AUTOMOC )
    qt4_wrap_cpp( MOC_FILES_CPP ${QOBJECT_HEADERS} )
endif()


add_library( ${PROJECT_NAME}
	cafCadNavigation.cpp
	cafCadNavigation.h
	cafCeetronNavigation.cpp
	cafCeetronNavigation.h
	cafCeetronPlusNavigation.cpp
	cafCeetronPlusNavigation.h
    cafTrackBallBasedNavigation.cpp
    cafTrackBallBasedNavigation.h
	cafNavigationPolicy.cpp
	cafNavigationPolicy.h
	cafOpenGLWidget.cpp
	cafOpenGLWidget.h
	cafViewer.cpp
	cafViewer.h

    ${MOC_FILES_CPP}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries ( ${PROJECT_NAME}
    LibCore
    LibGeometry
    LibViewing
    LibRender
    LibGuiQt

    cafAnimControl

    ${QT_LIBRARIES}
)

if (MSVC) 
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
endif() 

source_group("" FILES ${PROJECT_FILES})
