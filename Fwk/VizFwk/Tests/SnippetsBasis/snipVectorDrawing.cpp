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

#include "snipVectorDrawing.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
static const char sg_FixedHeadlightVectorVertexShader[] = 
"varying vec3 v_V;                                                          \n"
"varying vec3 v_N;                                                          \n"
"                                                                           \n"
"uniform mat4 cvfu_modelViewProjectionMatrix;                               \n"
"uniform mat4 cvfu_modelViewMatrix;                                         \n"
"uniform mat3 cvfu_normalMatrix;                                            \n"
"                                                                           \n"
"// Transformation matrix to orient and position the vector                 \n"
"uniform mat4 mat_Vector;                                                   \n"
"                                                                           \n"
"attribute vec4 cvfa_vertex;                                                \n"
"attribute vec3 cvfa_normal;                                                \n"
"                                                                           \n"
"void main ()                                                               \n"
"{                                                                          \n"
"    gl_Position = cvfu_modelViewProjectionMatrix*mat_Vector*cvfa_vertex;   \n"
"    v_V = (cvfu_modelViewMatrix*mat_Vector*cvfa_vertex).xyz;               \n"
"    mat3 mat3_Vector = mat3(mat_Vector);                                   \n"
"    v_N = cvfu_normalMatrix*mat3_Vector*cvfa_normal;                       \n"
"}                                                                          \n";


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
static const char sg_FixedHeadlightVectorFragmentShader[] = 
"varying vec3 v_V;                                                          \n"
"varying vec3 v_N;                                                          \n"
"                                                                           \n"
"uniform float ambientIntensity;                                            \n"
"uniform vec3 color;                                                        \n"
"                                                                           \n"
"const float specularIntensity = 0.5;                                       \n"
"const vec3 lightSourcePosition = vec3(0.5f, 5.0f, 7.0f);                   \n"
"                                                                           \n"
"void main ()                                                               \n"
"{                                                                          \n"
"    vec3 N = normalize(v_N);                                               \n"
"    vec3 V = normalize(v_V);                                               \n"
"    vec3 R = reflect(V, N);                                                \n"
"    vec3 L = normalize(lightSourcePosition);                               \n"
"                                                                           \n"
"    vec3 ambient  = color*ambientIntensity;                                \n"
"    vec3 diffuse  = color*(1.0 - ambientIntensity)*max(dot(L, N), 0.0);    \n"
"    vec3 specular = vec3(specularIntensity*pow(max(dot(R, L), 0.0), 8.0)); \n"
"                                                                           \n"
"    gl_FragColor = vec4(ambient + diffuse + specular, 1.0);                \n"
"}                                                                          \n";


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool VectorDrawing::onInitialize()
{
    ref<DrawableGeo> geo1 = new DrawableGeo;
    {
        GeometryBuilderFaceList builder;
        GeometryUtils::createBox(Vec3f(0,0,0), 2.0, 2.0, 2.0, &builder);

        ref<Vec3fArray> vertices = builder.vertices();
        ref<UIntArray> faceList = builder.faceList();

        geo1->setVertexArray(vertices.p());
        geo1->setFromFaceList(*faceList);
        geo1->computeNormals();
    }

    ref<Part> part1 = new Part;
    part1->setDrawable(geo1.p());

    ref<Effect> eff1 = new Effect;
    eff1->setRenderState(new RenderStateMaterial_FF(Color3f(1, 0, 0)));

    part1->setEffect(eff1.p());

    // Vec result
    // -------------------------------------------------------------------------
    ref<DrawableVectors> vectorDrawable = new DrawableVectors("u_transformationMatrix", "u_color");
    
    ref<cvf::ShaderProgram> prog;
    //prog = m_openGLContext->resourceManager()->getOrCreateVectorDrawerShaderProgram(m_openGLContext.p()); //  Comment out this for fixed function

    ref<Vec3fArray> vertices = new Vec3fArray;
    ref<Vec3fArray> vecRes = new Vec3fArray;
    ref<Color3fArray> colors = new Color3fArray;

    int numVecs = 7;
    vertices->reserve(numVecs);
    vecRes->reserve(numVecs);
    colors->reserve(numVecs);

    vertices->add(Vec3f(1.0f, 1.0f, 1.0f));     vecRes->add(Vec3f(1.0f, 1.0f, 1.0f));         colors->add(Color3f(Color3::BLUE));
    vertices->add(Vec3f(-1.0f, -1.0f, -1.0f));  vecRes->add(Vec3f(-0.5f, -0.5f, -0.5f));      colors->add(Color3f(Color3::RED));
    vertices->add(Vec3f(1.0f, -1.0f, 1.0f));    vecRes->add(Vec3f(0.33f, -0.33f, 0.33f));     colors->add(Color3f(Color3::GRAY));
    vertices->add(Vec3f(-1.0f, 1.0f, 1.0f));    vecRes->add(Vec3f(-1.0f, 1.0f, 1.0f));        colors->add(Color3f(Color3::GREEN));
    vertices->add(Vec3f(1.0f, 0.0f, 1.0f));     vecRes->add(Vec3f(1.0f, 0.0f, 1.0f));         colors->add(Color3f(Color3::YELLOW));
    vertices->add(Vec3f(1.0f, 0.0f, 0.0f));     vecRes->add(Vec3f(1.0f, 0.0f, 0.0f));         colors->add(Color3f(Color3::BROWN));
    vertices->add(Vec3f(0.0f, 0.0f, -1.0f));    vecRes->add(Vec3f(0.0f, 0.0f, -1.0f));        colors->add(Color3f(Color3::PINK));

    vectorDrawable->setVectors(vertices.p(), vecRes.p());
    vectorDrawable->setColors(colors.p());

    // Create the arrow glyph for the vector drawer
    GeometryBuilderTriangles builder;
    ArrowGenerator gen;
    gen.generate(&builder);
    vectorDrawable->setGlyph(builder.trianglesUShort().p(), builder.vertices().p());

    ref<Part> part2 = new Part;
    part2->setDrawable(vectorDrawable.p());

    ref<Effect> eff2 = new Effect;
    eff2->setShaderProgram(prog.p());
    part2->setEffect(eff2.p());

    ref<ModelBasicList> myModel = new ModelBasicList;
    myModel->addPart(part1.p());
    myModel->addPart(part2.p());
    myModel->updateBoundingBoxesRecursive();

    m_renderSequence->rendering(0)->scene()->addModel(myModel.p());

    BoundingBox bb = m_renderSequence->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, -Vec3d::Z_AXIS, Vec3d::X_AXIS);
        m_trackball->setRotationPoint(bb.center());
    }

    return true;
}


} // namespace snip

