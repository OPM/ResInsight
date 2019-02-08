cmake_minimum_required(VERSION 2.8.12)

project(LibGuiQt)


# We're getting too much trouble from Qt using strict
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${CEE_BASE_CXX_FLAGS}")

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wno-long-long")
endif()

if (CEE_USE_QT5)
	find_package(Qt5 COMPONENTS Core Gui OpenGL Widgets REQUIRED)
else()
	find_package(Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
    include(${QT_USE_FILE})
endif(CEE_USE_QT5)

set(CEE_HEADER_FILES
cvfqtBasicAboutDialog.h
cvfqtCvfBoundQGLContext.h
cvfqtMouseState.h
cvfqtOpenGLContext.h
cvfqtOpenGLWidget.h
cvfqtPerformanceInfoHud.h
cvfqtUtils.h
)

set(CEE_SOURCE_FILES
cvfqtBasicAboutDialog.cpp
cvfqtCvfBoundQGLContext.cpp
cvfqtMouseState.cpp
cvfqtOpenGLContext.cpp
cvfqtOpenGLWidget.cpp
cvfqtPerformanceInfoHud.cpp
cvfqtUtils.cpp
)

add_library(${PROJECT_NAME} ${CEE_HEADER_FILES} ${CEE_SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if ( NOT CMAKE_AUTOMOC )
	if (CEE_USE_QT5)
		qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )
	
	else()
		qt4_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )
	endif()
endif()

if (CEE_USE_QT5)
    set(QT_LIBRARIES Qt5::Core Qt5::OpenGL Qt5::Widgets)
endif()

target_link_libraries ( ${PROJECT_NAME}
    LibCore
    LibGeometry
    LibRender
    LibViewing

    ${QT_LIBRARIES}
)
set(PROJECT_FILES ${CEE_HEADER_FILES} ${CEE_SOURCE_FILES})
source_group("" FILES ${PROJECT_FILES})

#if (COMMAND ri_apply_cotire)
#  set_source_files_properties (cvfqtOpenGLWidget.cpp PROPERTIES COTIRE_EXCLUDED TRUE)
#  set_source_files_properties (cvfqtOpenGLContext.cpp PROPERTIES COTIRE_EXCLUDED TRUE)
#  ri_apply_cotire()
#endif()
