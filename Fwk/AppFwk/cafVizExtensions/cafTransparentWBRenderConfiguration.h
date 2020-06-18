
#include "cafEffectGenerator.h"
#include "cvfBase.h"
#include "cvfColor4.h"
#include "cvfObject.h"
#include "cvfRendering.h"
#include "cvfString.h"

namespace cvf
{
class Rendering;
class Scene;
class Camera;
class RenderSequence;
class FramebufferObject;
class UniformInt;
class UniformFloat;
class Effect;
class RenderStateBlending;
class RenderStateDepth;
class ShaderProgram;
} // namespace cvf

namespace caf
{
class TransparentWBRenderConfiguration : public cvf::Object
{
public:
    TransparentWBRenderConfiguration();
    ~TransparentWBRenderConfiguration() override;

    void resize( int width, int height );
    void prepareForRendering(); // UpdateCameras and scene ...

    void setUpRenderSequence( cvf::RenderSequence* renderSeq );

    // cvf::ref<cvf::Effect> transparentPartEffect(const cvf::Color4f& color);
    // void setupTransparentPartEffect(cvf::Effect* effectToModify, ShaderProgramGenerator& shaderGen);

    cvf::ref<cvf::Rendering> overlayRendering();

private:
    // void initShaders();
    static void copyCameraView( cvf::Camera* srcCamera, cvf::Camera* dstCamera );
    friend class RenderPassPreparator;
    void updateEffectsForRendering( cvf::Rendering* rendering );

    cvf::ref<cvf::Rendering> m_opaceRendering;
    cvf::ref<cvf::Rendering> m_transparentRendering;
    cvf::ref<cvf::Rendering> m_combinationRendering;

    cvf::ref<cvf::FramebufferObject> m_opaceRenderingFbo;
    cvf::ref<cvf::FramebufferObject> m_transparencyFbo;
#if 0
        cvf::ref<cvf::UniformInt>         m_renderPassUniform;
        cvf::ref<cvf::RenderStateBlending> m_blending;
        cvf::ref<cvf::RenderStateDepth>   m_depth;

        cvf::ref<cvf::ShaderProgram>      m_shaderForTransparentParts;
        cvf::ref<cvf::ShaderProgram>      m_simpleSolidShader;
        cvf::ref<cvf::ShaderProgram>      m_partlyTranspPartShader;
#endif

    cvf::String m_finalFragShaderCode;
};

//==================================================================================================
//
// SurfaceEffectGenerator
//
//==================================================================================================
class WBTransparencySurfaceEffectGenerator : public EffectGenerator
{
public:
    WBTransparencySurfaceEffectGenerator( const cvf::Color4f& color, caf::PolygonOffset polygonOffset, bool useSpecular );
    ~WBTransparencySurfaceEffectGenerator() override;

protected:
    bool             isEqual( const EffectGenerator* other ) const override;
    EffectGenerator* copy() const override;

    void updateForShaderBasedRendering( cvf::Effect* effect ) const override;
    void updateForFixedFunctionRendering( cvf::Effect* effect ) const override;

private:
    void        updateCommonEffect( cvf::Effect* effect ) const;
    static void initStaticData();

private:
    cvf::Color4f       m_color;
    caf::PolygonOffset m_polygonOffset;
    bool               m_useSpecularColor;
    friend class TransparentWBRenderConfiguration;

    static cvf::ref<cvf::ShaderProgram> m_shaderForTransparentParts;
    static cvf::ref<cvf::ShaderProgram> m_shaderForTransparentPartsSpec;
    static cvf::ref<cvf::ShaderProgram> m_shaderForOpaqueParts;
    static cvf::ref<cvf::ShaderProgram> m_shaderForOpaquePartsSpec;

    static cvf::ref<cvf::UniformInt>          m_renderPassUniform;
    static cvf::ref<cvf::RenderStateBlending> m_blending;
    static cvf::ref<cvf::RenderStateBlending> m_oGL11Blending;

    static cvf::ref<cvf::UniformFloat> m_cameraNearUniform;
    static cvf::ref<cvf::UniformFloat> m_cameraFarUniform;

    static cvf::ref<cvf::RenderStateDepth> m_depth;
};

} // namespace caf