//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
//   This library is free software: you can redistribute it and/or modify 
//   it under the terms of the GNU General Public License as published by 
//   the Free Software Foundation, either version 3 of the License, or 
//   (at your option) any later version. 
//    
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY 
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or 
//   FITNESS FOR A PARTICULAR PURPOSE.   
//    
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>> 
//   for more details. 
//
//##################################################################################################

// Doxygen module definition
/// \ingroup CeeVizFramework
/// @{
/// \defgroup Render Render module
/// @}

#include "cvfBufferObjectManaged.h"
#include "cvfCamera.h"
#include "cvfDrawable.h"
#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfDrawableVectors.h"
#include "cvfFixedAtlasFont.h"
#include "cvfFont.h"
#include "cvfFontManager.h"
#include "cvfFramebufferObject.h"
#include "cvfGeometryBuilderDrawableGeo.h"
#include "cvfGlyph.h"
#include "cvfHitDetail.h"
#include "cvfMatrixState.h"
#include "cvfOpenGL.h"
#include "cvfOglRc.h"
#include "cvfOpenGLCapabilities.h"
#include "cvfOpenGLContext.h"
#include "cvfOpenGLContextGroup.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfOpenGLTypes.h"
#include "cvfOverlayAxisCross.h"
#include "cvfOverlayColorLegend.h"
#include "cvfOverlayItem.h"
#include "cvfOverlayTextBox.h"
#include "cvfPrimitiveSet.h"
#include "cvfPrimitiveSetDirect.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfPrimitiveSetIndexedUShort.h"
#include "cvfPrimitiveSetIndexedUIntScoped.h"
#include "cvfRenderbufferObject.h"
#include "cvfRenderState.h"
#include "cvfRenderStateSet.h"
#include "cvfRenderStateTracker.h"
#include "cvfSampler.h"
#include "cvfScalarMapper.h"
#include "cvfScalarMapperUniformLevels.h"
#include "cvfShader.h"
#include "cvfShaderSourceProvider.h"
#include "cvfShaderProgram.h"
#include "cvfShaderProgramGenerator.h"
#include "cvfShaderSourceRepository.h"
#include "cvfShaderSourceRepositoryFile.h"
#include "cvfTexture.h"
#include "cvfTextureImage.h"
#include "cvfUniform.h"
#include "cvfUniformSet.h"
#include "cvfVertexAttribute.h"
#include "cvfVertexBundle.h"
#include "cvfViewport.h"

#ifndef CVF_OPENGL_ES
#include "cvfRenderState_FF.h"
#include "cvfTexture2D_FF.h"
#endif
