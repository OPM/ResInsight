cmake_minimum_required(VERSION 2.8)

project(LibGuiQt)


# We're getting too much trouble from Qt using strict
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${CEE_BASE_CXX_FLAGS}")

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wno-long-long")
endif()


find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})


include_directories(../LibCore)
include_directories(../LibGeometry)
include_directories(../LibRender)
include_directories(../LibViewing)


set(CEE_SOURCE_FILES
cvfqtBasicAboutDialog.cpp
cvfqtCvfBoundQGLContext.cpp
cvfqtMouseState.cpp
cvfqtOpenGLContext.cpp
cvfqtOpenGLWidget.cpp
cvfqtPerformanceInfoHud.cpp
cvfqtUtils.cpp
)

add_library(${PROJECT_NAME} ${CEE_SOURCE_FILES})

