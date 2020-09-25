cmake_minimum_required(VERSION 3.12)

project(LibGuiQt)

# CMake Unity Build
if (CVF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

# We're getting too much trouble from Qt using strict
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${CEE_BASE_CXX_FLAGS}")

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wno-long-long")
endif()

find_package(OpenGL)

find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets OpenGL)
set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL)

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

# Unity Build
if (CMAKE_UNITY_BUILD)
  set_source_files_properties (cvfqtOpenGLWidget.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION TRUE)
  set_source_files_properties (cvfqtOpenGLContext.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION TRUE)
endif()
