cmake_minimum_required (VERSION 2.8)

project (cafAnimControl)

# Qt
option(USE_QT5 "Use version 5.x of the Qt library" OFF)
if(USE_QT5)
  find_package(Qt5 COMPONENTS Core Gui Widgets)
else()
  find_package ( Qt4 COMPONENTS QtCore QtGui QtMain )
  include (${QT_USE_FILE})
endif()

set( QOBJECT_HEADERS    
    cafFrameAnimationControl.h
    cafAnimationToolBar.h
)

if ( (${CMAKE_VERSION} VERSION_LESS 2.8.6) OR (NOT CMAKE_AUTOMOC) )
  if(USE_QT5)
    qt5_wrap_cpp( MOC_FILES_CPP ${QOBJECT_HEADERS} )
  else()
    qt4_wrap_cpp( MOC_FILES_CPP ${QOBJECT_HEADERS} )
  endif()
endif()

# NOTE! Resources in this subfolder appends to the variable QRC_FILES in parent scope
# CMakeList.txt in the application folder (parent scope) must use the following syntax
# to make sure the QRC_FILES variable contains appended files in subfolders

#   set( QRC_FILES
#       ${QRC_FILES} 
#       Resources/MyaApplication.qrc
#   )

set( QRC_FILES
    ${QRC_FILES}
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/cafAnimControl.qrc
    PARENT_SCOPE
)

add_library( ${PROJECT_NAME}
    cafFrameAnimationControl.cpp
    cafAnimationToolBar.cpp
    ${MOC_FILES_CPP}
)
if(USE_QT5)
  qt5_use_modules(${PROJECT_NAME} Core Gui Widgets)
endif()
