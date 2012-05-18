cmake_minimum_required(VERSION 2.8)

project(LibGeometry)


# Use our strict compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${CEE_STRICT_CXX_FLAGS}")


include_directories(../LibCore)


set(CEE_HEADER_FILES
cvfArrowGenerator.h
cvfBoundingBox.h
cvfBoxGenerator.h
cvfEdgeKey.h
cvfFrustum.h
cvfGeometryBuilder.h
cvfGeometryBuilderFaceList.h
cvfGeometryBuilderTriangles.h
cvfGeometryUtils.h
cvfLibGeometry.h
cvfMeshEdgeExtractor.h
cvfOutlineEdgeExtractor.h
cvfPatchGenerator.h
cvfRay.h
cvfTriangleMeshEdgeExtractor.h
cvfTriangleVertexSplitter.h
cvfVertexCompactor.h
cvfVertexWelder.h
)

set(CEE_SOURCE_FILES
cvfArrowGenerator.cpp
cvfBoundingBox.cpp
cvfBoxGenerator.cpp
cvfFrustum.cpp
cvfEdgeKey.cpp
cvfGeometryBuilder.cpp
cvfGeometryBuilderFaceList.cpp
cvfGeometryBuilderTriangles.cpp
cvfGeometryUtils.cpp
cvfMeshEdgeExtractor.cpp
cvfOutlineEdgeExtractor.cpp
cvfPatchGenerator.cpp
cvfRay.cpp
cvfTriangleMeshEdgeExtractor.cpp
cvfTriangleVertexSplitter.cpp
cvfVertexCompactor.cpp
cvfVertexWelder.cpp
)

add_library(${PROJECT_NAME} ${CEE_HEADER_FILES} ${CEE_SOURCE_FILES})

