cmake_minimum_required(VERSION 2.8)

project(SnippetsBasis)


# Just use base compile flags for the snippets
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${CEE_BASE_CXX_FLAGS}")


include_directories(${LibCore_SOURCE_DIR})
include_directories(${LibIo_SOURCE_DIR})
include_directories(${LibRender_SOURCE_DIR})
include_directories(${LibGeometry_SOURCE_DIR})
include_directories(${LibViewing_SOURCE_DIR})
include_directories(${LibUtilities_SOURCE_DIR})
include_directories(${LibFreeType_SOURCE_DIR})

set(CEE_SOURCE_FILES
snipBenchmarkManyPartsFewGeos.cpp
snipBenchmarkManyPartsManyGeos.cpp
snipBenchmarkPart1M.cpp
snipBenchmarkPart250K.cpp
snipBenchmarkPart5K.cpp
snipBlendTest.cpp
snipClipping.cpp
snipCoreProfileTest.cpp
snipCubeMapping.cpp
snipDepthPeeling.cpp
snipDepthPeelingFront.cpp
snipDrawableAnimation.cpp
snipEnableMasks.cpp
snipEnvironmentMapping.cpp
snipExtractedEdges.cpp
snipGeometryCreation.cpp
snipHiddenLines.cpp
snipHighlight.cpp
snipImageFiltering.cpp
snipImmediateVbo.cpp
snipIncrementalRenderer.cpp
snipLabels.cpp
snipLargeSphereModel.cpp
snipLineDrawing.cpp
snipManipulators.cpp
snipManySpheres.cpp
snipMergeGeo.cpp
snipMinimalModel.cpp
snipMultiRenderingEffectOverride.cpp
snipNavigation.cpp
snipOverlayItems.cpp
snipPartMerger.cpp
snipPicking.cpp
snipPointSprites.cpp
snipRenderbufferTest.cpp
snipRenderPriority.cpp
snipRenderStateExperiments.cpp
snipRenderToTexture.cpp
snipScalarMapping.cpp
snipShaderExperiments.cpp
snipShaderLighting.cpp
snipShaderUniforms.cpp
snipShadows.cpp
snipSingleBox.cpp
snipStripsAndFans.cpp
snipSingleTexturedQuad.cpp
snipStencil.cpp
snipTextDrawing.cpp
snipTexturing.cpp
snipTransparentWeightedAverage.cpp
snipTutorial1.cpp
snipVectorDrawing.cpp
snipVertexAttributes.cpp
snipVertexColoring.cpp
snipVisualPriority.cpp
snipWiredShaders.cpp
SnippetFactoryBasis.cpp
)

add_library(${PROJECT_NAME} ${CEE_SOURCE_FILES})
