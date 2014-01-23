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
#include "cvfLibGeometry.h"
#include "cvfLibRender.h"
#include "cvfLibViewing.h"

#include "cvfuInputEvents.h"
#include "cvfuSnippetPropertyPublisher.h"

#include "snipScalarMapping.h"

namespace snip {


//==================================================================================================
///
/// \class snip::ScalarMapping::QuadMesh
/// \ingroup SnippetsBasis
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ScalarMapping::QuadMesh::QuadMesh(cvf::uint numPointsX, cvf::uint numPointsY)
{
    cvf::uint numVertices = numPointsX*numPointsY;
    m_vertices.reserve(numVertices);

    cvf::uint y;
    for (y = 0; y < numPointsY; y++)
    {
        cvf::uint x;
        for (x = 0; x < numPointsX; x++)
        {
            Vec3f v((float)x, (float)y, 0);
            m_vertices.add(v);
        }
    }

    GeometryUtils::tesselatePatchAsQuads(numPointsX, numPointsY, 0, true, &m_indices);

    // Create some vertex results
    {
        m_vertexResults.resize(numVertices);
        cvf::uint i;
        for (i = 0; i < numVertices; i++)
        {
            const Vec3f& v = m_vertices[i];
            m_vertexResults[i] = v.length();
        }
    }

    // and some element results
    {
        cvf::uint numQuads = (numPointsX - 1)*(numPointsY - 1);
        m_elementResults.resize(numQuads);
        cvf::uint q;
        for (q = 0; q < numQuads; q++)
        {
            Vec3f centroid(0, 0, 0);
            cvf::uint i;
            for (i = 0; i < 4; i++)
            {
                centroid += m_vertices[m_indices[4*q + i]];
            }

            centroid /= 4;
            m_elementResults[q] = centroid.length();
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> ScalarMapping::QuadMesh::generateSurface(const ScalarMapper* mapper, bool useVertexResults) const
{
    size_t numIndices = m_indices.size();
    size_t numQuads = numIndices/4;

    ref<Vec3fArray> vertices = new Vec3fArray;
    ref<Vec2fArray> textureCoords = new Vec2fArray;
    ref<Color3ubArray> colors = new Color3ubArray;
    vertices->reserve(numIndices);
    textureCoords->reserve(numIndices);
    colors->reserve(numIndices);

    size_t q;
    for (q = 0; q < numQuads; q++)
    {
        cvf::uint i;
        for (i = 0; i < 4; i++)
        {
            cvf::uint idx = m_indices[4*q + i];
            const Vec3f& vertex = m_vertices[idx];
            
            // Use vertex or element value
            double resVal = useVertexResults ? m_vertexResults[idx] : m_elementResults[q];

            vertices->add(vertex);
            textureCoords->add(mapper->mapToTextureCoord(resVal));
            colors->add(mapper->mapToColor(resVal));
        }
    }

    GeometryBuilderDrawableGeo builder;
    builder.addVertices(m_vertices);
    builder.addQuads(m_indices);

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setFromQuadVertexArray(vertices.p());
    geo->setTextureCoordArray(textureCoords.p());
    geo->setColorArray(colors.p());

    return geo;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> ScalarMapping::QuadMesh::generateMesh() const
{
    MeshEdgeExtractor mee;
    mee.addPrimitives(4, &m_indices[0], m_indices.size());

    ref<UIntArray> lineIndices = mee.lineIndices();
    ref<PrimitiveSetIndexedUInt> primSet = new PrimitiveSetIndexedUInt(PT_LINES);
    primSet->setIndices(lineIndices.p());

    ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;;
    geo->setVertexArray(new Vec3fArray(m_vertices));
    geo->addPrimitiveSet(primSet.p());

    return geo.p();
}



//==================================================================================================
///
/// \class snip::ScalarMapping
/// \ingroup SnippetsBasis
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ScalarMapping::ScalarMapping()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ScalarMapping::onInitialize()
{
    m_quadMesh = new QuadMesh(30, 20);

    // Can be used to provoke problems with too small texture images
    // m_quadMesh = new QuadMesh(3, 2);
    // m_quadMesh->m_elementResults[0] = 107.691;
    // m_quadMesh->m_elementResults[1] = 107.693;
    // m_mapper->setRange(0, 200);


    m_propResMapping = new PropertyEnum("Res. mapping");
    m_propResMapping->addItem("vertex", "Vertex results");
    m_propResMapping->addItem("element", "Element results");
    m_propResMapping->setCurrentIdent("vertex");
    m_propertyPublisher->publishProperty(m_propResMapping.p());

    m_propFringesMode = new PropertyEnum("Fringes mode");
    m_propFringesMode->addItem("colors",        "Colors");
    m_propFringesMode->addItem("texture",       "Texture");
    m_propFringesMode->addItem("shader_texture","Shader texture");
    m_propFringesMode->setCurrentIdent("texture");
    m_propertyPublisher->publishProperty(m_propFringesMode.p());

    m_propRangeMin = new PropertyDouble("Range min", 0.0);
    m_propRangeMax = new PropertyDouble("Range max", 1.0);
    m_propertyPublisher->publishProperty(m_propRangeMin.p());
    m_propertyPublisher->publishProperty(m_propRangeMax.p());
    setRangeMinMaxPropertiesFromResultRange(true);

    m_propLegendPalette = new PropertyEnum("Legend color palette");
    m_propLegendPalette->addItem("normal",           "Normal");
    m_propLegendPalette->addItem("yellow_red",       "Yellow-red");
    m_propLegendPalette->addItem("thermal_2",        "Thermal_2");
    m_propLegendPalette->addItem("metal_casting",    "Metal casting");
    m_propLegendPalette->setCurrentIdent("normal");
    m_propertyPublisher->publishProperty(m_propLegendPalette.p());

    m_propLegendColorCount = new PropertyInt("Num legend colors", 10);
    m_propLegendColorCount->setRange(1, 10000);
    m_propLegendColorCount->setGuiStep(1);
    m_propertyPublisher->publishProperty(m_propLegendColorCount.p());

    m_propTextureSizeAuto = new PropertyBool("Automatic texture size", true);
    m_propertyPublisher->publishProperty(m_propTextureSizeAuto.p());

    m_propTextureSize = new PropertyInt("Texture size", 1024);
    m_propTextureSize->setRange(m_propLegendColorCount->value(), 16384);
    m_propTextureSize->setGuiStep(1);
    m_propertyPublisher->publishProperty(m_propTextureSize.p());

    m_propMeshLines = new PropertyBool("Mesh lines", true);
    m_propertyPublisher->publishProperty(m_propMeshLines.p());
    

    // Will be configured when updating visualization
    m_mapper = new ScalarMapperUniformLevels;

    m_legend = new OverlayColorLegend(new FixedAtlasFont(FixedAtlasFont::STANDARD));
    m_legend->setColor(Color3::BROWN);
    m_legend->setTitle("Temperature\nUnit: K");
    m_legend->setLayout( OverlayItem::VERTICAL, OverlayItem::BOTTOM_LEFT);

    m_axisCross = new OverlayAxisCross(m_camera.p(), new FixedAtlasFont(FixedAtlasFont::STANDARD));
    m_legend->setLayout( OverlayItem::VERTICAL, OverlayItem::BOTTOM_LEFT);

    m_model = new ModelBasicList;

    Rendering* rendering = m_renderSequence->firstRendering();
    rendering->scene()->addModel(m_model.p());
    rendering->addOverlayItem(m_axisCross.p());
    rendering->addOverlayItem(m_legend.p());

    updateVisualization();

    BoundingBox bb = m_renderSequence->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, -Vec3d::Z_AXIS, Vec3d::Y_AXIS);
        m_trackball->setRotationPoint(bb.center());
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ScalarMapping::onPropertyChanged(Property* property, PostEventAction* postEventAction)
{
    *postEventAction = REDRAW;

    if (property == m_propResMapping)
    {
        setRangeMinMaxPropertiesFromResultRange(true);
    }

    if (property == m_propLegendColorCount)
    {
        m_propTextureSize->setRange(m_propLegendColorCount->value(), m_propTextureSize->max());
        m_propertyPublisher->notifyPropertyChangedBySnippet(m_propTextureSize.p());
    }

    updateVisualization();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ScalarMapping::onKeyPressEvent(KeyEvent* keyEvent)
{
    Key key = keyEvent->key();
    char character = keyEvent->character();

    bool needUpdate = false;

    if (key == Key_V)
    {
        if (character == 'v')  m_propResMapping->setCurrentIdent("vertex");
        else                   m_propResMapping->setCurrentIdent("element");
        m_propertyPublisher->notifyPropertyValueChangedBySnippet(m_propResMapping.p());
        needUpdate = true;
    }

    if (key == Key_F)
    {
        cvf::uint newIdx = (m_propFringesMode->currentIndex() + 1) % m_propFringesMode->itemCount();
        m_propFringesMode->setCurrentIndex(newIdx);
        m_propertyPublisher->notifyPropertyValueChangedBySnippet(m_propFringesMode.p());
        needUpdate = true;
    }

    if (key == Key_R)
    {
        bool fullRange = (character == 'r') ? false : true;
        setRangeMinMaxPropertiesFromResultRange(fullRange);
        needUpdate = true;
    }

    if (needUpdate)
    {
        updateVisualization();

        keyEvent->setRequestedAction(REDRAW);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::String> ScalarMapping::helpText() const
{
    std::vector<String> help;

    help.push_back("'v/V' : Vertex or element result");
    help.push_back("'f'   : Cycle fringes mode");
    help.push_back("'r/R' : Reduced or full range");

    return help;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ScalarMapping::setRangeMinMaxPropertiesFromResultRange(bool useFullRange)
{
    double min = 0.0;
    double max = 1.0;

    if (m_propResMapping->currentIdent() == "vertex")
    {
        min = m_quadMesh->m_vertexResults.min();
        max = m_quadMesh->m_vertexResults.max();
    }
    else
    {
        min = m_quadMesh->m_elementResults.min();
        max = m_quadMesh->m_elementResults.max();
    }

    // Full or reduced range
    if (useFullRange)
    {
        m_propRangeMin->setValue(min);
        m_propRangeMax->setValue(max);
    }
    else
    {
        double delta = max - min;
        m_propRangeMin->setValue(min + 0.25*delta);
        m_propRangeMax->setValue(max - 0.25*delta);
    }

    m_propertyPublisher->notifyPropertyValueChangedBySnippet(m_propRangeMin.p());
    m_propertyPublisher->notifyPropertyValueChangedBySnippet(m_propRangeMax.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ScalarMapping::updateVisualization()
{
    m_model->removeAllParts();

    {
        cvf::uint numColors = m_propLegendColorCount->value();
        String legClr = m_propLegendPalette->currentIdent();
        if      (legClr == "normal")        m_mapper->setColors(ScalarMapper::NORMAL, numColors);
        else if (legClr == "yellow_red")    m_mapper->setColors(ScalarMapper::YELLOW_RED, numColors);
        else if (legClr == "thermal_2")     m_mapper->setColors(ScalarMapper::THERMAL_2, numColors);
        else if (legClr == "metal_casting") m_mapper->setColors(ScalarMapper::METAL_CASTING, numColors);

        if (m_propTextureSizeAuto->value())
        {
            cvf::uint textureSize = Math::roundUpPow2(numColors);
            m_mapper->setTextureSize(textureSize);
            m_propTextureSize->setValue(static_cast<int>(textureSize));
            m_propertyPublisher->notifyPropertyChangedBySnippet(m_propTextureSize.p());
        }
        else
        {
            m_mapper->setTextureSize(m_propTextureSize->value());
        }

        m_mapper->setRange(m_propRangeMin->value(), m_propRangeMax->value());
    }

    {
        bool useVertexResults = (m_propResMapping->currentIdent() == "vertex") ? true : false;
        ref<DrawableGeo> geo = m_quadMesh->generateSurface(m_mapper.p(), useVertexResults);
        geo->computeNormals();

        ref<Effect> eff;
        String frinMode = m_propFringesMode->currentIdent();
        if      (frinMode == "colors")         eff = createColorResultEffect();
        else if (frinMode == "texture")        eff = createFixedFunctionTextureResultEffect(*m_mapper);
        else if (frinMode == "shader_texture") eff = createShaderTextureResultEffect(*m_mapper);

        ref<Part> part = new Part;
        part->setDrawable(geo.p());
        part->setEffect(eff.p());
        m_model->addPart(part.p());
    }

    if (m_propMeshLines->value())
    {
        ref<DrawableGeo> geo = m_quadMesh->generateMesh();
        ref<Part> part = new Part;
        part->setDrawable(geo.p());
        part->setEffect(createMeshEffect().p());
        m_model->addPart(part.p());
    }

    m_model->updateBoundingBoxesRecursive();

    m_mapper->updateColorLegend(m_legend.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Effect> ScalarMapping::createShaderTextureResultEffect(const cvf::ScalarMapperUniformLevels& scalarMapper)
{
    ref<cvf::Effect> eff = new cvf::Effect;

    cvf::ShaderProgramGenerator gen("SimpleHeadlight", cvf::ShaderSourceProvider::instance());
    gen.configureStandardHeadlightTexture();
    ref<ShaderProgram> prog = gen.generate();

    eff->setShaderProgram(prog.p());

    /*
    ShaderFactory* sf = ShaderFactory::instance();
    ref<ShaderProgram> prog = new ShaderProgram;
    prog->addShader(sf->createFromRepository(Shader::VERTEX_SHADER, ShaderSourceRepository::Standard_vs).p());
    prog->addShader(sf->createFromRepository(Shader::FRAGMENT_SHADER, ShaderSourceRepository::Standard_fs).p());
    prog->addShader(sf->createFromRepository(Shader::FRAGMENT_SHADER, ShaderSourceRepository::Texture_src).p());
    prog->addShader(sf->createFromRepository(Shader::FRAGMENT_SHADER, ShaderSourceRepository::Phong_light).p());

    // Link and show log to see any warnings
    prog->linkProgram();
    Trace::show(prog->programInfoLog(m_openGLContext.p()));

    eff->setShaderProgram(prog.p());
    eff->setUniform(new cvf::UniformFloat("u_ambientIntensity", 0.2f));
    */

    ref<cvf::TextureImage> texImg = new cvf::TextureImage;
    scalarMapper.updateTexture(texImg.p());

    ref<cvf::Texture> texture = new cvf::Texture(texImg.p());
    ref<cvf::Sampler> sampler = new cvf::Sampler;
    sampler->setWrapMode(Sampler::CLAMP_TO_EDGE);
    sampler->setMinFilter(Sampler::NEAREST);
    sampler->setMagFilter(Sampler::NEAREST);

    ref<cvf::RenderStateTextureBindings> texBind = new cvf::RenderStateTextureBindings;
    texBind->addBinding(texture.p(), sampler.p(), "u_texture2D");
    eff->setRenderState(texBind.p());

    ref<cvf::RenderStatePolygonOffset> polyOffset = new cvf::RenderStatePolygonOffset;
    polyOffset->configurePolygonPositiveOffset();
    eff->setRenderState(polyOffset.p());

    return eff;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Effect> ScalarMapping::createFixedFunctionTextureResultEffect(const ScalarMapperUniformLevels& scalarMapper)
{
    ref<cvf::Effect> eff = new cvf::Effect;

    ref<cvf::RenderStateMaterial_FF> mat = new cvf::RenderStateMaterial_FF(cvf::Color3::WHITE);
    eff->setRenderState(mat.p());

    ref<cvf::RenderStateLighting_FF> lighting = new cvf::RenderStateLighting_FF;
    lighting->enableTwoSided(true);
    eff->setRenderState(lighting.p());

    ref<cvf::TextureImage> texImg = new cvf::TextureImage;
    scalarMapper.updateTexture(texImg.p());

    // Use fixed function texture setup
    ref<cvf::Texture2D_FF> texture = new cvf::Texture2D_FF(texImg.p());
    texture->setWrapMode(Texture2D_FF::CLAMP);
    texture->setMinFilter(Texture2D_FF::NEAREST);
    texture->setMagFilter(Texture2D_FF::NEAREST);
    ref<cvf::RenderStateTextureMapping_FF> texMapping = new cvf::RenderStateTextureMapping_FF(texture.p());
    eff->setRenderState(texMapping.p());

//     // Uses full blown texture setup
//     CVF_ASSERT(CVF_OPENGL_1_3);
//     ref<cvf::Sampler> sampler = new cvf::Sampler;
//     sampler->setWrapMode(Sampler::CLAMP_TO_EDGE);
//     sampler->setMinFilter(Sampler::NEAREST);
//     sampler->setMagFilter(Sampler::NEAREST);
//     ref<cvf::Texture> texture = new cvf::Texture(texImg.p());
//     ref<cvf::RenderStateTextureBindings> texBind = new cvf::RenderStateTextureBindings;
//     texBind->addBinding(texture.p(), sampler.p(), "dummy");
//     eff->setRenderState(texBind.p());

    ref<cvf::RenderStatePolygonOffset> polyOffset = new cvf::RenderStatePolygonOffset;
    polyOffset->configurePolygonPositiveOffset();
    eff->setRenderState(polyOffset.p());

    return eff;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Effect> ScalarMapping::createColorResultEffect()
{
    ref<cvf::Effect> eff = new cvf::Effect;

    ref<cvf::RenderStateMaterial_FF> mat = new cvf::RenderStateMaterial_FF(cvf::Color3::GRAY);
    mat->enableColorMaterial(true);
    eff->setRenderState(mat.p());

    ref<cvf::RenderStateLighting_FF> lighting = new cvf::RenderStateLighting_FF;
    lighting->enableTwoSided(true);
    eff->setRenderState(lighting.p());

    ref<cvf::RenderStatePolygonOffset> polyOffset = new cvf::RenderStatePolygonOffset;
    polyOffset->configurePolygonPositiveOffset();
    eff->setRenderState(polyOffset.p());

    return eff;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Effect> ScalarMapping::createMeshEffect()
{
    ref<Effect> eff = new Effect;
    eff->setRenderState(new cvf::RenderStateMaterial_FF(Color3f::WHITE));
    eff->setRenderState(new cvf::RenderStateDepth(true, cvf::RenderStateDepth::LEQUAL));
    eff->setRenderState(new cvf::RenderStateLighting_FF(false));

    return eff;
}

} // namespace snip

