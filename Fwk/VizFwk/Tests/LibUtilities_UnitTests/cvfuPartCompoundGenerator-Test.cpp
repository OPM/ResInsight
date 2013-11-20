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
#include "cvfuPartCompoundGenerator.h"
#include "cvfDrawable.h"

#include "gtest/gtest.h"

using namespace cvf;
using namespace cvfu;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PartCompoundGeneratorTest, BasicConstruction)
{
    ref<PartCompoundGenerator> gen = new PartCompoundGenerator;

    ASSERT_EQ(1, gen->refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PartCompoundGeneratorTest, CreateSpheres)
{
    Collection<Part> parts;
    PartCompoundGenerator gen;

    gen.setPartDistribution(Vec3i(3, 4, 5));
    gen.setNumEffects(2);
    gen.useRandomEffectAssignment(false);
    gen.setExtent(Vec3f(3,3,3));
    gen.generateSpheres(10, 10, &parts);

    ASSERT_EQ(60u, parts.size());

    ref<Part> part0 = parts[0];
    ASSERT_TRUE(part0->drawable() != NULL);
    ASSERT_EQ(180u, part0->drawable()->faceCount());

    ref<Part> part20 = parts[20];
    ASSERT_TRUE(part20->drawable() != NULL);
    ASSERT_EQ(180u, part20->drawable()->faceCount());

    // Check that effects are assigned properly (2 effects alternating)
    ref<Part> part1 = parts[1];
    ref<Part> part2 = parts[2];
    ASSERT_TRUE(part0->effect() != NULL);
    ASSERT_TRUE(part1->effect() != NULL);
    ASSERT_TRUE(part2->effect() != NULL);
    ASSERT_TRUE(part0->effect() == part2->effect());
    ASSERT_TRUE(part0->effect() != part1->effect());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PartCompoundGeneratorTest, CreateBoxes)
{
    Collection<Part> parts;
    PartCompoundGenerator gen;

    gen.setPartDistribution(Vec3i(2, 3, 4));
    gen.setNumEffects(2);
    gen.useRandomEffectAssignment(true);
    gen.setExtent(Vec3f(3,3,3));
    gen.generateBoxes(&parts);

    ASSERT_EQ(24u, parts.size());

    ref<Part> part0 = parts[0];
    ASSERT_TRUE(part0->drawable() != NULL);
    ASSERT_EQ(12u, part0->drawable()->faceCount());
    ASSERT_EQ(12u, part0->drawable()->triangleCount());

    ref<Part> part20 = parts[20];
    ASSERT_TRUE(part20->drawable() != NULL);
    ASSERT_EQ(12u, part20->drawable()->faceCount());
    ASSERT_EQ(12u, part20->drawable()->triangleCount());
}
