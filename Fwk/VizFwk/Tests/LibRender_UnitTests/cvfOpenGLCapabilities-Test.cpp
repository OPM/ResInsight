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
#include "cvfOpenGLCapabilities.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OpenGLCapabilitiesTest, Defaults)
{
    OpenGLCapabilities cap;
    EXPECT_FALSE(cap.hasCapability((OpenGLCapabilities::Capability)0xffffffff));
    EXPECT_FALSE(cap.supportsOpenGL2());
    EXPECT_TRUE(cap.supportsOpenGLVer(1));
    EXPECT_FALSE(cap.supportsOpenGLVer(2));
    EXPECT_FALSE(cap.supportsOpenGLVer(3));
    EXPECT_FALSE(cap.supportsFixedFunction());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OpenGLCapabilitiesTest, CopyConstructorAndAssignment)
{
    OpenGLCapabilities cap;
    cap.addCapablity(OpenGLCapabilities::TEXTURE_FLOAT);
    cap.configureOpenGLSupport(2);
    cap.setSupportsFixedFunction(true);

    {
        OpenGLCapabilities c(cap);
        EXPECT_TRUE(cap.hasCapability(OpenGLCapabilities::TEXTURE_FLOAT));
        EXPECT_TRUE(cap.supportsOpenGL2());
        EXPECT_TRUE(cap.supportsOpenGLVer(2));
        EXPECT_TRUE(cap.supportsFixedFunction());
    }

    {
        OpenGLCapabilities c;
        c = cap;
        EXPECT_TRUE(cap.hasCapability(OpenGLCapabilities::TEXTURE_FLOAT));
        EXPECT_TRUE(cap.supportsOpenGL2());
        EXPECT_TRUE(cap.supportsOpenGLVer(2));
        EXPECT_TRUE(cap.supportsFixedFunction());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OpenGLCapabilitiesTest, AddRemoveCapabilties)
{
    OpenGLCapabilities cap;

    cap.addCapablity(OpenGLCapabilities::TEXTURE_FLOAT);
    cap.addCapablity(OpenGLCapabilities::TEXTURE_RECTANGLE);
    EXPECT_TRUE(cap.hasCapability(OpenGLCapabilities::TEXTURE_FLOAT));
    EXPECT_TRUE(cap.hasCapability(OpenGLCapabilities::TEXTURE_RECTANGLE));

    cap.removeCapablity(OpenGLCapabilities::TEXTURE_RECTANGLE);
    EXPECT_TRUE(cap.hasCapability(OpenGLCapabilities::TEXTURE_FLOAT));
    EXPECT_FALSE(cap.hasCapability(OpenGLCapabilities::TEXTURE_RECTANGLE));
}


