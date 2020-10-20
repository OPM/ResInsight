// clang-format off

#include "cafTransparentWBRenderConfiguration.h"

#include "cvfDynamicUniformSet.h"
#include "cvfLibCore.h"
#include "cvfLibRender.h"
#include "cvfLibViewing.h"
#include "cvfRenderQueueSorter.h"

#include "TranspWB_Shaders.h"

namespace caf
{
using namespace cvf;

/*
    This is an implementation of
    Weighted Blended Order-Independent Transparency
    based on
    Morgan McGuire and Louis Bavoil NVIDIA
    Journal of Computer Graphics Techniques Vol. 2, No. 2, 2013

    It can be developed further to support completely unsorted and partially transparent objects
    by making the first pass include all parts, but discard transparent fragments,
    while the transparency pass discards all opaque fragments.
    This should be quite possible. (Experimentally done now)

    A more experimental idea To improve high opaque transparent object looks, and make them
    hide the transparent object behind, better,
    we can set a threshold on the opacity (above 0.9 e.g), and limis it contribution to the
    blended color to just include one (possible) transparent layer behind it.
    To do this we will try to make the Z-buffer for the transparent pass contain the
    depth of the fragment behind the one with opaqueThreshold < alpha < 1.0

    To do this we need to do somewhat the following (experimental idea):
    P0 Opaque Pass => TB: OpaqueBuffer, DB: DepthBufferOpaque, TB: DepthBufferToPeel
        Including all parts
        Discarding all transparent fragments  with alpha < opaqueThreshold
        Opaque fragments rendered normally, also write to depth to DepthbufferToPeel
        If (opaqueThreshold < alpha < 1.0) discard color, write depth to DepthBufferToPeel
        




    P1 Peel Pass (TB: DepthBufferToPeel) ==> PeeledDepthBuffer
        Discarding all transparent fragments  with alpha < opaqueThreshold
        Discard all fragments with depth <= DepthBufferToPeel,
        Normal depth test

    P3 Transparent Pass (DB: DepthBufferOpaque, TB: PeeledDepthBuffer) => WeightedSumColorAndAlpha, ProdOneMinusAlpha
       This pass is the mostly the same as the one described in the original method except (No write to depth buffer)
       Discard Opaque fragments
       if (depth > PeeledDepthBuffer) discard

    P4 Combination pass

    ====================================
     Usage
    ====================================
    class MyViewer : public caf::Viewer
    {
        Q_OBJECT;
    public:
        MyViewer(const QGLFormat& format, QWidget* parent)
        : caf::Viewer(format, parent)
        {
            m_renderConf = new caf::TransparentWBRenderConfiguration;
            m_renderConf->setUpRenderSequence(m_renderingSequence.p());
        




            // Cerate overlay item if needed
            cvf::OverlayItem* overlayItem; // = new someTtem
            m_renderConf->overlayRendering()->addOverlayItem(overlayItem);
        }
    




        ~MyViewer();
    




        virtual void optimizeClippingPlanes()
        {
            // ... Do ordinary clipplane adjustments
    




            m_renderConf->prepareForRendering();
        }
        virtual void resizeGL(int width, int height)
        {
            m_renderConf->resize(width, height);
            caf::Viewer::resizeGL(width, height);
        }
    




    private:
        cvf::ref<caf::TransparentWBRenderConfiguration> m_renderConf;
    };

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    




    ref<Part> createFacePartFromDrawableGeo(cvf::DrawableGeo* geo,
                                            const Color3f& color,
                                            float opacity,
                                            bool useSpecularReflection)
    {
        ref<Part> part = new Part;
        part->setDrawable(geo);
    




        cvf::Color4f colorWithAlpha(color);
        colorWithAlpha.a() = opacity;
    




        caf::WBTransparencySurfaceEffectGenerator effGen(colorWithAlpha, caf::PO_NONE, useSpecularReflection);
        ref<Effect> eff = effGen.generateEffectFromCache();
    




        if (opacity < 1.0)
        {
            part->setPriority(100);
        }
    




        part->setEffect(eff.p());
    




        return part;
    }




*/

class RenderPassPreparator : public cvf::DynamicUniformSet
{
public:
    explicit RenderPassPreparator( TransparentWBRenderConfiguration* renderConfiguration )
    {
        CVF_ASSERT( renderConfiguration );
        m_renderConfiguration = renderConfiguration;
    }
    cvf::UniformSet* uniformSet() override { return nullptr; }
    void update( cvf::Rendering* rendering ) override { m_renderConfiguration->updateEffectsForRendering( rendering ); }

private:
    TransparentWBRenderConfiguration* m_renderConfiguration;
};

class RenderQueueFilter : public cvf::RenderQueueSorterBasic
{
public:
    RenderQueueFilter( cvf::RenderStateBlending*                 blendingToRespect,
                       bool                                      isOpaquePass,
                       cvf::RenderQueueSorterBasic::SortStrategy sortStrategy )
        : cvf::RenderQueueSorterBasic( sortStrategy )
    {
        CVF_ASSERT( blendingToRespect );

        m_blendingToRespect = blendingToRespect;
        m_isOpaquePass      = isOpaquePass;
    }

    void sort( cvf::RenderQueue* renderQueue ) const override
    {
        using namespace cvf;
        std::vector<RenderItem*>* renderItems = renderQueue->renderItemsForSorting();
        std::vector<RenderItem*>  filteredRenderItems;
        filteredRenderItems.reserve( renderItems->size() );

        for ( size_t riIdx = 0; riIdx < renderItems->size(); ++riIdx )
        {
            bool hasBlending = ( *renderItems )[riIdx]->effect()->renderStateOfType( cvf::RenderState::BLENDING ) ==
                               m_blendingToRespect.p();
            if ( m_isOpaquePass && !hasBlending )
            {
                filteredRenderItems.push_back( ( *renderItems )[riIdx] );
            }
            if ( !m_isOpaquePass && hasBlending )
            {
                filteredRenderItems.push_back( ( *renderItems )[riIdx] );
            }
        }

        renderItems->swap( filteredRenderItems );

        cvf::RenderQueueSorterBasic::sort( renderQueue );
    }

private:
    cvf::ref<cvf::RenderStateBlending> m_blendingToRespect;
    bool                               m_isOpaquePass;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TransparentWBRenderConfiguration::TransparentWBRenderConfiguration()
{
    // Initialize effect system
    WBTransparencySurfaceEffectGenerator::initStaticData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TransparentWBRenderConfiguration::~TransparentWBRenderConfiguration()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TransparentWBRenderConfiguration::setUpRenderSequence( cvf::RenderSequence* renderSeq )
{
    if ( EffectGenerator::renderingMode() == EffectGenerator::SHADER_BASED )
    {
        // Set up opaque pass
        ref<RenderbufferObject> depthBuffer = new RenderbufferObject( RenderbufferObject::DEPTH_COMPONENT24, 1, 1 );
        ref<Texture>            opaceColorTexture = new Texture( Texture::TEXTURE_RECTANGLE, Texture::RGBA32F );
        ref<Texture>            sumWeightedRgbAndProductOneMinusAlphaTexture =
            new Texture( Texture::TEXTURE_RECTANGLE, Texture::RGBA32F );
        ref<Texture> sumWeightedAlphaTexture = new Texture( Texture::TEXTURE_RECTANGLE, Texture::R32F );

        ref<RenderPassPreparator> renderPassPrep = new RenderPassPreparator( this );

        {
            m_opaceRendering = renderSeq->firstRendering();
            CVF_ASSERT( m_opaceRendering.notNull() );

            m_opaceRenderingFbo = new FramebufferObject;

            m_opaceRenderingFbo->attachDepthRenderbuffer( depthBuffer.p() );

            opaceColorTexture->setSize( 1, 1 );
            m_opaceRenderingFbo->attachColorTexture2d( 0, opaceColorTexture.p() );

            m_opaceRendering->setTargetFramebuffer( m_opaceRenderingFbo.p() );
            // m_opaceRendering->camera()->viewport()->setClearColor(Color4f(0.69f, 0.77f, 0.87f, 1.0f));
            m_opaceRendering->addDynamicUniformSet( renderPassPrep.p() );
            m_opaceRendering->setRenderQueueSorter(
                new RenderQueueFilter( WBTransparencySurfaceEffectGenerator::m_blending.p(),
                                       true,
                                       RenderQueueSorterBasic::EFFECT_ONLY ) );
        }

        // Set up transparency pass
        {
            // Frame buffer
            m_transparencyFbo = new FramebufferObject;

            m_transparencyFbo->attachDepthRenderbuffer( depthBuffer.p() );

            sumWeightedRgbAndProductOneMinusAlphaTexture->setSize( 1, 1 );
            m_transparencyFbo->attachColorTexture2d( 0, sumWeightedRgbAndProductOneMinusAlphaTexture.p() );

            sumWeightedAlphaTexture->setSize( 1, 1 );
            m_transparencyFbo->attachColorTexture2d( 1, sumWeightedAlphaTexture.p() );

            // Rendering
            m_transparentRendering = new Rendering;

            m_transparentRendering->setTargetFramebuffer( m_transparencyFbo.p() );
            ref<Camera> transpCam = new Camera;
            transpCam->viewport()->setClearColor( Color4f( 0, 0, 0, 1 ) );
            copyCameraView( renderSeq->firstRendering()->camera(), transpCam.p() );
            m_transparentRendering->setCamera( transpCam.p() );
            m_transparentRendering->setClearMode( Viewport::CLEAR_COLOR );

            renderSeq->addRendering( m_transparentRendering.p() );
            m_transparentRendering->addDynamicUniformSet( renderPassPrep.p() );
            m_transparentRendering->setRenderQueueSorter(
                new RenderQueueFilter( WBTransparencySurfaceEffectGenerator::m_blending.p(),
                                       false,
                                       RenderQueueSorterBasic::EFFECT_ONLY ) );
        }

        // Setup last rendering combining the textures on the screen
        // -------------------------------------------------------------------------
        {
            SingleQuadRenderingGenerator quadRenderGen;
            ref<Sampler>                 sampler = new Sampler;
            sampler->setWrapMode( cvf::Sampler::CLAMP_TO_EDGE );
            sampler->setMinFilter( cvf::Sampler::NEAREST );
            sampler->setMagFilter( cvf::Sampler::NEAREST );

            quadRenderGen.addTexture( sumWeightedRgbAndProductOneMinusAlphaTexture.p(),
                                      sampler.p(),
                                      "sumWeightedRgbAndProductOneMinusAlphaTexture" );
            quadRenderGen.addTexture( sumWeightedAlphaTexture.p(), sampler.p(), "sumWeightedAlphaTexture" );
            quadRenderGen.addTexture( opaceColorTexture.p(), sampler.p(), "opaceColorTexture" );

            quadRenderGen.addFragmentShaderCode( String( TranspWB_CombinationFrag_inl ) );
            m_combinationRendering = quadRenderGen.generate();
            renderSeq->addRendering( m_combinationRendering.p() );
            m_combinationRendering->addDynamicUniformSet( renderPassPrep.p() );
        }
    }
    else
    {
        m_opaceRendering = renderSeq->firstRendering();
        m_opaceRendering->setRenderQueueSorter(
            new RenderQueueFilter( WBTransparencySurfaceEffectGenerator::m_oGL11Blending.p(),
                                   true,
                                   RenderQueueSorterBasic::EFFECT_ONLY ) );

        m_transparentRendering = new Rendering;
        ref<Camera> transpCam  = new Camera;
        copyCameraView( renderSeq->firstRendering()->camera(), transpCam.p() );
        m_transparentRendering->setCamera( transpCam.p() );
        m_transparentRendering->setClearMode( Viewport::DO_NOT_CLEAR );

        renderSeq->addRendering( m_transparentRendering.p() );
        m_transparentRendering->setRenderQueueSorter(
            new RenderQueueFilter( WBTransparencySurfaceEffectGenerator::m_oGL11Blending.p(),
                                   false,
                                   RenderQueueSorterBasic::BACK_TO_FRONT ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TransparentWBRenderConfiguration::resize( int width, int height )
{
    // Resize the FBO (and the rendering viewports)

    m_opaceRendering->camera()->viewport()->set( 0, 0, width, height );
    m_transparentRendering->camera()->viewport()->set( 0, 0, width, height );

    if ( EffectGenerator::renderingMode() == EffectGenerator::SHADER_BASED )
    {
        m_combinationRendering->camera()->viewport()->set( 0, 0, width, height );

        m_opaceRenderingFbo->resizeAttachedBuffers( width, height );
        m_transparencyFbo->resizeAttachedBuffers( width, height );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TransparentWBRenderConfiguration::copyCameraView( Camera* srcCamera, Camera* dstCamera )
{
    if ( srcCamera->projection() == Camera::PERSPECTIVE )
    {
        dstCamera->setProjectionAsPerspective( srcCamera->fieldOfViewYDeg(), srcCamera->nearPlane(), srcCamera->farPlane() );
    }
    else
    {
        dstCamera->setProjectionAsOrtho( srcCamera->frontPlaneFrustumHeight(),
                                         srcCamera->nearPlane(),
                                         srcCamera->farPlane() );
    }

    dstCamera->setViewMatrix( srcCamera->viewMatrix() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TransparentWBRenderConfiguration::prepareForRendering()
{
    m_transparentRendering->setScene( m_opaceRendering->scene() );
    copyCameraView( m_opaceRendering->camera(), m_transparentRendering->camera() );
    WBTransparencySurfaceEffectGenerator::m_cameraNearUniform->set( (float)( m_opaceRendering->camera()->nearPlane() ) );
    WBTransparencySurfaceEffectGenerator::m_cameraFarUniform->set( (float)( m_opaceRendering->camera()->farPlane() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void TransparentWBRenderConfiguration::updateEffectsForRendering( Rendering* rendering )
{
    if ( EffectGenerator::renderingMode() == EffectGenerator::SHADER_BASED )
    {
        // This is to support partially transparent objects. So not yet actually operational.
        if ( rendering == m_opaceRendering )
        {
            WBTransparencySurfaceEffectGenerator::m_renderPassUniform->set( 1 );
            WBTransparencySurfaceEffectGenerator::m_blending->enableBlending( false );
            WBTransparencySurfaceEffectGenerator::m_depth->enableDepthWrite( true );
        }
        else if ( rendering == m_transparentRendering )
        {
            WBTransparencySurfaceEffectGenerator::m_renderPassUniform->set( 0 );
            WBTransparencySurfaceEffectGenerator::m_blending->enableBlending( true );
            WBTransparencySurfaceEffectGenerator::m_depth->enableDepthWrite( false );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Rendering> TransparentWBRenderConfiguration::overlayRendering()
{
    if ( EffectGenerator::renderingMode() == EffectGenerator::SHADER_BASED )
    {
        return m_combinationRendering;
    }
    else
    {
        return m_transparentRendering;
    }
}

//==================================================================================================
//
// WBTransparencySurfaceEffectGenerator
//
//==================================================================================================

cvf::ref<cvf::ShaderProgram>       WBTransparencySurfaceEffectGenerator::m_shaderForTransparentParts;
cvf::ref<cvf::ShaderProgram>       WBTransparencySurfaceEffectGenerator::m_shaderForOpaqueParts;
cvf::ref<cvf::ShaderProgram>       WBTransparencySurfaceEffectGenerator::m_shaderForTransparentPartsSpec;
cvf::ref<cvf::ShaderProgram>       WBTransparencySurfaceEffectGenerator::m_shaderForOpaquePartsSpec;
cvf::ref<cvf::UniformInt>          WBTransparencySurfaceEffectGenerator::m_renderPassUniform;
cvf::ref<cvf::RenderStateBlending> WBTransparencySurfaceEffectGenerator::m_blending;
cvf::ref<cvf::RenderStateBlending> WBTransparencySurfaceEffectGenerator::m_oGL11Blending;
cvf::ref<cvf::RenderStateDepth>    WBTransparencySurfaceEffectGenerator::m_depth;

cvf::ref<cvf::UniformFloat> WBTransparencySurfaceEffectGenerator::m_cameraNearUniform;
cvf::ref<cvf::UniformFloat> WBTransparencySurfaceEffectGenerator::m_cameraFarUniform;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
WBTransparencySurfaceEffectGenerator::WBTransparencySurfaceEffectGenerator( const cvf::Color4f& color,
                                                                            PolygonOffset       polygonOffset,
                                                                            bool                useSpecular )
{
    m_color            = color;
    m_polygonOffset    = polygonOffset;
    m_useSpecularColor = useSpecular;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WBTransparencySurfaceEffectGenerator::updateForShaderBasedRendering( cvf::Effect* effect ) const
{
    if ( m_color.a() < 1.0f )
    {
        if ( m_useSpecularColor )
        {
            effect->setShaderProgram( m_shaderForTransparentPartsSpec.p() );
        }
        else
        {
            effect->setShaderProgram( m_shaderForTransparentParts.p() );
        }
        // effect->setUniform(m_renderPassUniform.p()); // To be used in effects for partially transparent objects
        effect->setUniform( m_cameraNearUniform.p() );
        effect->setUniform( m_cameraFarUniform.p() );
    }
    else
    {
        if ( m_useSpecularColor )
        {
            effect->setShaderProgram( m_shaderForOpaquePartsSpec.p() );
        }
        else
        {
            effect->setShaderProgram( m_shaderForOpaqueParts.p() );
        }
    }

    effect->setUniform( new cvf::UniformFloat( "u_color", m_color ) );

    if ( m_color.a() < 1.0f )
    {
        effect->setRenderState( m_blending.p() );
    }

    this->updateCommonEffect( effect );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WBTransparencySurfaceEffectGenerator::updateForFixedFunctionRendering( cvf::Effect* effect ) const
{
    cvf::ref<cvf::Effect> eff = effect;

    cvf::ref<cvf::RenderStateMaterial_FF> mat = new cvf::RenderStateMaterial_FF( m_color.toColor3f() );
    mat->setAlpha( m_color.a() );

    eff->setRenderState( mat.p() );

    cvf::ref<cvf::RenderStateLighting_FF> lighting = new cvf::RenderStateLighting_FF;
    lighting->enableTwoSided( true );
    eff->setRenderState( lighting.p() );

    if ( m_color.a() < 1.0f )
    {
        effect->setRenderState( m_oGL11Blending.p() );
    }

    this->updateCommonEffect( effect );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WBTransparencySurfaceEffectGenerator::updateCommonEffect( cvf::Effect* effect ) const
{
    if ( m_polygonOffset != PO_NONE )
    {
        cvf::ref<cvf::RenderStatePolygonOffset> polyOffset =
            EffectGenerator::createAndConfigurePolygonOffsetRenderState( m_polygonOffset );
        effect->setRenderState( polyOffset.p() );
    }

    if ( m_color.a() < 1.0f )
    {
        effect->setRenderState( m_depth.p() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool WBTransparencySurfaceEffectGenerator::isEqual( const EffectGenerator* other ) const
{
    const WBTransparencySurfaceEffectGenerator* otherSurfaceEffect =
        dynamic_cast<const WBTransparencySurfaceEffectGenerator*>( other );

    if ( otherSurfaceEffect )
    {
        if ( m_color == otherSurfaceEffect->m_color && m_polygonOffset == otherSurfaceEffect->m_polygonOffset &&
             m_useSpecularColor == otherSurfaceEffect->m_useSpecularColor )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
EffectGenerator* WBTransparencySurfaceEffectGenerator::copy() const
{
    WBTransparencySurfaceEffectGenerator* effGen =
        new WBTransparencySurfaceEffectGenerator( m_color, m_polygonOffset, m_useSpecularColor );

    return effGen;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WBTransparencySurfaceEffectGenerator::initStaticData()
{
    if ( m_blending.notNull() ) return;

    {
        ShaderProgramGenerator transpPartsShaderGen( "TranspPartsShader", ShaderSourceProvider::instance() );
        transpPartsShaderGen.addVertexCode( cvf::ShaderSourceRepository::vs_Standard );
        transpPartsShaderGen.addFragmentCode( ShaderSourceRepository::src_Color );
        transpPartsShaderGen.addFragmentCode( CommonShaderSources::light_AmbientDiffuse() );

        transpPartsShaderGen.addFragmentCode( String( TranspWB_TransparentPartsFrag_inl ) );

        m_shaderForTransparentParts = transpPartsShaderGen.generate();
    }
    {
        ShaderProgramGenerator transpPartsShaderGen( "TranspPartsShader", ShaderSourceProvider::instance() );
        transpPartsShaderGen.addVertexCode( cvf::ShaderSourceRepository::vs_Standard );
        transpPartsShaderGen.addFragmentCode( ShaderSourceRepository::src_Color );
        transpPartsShaderGen.addFragmentCode( ShaderSourceRepository::light_SimpleHeadlight );

        transpPartsShaderGen.addFragmentCode( String( TranspWB_TransparentPartsFrag_inl ) );

        m_shaderForTransparentPartsSpec = transpPartsShaderGen.generate();
    }

#if 0 // Code to use when implementing support for partial transparency, but supposedly in a different effect generator
    {
    // Shader for partly transparent objects

    ShaderProgramGenerator partlyTranspShaderGen("CommonShader", ShaderSourceProvider::instance());
    partlyTranspShaderGen.addVertexCode(cvf::ShaderSourceRepository::vs_Standard);
    partlyTranspShaderGen.addFragmentCode(ShaderSourceRepository::src_Color);
    partlyTranspShaderGen.addFragmentCode(ShaderSourceRepository::light_SimpleHeadlight);
    partlyTranspShaderGen.addFragmentCode(String(TranspWB_PartlyTranspPartsFrag_inl));

    m_partlyTranspPartShader = partlyTranspShaderGen.generate();
    }
#endif

    {
        ShaderProgramGenerator opaquePartsShaderGen( "OpaquePartsShader", ShaderSourceProvider::instance() );
        opaquePartsShaderGen.addVertexCode( cvf::ShaderSourceRepository::vs_Standard );
        opaquePartsShaderGen.addFragmentCode( ShaderSourceRepository::src_Color );
        opaquePartsShaderGen.addFragmentCode( CommonShaderSources::light_AmbientDiffuse() );
        opaquePartsShaderGen.addFragmentCode( ShaderSourceRepository::fs_Standard );
        m_shaderForOpaqueParts = opaquePartsShaderGen.generate();
    }
    {
        ShaderProgramGenerator opaquePartsShaderGen( "OpaquePartsShader", ShaderSourceProvider::instance() );
        opaquePartsShaderGen.addVertexCode( cvf::ShaderSourceRepository::vs_Standard );
        opaquePartsShaderGen.addFragmentCode( ShaderSourceRepository::src_Color );
        opaquePartsShaderGen.addFragmentCode( ShaderSourceRepository::light_SimpleHeadlight );
        opaquePartsShaderGen.addFragmentCode( ShaderSourceRepository::fs_Standard );
        m_shaderForOpaquePartsSpec = opaquePartsShaderGen.generate();
    }

    m_blending = new RenderStateBlending;
    m_blending->setFunctionSeparate( RenderStateBlending::ONE,
                                     RenderStateBlending::ONE,
                                     RenderStateBlending::ZERO,
                                     RenderStateBlending::ONE_MINUS_SRC_ALPHA );
    m_blending->setEquation( RenderStateBlending::FUNC_ADD );
    m_blending->enableBlending( true );

    m_oGL11Blending = new RenderStateBlending;
    m_oGL11Blending->configureTransparencyBlending();

    m_depth = new RenderStateDepth;
    m_depth->enableDepthWrite( false );

    m_renderPassUniform = new UniformInt( "isOpaquePass", 1 );
    m_cameraNearUniform = new UniformFloat( "cameraNear", 0.01f );
    m_cameraFarUniform  = new UniformFloat( "cameraFar", 1000 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
WBTransparencySurfaceEffectGenerator::~WBTransparencySurfaceEffectGenerator()
{
}

} // namespace caf

// clang-format off
