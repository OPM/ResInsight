cmake_minimum_required(VERSION 3.12)

project(LibViewing)

# CMake Unity Build
if (CVF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

# Use our strict compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${CEE_STRICT_CXX_FLAGS}")

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-deprecated-declarations")
endif()

set(CEE_HEADER_FILES
cvfClipPlaneSet.h
cvfConstantFrameRate.h
cvfCullSettings.h
cvfDynamicUniformSet.h
cvfEffect.h
cvfFixedSizeTransform.h
cvfGaussianBlur.h
cvfHitItem.h
cvfHitItemCollection.h
cvfLibViewing.h
cvfLocators.h
cvfManipulatorTrackball.h
cvfModel.h
cvfModelBasicList.h
cvfModelBasicTree.h
cvfPart.h
cvfPartHighlighter.h
cvfPartRenderHintCollection.h
cvfPerformanceInfo.h
cvfRayIntersectSpec.h
cvfRenderEngine.h
cvfRendering.h
cvfRenderQueue.h
cvfRenderQueueBuilder.h
cvfRenderQueueSorter.h
cvfRenderSequence.h
cvfScene.h
cvfSingleQuadRenderingGenerator.h
cvfTransform.h
)

set(CEE_SOURCE_FILES
cvfClipPlaneSet.cpp
cvfConstantFrameRate.cpp
cvfCullSettings.cpp
cvfDynamicUniformSet.cpp
cvfEffect.cpp
cvfFixedSizeTransform.cpp
cvfGaussianBlur.cpp
cvfHitItem.cpp
cvfHitItemCollection.cpp
cvfLocators.cpp
cvfManipulatorTrackball.cpp
cvfModel.cpp
cvfModelBasicList.cpp
cvfModelBasicTree.cpp
cvfPart.cpp
cvfPartHighlighter.cpp
cvfPartRenderHintCollection.cpp
cvfPerformanceInfo.cpp
cvfRayIntersectSpec.cpp
cvfRenderEngine.cpp
cvfRendering.cpp
cvfRenderQueue.cpp
cvfRenderQueueBuilder.cpp
cvfRenderQueueSorter.cpp
cvfRenderSequence.cpp
cvfScene.cpp
cvfSingleQuadRenderingGenerator.cpp
cvfTransform.cpp
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
)

set(PROJECT_FILES ${CEE_HEADER_FILES} ${CEE_SOURCE_FILES})
source_group("" FILES ${PROJECT_FILES})
