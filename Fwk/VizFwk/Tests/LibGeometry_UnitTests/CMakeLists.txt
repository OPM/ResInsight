cmake_minimum_required(VERSION 2.8)

project(LibGeometry_UnitTests)

# Compile flags should already be setup by caller


include_directories(${LibCore_SOURCE_DIR})
include_directories(${LibGeometry_SOURCE_DIR})
include_directories(${VizFramework_SOURCE_DIR}/ThirdParty)

set(CEE_LIBS LibGeometry LibCore)


set(CEE_SOURCE_FILES
cvfArrowGenerator-Test.cpp
cvfBoundingBox-Test.cpp
cvfBoundingBoxTree-Test.cpp
cvfBoxGenerator-Test.cpp
cvfFrustum-Test.cpp
cvfEdgeKey-Test.cpp
cvfGeometryBuilderFaceList-Test.cpp
cvfGeometryBuilderTriangles-Test.cpp
cvfGeometryBuilder-Test.cpp
cvfGeometryUtils-Test.cpp
cvfMeshEdgeExtractor-Test.cpp
cvfOutlineEdgeExtractor-Test.cpp
cvfPatchGenerator-Test.cpp
cvfPrimitiveTests-Test.cpp
cvfRay-Test.cpp
cvfTriangleMeshEdgeExtractor-Test.cpp
cvfTriangleVertexSplitter-Test.cpp
cvfVertexCompactor-Test.cpp
cvfVertexWelder-Test.cpp
../../ThirdParty/gtest/gtest-all.cpp
LibGeometry_UnitTests.cpp
)

add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
add_executable(${PROJECT_NAME} ${CEE_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${CEE_LIBS} ${CEE_PLATFORM_LIBS} )

