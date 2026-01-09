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
    RigBoundingBoxIjk<cvf::Vec3st> defaultBox;
    EXPECT_TRUE( defaultBox.isValid() ); // Min and max are both (0,0,0)
    EXPECT_EQ( cvf::Vec3st::ZERO, defaultBox.min() );
    EXPECT_EQ( cvf::Vec3st::ZERO, defaultBox.max() );

    // Constructor with min and max
    RigBoundingBoxIjk<cvf::Vec3st> box( cvf::Vec3st( 1, 2, 3 ), cvf::Vec3st( 5, 6, 7 ) );
    EXPECT_TRUE( box.isValid() );
    EXPECT_EQ( cvf::Vec3st( 1, 2, 3 ), box.min() );
    EXPECT_EQ( cvf::Vec3st( 5, 6, 7 ), box.max() );

    // Invalid box (min > max)
    RigBoundingBoxIjk<cvf::Vec3st> invalidBox( cvf::Vec3st( 10, 10, 10 ), cvf::Vec3st( 5, 5, 5 ) );
    EXPECT_FALSE( invalidBox.isValid() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigBoundingBoxIjk, IsValid )
{
    // Valid boxes
    EXPECT_TRUE( RigBoundingBoxIjk<cvf::Vec3st>( cvf::Vec3st( 0, 0, 0 ), cvf::Vec3st( 10, 10, 10 ) ).isValid() );
    EXPECT_TRUE( RigBoundingBoxIjk<cvf::Vec3st>( cvf::Vec3st( 5, 5, 5 ), cvf::Vec3st( 5, 5, 5 ) ).isValid() ); // Single point

    // Invalid boxes (min > max in at least one dimension)
    EXPECT_FALSE( RigBoundingBoxIjk<cvf::Vec3st>( cvf::Vec3st( 10, 0, 0 ), cvf::Vec3st( 5, 10, 10 ) ).isValid() ); // X invalid
    EXPECT_FALSE( RigBoundingBoxIjk<cvf::Vec3st>( cvf::Vec3st( 0, 10, 0 ), cvf::Vec3st( 10, 5, 10 ) ).isValid() ); // Y invalid
    EXPECT_FALSE( RigBoundingBoxIjk<cvf::Vec3st>( cvf::Vec3st( 0, 0, 10 ), cvf::Vec3st( 10, 10, 5 ) ).isValid() ); // Z invalid
    EXPECT_FALSE( RigBoundingBoxIjk<cvf::Vec3st>( cvf::Vec3st( 10, 10, 10 ), cvf::Vec3st( 5, 5, 5 ) ).isValid() ); // All invalid
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigBoundingBoxIjk, OverlapDetection )
{
    RigBoundingBoxIjk<cvf::Vec3st> box1( cvf::Vec3st( 0, 0, 0 ), cvf::Vec3st( 10, 10, 10 ) );

    // Same box overlaps with itself
    EXPECT_TRUE( box1.overlaps( box1 ) );

    // Completely inside
    RigBoundingBoxIjk<cvf::Vec3st> box2( cvf::Vec3st( 2, 2, 2 ), cvf::Vec3st( 8, 8, 8 ) );
    EXPECT_TRUE( box1.overlaps( box2 ) );
    EXPECT_TRUE( box2.overlaps( box1 ) ); // Symmetric

    // Partially overlapping
    RigBoundingBoxIjk<cvf::Vec3st> box3( cvf::Vec3st( 5, 5, 5 ), cvf::Vec3st( 15, 15, 15 ) );
    EXPECT_TRUE( box1.overlaps( box3 ) );
    EXPECT_TRUE( box3.overlaps( box1 ) );

    // Touching at edge (inclusive bounds - should overlap)
    RigBoundingBoxIjk<cvf::Vec3st> box4( cvf::Vec3st( 10, 10, 10 ), cvf::Vec3st( 20, 20, 20 ) );
    EXPECT_TRUE( box1.overlaps( box4 ) );
    EXPECT_TRUE( box4.overlaps( box1 ) );

    // Completely separate (no overlap)
    RigBoundingBoxIjk<cvf::Vec3st> box5( cvf::Vec3st( 20, 20, 20 ), cvf::Vec3st( 30, 30, 30 ) );
    EXPECT_FALSE( box1.overlaps( box5 ) );
    EXPECT_FALSE( box5.overlaps( box1 ) );

    // Overlap in 2D but not 3D (separated in Z)
    RigBoundingBoxIjk<cvf::Vec3st> box6( cvf::Vec3st( 5, 5, 20 ), cvf::Vec3st( 15, 15, 30 ) );
    EXPECT_FALSE( box1.overlaps( box6 ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigBoundingBoxIjk, Intersection )
{
    RigBoundingBoxIjk<cvf::Vec3st> box1( cvf::Vec3st( 0, 0, 0 ), cvf::Vec3st( 10, 10, 10 ) );

    // No overlap returns nullopt
    RigBoundingBoxIjk<cvf::Vec3st> box2( cvf::Vec3st( 20, 20, 20 ), cvf::Vec3st( 30, 30, 30 ) );
    auto                           result1 = box1.intersection( box2 );
    EXPECT_FALSE( result1.has_value() );

    // Partial overlap
    RigBoundingBoxIjk<cvf::Vec3st> box3( cvf::Vec3st( 5, 5, 5 ), cvf::Vec3st( 15, 15, 15 ) );
    auto                           result2 = box1.intersection( box3 );
    EXPECT_TRUE( result2.has_value() );
    EXPECT_EQ( cvf::Vec3st( 5, 5, 5 ), result2->min() );
    EXPECT_EQ( cvf::Vec3st( 10, 10, 10 ), result2->max() );

    // Complete containment (box4 inside box1)
    RigBoundingBoxIjk<cvf::Vec3st> box4( cvf::Vec3st( 2, 2, 2 ), cvf::Vec3st( 8, 8, 8 ) );
    auto                           result3 = box1.intersection( box4 );
    EXPECT_TRUE( result3.has_value() );
    EXPECT_EQ( cvf::Vec3st( 2, 2, 2 ), result3->min() );
    EXPECT_EQ( cvf::Vec3st( 8, 8, 8 ), result3->max() );

    // Same box
    auto result4 = box1.intersection( box1 );
    EXPECT_TRUE( result4.has_value() );
    EXPECT_EQ( cvf::Vec3st( 0, 0, 0 ), result4->min() );
    EXPECT_EQ( cvf::Vec3st( 10, 10, 10 ), result4->max() );

    // Single cell intersection
    RigBoundingBoxIjk<cvf::Vec3st> box5( cvf::Vec3st( 10, 10, 10 ), cvf::Vec3st( 20, 20, 20 ) );
    auto                           result5 = box1.intersection( box5 );
    EXPECT_TRUE( result5.has_value() );
    EXPECT_EQ( cvf::Vec3st( 10, 10, 10 ), result5->min() );
    EXPECT_EQ( cvf::Vec3st( 10, 10, 10 ), result5->max() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigBoundingBoxIjk, Clamp )
{
    RigBoundingBoxIjk<cvf::Vec3st> bounds( cvf::Vec3st( 0, 0, 0 ), cvf::Vec3st( 10, 10, 10 ) );

    // Box completely inside bounds
    RigBoundingBoxIjk<cvf::Vec3st> box1( cvf::Vec3st( 2, 2, 2 ), cvf::Vec3st( 8, 8, 8 ) );
    auto                           result1 = box1.clamp( bounds );
    EXPECT_TRUE( result1.has_value() );
    EXPECT_EQ( cvf::Vec3st( 2, 2, 2 ), result1->min() );
    EXPECT_EQ( cvf::Vec3st( 8, 8, 8 ), result1->max() );

    // Box partially outside bounds - should be clamped
    RigBoundingBoxIjk<cvf::Vec3st> box2( cvf::Vec3st( 5, 5, 5 ), cvf::Vec3st( 15, 15, 15 ) );
    auto                           result2 = box2.clamp( bounds );
    EXPECT_TRUE( result2.has_value() );
    EXPECT_EQ( cvf::Vec3st( 5, 5, 5 ), result2->min() );
    EXPECT_EQ( cvf::Vec3st( 10, 10, 10 ), result2->max() );

    // Box completely outside bounds
    RigBoundingBoxIjk<cvf::Vec3st> box3( cvf::Vec3st( 20, 20, 20 ), cvf::Vec3st( 30, 30, 30 ) );
    auto                           result3 = box3.clamp( bounds );
    EXPECT_FALSE( result3.has_value() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigBoundingBoxIjk, ClampBehavesLikeIntersection )
{
    // Verify that clamp() and intersection() behave identically
    RigBoundingBoxIjk<cvf::Vec3st> bounds( cvf::Vec3st( 0, 0, 0 ), cvf::Vec3st( 10, 10, 10 ) );
    RigBoundingBoxIjk<cvf::Vec3st> box( cvf::Vec3st( 5, 5, 5 ), cvf::Vec3st( 15, 15, 15 ) );

    auto clampResult        = box.clamp( bounds );
    auto intersectionResult = box.intersection( bounds );

    EXPECT_EQ( clampResult.has_value(), intersectionResult.has_value() );
    if ( clampResult.has_value() && intersectionResult.has_value() )
    {
        EXPECT_EQ( clampResult->min(), intersectionResult->min() );
        EXPECT_EQ( clampResult->max(), intersectionResult->max() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigBoundingBoxIjk, ContainsPoint )
{
    RigBoundingBoxIjk<cvf::Vec3st> box( cvf::Vec3st( 10, 20, 30 ), cvf::Vec3st( 50, 60, 70 ) );

    // Points inside the box
    EXPECT_TRUE( box.contains( cvf::Vec3st( 25, 40, 50 ) ) ); // Center
    EXPECT_TRUE( box.contains( cvf::Vec3st( 11, 21, 31 ) ) ); // Near min corner
    EXPECT_TRUE( box.contains( cvf::Vec3st( 49, 59, 69 ) ) ); // Near max corner

    // Points at min corner (inclusive)
    EXPECT_TRUE( box.contains( cvf::Vec3st( 10, 20, 30 ) ) ); // Exact min corner
    EXPECT_TRUE( box.contains( cvf::Vec3st( 10, 40, 50 ) ) ); // Min X
    EXPECT_TRUE( box.contains( cvf::Vec3st( 25, 20, 50 ) ) ); // Min Y
    EXPECT_TRUE( box.contains( cvf::Vec3st( 25, 40, 30 ) ) ); // Min Z

    // Points at max corner (inclusive)
    EXPECT_TRUE( box.contains( cvf::Vec3st( 50, 60, 70 ) ) ); // Exact max corner
    EXPECT_TRUE( box.contains( cvf::Vec3st( 50, 40, 50 ) ) ); // Max X
    EXPECT_TRUE( box.contains( cvf::Vec3st( 25, 60, 50 ) ) ); // Max Y
    EXPECT_TRUE( box.contains( cvf::Vec3st( 25, 40, 70 ) ) ); // Max Z

    // Points on edges
    EXPECT_TRUE( box.contains( cvf::Vec3st( 10, 20, 50 ) ) ); // Min X and Y edge
    EXPECT_TRUE( box.contains( cvf::Vec3st( 50, 60, 50 ) ) ); // Max X and Y edge
    EXPECT_TRUE( box.contains( cvf::Vec3st( 10, 60, 70 ) ) ); // Min X, max Y and Z edge

    // Points outside the box (below min)
    EXPECT_FALSE( box.contains( cvf::Vec3st( 9, 40, 50 ) ) ); // X too small
    EXPECT_FALSE( box.contains( cvf::Vec3st( 25, 19, 50 ) ) ); // Y too small
    EXPECT_FALSE( box.contains( cvf::Vec3st( 25, 40, 29 ) ) ); // Z too small
    EXPECT_FALSE( box.contains( cvf::Vec3st( 9, 19, 29 ) ) ); // All too small

    // Points outside the box (above max)
    EXPECT_FALSE( box.contains( cvf::Vec3st( 51, 40, 50 ) ) ); // X too large
    EXPECT_FALSE( box.contains( cvf::Vec3st( 25, 61, 50 ) ) ); // Y too large
    EXPECT_FALSE( box.contains( cvf::Vec3st( 25, 40, 71 ) ) ); // Z too large
    EXPECT_FALSE( box.contains( cvf::Vec3st( 51, 61, 71 ) ) ); // All too large

    // Points far outside
    EXPECT_FALSE( box.contains( cvf::Vec3st( 0, 0, 0 ) ) );
    EXPECT_FALSE( box.contains( cvf::Vec3st( 100, 100, 100 ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigBoundingBoxIjk, ContainsSinglePointBox )
{
    // Box that is a single point
    RigBoundingBoxIjk pointBox( cvf::Vec3st( 5, 5, 5 ), cvf::Vec3st( 5, 5, 5 ) );

    // Should contain exactly the point
    EXPECT_TRUE( pointBox.contains( cvf::Vec3st( 5, 5, 5 ) ) );

    // Should not contain anything else
    EXPECT_FALSE( pointBox.contains( cvf::Vec3st( 4, 5, 5 ) ) );
    EXPECT_FALSE( pointBox.contains( cvf::Vec3st( 5, 4, 5 ) ) );
    EXPECT_FALSE( pointBox.contains( cvf::Vec3st( 5, 5, 4 ) ) );
    EXPECT_FALSE( pointBox.contains( cvf::Vec3st( 6, 5, 5 ) ) );
    EXPECT_FALSE( pointBox.contains( cvf::Vec3st( 5, 6, 5 ) ) );
    EXPECT_FALSE( pointBox.contains( cvf::Vec3st( 5, 5, 6 ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigBoundingBoxIjk, ContainsZeroBasedBox )
{
    // Box starting at origin (common case in grid applications)
    RigBoundingBoxIjk<cvf::Vec3st> box( cvf::Vec3st( 0, 0, 0 ), cvf::Vec3st( 10, 10, 10 ) );

    // Origin should be inside
    EXPECT_TRUE( box.contains( cvf::Vec3st( 0, 0, 0 ) ) );

    // Points inside
    EXPECT_TRUE( box.contains( cvf::Vec3st( 5, 5, 5 ) ) );
    EXPECT_TRUE( box.contains( cvf::Vec3st( 10, 10, 10 ) ) );

    // Points on faces
    EXPECT_TRUE( box.contains( cvf::Vec3st( 0, 5, 5 ) ) );
    EXPECT_TRUE( box.contains( cvf::Vec3st( 10, 5, 5 ) ) );

    // Points outside
    EXPECT_FALSE( box.contains( cvf::Vec3st( 11, 5, 5 ) ) );
    EXPECT_FALSE( box.contains( cvf::Vec3st( 5, 11, 5 ) ) );
    EXPECT_FALSE( box.contains( cvf::Vec3st( 5, 5, 11 ) ) );
}
