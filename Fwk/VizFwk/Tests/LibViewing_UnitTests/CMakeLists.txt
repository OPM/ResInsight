cmake_minimum_required(VERSION 2.8)

project(LibViewing_UnitTests)

# Compile flags should already be setup by caller


find_package(OpenGL)

include_directories(${LibCore_SOURCE_DIR})
include_directories(${LibGeometry_SOURCE_DIR})
include_directories(${LibRender_SOURCE_DIR})
include_directories(${LibViewing_SOURCE_DIR})
include_directories(${VizFramework_SOURCE_DIR}/ThirdParty)

set(CEE_LIBS LibViewing LibRender LibGeometry LibCore)


set(CEE_SOURCE_FILES
cvfClipPlaneSet-Test.cpp
cvfCullSettings-Test.cpp
cvfEffect-Test.cpp
cvfHitItem-Test.cpp
cvfHitItemCollection-Test.cpp
cvfModelBasicList-Test.cpp
cvfModelBasicTree-Test.cpp
cvfPart-Test.cpp
cvfPartRenderHintCollection-Test.cpp
cvfPerformanceInfo-Test.cpp
cvfRendering-Test.cpp
cvfRenderQueue-Test.cpp
cvfRenderQueueSorter-Test.cpp
cvfRenderSequence-Test.cpp
cvfTransform-Test.cpp
cvfScene-Test.cpp
../../ThirdParty/gtest/gtest-all.cpp
LibViewing_UnitTests.cpp
)

if (MSVC AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 19.11))
  # VS 2017 : Disable warnings from from gtest code, using deprecated code related to TR1 
  add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
endif()

add_executable(${PROJECT_NAME} ${CEE_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${CEE_LIBS} ${OPENGL_LIBRARIES} ${CEE_PLATFORM_LIBS})

