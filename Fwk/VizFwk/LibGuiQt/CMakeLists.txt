cmake_minimum_required(VERSION 2.8)

project(LibGuiQt)


# We're getting too much trouble from Qt using strict
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${CEE_BASE_CXX_FLAGS}")

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wno-long-long")
endif()


find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

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

target_link_libraries ( ${PROJECT_NAME}
    LibCore
    LibGeometry
    LibRender
    LibViewing

    ${QT_LIBRARIES}
)
set(PROJECT_FILES ${CEE_HEADER_FILES} ${CEE_SOURCE_FILES})
source_group("" FILES ${PROJECT_FILES})
