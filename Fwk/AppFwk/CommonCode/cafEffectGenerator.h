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

#pragma once

#include "cvfBase.h"
#include "cvfCollection.h"
#include "cvfEffect.h"
#include "cvfObject.h"
#include "cvfRenderStatePolygonOffset.h"
#include "cvfScalarMapper.h"
#include "cvfString.h"
#include "cvfTextureImage.h"

namespace cvf
{
class RenderStatePolygonOffset;
}

namespace caf
{
class CommonShaderSources
{
public:
    static cvf::String light_AmbientDiffuse();
};

// Enumerates different levels of polygon offsetting
enum PolygonOffset
{
    PO_NONE, // No polygon offset
    PO_1, // 'Normal' positive polygon offset, equal to configurePolygonPositiveOffset(), ie factor=unit=1.0
    PO_2, // More positive offset
    PO_POS_LARGE, // Super high poly offset for special circumstances
    PO_NEG_LARGE // Currently, a large negative offset
};

// Enumerates face culling
enum FaceCulling
{
    FC_BACK,
    FC_FRONT,
    FC_FRONT_AND_BACK,
    FC_NONE
};

//==================================================================================================
//
//
//
//==================================================================================================
class EffectGenerator
{
public:
    enum RenderingModeType
    {
        FIXED_FUNCTION,
        SHADER_BASED
    };

    EffectGenerator() {}
    virtual ~EffectGenerator() {}

    cvf::ref<cvf::Effect> generateUnCachedEffect() const;
    cvf::ref<cvf::Effect> generateCachedEffect() const;
    void                  updateEffect( cvf::Effect* effect ) const;

    static void              setRenderingMode( RenderingModeType effectType );
    static RenderingModeType renderingMode();

    static void clearEffectCache();
    static void releaseUnreferencedEffects();

    static cvf::ref<cvf::RenderStatePolygonOffset>
        createAndConfigurePolygonOffsetRenderState( caf::PolygonOffset polygonOffset );

protected:
    // Interface that must be implemented in base classes
    virtual bool             isEqual( const EffectGenerator* other ) const = 0;
    virtual EffectGenerator* copy() const                                  = 0;
    friend class EffectCache;

    // When these are called, the effect is already cleared by updateEffect()
    virtual void updateForShaderBasedRendering( cvf::Effect* effect ) const   = 0;
    virtual void updateForFixedFunctionRendering( cvf::Effect* effect ) const = 0;

private:
    static RenderingModeType sm_renderingMode;
};

//==================================================================================================
//
// SurfaceEffectGenerator
//
//==================================================================================================
class SurfaceEffectGenerator : public EffectGenerator
{
public:
    SurfaceEffectGenerator( const cvf::Color4f& color, PolygonOffset polygonOffset );
    SurfaceEffectGenerator( const cvf::Color3f& color, PolygonOffset polygonOffset );

    void setCullBackfaces( FaceCulling cullBackFaces ) { m_cullBackfaces = cullBackFaces; }

    void enableColorMask( bool enableColors ) { m_enableColorMask = enableColors; }
    void enableDepthTest( bool enableTest ) { m_enableDepthTest = enableTest; }
    void enableDepthWrite( bool enableWrite ) { m_enableDepthWrite = enableWrite; }
    void enableLighting( bool enableLighting ) { m_enableLighting = enableLighting; }

protected:
    bool             isEqual( const EffectGenerator* other ) const override;
    EffectGenerator* copy() const override;

    void updateForShaderBasedRendering( cvf::Effect* effect ) const override;
    void updateForFixedFunctionRendering( cvf::Effect* effect ) const override;

private:
    void updateCommonEffect( cvf::Effect* effect ) const;

private:
    cvf::Color4f  m_color;
    PolygonOffset m_polygonOffset;
    FaceCulling   m_cullBackfaces;
    bool          m_enableColorMask;
    bool          m_enableDepthTest;
    bool          m_enableDepthWrite;
    bool          m_enableLighting;
};

//==================================================================================================
//
// ScalarMapperEffectGenerator
//
//==================================================================================================
class ScalarMapperEffectGenerator : public EffectGenerator
{
public:
    ScalarMapperEffectGenerator( const cvf::ScalarMapper* scalarMapper, PolygonOffset polygonOffset );

    void setOpacityLevel( float opacity ) { m_opacityLevel = cvf::Math::clamp( opacity, 0.0f, 1.0f ); }
    void setUndefinedColor( cvf::Color3f color ) { m_undefinedColor = color; }
    void setFaceCulling( FaceCulling faceCulling ) { m_faceCulling = faceCulling; }
    void enableDepthWrite( bool enableWrite ) { m_enableDepthWrite = enableWrite; }
    void disableLighting( bool disable ) { m_disableLighting = disable; }
    void discardTransparentFragments( bool discard ) { m_discardTransparentFragments = discard; }

public:
    static cvf::ref<cvf::TextureImage> addAlphaAndUndefStripes( const cvf::TextureImage* texImg,
                                                                const cvf::Color3f&      undefScalarColor,
                                                                float                    opacityLevel );
    static bool isImagesEqual( const cvf::TextureImage* texImg1, const cvf::TextureImage* texImg2 );

protected:
    bool             isEqual( const EffectGenerator* other ) const override;
    EffectGenerator* copy() const override;

    void updateForShaderBasedRendering( cvf::Effect* effect ) const override;
    void updateForFixedFunctionRendering( cvf::Effect* effect ) const override;

private:
    void updateCommonEffect( cvf::Effect* effect ) const;

private:
    cvf::cref<cvf::ScalarMapper>        m_scalarMapper;
    mutable cvf::ref<cvf::TextureImage> m_textureImage;
    PolygonOffset                       m_polygonOffset;
    float                               m_opacityLevel;
    cvf::Color3f                        m_undefinedColor;
    FaceCulling                         m_faceCulling;
    bool                                m_enableDepthWrite;
    bool                                m_disableLighting;
    bool                                m_discardTransparentFragments;
};

//==================================================================================================
//
// ScalarMapperMeshEffectGenerator
//
//==================================================================================================
class ScalarMapperMeshEffectGenerator : public EffectGenerator
{
public:
    explicit ScalarMapperMeshEffectGenerator( const cvf::ScalarMapper* scalarMapper );

    void setOpacityLevel( float opacity ) { m_opacityLevel = cvf::Math::clamp( opacity, 0.0f, 1.0f ); }
    void setUndefinedColor( cvf::Color3f color ) { m_undefinedColor = color; }

protected:
    bool             isEqual( const EffectGenerator* other ) const override;
    EffectGenerator* copy() const override;

    void updateForShaderBasedRendering( cvf::Effect* effect ) const override;
    void updateForFixedFunctionRendering( cvf::Effect* effect ) const override;

private:
    void updateCommonEffect( cvf::Effect* effect ) const;

private:
    cvf::cref<cvf::ScalarMapper>        m_scalarMapper;
    mutable cvf::ref<cvf::TextureImage> m_textureImage;
    float                               m_opacityLevel;
    cvf::Color3f                        m_undefinedColor;
};

//==================================================================================================
//
// MeshEffectGenerator
//
//==================================================================================================
class MeshEffectGenerator : public EffectGenerator
{
public:
    explicit MeshEffectGenerator( const cvf::Color3f& color );
    void setLineStipple( bool enable ) { m_lineStipple = enable; }
    void setLineWidth( float lineWidth );

protected:
    bool             isEqual( const EffectGenerator* other ) const override;
    EffectGenerator* copy() const override;

    void updateForShaderBasedRendering( cvf::Effect* effect ) const override;
    void updateForFixedFunctionRendering( cvf::Effect* effect ) const override;

private:
    cvf::Color3f m_color;
    bool         m_lineStipple;
    float        m_lineWidth;
};

//==================================================================================================
//
// TextEffectGenerator
//
//==================================================================================================
class TextEffectGenerator : public EffectGenerator
{
public:
    TextEffectGenerator();

protected:
    bool             isEqual( const EffectGenerator* other ) const override;
    EffectGenerator* copy() const override;

    void updateForShaderBasedRendering( cvf::Effect* effect ) const override;
    void updateForFixedFunctionRendering( cvf::Effect* effect ) const override;
};

//==================================================================================================
//
// VectorEffectGenerator
//
//==================================================================================================
class VectorEffectGenerator : public EffectGenerator
{
public:
    VectorEffectGenerator();

protected:
    bool             isEqual( const EffectGenerator* other ) const override;
    EffectGenerator* copy() const override;

    void updateForShaderBasedRendering( cvf::Effect* effect ) const override;
    void updateForFixedFunctionRendering( cvf::Effect* effect ) const override;
};
} // namespace caf
