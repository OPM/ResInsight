/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025- Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "gtest/gtest.h"

#include "RigBoundingBoxIjk.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigBoundingBoxIjk, BasicConstruction )
{
    // Default constructor
    RigBoundingBoxIjk defaultBox;
    EXPECT_TRUE( defaultBox.isValid() ); // Min and max are both (0,0,0)
    EXPECT_EQ( cvf::Vec3st::ZERO, defaultBox.min() );
    EXPECT_EQ( cvf::Vec3st::ZERO, defaultBox.max() );

    // Constructor with min and max
    RigBoundingBoxIjk box( cvf::Vec3st( 1, 2, 3 ), cvf::Vec3st( 5, 6, 7 ) );
    EXPECT_TRUE( box.isValid() );
    EXPECT_EQ( cvf::Vec3st( 1, 2, 3 ), box.min() );
    EXPECT_EQ( cvf::Vec3st( 5, 6, 7 ), box.max() );

    // Invalid box (min > max)
    RigBoundingBoxIjk invalidBox( cvf::Vec3st( 10, 10, 10 ), cvf::Vec3st( 5, 5, 5 ) );
    EXPECT_FALSE( invalidBox.isValid() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigBoundingBoxIjk, IsValid )
{
    // Valid boxes
    EXPECT_TRUE( RigBoundingBoxIjk( cvf::Vec3st( 0, 0, 0 ), cvf::Vec3st( 10, 10, 10 ) ).isValid() );
    EXPECT_TRUE( RigBoundingBoxIjk( cvf::Vec3st( 5, 5, 5 ), cvf::Vec3st( 5, 5, 5 ) ).isValid() ); // Single point

    // Invalid boxes (min > max in at least one dimension)
    EXPECT_FALSE( RigBoundingBoxIjk( cvf::Vec3st( 10, 0, 0 ), cvf::Vec3st( 5, 10, 10 ) ).isValid() ); // X invalid
    EXPECT_FALSE( RigBoundingBoxIjk( cvf::Vec3st( 0, 10, 0 ), cvf::Vec3st( 10, 5, 10 ) ).isValid() ); // Y invalid
    EXPECT_FALSE( RigBoundingBoxIjk( cvf::Vec3st( 0, 0, 10 ), cvf::Vec3st( 10, 10, 5 ) ).isValid() ); // Z invalid
    EXPECT_FALSE( RigBoundingBoxIjk( cvf::Vec3st( 10, 10, 10 ), cvf::Vec3st( 5, 5, 5 ) ).isValid() ); // All invalid
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigBoundingBoxIjk, OverlapDetection )
{
    RigBoundingBoxIjk box1( cvf::Vec3st( 0, 0, 0 ), cvf::Vec3st( 10, 10, 10 ) );

    // Same box overlaps with itself
    EXPECT_TRUE( box1.overlaps( box1 ) );

    // Completely inside
    RigBoundingBoxIjk box2( cvf::Vec3st( 2, 2, 2 ), cvf::Vec3st( 8, 8, 8 ) );
    EXPECT_TRUE( box1.overlaps( box2 ) );
    EXPECT_TRUE( box2.overlaps( box1 ) ); // Symmetric

    // Partially overlapping
    RigBoundingBoxIjk box3( cvf::Vec3st( 5, 5, 5 ), cvf::Vec3st( 15, 15, 15 ) );
    EXPECT_TRUE( box1.overlaps( box3 ) );
    EXPECT_TRUE( box3.overlaps( box1 ) );

    // Touching at edge (inclusive bounds - should overlap)
    RigBoundingBoxIjk box4( cvf::Vec3st( 10, 10, 10 ), cvf::Vec3st( 20, 20, 20 ) );
    EXPECT_TRUE( box1.overlaps( box4 ) );
    EXPECT_TRUE( box4.overlaps( box1 ) );

    // Completely separate (no overlap)
    RigBoundingBoxIjk box5( cvf::Vec3st( 20, 20, 20 ), cvf::Vec3st( 30, 30, 30 ) );
    EXPECT_FALSE( box1.overlaps( box5 ) );
    EXPECT_FALSE( box5.overlaps( box1 ) );

    // Overlap in 2D but not 3D (separated in Z)
    RigBoundingBoxIjk box6( cvf::Vec3st( 5, 5, 20 ), cvf::Vec3st( 15, 15, 30 ) );
    EXPECT_FALSE( box1.overlaps( box6 ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigBoundingBoxIjk, Intersection )
{
    RigBoundingBoxIjk box1( cvf::Vec3st( 0, 0, 0 ), cvf::Vec3st( 10, 10, 10 ) );

    // No overlap returns nullopt
    RigBoundingBoxIjk box2( cvf::Vec3st( 20, 20, 20 ), cvf::Vec3st( 30, 30, 30 ) );
    auto              result1 = box1.intersection( box2 );
    EXPECT_FALSE( result1.has_value() );

    // Partial overlap
    RigBoundingBoxIjk box3( cvf::Vec3st( 5, 5, 5 ), cvf::Vec3st( 15, 15, 15 ) );
    auto              result2 = box1.intersection( box3 );
    EXPECT_TRUE( result2.has_value() );
    EXPECT_EQ( cvf::Vec3st( 5, 5, 5 ), result2->min() );
    EXPECT_EQ( cvf::Vec3st( 10, 10, 10 ), result2->max() );

    // Complete containment (box4 inside box1)
    RigBoundingBoxIjk box4( cvf::Vec3st( 2, 2, 2 ), cvf::Vec3st( 8, 8, 8 ) );
    auto              result3 = box1.intersection( box4 );
    EXPECT_TRUE( result3.has_value() );
    EXPECT_EQ( cvf::Vec3st( 2, 2, 2 ), result3->min() );
    EXPECT_EQ( cvf::Vec3st( 8, 8, 8 ), result3->max() );

    // Same box
    auto result4 = box1.intersection( box1 );
    EXPECT_TRUE( result4.has_value() );
    EXPECT_EQ( cvf::Vec3st( 0, 0, 0 ), result4->min() );
    EXPECT_EQ( cvf::Vec3st( 10, 10, 10 ), result4->max() );

    // Single cell intersection
    RigBoundingBoxIjk box5( cvf::Vec3st( 10, 10, 10 ), cvf::Vec3st( 20, 20, 20 ) );
    auto              result5 = box1.intersection( box5 );
    EXPECT_TRUE( result5.has_value() );
    EXPECT_EQ( cvf::Vec3st( 10, 10, 10 ), result5->min() );
    EXPECT_EQ( cvf::Vec3st( 10, 10, 10 ), result5->max() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigBoundingBoxIjk, Clamp )
{
    RigBoundingBoxIjk bounds( cvf::Vec3st( 0, 0, 0 ), cvf::Vec3st( 10, 10, 10 ) );

    // Box completely inside bounds
    RigBoundingBoxIjk box1( cvf::Vec3st( 2, 2, 2 ), cvf::Vec3st( 8, 8, 8 ) );
    auto              result1 = box1.clamp( bounds );
    EXPECT_TRUE( result1.has_value() );
    EXPECT_EQ( cvf::Vec3st( 2, 2, 2 ), result1->min() );
    EXPECT_EQ( cvf::Vec3st( 8, 8, 8 ), result1->max() );

    // Box partially outside bounds - should be clamped
    RigBoundingBoxIjk box2( cvf::Vec3st( 5, 5, 5 ), cvf::Vec3st( 15, 15, 15 ) );
    auto              result2 = box2.clamp( bounds );
    EXPECT_TRUE( result2.has_value() );
    EXPECT_EQ( cvf::Vec3st( 5, 5, 5 ), result2->min() );
    EXPECT_EQ( cvf::Vec3st( 10, 10, 10 ), result2->max() );

    // Box completely outside bounds
    RigBoundingBoxIjk box3( cvf::Vec3st( 20, 20, 20 ), cvf::Vec3st( 30, 30, 30 ) );
    auto              result3 = box3.clamp( bounds );
    EXPECT_FALSE( result3.has_value() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigBoundingBoxIjk, ClampBehavesLikeIntersection )
{
    // Verify that clamp() and intersection() behave identically
    RigBoundingBoxIjk bounds( cvf::Vec3st( 0, 0, 0 ), cvf::Vec3st( 10, 10, 10 ) );
    RigBoundingBoxIjk box( cvf::Vec3st( 5, 5, 5 ), cvf::Vec3st( 15, 15, 15 ) );

    auto clampResult        = box.clamp( bounds );
    auto intersectionResult = box.intersection( bounds );

    EXPECT_EQ( clampResult.has_value(), intersectionResult.has_value() );
    if ( clampResult.has_value() && intersectionResult.has_value() )
    {
        EXPECT_EQ( clampResult->min(), intersectionResult->min() );
        EXPECT_EQ( clampResult->max(), intersectionResult->max() );
    }
}
