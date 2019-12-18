cmake_minimum_required(VERSION 2.8)

project(LibRender)


# Use our strict compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${CEE_STRICT_CXX_FLAGS}")

# For now, disable warning about unknown pragmas locally here (due to usage of OpenMP)
# Probably need to do this more centralized whenever we use OpenMP more extensively
if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas -Wno-deprecated -Wno-deprecated-declarations")
endif()

# For now, remove pedantic flag for OSX since it clashes with glew
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	string(REPLACE "-pedantic" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
endif()


include_directories(glew)


set(CEE_HEADER_FILES
cvfBufferObjectManaged.h
cvfCamera.h
cvfCameraAnimation.h
cvfDrawable.h
cvfDrawableGeo.h
cvfDrawableText.h
cvfDrawableVectors.h
cvfFixedAtlasFont.h
cvfFont.h
cvfFramebufferObject.h
cvfGeometryBuilderDrawableGeo.h
cvfGlyph.h
cvfHitDetail.h
cvfLibRender.h
cvfMatrixState.h
cvfOglRc.h
cvfOpenGL.h
cvfOpenGLCapabilities.h
cvfOpenGLContext.h
cvfOpenGLContextGroup.h
cvfOpenGLResourceManager.h
cvfOpenGLTypes.h
cvfOverlayAxisCross.h
cvfOverlayScalarMapperLegend.h
cvfOverlayColorLegend.h
cvfOverlayImage.h
cvfOverlayItem.h
cvfOverlayNavigationCube.h
cvfOverlayTextBox.h
cvfPrimitiveSet.h
cvfPrimitiveSetDirect.h
cvfPrimitiveSetIndexedUInt.h
cvfPrimitiveSetIndexedUIntScoped.h
cvfPrimitiveSetIndexedUShort.h
cvfPrimitiveSetIndexedUShortScoped.h
cvfRenderbufferObject.h
cvfRenderState.h
cvfRenderStateBlending.h
cvfRenderStateColorMask.h
cvfRenderStateCullFace.h
cvfRenderStateDepth.h
cvfRenderStateFrontFace.h
cvfRenderStateLine.h
cvfRenderStatePoint.h
cvfRenderStatePolygonMode.h
cvfRenderStatePolygonOffset.h
cvfRenderStateSet.h
cvfRenderStateStencil.h
cvfRenderStateTextureBindings.h
cvfRenderStateTracker.h
cvfRenderState_FF.h
cvfSampler.h
cvfScalarMapper.h
cvfScalarMapperRangeBased.h
cvfScalarMapperContinuousLog.h
cvfScalarMapperContinuousLinear.h
cvfScalarMapperDiscreteLinear.h
cvfScalarMapperDiscreteLog.h
cvfScalarMapperUniformLevels.h
cvfShader.h
cvfShaderProgram.h
cvfShaderProgramGenerator.h
cvfShaderSourceProvider.h
cvfShaderSourceRepository.h
cvfShaderSourceRepositoryFile.h
cvfShaderSourceStrings.h
cvfTextDrawer.h
cvfTexture.h
cvfTexture2D_FF.h
cvfTextureImage.h
cvfUniform.h
cvfUniformSet.h
cvfVertexAttribute.h
cvfVertexBundle.h
cvfViewport.h
cvfRenderingScissor.h
)

set(CEE_SOURCE_FILES
cvfBufferObjectManaged.cpp
cvfCamera.cpp
cvfCameraAnimation.cpp
cvfDrawable.cpp
cvfDrawableGeo.cpp
cvfDrawableVectors.cpp
cvfDrawableText.cpp
cvfFramebufferObject.cpp
cvfFixedAtlasFont.cpp
cvfFont.cpp
cvfGeometryBuilderDrawableGeo.cpp
cvfGlyph.cpp
cvfHitDetail.cpp
cvfMatrixState.cpp
cvfOglRc.cpp
cvfOpenGLCapabilities.cpp
cvfOpenGLContext.cpp
cvfOpenGLContextGroup.cpp
cvfOpenGLResourceManager.cpp
cvfOpenGL.cpp
cvfOverlayAxisCross.cpp
cvfOverlayScalarMapperLegend.cpp
cvfOverlayColorLegend.cpp
cvfOverlayImage.cpp
cvfOverlayItem.cpp
cvfOverlayNavigationCube.cpp
cvfOverlayTextBox.cpp
cvfPrimitiveSet.cpp
cvfPrimitiveSetDirect.cpp
cvfPrimitiveSetIndexedUInt.cpp
cvfPrimitiveSetIndexedUShort.cpp
cvfPrimitiveSetIndexedUIntScoped.cpp
cvfPrimitiveSetIndexedUShortScoped.cpp
cvfRenderbufferObject.cpp
cvfRenderState.cpp
cvfRenderStateBlending.cpp
cvfRenderStateColorMask.cpp
cvfRenderStateCullFace.cpp
cvfRenderStateDepth.cpp
cvfRenderStateFrontFace.cpp
cvfRenderStateLine.cpp
cvfRenderStatePoint.cpp
cvfRenderStatePolygonMode.cpp
cvfRenderStatePolygonOffset.cpp
cvfRenderStateSet.cpp
cvfRenderStateStencil.cpp
cvfRenderStateTextureBindings.cpp
cvfRenderStateTracker.cpp
cvfRenderState_FF.cpp
cvfScalarMapper.cpp
cvfScalarMapperRangeBased.cpp
cvfScalarMapperContinuousLog.cpp
cvfScalarMapperContinuousLinear.cpp
cvfScalarMapperDiscreteLinear.cpp
cvfScalarMapperDiscreteLog.cpp
cvfScalarMapperUniformLevels.cpp
cvfSampler.cpp
cvfShader.cpp
cvfShaderSourceProvider.cpp
cvfShaderProgram.cpp
cvfShaderProgramGenerator.cpp
cvfShaderSourceRepository.cpp
cvfShaderSourceRepositoryFile.cpp
cvfTextDrawer.cpp
cvfTexture.cpp
cvfTexture2D_FF.cpp
cvfTextureImage.cpp
cvfUniform.cpp
cvfUniformSet.cpp
cvfVertexAttribute.cpp
cvfVertexBundle.cpp
cvfViewport.cpp
cvfRenderingScissor.cpp
)

add_library(${PROJECT_NAME} ${CEE_HEADER_FILES} ${CEE_SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries ( ${PROJECT_NAME}
    LibCore
    LibGeometry
)

set(PROJECT_FILES ${CEE_HEADER_FILES} ${CEE_SOURCE_FILES})
source_group("" FILES ${PROJECT_FILES})

#if (COMMAND ri_apply_cotire)
#  set_source_files_properties (cvfOpenGLCapabilities.cpp PROPERTIES COTIRE_EXCLUDED TRUE)
#  set_source_files_properties (cvfOpenGLContext.cpp PROPERTIES COTIRE_EXCLUDED TRUE)
#  set_source_files_properties (cvfOpenGLContextGroup.cpp PROPERTIES COTIRE_EXCLUDED TRUE)
#  set_source_files_properties (cvfOpenGLResourceManager.cpp PROPERTIES COTIRE_EXCLUDED TRUE)
#  set_source_files_properties (cvfOpenGL.cpp PROPERTIES COTIRE_EXCLUDED TRUE)
#  ri_apply_cotire()
#endif()
