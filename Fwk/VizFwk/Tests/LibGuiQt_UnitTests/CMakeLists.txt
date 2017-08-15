cmake_minimum_required(VERSION 2.8)

project(LibGuiQt_UnitTests)

# Compile flags should already be setup by caller


find_package(OpenGL)

option(USE_QT5 "Use version 5.x of the Qt library" OFF)
if(USE_QT5)
  find_package(Qt5 COMPONENTS Core Gui OpenGL REQUIRED)
else()
  find_package(Qt4 COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
  include(${QT_USE_FILE})
endif()

include_directories(${LibCore_SOURCE_DIR})
include_directories(${LibIo_SOURCE_DIR})
include_directories(${LibGeometry_SOURCE_DIR})
include_directories(${LibRender_SOURCE_DIR})
include_directories(${LibViewing_SOURCE_DIR})
include_directories(${LibGuiQt_SOURCE_DIR})
include_directories(${VizFramework_SOURCE_DIR}/ThirdParty)

set(CEE_LIBS LibGuiQt LibViewing LibRender LibGeometry LibIo LibCore)


set(CEE_SOURCE_FILES
cvfqtUtils-Test.cpp 
../../ThirdParty/gtest/gtest-all.cpp
LibGuiQt_UnitTests.cpp
)

add_executable(${PROJECT_NAME} ${CEE_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${CEE_LIBS} ${OPENGL_LIBRARIES} ${QT_LIBRARIES} ${CEE_PLATFORM_LIBS})
if(USE_QT5)
  qt5_use_modules(${PROJECT_NAME} Core Gui OpenGL)
endif()
