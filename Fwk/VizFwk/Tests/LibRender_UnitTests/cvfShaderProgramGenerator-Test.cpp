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
#include "cvfShaderProgramGenerator.h"
#include "cvfTrace.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ShaderSourceCombinerTest, OneLineShaders)
{
    static const char str1[] =
    "uniform float uni1;\n";

    static const char str2[] =
    "uniform float uni2;\n";

    static const char str3[] =
    "uniform float uni3;\n";

    static const char res[] =
    "uniform float uni1;\n"
    "uniform float uni2;\n"
    "uniform float uni3;\n";

    std::vector<String> shaderCodes;
    std::vector<String> shaderNames;
    shaderCodes.push_back(str1);
    shaderNames.push_back("str1");
    shaderCodes.push_back(str2);
    shaderNames.push_back("str2");
    shaderCodes.push_back(str3);
    shaderNames.push_back("str3");

    ShaderSourceCombiner combiner(shaderCodes, shaderNames);
    combiner.enableDebugComments(false);
    String combSrc = combiner.combinedSource();

    EXPECT_STREQ(res, combSrc.toAscii().ptr());
    //cvf::Trace::show(combSrc.toAscii().ptr());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ShaderSourceCombinerTest, DifferentVersions)
{
    static const char str1[] =
    "#version 310\n"
    "void myFunc() {}\n";

    static const char str2[] =
    "#version 330\n"
    "void main() {}\n";

    static const char res[] =
    "#version 330\n"
    "\n"
    "void myFunc() {}\n"
    "void main() {}\n";

    std::vector<String> shaderCodes;
    std::vector<String> shaderNames;
    shaderCodes.push_back(str1);
    shaderNames.push_back("str1");
    shaderCodes.push_back(str2);
    shaderNames.push_back("str2");

    ShaderSourceCombiner combiner(shaderCodes, shaderNames);
    combiner.enableDebugComments(false);
    String combSrc = combiner.combinedSource();

    EXPECT_STREQ(res, combSrc.toAscii().ptr());
    //cvf::Trace::show(combSrc.toAscii().ptr());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ShaderSourceCombinerTest, SameUniformName)
{
    static const char str1[] =  "uniform float uni1;\n";
    static const char str2[] =  "uniform float uni1;\n";
    static const char res[] =   "uniform float uni1;\n";

    std::vector<String> shaderCodes;
    std::vector<String> shaderNames;
    shaderCodes.push_back(str1);
    shaderNames.push_back("str1");
    shaderCodes.push_back(str2);
    shaderNames.push_back("str2");

    ShaderSourceCombiner combiner(shaderCodes, shaderNames);
    combiner.enableDebugComments(false);
    String combSrc = combiner.combinedSource();

    //cvf::Trace::show(combSrc.toAscii().ptr());
    EXPECT_STREQ(res, combSrc.toAscii().ptr());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ShaderSourceCombinerTest, SameVaryingName)
{
    static const char str1[] =  "varying float var1;\n";
    static const char str2[] =  "varying float var1;\n";
    static const char res[] =   "varying float var1;\n";

    std::vector<String> shaderCodes;
    std::vector<String> shaderNames;
    shaderCodes.push_back(str1);
    shaderNames.push_back("str1");
    shaderCodes.push_back(str2);
    shaderNames.push_back("str2");

    ShaderSourceCombiner combiner(shaderCodes, shaderNames);
    combiner.enableDebugComments(false);
    String combSrc = combiner.combinedSource();

    //cvf::Trace::show(combSrc.toAscii().ptr());
    EXPECT_STREQ(res, combSrc.toAscii().ptr());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ShaderSourceCombinerTest, SameAttributeName)
{
    static const char str1[] =  "attribute float attr1;\n";
    static const char str2[] =  "attribute float attr1;\n";
    static const char res[] =   "attribute float attr1;\n";

    std::vector<String> shaderCodes;
    std::vector<String> shaderNames;
    shaderCodes.push_back(str1);
    shaderNames.push_back("str1");
    shaderCodes.push_back(str2);
    shaderNames.push_back("str2");

    ShaderSourceCombiner combiner(shaderCodes, shaderNames);
    combiner.enableDebugComments(false);
    String combSrc = combiner.combinedSource();

    //cvf::Trace::show(combSrc.toAscii().ptr());
    EXPECT_STREQ(res, combSrc.toAscii().ptr());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ShaderSourceCombinerTest, SameUniformDifferentWhitespace)
{
    static const char str1[] =  "uniform float uni1;\n";
    static const char str2[] =  "uniform\t float \t uni1;\n";
    static const char res[] =   "uniform float uni1;\n";

    std::vector<String> shaderCodes;
    std::vector<String> shaderNames;
    shaderCodes.push_back(str1);
    shaderNames.push_back("str1");
    shaderCodes.push_back(str2);
    shaderNames.push_back("str2");

    ShaderSourceCombiner combiner(shaderCodes, shaderNames);
    combiner.enableDebugComments(false);
    String combSrc = combiner.combinedSource();

    //cvf::Trace::show(combSrc.toAscii().ptr());
    EXPECT_STREQ(res, combSrc.toAscii().ptr());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ShaderSourceCombinerTest, DuplicateGlobalKeywords)
{
    static const char str1[] =
    "uniform float uni1;\n"
    "uniform  int  uni2;\n"
    "varying float var1;\n";

    static const char str2[] =
    " varying  float  var1; \n"
    "varying float var2;\n"
    "uniform  int uni2;\n";

    static const char res[] =
    "uniform float uni1;\n"
    "uniform  int  uni2;\n"
    "varying float var1;\n"
    "varying float var2;\n";

    std::vector<String> shaderCodes;
    std::vector<String> shaderNames;
    shaderCodes.push_back(str1);
    shaderNames.push_back("str1");
    shaderCodes.push_back(str2);
    shaderNames.push_back("str2");

    ShaderSourceCombiner combiner(shaderCodes, shaderNames);
    combiner.enableDebugComments(false);
    String combSrc = combiner.combinedSource();

    //cvf::Trace::show(combSrc.toAscii().ptr());
    EXPECT_STREQ(res, combSrc.toAscii().ptr());
}


