cmake_minimum_required (VERSION 2.8.12)

project (cafAnimControl)

# Qt
find_package ( Qt4 COMPONENTS QtCore QtGui QtMain )
include (${QT_USE_FILE})

set( QOBJECT_HEADERS    
    cafFrameAnimationControl.h
    cafAnimationToolBar.h
)

if ( NOT CMAKE_AUTOMOC )
    qt4_wrap_cpp( MOC_FILES_CPP ${QOBJECT_HEADERS} )
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

set( PROJECT_FILES
    cafFrameAnimationControl.h
    cafFrameAnimationControl.cpp
    cafAnimationToolBar.h
    cafAnimationToolBar.cpp
)

add_library( ${PROJECT_NAME}
 	${PROJECT_FILES}
    ${MOC_FILES_CPP}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

source_group("" FILES ${PROJECT_FILES})
