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


#include "cvfBase.h"
#include "cvfDrawableGeo.h"
#include "cvfTransform.h"
#include "cvfEffect.h"
#include "cvfGeometryUtils.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfRenderState_FF.h"
#include "cvfUniform.h"
#include "cvfShaderProgram.h"
#include "cvfShaderProgramGenerator.h"
#include "cvfShaderSourceProvider.h"

#include "cvfuPartCompoundGenerator.h"

#ifndef WIN32
#include <cstdlib>
#endif

namespace cvfu {

using cvf::ref;
using cvf::uint;
using cvf::Vec3d;
using cvf::Vec3f;


//==================================================================================================
///
/// \class cvf::PartCompoundGenerator
/// \ingroup Utilities
///
/// Utility class for generating collections of parts for performance testing
/// 
//==================================================================================================


// Std. colors used for the effects
const cvf::Color3f DEFAULT_EFFECT_COLORS[] =
{
    cvf::Color3f(1.0f,  0.0f, 0.0f),
    cvf::Color3f(0.0f,  1.0f, 0.0f),
    cvf::Color3f(1.0f,  1.0f, 0.0f),
    cvf::Color3f(0.0f,  0.0f, 1.0f),
    cvf::Color3f(1.0f,  0.0f, 1.0f),
    cvf::Color3f(0.0f,  1.0f, 1.0f),
    cvf::Color3f(1.0f,  0.5f, 0.0f),
    cvf::Color3f(0.25f, 0.5f, 1.0f),

    cvf::Color3f(1.0f,  0.0f, 0.0f),
    cvf::Color3f(0.0f,  1.0f, 0.0f),
    cvf::Color3f(1.0f,  1.0f, 0.0f),
    cvf::Color3f(0.0f,  0.0f, 1.0f),
    cvf::Color3f(1.0f,  0.0f, 1.0f),
    cvf::Color3f(0.0f,  1.0f, 1.0f),
    cvf::Color3f(1.0f,  0.5f, 0.0f),
    cvf::Color3f(0.25f, 0.5f, 1.0f)
};

const int NUM_EFFECTS = sizeof(DEFAULT_EFFECT_COLORS)/sizeof(cvf::Color3f);


//--------------------------------------------------------------------------------------------------
/// Setup default config wiht 3x3x3 primitives
//--------------------------------------------------------------------------------------------------
PartCompoundGenerator::PartCompoundGenerator()
:   m_partDistribution(3, 3, 3),
    m_extent(1.0f, 1.0f, 1.0f),
    m_origin(0.0f, 0.0f, 0.0f),
    m_useShaders(false),
    m_numEffects(-1),
    m_numDrawableGeos(-1),
    m_randomEffectAssignment(false),
    m_randomGeoAssignment(false)
{
}


//--------------------------------------------------------------------------------------------------
/// Returns the number of parts that will be generated
//--------------------------------------------------------------------------------------------------
int PartCompoundGenerator::numParts() const
{
    return m_partDistribution.x()*m_partDistribution.y()*m_partDistribution.z();
}


//--------------------------------------------------------------------------------------------------
/// Set the total extent the parts occupies
//--------------------------------------------------------------------------------------------------
void PartCompoundGenerator::setExtent(Vec3f extent)
{
    m_extent = extent;
}


//--------------------------------------------------------------------------------------------------
/// Set the origin ("lower left corner") of the 
//--------------------------------------------------------------------------------------------------
void PartCompoundGenerator::setOrigin(Vec3f origin)
{
    m_origin = origin;
}



//--------------------------------------------------------------------------------------------------
/// Set the number of parts in each direction. Total number will be x*y*z
//--------------------------------------------------------------------------------------------------
void PartCompoundGenerator::setPartDistribution(cvf::Vec3i partDistribution)
{
    m_partDistribution = partDistribution;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PartCompoundGenerator::setUseShaders(bool useShaders)
{
    m_useShaders = useShaders;
}


//--------------------------------------------------------------------------------------------------
/// Set the (maximum) number of effects to use. Up to 16 different colors are (re)used.
/// \sa useRandomEffectAssignment
//--------------------------------------------------------------------------------------------------
void PartCompoundGenerator::setNumEffects(int numEffects) 
{
    m_numEffects = numEffects;
}


//--------------------------------------------------------------------------------------------------
/// Set the (maximum) number of effects to use. Up to 16 different colors are (re)used.
/// \sa useRandomEffectAssignment
//--------------------------------------------------------------------------------------------------
void PartCompoundGenerator::setNumDrawableGeos(int numDrawableGeos)
{
    m_numDrawableGeos = numDrawableGeos;
}


//--------------------------------------------------------------------------------------------------
/// Set if effects should be randomly or sequentially assigned.
//--------------------------------------------------------------------------------------------------
void PartCompoundGenerator::useRandomEffectAssignment(bool use)
{
    m_randomEffectAssignment = use;
}


//--------------------------------------------------------------------------------------------------
/// Generate the parts using box primitives
//--------------------------------------------------------------------------------------------------
void PartCompoundGenerator::generateBoxes(cvf::Collection<cvf::Part>* parts)
{
    generateParts(BOX, cvf::UNDEFINED_INT, cvf::UNDEFINED_INT, parts);
}


//--------------------------------------------------------------------------------------------------
/// Generate the parts using sphere primitives
//--------------------------------------------------------------------------------------------------
void PartCompoundGenerator::generateSpheres(uint numSlices, uint numStacks, cvf::Collection<cvf::Part>* parts)
{
    generateParts(SPHERE, numSlices, numStacks, parts);
}


//--------------------------------------------------------------------------------------------------
/// Generate the parts using sphere primitives
//--------------------------------------------------------------------------------------------------
void PartCompoundGenerator::generateTriangles(cvf::Collection<cvf::Part>* parts)
{
    generateParts(TRIANGLE, cvf::UNDEFINED_INT, cvf::UNDEFINED_INT, parts);
}


//--------------------------------------------------------------------------------------------------
/// Generate x*y*z parts with either spheres or boxes
//--------------------------------------------------------------------------------------------------
void PartCompoundGenerator::generateParts(GenPrimType primType, uint numSlices, uint numStacks, cvf::Collection<cvf::Part>* parts)
{
    // -1 -> one effect/geo per part
    uint numEffects      = m_numEffects      > 0 ? static_cast<uint>(m_numEffects)      : numParts();
    uint numDrawableGeos = m_numDrawableGeos > 0 ? static_cast<uint>(m_numDrawableGeos) : numParts();

    // Setup effects
    cvf::Collection<cvf::Effect> effectCollection;

    cvf::ShaderProgramGenerator shaderGen("SimpleHeadlight", cvf::ShaderSourceProvider::instance());
    shaderGen.configureStandardHeadlightColor();
    //shaderGen.addVertexCode(cvf::ShaderSourceRepository::vs_Minimal);
    //shaderGen.addFragmentCode(cvf::ShaderSourceRepository::src_Color);
    //shaderGen.addFragmentCode(cvf::ShaderSourceRepository::fs_Unlit);

    uint i;
    for (i = 0; i < numEffects; i++)
    {
        ref<cvf::Effect> effect = new cvf::Effect;

        ref<cvf::RenderStateMaterial_FF> material = new cvf::RenderStateMaterial_FF;
        material->setAmbientAndDiffuse(DEFAULT_EFFECT_COLORS[i % NUM_EFFECTS]);
        material->setSpecular(cvf::Color3f(0.4f, 0.4f, 0.4f));
        material->setShininess(10.0f);
        effect->setRenderState(material.p());

        // Also create a uniform for the color
        effect->setUniform(new cvf::UniformFloat("u_color", cvf::Color4f(DEFAULT_EFFECT_COLORS[i % NUM_EFFECTS])));
        
        if (m_useShaders)
        {
            ref<cvf::ShaderProgram> prog = shaderGen.generate();
            effect->setShaderProgram(prog.p());
        }

        effectCollection.push_back(effect.p());
    }

    // Setup Geometries
    Vec3f delta;
    delta.x() = m_extent.x()/static_cast<float>(m_partDistribution.x());
    delta.y() = m_extent.y()/static_cast<float>(m_partDistribution.y());
    delta.z() = m_extent.z()/static_cast<float>(m_partDistribution.z());

    // Find min delta and set the diameter of the sphere to 80% of the smallest delta, hence radius is * 0.4
    double radius = static_cast<double>(CVF_MIN(CVF_MIN(delta.x(), delta.y()), delta.z()))*0.4f;

    cvf::Collection<cvf::DrawableGeo> drawableGeoCollection;

    for (i = 0; i < numDrawableGeos; i++)
    {
        ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;

        cvf::GeometryBuilderFaceList builder;

        switch (primType)
        {
            case SPHERE:
            {
                cvf::GeometryUtils::createSphere(radius, numSlices, numStacks, &builder);        
                break;
            }
            case BOX:
            {
                cvf::GeometryUtils::createBox(Vec3f(0,0,0), delta.x()*0.8f, delta.y()*0.8f, delta.z()*0.8f, &builder);
                break;
            }
            case TRIANGLE:
            {
                cvf::Vec3fArray vertices;
                vertices.reserve(3);
                vertices.add(Vec3f(-0.4f*delta.x(), -0.4f*delta.y(), 0));
                vertices.add(Vec3f( 0.4f*delta.x(), -0.4f*delta.y(), 0));
                vertices.add(Vec3f(             0,  0.4f*delta.y(), 0));

                builder.addTriangle(0,1,2);
                builder.addVertices(vertices);

                break;
            }
            default:
            {
                CVF_FAIL_MSG("Unknown type!");
                break;
            }
        }

        ref<cvf::Vec3fArray> vertices = builder.vertices();
        ref<cvf::UIntArray> faceList = builder.faceList();

        geo->setVertexArray(vertices.p());
        geo->setFromFaceList(*faceList);
        geo->computeNormals();

        drawableGeoCollection.push_back(geo.p());
    }

    // Generate the parts
    int partIndex = 0;
    int z;
    for (z = 0; z < m_partDistribution.z(); z++)
    {
        int y;
        for (y = 0; y < m_partDistribution.y(); y++)
        {
            int x;
            for (x = 0; x < m_partDistribution.x(); x++)
            {
                // Effect selection
                uint effectIndex = 0;

                if (m_randomEffectAssignment)
                {
                    effectIndex = rand() % static_cast<uint>(numEffects);
                }
                else
                {
                    effectIndex = partIndex%static_cast<uint>(numEffects);
                }

                // Geometry selection
                uint drawableGeoIndex = 0;

                if (m_randomGeoAssignment)
                {
                    drawableGeoIndex = rand() % static_cast<uint>(numDrawableGeos);
                }
                else
                {
                    drawableGeoIndex = partIndex%static_cast<uint>(numDrawableGeos);
                }

                // Setup the transform
                cvf::Mat4d transMat;
                Vec3d translation(  m_origin.x() + delta.x()*(0.5f + static_cast<float>(x)),
                                    m_origin.y() + delta.y()*(0.5f + static_cast<float>(y)),
                                    m_origin.z() + delta.z()*(0.5f + static_cast<float>(z)));

                transMat.setTranslation(translation);

                cvf::Transform* transform = new cvf::Transform;
                transform->setLocalTransform(transMat);

                // Configure the part
                ref<cvf::Part> part = new cvf::Part;
                part->setId(partIndex);
                part->setName("Part " + cvf::String(partIndex));
                part->setEffect(effectCollection[effectIndex].p());
                part->setDrawable(drawableGeoCollection[drawableGeoIndex].p());
                part->setTransform(transform);

                parts->push_back(part.p());
                partIndex++;
            }
        }
    }
}


} // namespace cvfu

