//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
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
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#include "cvfLibCore.h"
#include "cvfLibRender.h"
#include "cvfLibViewing.h"

#include "SnippetFactoryBasis.h"
#include "snipMinimalModel.h"
#include "snipLargeSphereModel.h"
#include "snipSingleBox.h"
#include "snipRenderbufferTest.h"
#include "snipMergeGeo.h"
#include "snipRenderStateExperiments.h"
#include "snipStripsAndFans.h"
#include "snipManySpheres.h"
#include "snipBenchmarkPart1M.h"
#include "snipBenchmarkPart250K.h"
#include "snipBenchmarkManyPartsManyGeos.h"
#include "snipBenchmarkManyPartsFewGeos.h"
#include "snipBenchmarkPart5K.h"
#include "snipPartMerger.h"
#include "snipPicking.h"
#include "snipNavigation.h"
#include "snipManipulators.h"
#include "snipEnableMasks.h"
#include "snipRenderPriority.h"
#include "snipVisualPriority.h"
#include "snipMultiRenderingEffectOverride.h"
#include "snipGeometryCreation.h"
#include "snipHiddenLines.h"
#include "snipExtractedEdges.h"
#include "snipVertexColoring.h"
#include "snipScalarMapping.h"
#include "snipIncrementalRenderer.h"
#include "snipImmediateVbo.h"
#include "snipWiredShaders.h"
#include "snipShaderUniforms.h"
#include "snipShaderExperiments.h"
#include "snipShaderLighting.h"
#include "snipCoreProfileTest.h"
#include "snipTexturing.h"
#include "snipVectorDrawing.h"
#include "snipDepthPeeling.h"
#include "snipDepthPeelingFront.h"
#include "snipTransparentWeightedAverage.h"
#include "snipRenderToTexture.h"
#include "snipShadows.h"
#include "snipCubeMapping.h"
#include "snipBlendTest.h"
#include "snipEnvironmentMapping.h"
#include "snipDrawableAnimation.h"
#include "snipPointSprites.h"
#include "snipClipping.h"
#include "snipTutorial1.h"
#include "snipSingleTexturedQuad.h"
#include "snipVertexAttributes.h"
#include "snipLineDrawing.h"
#include "snipOverlayItems.h"
#include "snipLabels.h"
#include "snipTextDrawing.h"
#include "snipHighlight.h"
#include "snipStencil.h"
#include "snipImageFiltering.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
SnippetFactoryBasis::SnippetFactoryBasis()
{
    CVFU_REGISTER_SNIPPET(snip::MinimalModel,                 cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::OverlayItems,                 cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::LargeSphereModel,             cvfu::SC_TEST_HEAVY);
    CVFU_REGISTER_SNIPPET(snip::SingleBox,                    cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::RenderbufferTest,             cvfu::SC_EXPERIMENT);
    CVFU_REGISTER_SNIPPET(snip::MergeGeo,                     cvfu::SC_EXPERIMENT);
    CVFU_REGISTER_SNIPPET(snip::RenderStateExperiments,       cvfu::SC_EXPERIMENT);
    CVFU_REGISTER_SNIPPET(snip::StripsAndFans,                cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::ManySpheresModel,             cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::BenchmarkPart1M,              cvfu::SC_TEST_HEAVY);
    CVFU_REGISTER_SNIPPET(snip::BenchmarkPart250K,            cvfu::SC_TEST_HEAVY);
    CVFU_REGISTER_SNIPPET(snip::BenchmarkManyPartsManyGeos,   cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::BenchmarkManyPartsFewGeos,    cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::BenchmarkPart5K,              cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::PartMerger,                   cvfu::SC_EXPERIMENT);
    CVFU_REGISTER_SNIPPET(snip::Picking,                      cvfu::SC_EXPERIMENT);
    CVFU_REGISTER_SNIPPET(snip::Navigation,                   cvfu::SC_EXPERIMENT);
    CVFU_REGISTER_SNIPPET(snip::Manipulators,                 cvfu::SC_EXPERIMENT);
    CVFU_REGISTER_SNIPPET(snip::EnableMasks,                  cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::RenderPriority,               cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::VisualPriority,               cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::MultiRenderingEffectOverride, cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::GeometryCreation,             cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::HiddenLines,                  cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::ExtractedEdges,               cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::VertexColoring,               cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::ScalarMapping,                cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::IncrementalRenderer,          cvfu::SC_EXPERIMENT);
    CVFU_REGISTER_SNIPPET(snip::ImmediateVbo,                 cvfu::SC_EXPERIMENT);
    CVFU_REGISTER_SNIPPET(snip::WiredShaders,                 cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::ShaderUniforms,               cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::ShaderExperiments,            cvfu::SC_EXPERIMENT);
    CVFU_REGISTER_SNIPPET(snip::ShaderLighting,               cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::CoreProfileTest,              cvfu::SC_EXPERIMENT);
    CVFU_REGISTER_SNIPPET(snip::Texturing,                    cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::VectorDrawing,                cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::DepthPeeling,                 cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::DepthPeelingFront,            cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::TransparentWeightedAverage,   cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::RenderToTexture,              cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::Shadows,                      cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::CubeMapping,                  cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::BlendTest,                    cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::EnvironmentMapping,           cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::DrawableAnimation,            cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::Highlight,                    cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::Stencil,                      cvfu::SC_TEST);
    CVFU_REGISTER_SNIPPET(snip::PointSprites,                 cvfu::SC_EXPERIMENT);
    CVFU_REGISTER_SNIPPET(snip::Clipping,                     cvfu::SC_EXPERIMENT);
    CVFU_REGISTER_SNIPPET(snip::Tutorial1,                    cvfu::SC_EXPERIMENT);
    CVFU_REGISTER_SNIPPET(snip::SingleTexturedQuad,           cvfu::SC_EXPERIMENT);
    CVFU_REGISTER_SNIPPET(snip::VertexAttributes,             cvfu::SC_EXPERIMENT);
    CVFU_REGISTER_SNIPPET(snip::LineDrawing,                  cvfu::SC_EXPERIMENT);
    CVFU_REGISTER_SNIPPET(snip::ImageFiltering,               cvfu::SC_EXPERIMENT);

    CVFU_REGISTER_SNIPPET(snip::Labels,                       cvfu::SC_EXPERIMENT);
    CVFU_REGISTER_SNIPPET(snip::TextDrawing,                  cvfu::SC_TEST);
}
