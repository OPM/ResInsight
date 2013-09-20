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
#include "cvfObject.h"
#include "cvfEffect.h"
#include "cvfScalarMapper.h"
#include "cvfTextureImage.h"
#include "cvfCollection.h"
#include "cvfString.h"

namespace caf {

class CommonShaderSources
{
public:
    static cvf::String light_AmbientDiffuse();

};

//==================================================================================================
//
// 
//
//==================================================================================================
class EffectGenerator
{
public:
    enum RenderingModeType      { FIXED_FUNCTION, SHADER_BASED };

    EffectGenerator()           {}
    virtual ~EffectGenerator()  {}

    cvf::ref<cvf::Effect>       generateEffect() const;
    void                        updateEffect(cvf::Effect* effect) const;

    static void                 setRenderingMode(RenderingModeType effectType);
    static RenderingModeType    renderingMode();

    static void                 clearEffectCache();
    static void                 releaseUnreferencedEffects();

protected:

    // Interface that must be implemented in base classes
    virtual bool                isEqual(const EffectGenerator* other) const = 0;
    virtual EffectGenerator*    copy() const = 0;
    friend class EffectCache;

    // When these are called, the effect is already cleared by updateEffect()
    virtual void                updateForShaderBasedRendering(cvf::Effect* effect) const = 0;
    virtual void                updateForFixedFunctionRendering(cvf::Effect* effect) const = 0;

private:
    static RenderingModeType   sm_renderingMode;
};


//==================================================================================================
//
// SurfaceEffectGenerator
//
//==================================================================================================
class SurfaceEffectGenerator : public EffectGenerator
{
public:
    SurfaceEffectGenerator(const cvf::Color4f& color, bool polygonOffset);
    SurfaceEffectGenerator(const cvf::Color3f& color, bool polygonOffset);

    void                            setCullBackfaces(bool cullBackFaces) { m_cullBackfaces = cullBackFaces; }

protected:
    virtual bool                    isEqual(const EffectGenerator* other) const;
    virtual EffectGenerator*        copy() const;

    virtual void                    updateForShaderBasedRendering(cvf::Effect* effect) const;
    virtual void                    updateForFixedFunctionRendering(cvf::Effect* effect) const;

private:
    void                            updateCommonEffect(cvf::Effect* effect) const;

private:
    cvf::Color4f    m_color;
    bool            m_polygonOffset;
    bool            m_cullBackfaces;

};


//==================================================================================================
//
// ScalarMapperEffectGenerator
//
//==================================================================================================
class ScalarMapperEffectGenerator : public EffectGenerator
{
public:
    ScalarMapperEffectGenerator(const cvf::ScalarMapper* scalarMapper, bool polygonOffset);

    void                            setOpacityLevel(float opacity)        { m_opacityLevel = cvf::Math::clamp(opacity, 0.0f , 1.0f ); }
    void                            setUndefinedColor(cvf::Color3f color) { m_undefinedColor = color; }
    void                            setCullBackfaces(bool cullBackFaces)  { m_cullBackfaces = cullBackFaces; }
public: 
    static cvf::ref<cvf::TextureImage> addAlphaAndUndefStripes(const cvf::TextureImage* texImg, const cvf::Color3f& undefScalarColor, float opacityLevel);
    static bool                     isImagesEqual(const cvf::TextureImage* texImg1, const cvf::TextureImage* texImg2);

protected:
    virtual bool                    isEqual(const EffectGenerator* other) const;
    virtual EffectGenerator*        copy() const;

    virtual void                    updateForShaderBasedRendering(cvf::Effect* effect) const;
    virtual void                    updateForFixedFunctionRendering(cvf::Effect* effect) const;

private:
    void                            updateCommonEffect(cvf::Effect* effect) const;

private:
    cvf::cref<cvf::ScalarMapper>    m_scalarMapper;
    mutable cvf::ref<cvf::TextureImage>     m_textureImage;
    bool                            m_polygonOffset;
    float                           m_opacityLevel;
    cvf::Color3f                    m_undefinedColor;
    bool                            m_cullBackfaces;
};


//==================================================================================================
//
// ScalarMapperMeshEffectGenerator
//
//==================================================================================================
class ScalarMapperMeshEffectGenerator : public EffectGenerator
{
public:
    ScalarMapperMeshEffectGenerator(const cvf::ScalarMapper* scalarMapper);

    void                            setOpacityLevel(float opacity)        { m_opacityLevel = cvf::Math::clamp(opacity, 0.0f , 1.0f ); }
    void                            setUndefinedColor(cvf::Color3f color) { m_undefinedColor = color; }

protected:
    virtual bool                    isEqual(const EffectGenerator* other) const;
    virtual EffectGenerator*        copy() const;

    virtual void                    updateForShaderBasedRendering(cvf::Effect* effect) const;
    virtual void                    updateForFixedFunctionRendering(cvf::Effect* effect) const;

private:
    void                            updateCommonEffect(cvf::Effect* effect) const;

private:
    cvf::cref<cvf::ScalarMapper>    m_scalarMapper;
    mutable cvf::ref<cvf::TextureImage>     m_textureImage;
    float                           m_opacityLevel;
    cvf::Color3f                    m_undefinedColor;
};


//==================================================================================================
//
// MeshEffectGenerator
//
//==================================================================================================
class MeshEffectGenerator : public EffectGenerator
{
public:
    MeshEffectGenerator(const cvf::Color3f& color);

protected:
    virtual bool                    isEqual(const EffectGenerator* other) const;
    virtual EffectGenerator*        copy() const;

    virtual void                    updateForShaderBasedRendering(cvf::Effect* effect) const;
    virtual void                    updateForFixedFunctionRendering(cvf::Effect* effect) const;

private:
    cvf::Color3f m_color;
};


}
