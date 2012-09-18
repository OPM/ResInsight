cmake_minimum_required (VERSION 2.8)

project (cafAnimControl)

set( QOBJECT_HEADERS    
    cafFrameAnimationControl.h
    cafAnimationToolBar.h
)

if ( (${CMAKE_VERSION} VERSION_LESS 2.8.6) OR (NOT CMAKE_AUTOMOC) )
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

add_library( ${PROJECT_NAME}
    cafFrameAnimationControl.cpp
    cafAnimationToolBar.cpp
    ${MOC_FILES_CPP}
)
