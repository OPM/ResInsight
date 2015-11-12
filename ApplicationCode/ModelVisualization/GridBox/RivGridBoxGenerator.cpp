/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////


#include "RivGridBoxGenerator.h"

#include "RivPatchGenerator.h"

#include "cafEffectGenerator.h"
#include "cvfCamera.h"
#include "cvfDrawableText.h"
#include "cvfFixedAtlasFont.h"
#include "cvfGeometryBuilderDrawableGeo.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfMeshEdgeExtractor.h"
#include "cvfPrimitiveSetIndexedUInt.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivGridBoxGenerator::RivGridBoxGenerator()
{
    m_gridBoxModel = new cvf::ModelBasicList;

    m_linDiscreteScalarMapper = new cvf::ScalarMapperDiscreteLinear;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridBoxGenerator::setTransform(cvf::Transform* scaleTransform)
{
    m_scaleTransform = scaleTransform;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridBoxGenerator::setBoundingBox(const cvf::BoundingBox& boundingBox)
{
    m_boundingBox = boundingBox;

    m_xValues.clear();
    m_yValues.clear();
    m_zValues.clear();

    cvf::Vec3d min = m_boundingBox.min();
    cvf::Vec3d max = m_boundingBox.max();

    m_linDiscreteScalarMapper->setRange(min.x(), max.x());
    m_linDiscreteScalarMapper->setLevelCount(5, true);
    m_linDiscreteScalarMapper->majorTickValues(&m_xValues);

    m_linDiscreteScalarMapper->setRange(min.y(), max.y());
    m_linDiscreteScalarMapper->setLevelCount(5, true);
    m_linDiscreteScalarMapper->majorTickValues(&m_yValues);

    m_linDiscreteScalarMapper->setRange(min.z(), max.z());
    m_linDiscreteScalarMapper->setLevelCount(5, true);
    m_linDiscreteScalarMapper->majorTickValues(&m_zValues);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridBoxGenerator::createGridBoxParts()
{
    createGridBoxSideParts();
    createGridBoxLegendParts();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridBoxGenerator::updateFromCamera(const cvf::Camera* camera)
{
    m_gridBoxModel->removeAllParts();

    if (m_gridBoxSideParts.size() == 0) return;

    for (size_t i = POS_X; i <= NEG_Z; i++)
    {
        cvf::Vec3f sideNorm = sideNormalOutwards((FaceType)i);

        cvf::Vec3d camToSide = camera->position() - pointOnSide((FaceType)i);
        camToSide.normalize();

        if (sideNorm.dot(cvf::Vec3f(camToSide)) < 0.0)
        {
            m_gridBoxModel->addPart(m_gridBoxSideParts[i].p());
        }
    }

    for (size_t i = 0; i < m_gridBoxLegendParts.size(); i++)
    {
        m_gridBoxModel->addPart(m_gridBoxLegendParts[i].p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Model* RivGridBoxGenerator::model()
{
    return m_gridBoxModel.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridBoxGenerator::createGridBoxSideParts()
{
    m_gridBoxSideParts.clear();

    cvf::Vec3d min = m_boundingBox.min();
    cvf::Vec3d max = m_boundingBox.max();


    for (int face = POS_X; face <= NEG_Z; face++)
    {
        // TODO: move out of loop
        RivPatchGenerator patchGen;

        if (face == POS_X)
        {
            patchGen.setOrigin(cvf::Vec3d(max.x(), 0.0, 0.0));
            patchGen.setAxes(cvf::Vec3d::Y_AXIS, cvf::Vec3d::Z_AXIS);
            patchGen.setSubdivisions(m_yValues, m_zValues);
        }
        else if (face == NEG_X)
        {
            patchGen.setOrigin(cvf::Vec3d(min.x(), 0.0, 0.0));
            patchGen.setAxes(cvf::Vec3d::Y_AXIS, cvf::Vec3d::Z_AXIS);
            patchGen.setSubdivisions(m_yValues, m_zValues);
        }
        else if (face == POS_Y)
        {
            patchGen.setOrigin(cvf::Vec3d(0.0, max.y(), 0.0));
            patchGen.setAxes(cvf::Vec3d::X_AXIS, cvf::Vec3d::Z_AXIS);
            patchGen.setSubdivisions(m_xValues, m_zValues);
        }
        else if (face == NEG_Y)
        {
            patchGen.setOrigin(cvf::Vec3d(0.0, min.y(), 0.0));
            patchGen.setAxes(cvf::Vec3d::X_AXIS, cvf::Vec3d::Z_AXIS);
            patchGen.setSubdivisions(m_xValues, m_zValues);
        }
        else if (face == POS_Z)
        {
            patchGen.setOrigin(cvf::Vec3d(0.0, 0.0, max.z()));
            patchGen.setAxes(cvf::Vec3d::X_AXIS, cvf::Vec3d::Y_AXIS);
            patchGen.setSubdivisions(m_xValues, m_yValues);
        }
        else if (face == NEG_Z)
        {
            patchGen.setOrigin(cvf::Vec3d(0.0, 0.0, min.z()));
            patchGen.setAxes(cvf::Vec3d::X_AXIS, cvf::Vec3d::Y_AXIS);
            patchGen.setSubdivisions(m_xValues, m_yValues);
        }
        else
        {
            CVF_ASSERT(false);
        }

        cvf::GeometryBuilderFaceList builder;
        patchGen.generate(&builder);
        cvf::ref<cvf::Vec3fArray> vertexArray = builder.vertices();
        cvf::ref<cvf::UIntArray> faceList = builder.faceList();

        {
            // Box mesh
            cvf::MeshEdgeExtractor ee;
            ee.addFaceList(*faceList);

            cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
            geo->setVertexArray(vertexArray.p());
            geo->addPrimitiveSet(new cvf::PrimitiveSetIndexedUInt(cvf::PT_LINES, ee.lineIndices().p()));

            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("Grid box ");
            part->setDrawable(geo.p());

            part->setTransform(m_scaleTransform.p());
            part->updateBoundingBox();

            cvf::ref<cvf::Effect> eff;
            caf::MeshEffectGenerator effGen(cvf::Color3f::GRAY);
            eff = effGen.generateCachedEffect();

            part->setEffect(eff.p());

            m_gridBoxSideParts.push_back(part.p());
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridBoxGenerator::createGridBoxLegendParts()
{
    m_gridBoxLegendParts.clear();

    for (int edge = POS_Z_POS_X; edge <= NEG_X_NEG_Y; edge++)
    {
        cvf::Collection<cvf::Part> parts;

        createLegend((EdgeType)edge, &parts);

        for (int i = 0; i < parts.size(); i++)
        {
            m_gridBoxLegendParts.push_back(parts.at(i));
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivGridBoxGenerator::createLegend(EdgeType edge, cvf::Collection<cvf::Part>* parts)
{
    cvf::Vec3d posMin;
    cvf::Vec3d posMax;

    cvf::Vec3d min = m_boundingBox.min();
    cvf::Vec3d max = m_boundingBox.max();

    std::vector<double>* tickValues = NULL;
    AxisType axis;

    cvf::Vec3f tickMarkDir;

    switch (edge)
    {
    case RivGridBoxGenerator::POS_Z_POS_X:
        axis = Y_AXIS;
        tickValues = &m_yValues;
        posMin.set(max.x(), min.y(), max.z());
        posMax.set(max.x(), max.y(), max.z());
        tickMarkDir = cornerDirection(POS_Z, POS_X);
        break;
    case RivGridBoxGenerator::POS_Z_NEG_X:
        axis = Y_AXIS;
        tickValues = &m_yValues;
        posMin.set(min.x(), min.y(), max.z());
        posMax.set(min.x(), max.y(), max.z());
        tickMarkDir = cornerDirection(POS_Z, NEG_X);
        break;
    case RivGridBoxGenerator::POS_Z_POS_Y:
        axis = X_AXIS;
        tickValues = &m_xValues;
        posMin.set(min.x(), max.y(), max.z());
        posMax.set(max.x(), max.y(), max.z());
        tickMarkDir = cornerDirection(POS_Z, POS_Y);
        break;
    case RivGridBoxGenerator::POS_Z_NEG_Y:
        axis = X_AXIS;
        tickValues = &m_xValues;
        posMin.set(min.x(), min.y(), max.z());
        posMax.set(max.x(), min.y(), max.z());
        tickMarkDir = cornerDirection(POS_Z, NEG_Y);
        break;
    case RivGridBoxGenerator::NEG_Z_POS_X:
        axis = Y_AXIS;
        tickValues = &m_yValues;
        posMin.set(max.x(), min.y(), min.z());
        posMax.set(max.x(), max.y(), min.z());
        tickMarkDir = cornerDirection(NEG_Z, POS_X);
        break;
    case RivGridBoxGenerator::NEG_Z_NEG_X:
        axis = Y_AXIS;
        tickValues = &m_yValues;
        posMin.set(min.x(), min.y(), min.z());
        posMax.set(min.x(), max.y(), min.z());
        tickMarkDir = cornerDirection(NEG_Z, NEG_X);
        break;
    case RivGridBoxGenerator::NEG_Z_POS_Y:
        axis = X_AXIS;
        tickValues = &m_xValues;
        posMin.set(min.x(), max.y(), min.z());
        posMax.set(max.x(), max.y(), min.z());
        tickMarkDir = cornerDirection(NEG_Z, POS_Y);
        break;
    case RivGridBoxGenerator::NEG_Z_NEG_Y:
        axis = X_AXIS;
        tickValues = &m_xValues;
        posMin.set(min.x(), min.y(), min.z());
        posMax.set(max.x(), min.y(), min.z()); 
        tickMarkDir = cornerDirection(NEG_Z, NEG_Y);
        break;
    case RivGridBoxGenerator::POS_X_POS_Y:
        axis = Z_AXIS;
        tickValues = &m_zValues;
        posMin.set(max.x(), max.y(), min.z());
        posMax.set(max.x(), max.y(), max.z());
        tickMarkDir = cornerDirection(POS_X, POS_Y);
        break;
    case RivGridBoxGenerator::POS_X_NEG_Y:
        axis = Z_AXIS;
        tickValues = &m_zValues;
        posMin.set(max.x(), min.y(), min.z());
        posMax.set(max.x(), min.y(), max.z());
        tickMarkDir = cornerDirection(POS_X, NEG_Y);
        break;
    case RivGridBoxGenerator::NEG_X_POS_Y:
        axis = Z_AXIS;
        tickValues = &m_zValues;
        posMin.set(min.x(), max.y(), min.z());
        posMax.set(min.x(), max.y(), max.z());
        tickMarkDir = cornerDirection(NEG_X, POS_Y);
        break;
    case RivGridBoxGenerator::NEG_X_NEG_Y:
        axis = Z_AXIS;
        tickValues = &m_zValues;
        posMin.set(min.x(), min.y(), min.z());
        posMax.set(min.x(), min.y(), max.z());
        tickMarkDir = cornerDirection(NEG_X, NEG_Y);
        break;
    default:
        break;
    }

    CVF_ASSERT(tickValues);

    size_t numVerts = (tickValues->size()) * 2;
    size_t numLines = (tickValues->size()) + 1;

    cvf::ref<cvf::Vec3fArray> vertices = new cvf::Vec3fArray;
    vertices->reserve(numVerts);

    cvf::ref<cvf::UIntArray> indices = new cvf::UIntArray;
    indices->reserve(2 * numLines);


    float tickLength = static_cast<float>(m_boundingBox.extent().length() / 100.0);

    cvf::Vec3f point = cvf::Vec3f(posMin);
    cvf::Vec3f tickPoint;

    // Tick marks
    for (size_t i = 0; i < tickValues->size(); ++i)
    {
        point[axis] = static_cast<float>(tickValues->at(i));

        vertices->add(point);
        tickPoint = point + tickLength*tickMarkDir;;
        vertices->add(tickPoint);
        indices->add(2 * static_cast<cvf::uint>(i));
        indices->add(2 * static_cast<cvf::uint>(i) + 1);
    }

    // Backbone of legend
    indices->add(0);
    indices->add(static_cast<cvf::uint>(numVerts) - 2);

    {
        // Legend lines

        cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
        geo->setVertexArray(vertices.p());

        cvf::ref<cvf::PrimitiveSetIndexedUInt> primSet = new cvf::PrimitiveSetIndexedUInt(cvf::PT_LINES);
        primSet->setIndices(indices.p());
        geo->addPrimitiveSet(primSet.p());

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName("Legend lines ");
        part->setDrawable(geo.p());

        part->setTransform(m_scaleTransform.p());
        part->updateBoundingBox();

        cvf::ref<cvf::Effect> eff;
        caf::MeshEffectGenerator effGen(cvf::Color3f::WHITE);
        eff = effGen.generateCachedEffect();

        part->setEffect(eff.p());

        parts->push_back(part.p());
    }


    {
        // Text labels

        cvf::ref<cvf::DrawableText> geo = new cvf::DrawableText;
        geo->setFont(new cvf::FixedAtlasFont(cvf::FixedAtlasFont::STANDARD));
        geo->setTextColor(cvf::Color3::WHITE);
        geo->setDrawBackground(false);
        geo->setDrawBorder(false);
        //textGeo->setCheckPosVisible(false);
        
        for (size_t idx = 0; idx < tickValues->size(); idx++)
        {
            geo->addText(cvf::String(tickValues->at(idx)), vertices->get(idx*2 + 1) + (0.5f * tickLength) * tickMarkDir);
        }

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setDrawable(geo.p());
        part->setTransform(m_scaleTransform.p());
        part->updateBoundingBox();

        cvf::ref<cvf::Effect> eff = new cvf::Effect;
        part->setEffect(eff.p());

        //textPart->setPriority(11);
        part->setName("Legend text");

        parts->push_back(part.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3f RivGridBoxGenerator::sideNormalOutwards(FaceType face)
{
    switch (face)
    {
    case RivGridBoxGenerator::POS_X:
        return cvf::Vec3f::X_AXIS;
    case RivGridBoxGenerator::NEG_X:
        return -cvf::Vec3f::X_AXIS;
    case RivGridBoxGenerator::POS_Y:
        return cvf::Vec3f::Y_AXIS;
    case RivGridBoxGenerator::NEG_Y:
        return -cvf::Vec3f::Y_AXIS;
    case RivGridBoxGenerator::POS_Z:
        return cvf::Vec3f::Z_AXIS;
    case RivGridBoxGenerator::NEG_Z:
        return -cvf::Vec3f::Z_AXIS;
    default:
        break;
    }

    return cvf::Vec3f::ZERO;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RivGridBoxGenerator::pointOnSide(FaceType face)
{
    switch (face)
    {
    case RivGridBoxGenerator::POS_X:
    case RivGridBoxGenerator::POS_Y:
    case RivGridBoxGenerator::POS_Z:
        return cvf::Vec3d(m_boundingBox.max());

    case RivGridBoxGenerator::NEG_X:
    case RivGridBoxGenerator::NEG_Y:
    case RivGridBoxGenerator::NEG_Z:
        return cvf::Vec3d(m_boundingBox.min());
    default:
        break;
    }

    return cvf::Vec3d::ZERO;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3f RivGridBoxGenerator::cornerDirection(FaceType face1, FaceType face2)
{
    cvf::Vec3f dir = sideNormalOutwards(face1) + sideNormalOutwards(face2);
    dir.normalize();

    return dir;
}
