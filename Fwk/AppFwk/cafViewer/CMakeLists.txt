cmake_minimum_required(VERSION 3.12)

project(cafViewer)

# Unity Build
if(CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

# Qt

# These headers need to go through Qt's MOC compiler
set(MOC_HEADER_FILES cafViewer.h)

find_package(
  Qt5
  COMPONENTS
  REQUIRED Core Gui Widgets OpenGL)
set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL)
qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES})

add_library(
  ${PROJECT_NAME}
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
  cafPointOfInterestVisualizer.h
  cafOpenGLWidget.cpp
  cafOpenGLWidget.h
  cafViewer.cpp
  cafViewer.h
  ${MOC_SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  ${PROJECT_NAME}
  LibCore
  LibGeometry
  LibViewing
  LibRender
  LibGuiQt
  cafAnimControl
  ${QT_LIBRARIES})

if(MSVC)
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS
                                                   "/W4 /wd4100 /wd4127")
endif()

source_group("" FILES ${PROJECT_FILES})
