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
#include "cvfShaderProgram.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ShaderProgramTest, BasicConstruction)
{
    ref<Shader> vs = new Shader(Shader::VERTEX_SHADER, "VS");
    ref<Shader> fs = new Shader(Shader::FRAGMENT_SHADER, "FS");
    ref<Shader> gs = new Shader(Shader::GEOMETRY_SHADER, "GS");
    EXPECT_EQ(1, vs->refCount());
    EXPECT_EQ(1, fs->refCount());
    EXPECT_EQ(1, gs->refCount());

    {
        ref<ShaderProgram> prog = new ShaderProgram;
        ASSERT_EQ(1, prog->refCount());

        prog->addShader(vs.p());
        prog->addShader(fs.p());
        prog->addShader(gs.p());

        EXPECT_EQ(2, vs->refCount());
        EXPECT_EQ(2, fs->refCount());
        EXPECT_EQ(2, gs->refCount());

        ASSERT_EQ(3, prog->shaderCount());
        EXPECT_EQ(vs.p(), prog->shader(0));
        EXPECT_EQ(fs.p(), prog->shader(1));
        EXPECT_EQ(gs.p(), prog->shader(2));

		EXPECT_EQ(0, prog->programOglId());
    }

    EXPECT_EQ(1, vs->refCount());
    EXPECT_EQ(1, fs->refCount());
    EXPECT_EQ(1, gs->refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#ifdef _DEBUG
TEST(ShaderProgramDeathTest, AddWithNullPointerShader)
{
    ref<ShaderProgram> prog = new ShaderProgram;

    EXPECT_DEATH(prog->addShader(NULL), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#ifdef _DEBUG
TEST(ShaderProgramDeathTest, IllegalAccess)
{
    ref<ShaderProgram> prog = new ShaderProgram;

    EXPECT_DEATH(prog->shader(0), "Assertion");

    ref<Shader> s1 = new Shader(Shader::VERTEX_SHADER, "S1");
    ref<Shader> s2 = new Shader(Shader::VERTEX_SHADER, "S2");
    prog->addShader(s1.p());
    prog->addShader(s2.p());

    EXPECT_DEATH(prog->shader(2), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ShaderProgramTest, Name)
{
    {
        ref<ShaderProgram> prog = new ShaderProgram;
        EXPECT_TRUE(prog->programName() == "");
    }

    {
        ref<ShaderProgram> prog = new ShaderProgram("CustomName");
        EXPECT_TRUE(prog->programName() == "CustomName");
    }

    {
        ref<ShaderProgram> prog = new ShaderProgram;
        prog->addShader(new Shader(Shader::VERTEX_SHADER,   "VertShader"));
        prog->addShader(new Shader(Shader::GEOMETRY_SHADER, "GeoShader"));
        prog->addShader(new Shader(Shader::FRAGMENT_SHADER, "FragShader"));
        EXPECT_TRUE(prog->programName() == "VertShader # GeoShader # FragShader");
    }

    {
        ref<ShaderProgram> prog = new ShaderProgram("CustomName");
        prog->addShader(new Shader(Shader::VERTEX_SHADER,   "AnotherName"));
        EXPECT_TRUE(prog->programName() == "CustomName");

        prog->setProgramName("YetAnotherName");
        EXPECT_TRUE(prog->programName() == "YetAnotherName");
    }
}


