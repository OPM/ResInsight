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

#include "snipCoreProfileTest.h"

namespace snip {


static const char sg_MinimalVertexShader_v33[] = 
"#version 330                                                               \n"
"                                                                           \n"
"uniform mat4 cvfu_modelViewProjectionMatrix;                               \n"
"                                                                           \n"
"in vec4 cvfa_vertex;                                                       \n"
"                                                                           \n"
"void main ()                                                               \n"
"{                                                                          \n"
"    gl_Position = cvfu_modelViewProjectionMatrix*cvfa_vertex;              \n"
"}                                                                          \n";

static const char sg_FixedColorFragmentShader_v33[] = 
"#version 330                                                               \n"
"                                                                           \n"
"out vec4 outFragColor;                                                     \n"
"                                                                           \n"
"void main()                                                                \n"
"{                                                                          \n"
"    outFragColor = vec4(1.0, 0.0, 1.0, 1.0);                               \n"
"}                                                                          \n";



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool CoreProfileTest::onInitialize()
{
    ref<Vec3fArray> triVertices = new Vec3fArray;
    triVertices->reserve(3);
    triVertices->add(Vec3f(-1,-1, 0));
    triVertices->add(Vec3f( 1,-1, 0));
    triVertices->add(Vec3f( 0, 2, 0));

    ref<UIntArray> triIndices = new UIntArray;
    triIndices->reserve(3);
    triIndices->add(0);
    triIndices->add(1);
    triIndices->add(2);


    ref<DrawableGeo> geo = new DrawableGeo;
    ref<PrimitiveSetIndexedUInt> primSet = new PrimitiveSetIndexedUInt(PT_TRIANGLES);
    primSet->setIndices(triIndices.p());
    geo->addPrimitiveSet(primSet.p());
    geo->setVertexArray(triVertices.p());
    geo->computeNormals();

    ref<Part> part = new Part;
    part->setDrawable(geo.p());

    ref<ShaderProgram> prog = new ShaderProgram;
    prog->addShader(new Shader(Shader::VERTEX_SHADER,   "sg_MinimalVertexShader_v33",       sg_MinimalVertexShader_v33));
    prog->addShader(new Shader(Shader::FRAGMENT_SHADER, "sg_FixedColorFragmentShader_v33",  sg_FixedColorFragmentShader_v33));
    
    // Link and show log to see any warnings
    prog->linkProgram(m_openGLContext.p());
    Trace::show(prog->programInfoLog(m_openGLContext.p()));

    ref<Effect> eff = new Effect;
    eff->setShaderProgram(prog.p());
    part->setEffect(eff.p());
    
    ref<ModelBasicList> myModel = new ModelBasicList;
    myModel->addPart(part.p());
    myModel->updateBoundingBoxesRecursive();

    m_renderSequence->firstRendering()->scene()->addModel(myModel.p());
    m_camera->setFromLookAt(Vec3d(0,0,6), Vec3d(0,0,0), Vec3d(0,1,0));

    return true;
}

}


