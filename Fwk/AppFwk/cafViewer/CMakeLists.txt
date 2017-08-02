cmake_minimum_required (VERSION 2.8)

project (cafViewer)

# Qt
option(USE_QT5 "Use version 5.x of the Qt library" OFF)
if(USE_QT5)
  find_package(Qt5 COMPONENTS Core Gui Widgets)
else()
  find_package ( Qt4 COMPONENTS QtCore QtGui QtMain )
  include (${QT_USE_FILE})
endif()

include_directories(
    ${LibCore_SOURCE_DIR}
    ${LibGeometry_SOURCE_DIR}
    ${LibViewing_SOURCE_DIR}
    ${LibRender_SOURCE_DIR}
    ${LibGuiQt_SOURCE_DIR}

    ${cafAnimControl_SOURCE_DIR}
)

# These headers need to go through Qt's MOC compiler
set( QOBJECT_HEADERS    
    cafViewer.h
)

if ( (${CMAKE_VERSION} VERSION_LESS 2.8.6) OR (NOT CMAKE_AUTOMOC) )
  if(USE_QT5)
    qt5_wrap_cpp( MOC_FILES_CPP ${QOBJECT_HEADERS} )
  else()
    qt4_wrap_cpp( MOC_FILES_CPP ${QOBJECT_HEADERS} )
  endif()
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
if(USE_QT5)
  qt5_use_modules(${PROJECT_NAME} Core Gui Widgets)
endif()
