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
#include "cvfShader.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ShaderTest, BasicConstruction)
{
    ref<Shader> vs = new Shader(Shader::VERTEX_SHADER, "VertexShader");
    ref<Shader> fs = new Shader(Shader::FRAGMENT_SHADER, "FragmentShader");
    ref<Shader> gs = new Shader(Shader::GEOMETRY_SHADER, "GeometryShader");
    EXPECT_EQ(1, vs->refCount());
    EXPECT_EQ(1, fs->refCount());
    EXPECT_EQ(1, gs->refCount());

    EXPECT_EQ(Shader::VERTEX_SHADER, vs->shaderType());
    EXPECT_EQ(Shader::FRAGMENT_SHADER, fs->shaderType());
    EXPECT_EQ(Shader::GEOMETRY_SHADER, gs->shaderType());

    EXPECT_TRUE(vs->shaderName() == "VertexShader");
    EXPECT_TRUE(fs->shaderName() == "FragmentShader");
    EXPECT_TRUE(gs->shaderName() == "GeometryShader");

    EXPECT_EQ(0u, vs->shaderOglId());
    EXPECT_EQ(0u, fs->shaderOglId());
    EXPECT_EQ(0u, gs->shaderOglId());

    EXPECT_EQ(-1, vs->compiledVersionTick());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ShaderTest, ConstructionWithSourceCode)
{
    const String dummySrc("Some source code");
    ref<Shader> shad = new Shader(Shader::VERTEX_SHADER, "VertexShader", dummySrc);

    EXPECT_EQ(Shader::VERTEX_SHADER, shad->shaderType());
    EXPECT_TRUE(shad->shaderName() == "VertexShader");

    EXPECT_EQ(-1, shad->compiledVersionTick());
}
