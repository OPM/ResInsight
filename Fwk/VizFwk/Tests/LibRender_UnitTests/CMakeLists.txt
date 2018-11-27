cmake_minimum_required(VERSION 2.8)

project(LibRender_UnitTests)

# Compile flags should already be setup by caller


find_package(OpenGL)
   
include_directories(${LibCore_SOURCE_DIR})
include_directories(${LibGeometry_SOURCE_DIR})
include_directories(${LibRender_SOURCE_DIR})
include_directories(${VizFramework_SOURCE_DIR}/ThirdParty)

set(CEE_LIBS LibRender LibGeometry LibCore)


set(CEE_SOURCE_FILES
cvfCamera-Test.cpp
cvfDrawableGeo-Test.cpp
cvfGeometryBuilderDrawableGeo-Test.cpp
cvfMatrixState-Test.cpp
cvfOpenGLCapabilities-Test.cpp
cvfOpenGLContextGroup-Test.cpp
cvfOpenGLResourceManager-Test.cpp
cvfOverlayColorLegend-Test.cpp
cvfOverlayItem-Test.cpp
cvfOverlayTextBox-Test.cpp
cvfPrimitiveSetDirect-Test.cpp
cvfPrimitiveSetIndexedUInt-Test.cpp
cvfPrimitiveSetIndexedUIntScoped-Test.cpp
cvfPrimitiveSetIndexedUShortScoped-Test.cpp
cvfRenderState-Test.cpp
cvfSampler-Test.cpp
cvfScalarMapper-Test.cpp
cvfScalarMapperUniformLevels-Test.cpp
cvfShader-Test.cpp
cvfShaderProgram-Test.cpp
cvfShaderProgramGenerator-Test.cpp
cvfShaderSourceRepository-Test.cpp
cvfTexture-Test.cpp
cvfTextureImage-Test.cpp
cvfUniform-Test.cpp
cvfUniformSet-Test.cpp
cvfVertexAttribute-Test.cpp
cvfVertexBundle-Test.cpp
cvfViewport-Test.cpp
../../ThirdParty/gtest/gtest-all.cpp
LibRender_UnitTests.cpp
)

if (MSVC AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 19.11))
  # VS 2017 : Disable warnings from from gtest code, using deprecated code related to TR1 
  add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
endif()

add_executable(${PROJECT_NAME} ${CEE_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${CEE_LIBS} ${OPENGL_LIBRARIES} ${CEE_PLATFORM_LIBS})

