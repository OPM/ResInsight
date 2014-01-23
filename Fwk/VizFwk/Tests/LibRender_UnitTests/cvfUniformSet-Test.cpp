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
#include "cvfUniformSet.h"
#include "cvfUniform.h"

#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UniformSetTest, BasicConstruction)
{
    UniformSet us;
    ASSERT_EQ(0, us.count());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UniformSetTest, SetUniform)
{
    UniformSet us;

    ref<Uniform> u1 = new UniformFloat("u1");
    us.setUniform(u1.p());
    ASSERT_EQ(1, us.count());
    EXPECT_EQ(u1.p(), us.uniform(0));

    ref<Uniform> u2 = new UniformFloat("u2");
    us.setUniform(u2.p());
    ASSERT_EQ(2, us.count());
    EXPECT_EQ(u2.p(), us.uniform(1));

    // Add the uniform 1 again
    us.setUniform(u1.p());
    ASSERT_EQ(2, us.count());
    EXPECT_EQ(u1.p(), us.uniform(0));

    // Add a uniform with the same name
    ref<Uniform> u1_2 = new UniformFloat("u1");
    us.setUniform(u1_2.p());
    ASSERT_EQ(2, us.count());
    EXPECT_EQ(u1_2.p(), us.uniform(0));
    EXPECT_EQ(u2.p(), us.uniform(1));

    // Then another new one
    ref<Uniform> u3 = new UniformFloat("u3");
    us.setUniform(u3.p());
    ASSERT_EQ(3, us.count());
    EXPECT_EQ(u3.p(), us.uniform(2));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_TIGHT_ASSERTS == 1
TEST(UniformSetDeathTest, SetWithNullPtr)
{
    UniformSet us;
    EXPECT_DEATH(us.setUniform(NULL), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#if CVF_ENABLE_TIGHT_ASSERTS == 1
TEST(UniformSetDeathTest, AccessEmptySet)
{
    UniformSet us;
    ASSERT_EQ(0, us.count());

    EXPECT_DEATH(us.uniform(0), "Assertion");
    EXPECT_DEATH(us.uniform(1), "Assertion");

    us.setUniform(new UniformFloat("dummy"));
    EXPECT_DEATH(us.uniform(1), "Assertion");
}
#endif


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UniformSetTest, RemoveUniform)
{
    UniformSet us;

    ref<Uniform> u1 = new UniformFloat("u1");
    ref<Uniform> u2 = new UniformFloat("u2");
    ref<Uniform> u3 = new UniformFloat("u3");
    ASSERT_EQ(1, u1->refCount());
    ASSERT_EQ(1, u1->refCount());
    ASSERT_EQ(1, u1->refCount());

    us.setUniform(u1.p());
    us.setUniform(u2.p());
    us.setUniform(u3.p());
    ASSERT_EQ(3, us.count());
    ASSERT_EQ(2, u1->refCount());
    ASSERT_EQ(2, u2->refCount());
    ASSERT_EQ(2, u3->refCount());

    us.removeUniform(u2.p());
    ASSERT_EQ(2, us.count());
    ASSERT_EQ(1, u2->refCount());

    // Everything in place?
    EXPECT_EQ(u1.p(), us.uniform(0));
    EXPECT_EQ(u3.p(), us.uniform(1));

    // Remove again
    us.removeUniform(u2.p());
    ASSERT_EQ(2, us.count());
    ASSERT_EQ(1, u2->refCount());

    // Remove with NULL
    us.removeUniform(NULL);
    ASSERT_EQ(2, us.count());
    ASSERT_EQ(1, u2->refCount());

    us.removeUniform(u1.p());
    us.removeUniform(u3.p());
    ASSERT_EQ(0, us.count());
    ASSERT_EQ(1, u1->refCount());
    ASSERT_EQ(1, u2->refCount());
    ASSERT_EQ(1, u3->refCount());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(UniformSetTest, BasicLifeCycle)
{
    UniformSet us;
    ASSERT_EQ(0, us.count());

    ref<Uniform> u1 = new UniformFloat("u1");
    ref<Uniform> u2 = new UniformMatrixf("u2");

    us.setUniform(u1.p());
    us.setUniform(u2.p());
    ASSERT_EQ(2, us.count());

    EXPECT_EQ(u1.p(), us.uniform(0));
    EXPECT_EQ(u2.p(), us.uniform(1));
}


