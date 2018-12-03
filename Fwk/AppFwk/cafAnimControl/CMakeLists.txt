cmake_minimum_required (VERSION 2.8.12)

project (cafAnimControl)
# Qt
find_package(Qt5Core CONFIG QUIET)
if (Qt5Core_FOUND)
	find_package(Qt5 CONFIG REQUIRED Core Gui Widgets)
else()
	find_package(Qt4 COMPONENTS QtCore QtGui QtMain REQUIRED)
    include(${QT_USE_FILE})
endif(Qt5Core_FOUND)


set( MOC_HEADER_FILES    
    cafFrameAnimationControl.h
    cafAnimationToolBar.h
)

if (Qt5Core_FOUND)
	qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )
else()
	qt4_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )
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
    ${MOC_SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (Qt5Core_FOUND)
	set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)
endif()

target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES})

if (MSVC) 
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4127")
endif()

source_group("" FILES ${PROJECT_FILES})
