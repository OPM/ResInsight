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
#include "cvfSampler.h"

#include "gtest/gtest.h"

using namespace cvf;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(SamplerTest, Defaults)
{
    Sampler s;

    EXPECT_EQ(Sampler::REPEAT,                s.wrapModeS());
    EXPECT_EQ(Sampler::REPEAT,                s.wrapModeT());
    EXPECT_EQ(Sampler::NEAREST_MIPMAP_LINEAR, s.minFilter());
    EXPECT_EQ(Sampler::LINEAR,                s.magFilter());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(SamplerTest, SettersAndGetters)
{
    Sampler s;

    s.setWrapModeS(Sampler::REPEAT);
    s.setWrapModeT(Sampler::MIRRORED_REPEAT);
    s.setMinFilter(Sampler::LINEAR_MIPMAP_NEAREST);
    s.setMagFilter(Sampler::LINEAR);

    EXPECT_EQ(Sampler::REPEAT,                  s.wrapModeS());
    EXPECT_EQ(Sampler::MIRRORED_REPEAT,         s.wrapModeT());
    EXPECT_EQ(Sampler::LINEAR_MIPMAP_NEAREST,   s.minFilter());
    EXPECT_EQ(Sampler::LINEAR,                  s.magFilter());
}
