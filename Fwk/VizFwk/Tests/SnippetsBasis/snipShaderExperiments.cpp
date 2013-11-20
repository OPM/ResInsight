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

#include "snipShaderExperiments.h"

#include "cvfuPartCompoundGenerator.h"

namespace snip {


// From posting in www.opengl.org, subject:  Clean Wireframe Over Solid Mesh
// http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=252937&Searchpage=6&Main=49189&Words=wireframe&Search=true#Post252937

const char g_pcMeshVert[] = 
"uniform mat4 cvfu_projectionMatrix;                                        \n "
"uniform mat4 cvfu_modelViewMatrix;                                         \n "
"                                                                           \n "
"attribute vec4 cvfa_vertex;                                                \n "
"                                                                           \n "
"void main ()                                                               \n "
"{                                                                          \n "
"   vec4 v = cvfu_modelViewMatrix * cvfa_vertex;                            \n"
"   v.xyz = v.xyz * 0.99;                                                   \n"
"   gl_Position = cvfu_projectionMatrix * v;                                \n"
"}                                                                          \n ";


const char g_pcMeshFrag[] = 
"void main ()                                                               \n "
"{                                                                          \n "
"    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);                               \n "
"}                                                                          \n ";



//  ------------------  Vertex  Shader  ----------------------------------------
const char g_pcAALinesVert[] = 
"#version  120                                                              \n "
"#extension  GL_EXT_gpu_shader4  :  enable                                  \n "
"#extension GL_EXT_geometry_shader4 : enable                                \n"
"                                                                           \n "
"uniform mat4 cvfu_modelViewProjectionMatrix;                               \n "
"attribute vec4 cvfa_vertex;                                                \n "
"                                                                           \n "
"void  main()                                                               \n "
"{                                                                          \n "
"    //gl_Position  =    ftransform();                                      \n "
"    gl_Position  =    cvfu_modelViewProjectionMatrix*cvfa_vertex;          \n "
"}                                                                          \n ";



//  ------------------  Geometry  Shader  --------------------------------------
const char g_pcAALinesGeo[] = 
"#version  120                                                              \n"
"#extension  GL_EXT_gpu_shader4  :  enable                                  \n"
"#extension GL_EXT_geometry_shader4 : enable                                \n"
"uniform  vec2  WIN_SCALE;                                                  \n"
"noperspective  varying  vec3  dist;                                        \n"
"                                                                           \n"
"void  main()                                                               \n"
"{                                                                          \n"
"    vec2  p0  =  WIN_SCALE  *  gl_PositionIn[0].xy/gl_PositionIn[0].w;     \n"
"    vec2  p1  =  WIN_SCALE  *  gl_PositionIn[1].xy/gl_PositionIn[1].w;     \n"
"    vec2  p2  =  WIN_SCALE  *  gl_PositionIn[2].xy/gl_PositionIn[2].w;     \n"
"    vec2  v0  =  p2-p1;                                                    \n"
"    vec2  v1  =  p2-p0;                                                    \n"
"    vec2  v2  =  p1-p0;                                                    \n"
"    float  area  =  abs(v1.x*v2.y  -  v1.y  *  v2.x);                      \n"
"    dist  =  vec3(area/length(v0),0,0);                                    \n"
"    gl_Position  =  gl_PositionIn[0];                                      \n"
"    EmitVertex();                                                          \n"
"    dist  =  vec3(0,area/length(v1),0);                                    \n"
"    gl_Position  =  gl_PositionIn[1];                                      \n"
"    EmitVertex();                                                          \n"
"    dist  =  vec3(0,0,area/length(v2));                                    \n"
"    gl_Position  =  gl_PositionIn[2];                                      \n"
"    EmitVertex();                                                          \n"
"    EndPrimitive();                                                        \n"
"}                                                                          \n";
                                                                            

//  ------------------  Fragment  Shader  -------------------------------------
const char g_pcAALinesFrag[] = 
"#version  120                                                              \n" 
"#extension  GL_EXT_gpu_shader4  :  enable                                  \n"
"#extension GL_EXT_geometry_shader4 : enable                                \n"
"noperspective  varying  vec3  dist;                                        \n"
"const  vec4  WIRE_COL  =  vec4(1.0,0.0,0.0,1);                             \n"
"const  vec4  FILL_COL  =  vec4(1,1,1,1);                                   \n"
"                                                                           \n"
"void  main()                                                               \n"
"{                                                                          \n"
"    float  d  =  min(dist[0],min(dist[1],dist[2]));                        \n"
"    float  I  =  exp2(-2*d*d);                                             \n"
"    gl_FragColor  =  I*WIRE_COL  +  (1.0  -  I)*FILL_COL;                  \n"
"}                                                                          \n";




//  Stein vertex shader ------------------------------------------
const char g_pcSteinVert[] = 
"attribute vec2 a_LocalCoord;                                    \n" 
"attribute vec3 a_Color1;                                        \n"
"attribute vec3 a_Color2;                                        \n"
"attribute vec3 a_Color3;                                        \n"
"attribute vec3 a_Color4;                                        \n"
"attribute vec3 a_Color5;                                        \n"
"                                                                \n"
"varying vec2 v_LocalCoord;                                      \n"
"varying vec3 v_Color1;                                          \n"
"varying vec3 v_Color2;                                          \n"
"varying vec3 v_Color3;                                          \n"
"varying vec3 v_Color4;                                          \n"
"varying vec3 v_Color5;                                          \n" 
"                                                                \n"
"uniform mat4 cvfu_modelViewProjectionMatrix;                    \n"
"                                                                \n"
"attribute vec4 cvfa_vertex;                                     \n"
"                                                                \n"
"void main()                                                     \n"
"{	                                                             \n"
"    v_LocalCoord = a_LocalCoord;                                \n"
"    v_Color1	 = a_Color1;                                     \n"
"    v_Color2	 = a_Color2;                                     \n"
"    v_Color3	 = a_Color3;                                     \n"
"    v_Color4	 = a_Color4;                                     \n"
"    v_Color5	 = a_Color5;                                     \n"
"                                                                \n"
"    gl_Position = cvfu_modelViewProjectionMatrix*cvfa_vertex;   \n"
"}                                                               \n";


//  Stein fragment shader ------------------------------------------
const char g_pcSteinFrag[] = 
"varying vec2 v_LocalCoord;                                                                                 \n" 
"varying vec3 v_Color1;		// color along edge v0-v1                                                       \n"
"varying vec3 v_Color2;		// color along edge v1-v2                                                       \n"
"varying vec3 v_Color3;		// color along edge v2-v3                                                       \n"
"varying vec3 v_Color4;		// color along edge v3-v0                                                       \n"
"varying vec3 v_Color5;		// cell face color                                                              \n"
"                                                                                                           \n"
"                                                                                                           \n"
"void colormapCorners(in vec2 coord, inout vec3 color)                                                      \n"
"{                                                                                                          \n"
"    if (coord.x < 0.2 && coord.y < 0.2)				// lower, left corner                               \n"
"    {                                                                                                      \n"
"        if (coord.x > coord.y) color = v_Color1;                                                           \n" 
"        else				   color = v_Color4;                                                            \n"
"    }                                                                                                      \n"
"    else if (coord.x > 0.8 && coord.y < 0.2)		// lower, right corner                                  \n"
"    {                                                                                                      \n"
"        float x = 1.0 - coord.x;                                                                           \n"
"                                                                                                           \n"
"        if (x < coord.y) color = v_Color2;                                                                 \n"
"        else		     color = v_Color1;                                                                  \n"
"    }                                                                                                      \n"
"    else if (coord.x > 0.8 && coord.y > 0.8)		// upper, right corner                                  \n"
"    {                                                                                                      \n"
"        if (coord.x > coord.y) color = v_Color2;                                                           \n" 
"        else				   color = v_Color3;                                                            \n"
"    }                                                                                                      \n"
"    else if (coord.x < 0.2 && coord.y > 0.8)		// upper, left corner                                   \n"
"    {                                                                                                      \n"
"        float y = 1.0 - coord.y;                                                                           \n"
"                                                                                                           \n"
"        if (coord.x > y) color = v_Color3;                                                                 \n"
"        else		     color = v_Color4;                                                                  \n"
"    }                                                                                                      \n"
"}                                                                                                          \n"
"                                                                                                           \n"
"                                                                                                           \n" 
"void colormapEdgeRegions(in vec2 coord, inout vec3 color)                                                  \n"
"{                                                                                                          \n"
"    if (coord.y < 0.2 && coord.x > 0.2 && coord.x < 0.8)                                                   \n"
"    {                                                                                                      \n"
"        color = v_Color1;                                                                                  \n"
"    }                                                                                                      \n"
"    else if (coord.x > 0.8 && coord.y > 0.2 && coord.y < 0.8)                                              \n"
"    {                                                                                                      \n"
"        color = v_Color2;                                                                                  \n"
"    }                                                                                                      \n"
"    else if (coord.y > 0.8 && coord.x > 0.2 && coord.x < 0.8)                                              \n"
"    {                                                                                                      \n" 
"        color = v_Color3;                                                                                  \n"
"    }                                                                                                      \n"
"    else if (coord.x < 0.2 && coord.y > 0.2 && coord.y < 0.8)                                              \n"
"    {                                                                                                      \n"
"        color = v_Color4;                                                                                  \n"
"    }                                                                                                      \n"
"}                                                                                                          \n"
"                                                                                                           \n"
"                                                                                                           \n"
"void main()                                                                                                \n"
"{                                                                                                          \n"
"    vec3 color = v_Color5;		// cell color to be used for areas that are not along edges                 \n" 
"                                                                                                           \n"
"    colormapEdgeRegions(v_LocalCoord, color);                                                              \n"
"    colormapCorners(v_LocalCoord, color);                                                                  \n"
"                                                                                                           \n"
"    gl_FragColor = vec4(color, 1.0);                                                                       \n"
"}                                                                                                          \n";

                                                                                                           
                                                                                                           
                                                                                                           
//--------------------------------------------------------------------------------------------------       
///                                                                                                        
//--------------------------------------------------------------------------------------------------
bool ShaderExperiments::onInitialize()
{
    //ref<ModelBasicList> myModel = createSpheresModel();
    ref<ModelBasicList> myModel = createBoxesModel();
    //ref<ModelBasicList> myModel = createSteinModel();
    myModel->updateBoundingBoxesRecursive();
    m_renderSequence->firstRendering()->scene()->addModel(myModel.p());

    //configAllParts_Minimal();
    //configAllParts_SimpleHeadlight();
    buildAndAddWireframeModel();

    //configureAllPartsWithAALinesShader();

    BoundingBox bb = myModel->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, -Vec3d::Z_AXIS, Vec3d::Y_AXIS);
    }

    return true;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<ModelBasicList> ShaderExperiments::createSteinModel()
{
    ref<Vec3fArray> vertices = new Vec3fArray;
    vertices->reserve(4);
    vertices->add(Vec3f(0, 0, 0));
    vertices->add(Vec3f(1, 0, 0));
    vertices->add(Vec3f(1, 1, 0));
    vertices->add(Vec3f(0, 1, 0));

    ref<VertexAttribute> localCoordAttribArray;
    {
        ref<Vec2fArray> localCoords = new Vec2fArray;
        localCoords->reserve(4);
        localCoords->add(Vec2f(0, 0));
        localCoords->add(Vec2f(1, 0));
        localCoords->add(Vec2f(1, 1));
        localCoords->add(Vec2f(0, 1));
        localCoordAttribArray = new Vec2fVertexAttribute("a_LocalCoord", localCoords.p());
    }

    ref<VertexAttribute> color1AttribArray;
    {
        ref<Color3fArray> colors = new Color3fArray;
        colors->resize(4);
        colors->setAll(Color3::RED);
        color1AttribArray = new Color3fVertexAttribute("a_Color1", colors.p());
    }

    ref<VertexAttribute> color2AttribArray;
    {
        ref<Color3fArray> colors = new Color3fArray;
        colors->resize(4);
        colors->setAll(Color3::GREEN);
        color2AttribArray = new Color3fVertexAttribute("a_Color2", colors.p());
    }

    ref<VertexAttribute> color3AttribArray;
    {
        ref<Color3fArray> colors = new Color3fArray;
        colors->resize(4);
        colors->setAll(Color3::BLUE);
        color3AttribArray = new Color3fVertexAttribute("a_Color3", colors.p());
    }

    ref<VertexAttribute> color4AttribArray;
    {
        ref<Color3fArray> colors = new Color3fArray;
        colors->resize(4);
        colors->setAll(Color3::YELLOW);
        color4AttribArray = new Color3fVertexAttribute("a_Color4", colors.p());
    }

    ref<VertexAttribute> color5AttribArray;
    {
        ref<Color3fArray> colors = new Color3fArray;
        colors->resize(4);
        colors->setAll(Color3::MAGENTA);
        color5AttribArray = new Color3fVertexAttribute("a_Color5", colors.p());
    }


    ref<PrimitiveSetDirect> primSet = new PrimitiveSetDirect(PT_TRIANGLE_FAN);
    primSet->setIndexCount(4);

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray(vertices.p());
    geo->addPrimitiveSet(primSet.p());
    geo->setVertexAttribute(localCoordAttribArray.p());
    geo->setVertexAttribute(color1AttribArray.p());
    geo->setVertexAttribute(color2AttribArray.p());
    geo->setVertexAttribute(color3AttribArray.p());
    geo->setVertexAttribute(color4AttribArray.p());
    geo->setVertexAttribute(color5AttribArray.p());
    geo->computeNormals();

    ShaderProgramGenerator shaderGen("SteinProg", ShaderSourceProvider::instance());
    shaderGen.addVertexCode(g_pcSteinVert);
    shaderGen.addFragmentCode(g_pcSteinFrag);
    ref<ShaderProgram> prog = shaderGen.generate();

    ref<Effect> eff = new Effect;
    eff->setShaderProgram(prog.p());

    ref<Part> part = new Part;
    part->setDrawable(geo.p());
    part->setEffect(eff.p());

    ref<ModelBasicList> myModel = new ModelBasicList;
    myModel->addPart(part.p());

    return myModel;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<ModelBasicList> ShaderExperiments::createSpheresModel()
{
    PartCompoundGenerator gen;
    gen.setPartDistribution(Vec3i(3, 3, 1));
    gen.setNumEffects(8);
    gen.useRandomEffectAssignment(false);
    gen.setExtent(Vec3f(3,3,3));
    gen.setOrigin(Vec3f(-1.5f, -1.5f, -1.5f));

    Collection<Part> parts;
    gen.generateSpheres(20, 20, &parts);

    ref<ModelBasicList> myModel = new ModelBasicList;
    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        myModel->addPart(parts[i].p());
    }

    return myModel;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<ModelBasicList> ShaderExperiments::createBoxesModel()
{
    PartCompoundGenerator gen;
    gen.setPartDistribution(Vec3i(3, 3, 1));
    gen.setNumEffects(8);
    gen.useRandomEffectAssignment(false);
    gen.setExtent(Vec3f(3,3,3));
    gen.setOrigin(Vec3f(-1.5f, -1.5f, -1.5f));

    Collection<Part> parts;
    gen.generateBoxes(&parts);

    ref<ModelBasicList> myModel = new ModelBasicList;
    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        myModel->addPart(parts[i].p());
    }

    return myModel;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> extractMesh(const DrawableGeo& srcGeo)
{
    size_t numFaces = srcGeo.faceCount();

    ref<UIntArray> indices = new UIntArray;
    indices->reserve(6*numFaces);

    std::set<int64> edgeSet;

    size_t i;
    for (i = 0; i < numFaces; i++)
    {
        UIntArray faceIndices;
        srcGeo.getFaceIndices(i, &faceIndices);

        size_t numFaceIndices = faceIndices.size();
        size_t j;
        for (j = 0; j < numFaceIndices; j++)
        {
            cvf::uint v1 = faceIndices[j];
            cvf::uint v2 = faceIndices[(j < numFaceIndices - 1) ? j + 1 : 0];
            if (v1 > v2)
            {
                cvf::uint temp = v1;
                v1 = v2;
                v2 = temp;
            }

            int64 edgeId = static_cast<int64>(v1) + (static_cast<int64>(v2) << 32);
            if (edgeSet.find(edgeId) == edgeSet.end())
            {
                indices->add(v1);
                indices->add(v2);
                edgeSet.insert(edgeId);
            }
            else
            {
                //Trace::show("Jada");
            }

        }
    }

    ref<Vec3fArray> vertices = new Vec3fArray(*srcGeo.vertexArray());

    ref<PrimitiveSetIndexedUInt> primSet = new PrimitiveSetIndexedUInt(PT_LINES);
    primSet->setIndices(indices.p());

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray(vertices.p());
    geo->addPrimitiveSet(primSet.p());
    geo->computeNormals();

    return geo;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderExperiments::buildAndAddWireframeModel()
{
    ref<Effect> eff = new Effect;
    {
        ref<Shader> vs = new Shader(Shader::VERTEX_SHADER,   "g_pcMeshVert", g_pcMeshVert);
        ref<Shader> fs = new Shader(Shader::FRAGMENT_SHADER, "g_pcMeshFrag", g_pcMeshFrag);

        ref<ShaderProgram> prog = new ShaderProgram;
        prog->addShader(vs.p());
        prog->addShader(fs.p());

        linkProgramVerbose(prog.p());

        eff->setShaderProgram(prog.p());
    }


    Model* srcModel = m_renderSequence->rendering(0)->scene()->model(0);
    CVF_ASSERT(srcModel);

    Collection<Part> srcParts;
    srcModel->allParts(&srcParts);

    ref<ModelBasicList> newModel = new ModelBasicList;

    size_t i;
    for (i = 0; i < srcParts.size(); i++)
    {
        ref<Part> srcPart = srcParts[i];
        DrawableGeo* srcGeo = dynamic_cast<DrawableGeo*>(srcPart->drawable());

        ref<DrawableGeo> newGeo = extractMesh(*srcGeo);

        ref<Part> newPart = new Part;
        newPart->setDrawable(newGeo.p());
        newPart->setEffect(eff.p());
        newPart->setTransform(srcPart->transform());

        newModel->addPart(newPart.p());
    }

    newModel->updateBoundingBoxesRecursive();
    m_renderSequence->rendering(0)->scene()->addModel(newModel.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderExperiments::configAllParts_Minimal()
{
    ShaderProgramGenerator gen("UnlitSphere", ShaderSourceProvider::instance());
    gen.addVertexCode(ShaderSourceRepository::vs_Minimal);
    gen.addFragmentCode(ShaderSourceRepository::fs_FixedColorMagenta);
    ref<ShaderProgram> prog = gen.generate();

    linkProgramVerbose(prog.p());

    ref<Effect> eff = new Effect;
    eff->setShaderProgram(prog.p());

    setEffectForAllParts(eff.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderExperiments::configAllParts_SimpleHeadlight()
{
    cvf::ShaderProgramGenerator gen("SimpleHeadlight", cvf::ShaderSourceProvider::instance());
    gen.configureStandardHeadlightColor();
    ref<ShaderProgram> prog = gen.generate();
    linkProgramVerbose(prog.p());

    ref<Effect> eff = new Effect;
    eff->setShaderProgram(prog.p());
    eff->setUniform(new UniformFloat("u_color", Color4f(Color3::YELLOW)));

    setEffectForAllParts(eff.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderExperiments::configureAllPartsWithAALinesShader()
{
    ref<Effect> effAALines = new Effect;
    {
        ref<Shader> vs = new Shader(Shader::VERTEX_SHADER, "g_pcAALinesVert", g_pcAALinesVert);
        ref<Shader> gs = new Shader(Shader::GEOMETRY_SHADER, "g_pcAALinesGeo", g_pcAALinesGeo);
        ref<Shader> fs = new Shader(Shader::FRAGMENT_SHADER, "g_pcAALinesFrag", g_pcAALinesFrag);

        ref<ShaderProgram> prog = new ShaderProgram;
        prog->addShader(vs.p());
        prog->addShader(gs.p());
        prog->addShader(fs.p());

        effAALines->setShaderProgram(prog.p());
        
        ref<UniformFloat> uniformWinScale = new UniformFloat("WIN_SCALE");
        uniformWinScale->set(Vec2f(1000.0f, 1000.0f));
        effAALines->setUniform(uniformWinScale.p());
    }

    Model* model = m_renderSequence->rendering(0)->scene()->model(0);
    CVF_ASSERT(model);

    Collection<Part> parts;
    model->allParts(&parts);

    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        ref<Part> part = parts[i];
        part->setEffect(effAALines.p());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderExperiments::linkProgramVerbose(ShaderProgram* prog)
{
    // Link and show log to see any warnings
    prog->linkProgram(m_openGLContext.p());

    if (!prog->programInfoLog(m_openGLContext.p()).isEmpty()) 
    {
        Trace::show(prog->programInfoLog(m_openGLContext.p()));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderExperiments::setEffectForAllParts(Effect* effect)
{
    Model* model = m_renderSequence->firstRendering()->scene()->model(0);
    CVF_ASSERT(model);

    Collection<Part> parts;
    model->allParts(&parts);

    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        ref<Part> part = parts[i];
        part->setEffect(effect);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderExperiments::onPaintEvent(PostEventAction* postEventAction)
{
    if (m_renderSequence.notNull() && m_camera.notNull())
    {
        BoundingBox bb = m_renderSequence->boundingBox();
        if (bb.isValid() && bb.extent().length() > 0.0)
        {
            double minClipDist = bb.extent().length()/10000.0;
            m_camera->setClipPlanesFromBoundingBox(bb, minClipDist);
        }
    }

    m_renderSequence->render(m_openGLContext.p());
    CVF_CHECK_OGL(m_openGLContext.p());

    //testDirectShaderBasedDrawing();
    //glUseProgram(0);
    //CVF_CHECK_OGL;

    if (postEventAction)
    {
        *postEventAction = NONE;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderExperiments::testDirectShaderBasedDrawing(OpenGLContext* oglContext)
{
    GeometryBuilderDrawableGeo builder;
    BoxGenerator boxGen;
    boxGen.setMinMax(Vec3d(0, 0, 0), Vec3d(3, 3, 3));
    boxGen.generate(&builder);

    ref<DrawableGeo> geo = builder.drawableGeo();;
    geo->computeNormals();

    cvf::ShaderProgramGenerator gen("SimpleHeadlight", cvf::ShaderSourceProvider::instance());
    gen.configureStandardHeadlightColor();
    ref<ShaderProgram> prog = gen.generate();
    prog->linkProgram(m_openGLContext.p());
    prog->useProgram(m_openGLContext.p());

	// Not accepted by llvm-gcc-4.2: prog->applyUniform(UniformFloat("u_color", Color4f(Color3::RED)));
	UniformFloat uniform("u_color", Color4f(Color3::RED));
    prog->applyUniform(m_openGLContext.p(), uniform);

    // Should maybe set this one as well
    Vec2i viewportPos(0, 0);
    Vec2ui viewportSize(0, 0);

    Mat4d projectionMatrix;
    Mat4d modelViewMatrix;
    glGetDoublev(GL_PROJECTION_MATRIX, (double*)projectionMatrix.ptr());
    glGetDoublev(GL_MODELVIEW_MATRIX, (double*)modelViewMatrix.ptr());

    MatrixState matrixState(viewportPos, viewportSize, projectionMatrix, modelViewMatrix);
    prog->applyFixedUniforms(m_openGLContext.p(), matrixState);

    geo->render(oglContext, prog.p(), matrixState);
}


} // namespace snip

