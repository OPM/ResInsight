/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RigHexGradientTools.h"

#include "../cafHexInterpolator/cafHexInterpolator.h"

#include "cvfMatrix4.h"

//--------------------------------------------------------------------------------------------------
/// GTEST_FILTER="Test_RigHexGradientTools*"
//--------------------------------------------------------------------------------------------------
TEST( RigHexGradientTools, GradientsForIdentityElement )
{
    // Identity element
    std::array<cvf::Vec3d, 8> hexCorners = { cvf::Vec3d( -1.0, -1.0, -1.0 ),
                                             cvf::Vec3d( 1.0, -1.0, -1.0 ),
                                             cvf::Vec3d( 1.0, 1.0, -1.0 ),
                                             cvf::Vec3d( -1.0, 1.0, -1.0 ),
                                             cvf::Vec3d( -1.0, -1.0, 1.0 ),
                                             cvf::Vec3d( 1.0, -1.0, 1.0 ),
                                             cvf::Vec3d( 1.0, 1.0, 1.0 ),
                                             cvf::Vec3d( -1.0, 1.0, 1.0 ) };

    std::array<double, 8> cornerValues = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };

    std::array<cvf::Vec3d, 8> gradients = RigHexGradientTools::gradients( hexCorners, cornerValues );

    for ( int i = 0; i < 8; i++ )
    {
        EXPECT_DOUBLE_EQ( 0.0, gradients[i].x() );
        EXPECT_DOUBLE_EQ( 0.0, gradients[i].y() );
        EXPECT_DOUBLE_EQ( 0.0, gradients[i].z() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigHexGradientTools, GradientsForScaledIdentityElement )
{
    // Identity element
    std::array<cvf::Vec3d, 8> hexCorners = { cvf::Vec3d( -1.0, -1.0, -1.0 ),
                                             cvf::Vec3d( 1.0, -1.0, -1.0 ),
                                             cvf::Vec3d( 1.0, 1.0, -1.0 ),
                                             cvf::Vec3d( -1.0, 1.0, -1.0 ),
                                             cvf::Vec3d( -1.0, -1.0, 1.0 ),
                                             cvf::Vec3d( 1.0, -1.0, 1.0 ),
                                             cvf::Vec3d( 1.0, 1.0, 1.0 ),
                                             cvf::Vec3d( -1.0, 1.0, 1.0 ) };

    for ( auto& v : hexCorners )
        v *= 2.5;

    std::array<double, 8> cornerValues = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };

    std::array<cvf::Vec3d, 8> gradients = RigHexGradientTools::gradients( hexCorners, cornerValues );

    for ( int i = 0; i < 8; i++ )
    {
        EXPECT_DOUBLE_EQ( 0.0, gradients[i].x() );
        EXPECT_DOUBLE_EQ( 0.0, gradients[i].y() );
        EXPECT_DOUBLE_EQ( 0.0, gradients[i].z() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigHexGradientTools, GradientsForTranslatedIdentityElement )
{
    // Identity element
    std::array<cvf::Vec3d, 8> hexCorners = { cvf::Vec3d( -1.0, -1.0, -1.0 ),
                                             cvf::Vec3d( 1.0, -1.0, -1.0 ),
                                             cvf::Vec3d( 1.0, 1.0, -1.0 ),
                                             cvf::Vec3d( -1.0, 1.0, -1.0 ),
                                             cvf::Vec3d( -1.0, -1.0, 1.0 ),
                                             cvf::Vec3d( 1.0, -1.0, 1.0 ),
                                             cvf::Vec3d( 1.0, 1.0, 1.0 ),
                                             cvf::Vec3d( -1.0, 1.0, 1.0 ) };

    for ( auto& v : hexCorners )
        v += { 3.2, 9.5, -20.3 };

    std::array<double, 8> cornerValues = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };

    std::array<cvf::Vec3d, 8> gradients = RigHexGradientTools::gradients( hexCorners, cornerValues );

    for ( int i = 0; i < 8; i++ )
    {
        EXPECT_DOUBLE_EQ( 0.0, gradients[i].x() );
        EXPECT_DOUBLE_EQ( 0.0, gradients[i].y() );
        EXPECT_DOUBLE_EQ( 0.0, gradients[i].z() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigHexGradientTools, GradientsForRotatedIdentityElement )
{
    // Identity element
    std::array<cvf::Vec3d, 8> hexCorners = { cvf::Vec3d( -1.0, -1.0, -1.0 ),
                                             cvf::Vec3d( 1.0, -1.0, -1.0 ),
                                             cvf::Vec3d( 1.0, 1.0, -1.0 ),
                                             cvf::Vec3d( -1.0, 1.0, -1.0 ),
                                             cvf::Vec3d( -1.0, -1.0, 1.0 ),
                                             cvf::Vec3d( 1.0, -1.0, 1.0 ),
                                             cvf::Vec3d( 1.0, 1.0, 1.0 ),
                                             cvf::Vec3d( -1.0, 1.0, 1.0 ) };

    cvf::Mat4d rot = cvf::Mat4d::fromRotation( { 1, 1, 0 }, 3.0 );
    for ( auto& v : hexCorners )
        v.transformPoint( rot );

    std::array<double, 8> cornerValues = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };

    std::array<cvf::Vec3d, 8> gradients = RigHexGradientTools::gradients( hexCorners, cornerValues );

    for ( int i = 0; i < 8; i++ )
    {
        EXPECT_DOUBLE_EQ( 0.0, gradients[i].x() );
        EXPECT_DOUBLE_EQ( 0.0, gradients[i].y() );
        EXPECT_DOUBLE_EQ( 0.0, gradients[i].z() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigHexGradientTools, GradientsForElement )
{
    // Identity element
    std::array<cvf::Vec3d, 8> hexCorners = { cvf::Vec3d( -1.0, -1.0, -1.0 ),
                                             cvf::Vec3d( 1.0, -1.0, -1.0 ),
                                             cvf::Vec3d( 1.0, 1.0, -1.0 ),
                                             cvf::Vec3d( -1.0, 1.0, -1.0 ),
                                             cvf::Vec3d( -1.0, -1.0, 1.0 ),
                                             cvf::Vec3d( 1.0, -1.0, 1.0 ),
                                             cvf::Vec3d( 1.0, 1.0, 1.0 ),
                                             cvf::Vec3d( -1.0, 1.0, 1.0 ) };

    // Set a higher value in the first corner
    std::array<double, 8> cornerValues = { 2.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };

    std::array<cvf::Vec3d, 8> gradients = RigHexGradientTools::gradients( hexCorners, cornerValues );

    // The adjacent corners should a non-zero gradient in the direction
    // towards the first corner
    EXPECT_DOUBLE_EQ( -0.5, gradients[0].x() );
    EXPECT_DOUBLE_EQ( -0.5, gradients[0].y() );
    EXPECT_DOUBLE_EQ( -0.5, gradients[0].z() );

    EXPECT_DOUBLE_EQ( -0.5, gradients[1].x() );
    EXPECT_DOUBLE_EQ( 0.0, gradients[1].y() );
    EXPECT_DOUBLE_EQ( 0.0, gradients[1].z() );

    EXPECT_DOUBLE_EQ( 0.0, gradients[3].x() );
    EXPECT_DOUBLE_EQ( -0.5, gradients[3].y() );
    EXPECT_DOUBLE_EQ( 0.0, gradients[3].z() );

    EXPECT_DOUBLE_EQ( 0.0, gradients[4].x() );
    EXPECT_DOUBLE_EQ( 0.0, gradients[4].y() );
    EXPECT_DOUBLE_EQ( -0.5, gradients[4].z() );

    // Non-adjacent should be unaffected
    std::array<int, 4> independentCorners = { 2, 5, 7, 6 };
    for ( auto c : independentCorners )
    {
        EXPECT_DOUBLE_EQ( 0.0, gradients[c].x() );
        EXPECT_DOUBLE_EQ( 0.0, gradients[c].y() );
        EXPECT_DOUBLE_EQ( 0.0, gradients[c].z() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigHexGradientTools, GradientsForLongElement )
{
    // Identity element
    std::array<cvf::Vec3d, 8> hexCorners = { cvf::Vec3d( -1.0, -1.0, -1.0 ),
                                             cvf::Vec3d( 2.0, -1.0, -1.0 ),
                                             cvf::Vec3d( 2.0, 1.0, -1.0 ),
                                             cvf::Vec3d( -1.0, 1.0, -1.0 ),
                                             cvf::Vec3d( -1.0, -1.0, 1.0 ),
                                             cvf::Vec3d( 2.0, -1.0, 1.0 ),
                                             cvf::Vec3d( 2.0, 1.0, 1.0 ),
                                             cvf::Vec3d( -1.0, 1.0, 1.0 ) };

    // Set a higher value in the first corner
    std::array<double, 8> cornerValues = { 2.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };

    std::array<cvf::Vec3d, 8> gradients = RigHexGradientTools::gradients( hexCorners, cornerValues );

    // The adjacent corners should a non-zero gradient in the direction
    // towards the first corner
    EXPECT_DOUBLE_EQ( -1.0 / 3.0, gradients[0].x() );
    EXPECT_DOUBLE_EQ( -0.5, gradients[0].y() );
    EXPECT_DOUBLE_EQ( -0.5, gradients[0].z() );

    EXPECT_DOUBLE_EQ( -1.0 / 3.0, gradients[1].x() );
    EXPECT_DOUBLE_EQ( 0.0, gradients[1].y() );
    EXPECT_DOUBLE_EQ( 0.0, gradients[1].z() );

    EXPECT_DOUBLE_EQ( 0.0, gradients[3].x() );
    EXPECT_DOUBLE_EQ( -0.5, gradients[3].y() );
    EXPECT_DOUBLE_EQ( 0.0, gradients[3].z() );

    EXPECT_DOUBLE_EQ( 0.0, gradients[4].x() );
    EXPECT_DOUBLE_EQ( 0.0, gradients[4].y() );
    EXPECT_DOUBLE_EQ( -0.5, gradients[4].z() );

    // Non-adjacent should be unaffected
    std::array<int, 4> independentCorners = { 2, 5, 7, 6 };
    for ( auto c : independentCorners )
    {
        EXPECT_DOUBLE_EQ( 0.0, gradients[c].x() );
        EXPECT_DOUBLE_EQ( 0.0, gradients[c].y() );
        EXPECT_DOUBLE_EQ( 0.0, gradients[c].z() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigHexGradientTools, GenerateJacobianForIdentity )
{
    // Identity element
    std::array<cvf::Vec3d, 8> hexCorners = { cvf::Vec3d( -1.0, -1.0, -1.0 ),
                                             cvf::Vec3d( 1.0, -1.0, -1.0 ),
                                             cvf::Vec3d( 1.0, 1.0, -1.0 ),
                                             cvf::Vec3d( -1.0, 1.0, -1.0 ),
                                             cvf::Vec3d( -1.0, -1.0, 1.0 ),
                                             cvf::Vec3d( 1.0, -1.0, 1.0 ),
                                             cvf::Vec3d( 1.0, 1.0, 1.0 ),
                                             cvf::Vec3d( -1.0, 1.0, 1.0 ) };

    for ( int i = 0; i < 8; i++ )
    {
        cvf::Mat3d actualJacobian = caf::HexInterpolator::jacobi( hexCorners, hexCorners[i] );

        // Calculate expected jacobian
        double dx = 2.0;
        double dy = 2.0;
        double dz = 2.0;

        double di = 2.0;
        double dj = 2.0;
        double dk = 2.0;

        cvf::Mat3d expectedJacobian( dx / di, 0.0, 0.0, 0.0, dy / dj, 0.0, 0.0, 0.0, dz / dk );

        cvf::Mat3d identityMat = cvf::Mat3d::IDENTITY;

        for ( int col = 0; col < 3; col++ )
        {
            for ( int row = 0; row < 3; row++ )
            {
                EXPECT_DOUBLE_EQ( expectedJacobian.rowCol( row, col ), actualJacobian.rowCol( row, col ) );
                EXPECT_DOUBLE_EQ( identityMat.rowCol( row, col ), actualJacobian.rowCol( row, col ) );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigHexGradientTools, GenerateJacobianForIdentityScaled )
{
    // Identity element
    std::array<cvf::Vec3d, 8> normalizedCorners = { cvf::Vec3d( -1.0, -1.0, -1.0 ),
                                                    cvf::Vec3d( 1.0, -1.0, -1.0 ),
                                                    cvf::Vec3d( 1.0, 1.0, -1.0 ),
                                                    cvf::Vec3d( -1.0, 1.0, -1.0 ),
                                                    cvf::Vec3d( -1.0, -1.0, 1.0 ),
                                                    cvf::Vec3d( 1.0, -1.0, 1.0 ),
                                                    cvf::Vec3d( 1.0, 1.0, 1.0 ),
                                                    cvf::Vec3d( -1.0, 1.0, 1.0 ) };

    double scale = 2.5;

    std::array<cvf::Vec3d, 8> hexCorners;
    for ( int i = 0; i < 8; i++ )
    {
        hexCorners[i] = normalizedCorners[i] * scale;
    }

    for ( int i = 0; i < 8; i++ )
    {
        cvf::Mat3d actualJacobian = caf::HexInterpolator::jacobi( hexCorners, normalizedCorners[i] );

        // Calculate expected jacobian
        double dx = 5.0;
        double dy = 5.0;
        double dz = 5.0;

        double di = 2.0;
        double dj = 2.0;
        double dk = 2.0;

        cvf::Mat3d expectedJacobian( dx / di, 0.0, 0.0, 0.0, dy / dj, 0.0, 0.0, 0.0, dz / dk );

        cvf::Mat3d identityMat = cvf::Mat3d::IDENTITY;

        for ( int col = 0; col < 3; col++ )
        {
            for ( int row = 0; row < 3; row++ )
            {
                EXPECT_DOUBLE_EQ( expectedJacobian.rowCol( row, col ), actualJacobian.rowCol( row, col ) );
                EXPECT_DOUBLE_EQ( identityMat.rowCol( row, col ) * scale, actualJacobian.rowCol( row, col ) );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigHexGradientTools, GenerateJacobianForLongElement )
{
    // Try a more complex element
    std::array<cvf::Vec3d, 8> hexCorners = { cvf::Vec3d( -1.0, -1.0, -1.0 ),
                                             cvf::Vec3d( 2.0, -1.0, -1.0 ),
                                             cvf::Vec3d( 2.0, 1.0, -1.0 ),
                                             cvf::Vec3d( -1.0, 1.0, -1.0 ),
                                             cvf::Vec3d( -1.0, -1.0, 1.0 ),
                                             cvf::Vec3d( 2.0, -1.0, 1.0 ),
                                             cvf::Vec3d( 2.0, 1.0, 1.0 ),
                                             cvf::Vec3d( -1.0, 1.0, 1.0 ) };

    cvf::Vec3d corner0( -1.0, -1.0, -1.0 );

    cvf::Mat3d actualJacobian = caf::HexInterpolator::jacobi( hexCorners, corner0 );

    // Calculate expected jacobian
    double dx = 3.0;
    double dy = 2.0;
    double dz = 2.0;

    double di = 2.0;
    double dj = 2.0;
    double dk = 2.0;

    cvf::Mat3d expectedJacobian( dx / di, 0.0, 0.0, 0.0, dy / dj, 0.0, 0.0, 0.0, dz / dk );

    for ( int col = 0; col < 3; col++ )
    {
        for ( int row = 0; row < 3; row++ )
        {
            EXPECT_DOUBLE_EQ( expectedJacobian.rowCol( row, col ), actualJacobian.rowCol( row, col ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigHexGradientTools, GradientsForRotatedElement )
{
    // Try a more complex element
    std::array<cvf::Vec3d, 8> hexCorners = { cvf::Vec3d( -1.0, -1.0, -1.0 ),
                                             cvf::Vec3d( 1.0, -1.0, -1.0 ),
                                             cvf::Vec3d( 1.0, 1.0, -1.0 ),
                                             cvf::Vec3d( -1.0, 1.0, -1.0 ),
                                             cvf::Vec3d( -1.0, -1.0, 1.0 ),
                                             cvf::Vec3d( 1.0, -1.0, 1.0 ),
                                             cvf::Vec3d( 1.0, 1.0, 1.0 ),
                                             cvf::Vec3d( -1.0, 1.0, 1.0 ) };

    cvf::Vec3d corner0( -1.0, -1.0, -1.0 );

    cvf::Mat4d rot = cvf::Mat4d::fromRotation( { 1, 0, 0 }, cvf::PI_D / 2.0 );
    for ( auto& v : hexCorners )
        v.transformPoint( rot );

    EXPECT_DOUBLE_EQ( 1.0, hexCorners[0].y() );
    EXPECT_DOUBLE_EQ( -1.0, hexCorners[4].z() );

    // Set a higher value in the first corner
    std::array<double, 8> cornerValues = { 2.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };

    std::array<cvf::Vec3d, 8> gradients = RigHexGradientTools::gradients( hexCorners, cornerValues );

    EXPECT_NEAR( -0.5, gradients[0].x(), 1.0e-10 );
    EXPECT_NEAR( 0.5, gradients[0].y(), 1.0e-10 );
    EXPECT_NEAR( -0.5, gradients[0].z(), 1.0e-10 );

    EXPECT_NEAR( -0.5, gradients[1].x(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[1].y(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[1].z(), 1.0e-10 );

    EXPECT_NEAR( 0.0, gradients[2].x(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[2].y(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[2].z(), 1.0e-10 );

    EXPECT_NEAR( 0.0, gradients[3].x(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[3].y(), 1.0e-10 );
    EXPECT_NEAR( -0.5, gradients[3].z(), 1.0e-10 );

    EXPECT_NEAR( 0.0, gradients[4].x(), 1.0e-10 );
    EXPECT_NEAR( 0.5, gradients[4].y(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[4].z(), 1.0e-10 );

    EXPECT_NEAR( 0.0, gradients[5].x(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[5].y(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[5].z(), 1.0e-10 );

    EXPECT_NEAR( 0.0, gradients[6].x(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[6].y(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[6].z(), 1.0e-10 );

    EXPECT_NEAR( 0.0, gradients[7].x(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[7].y(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[7].z(), 1.0e-10 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigHexGradientTools, GradientsForRotatedAndStretchedElement )
{
    // Try a more complex element
    std::array<cvf::Vec3d, 8> hexCorners = { cvf::Vec3d( -1.0, -1.0, -1.0 ),
                                             cvf::Vec3d( 1.0, -1.0, -1.0 ),
                                             cvf::Vec3d( 1.0, 1.0, -1.0 ),
                                             cvf::Vec3d( -1.0, 1.0, -1.0 ),
                                             cvf::Vec3d( -1.0, -1.0, 3.0 ),
                                             cvf::Vec3d( 1.0, -1.0, 3.0 ),
                                             cvf::Vec3d( 1.0, 1.0, 3.0 ),
                                             cvf::Vec3d( -1.0, 1.0, 3.0 ) };

    cvf::Vec3d corner0( -1.0, -1.0, -1.0 );

    cvf::Mat4d rot = cvf::Mat4d::fromRotation( { 1, 0, 0 }, cvf::PI_D / 2.0 );
    for ( auto& v : hexCorners )
        v.transformPoint( rot );

    EXPECT_DOUBLE_EQ( 1.0, hexCorners[0].y() );
    EXPECT_DOUBLE_EQ( -1.0, hexCorners[4].z() );

    // Set a higher value in the first corner
    std::array<double, 8> cornerValues = { 2.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };

    std::array<cvf::Vec3d, 8> gradients = RigHexGradientTools::gradients( hexCorners, cornerValues );

    EXPECT_NEAR( -0.5, gradients[0].x(), 1.0e-10 );
    EXPECT_NEAR( 0.25, gradients[0].y(), 1.0e-10 );
    EXPECT_NEAR( -0.5, gradients[0].z(), 1.0e-10 );

    EXPECT_NEAR( -0.5, gradients[1].x(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[1].y(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[1].z(), 1.0e-10 );

    EXPECT_NEAR( 0.0, gradients[2].x(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[2].y(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[2].z(), 1.0e-10 );

    EXPECT_NEAR( 0.0, gradients[3].x(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[3].y(), 1.0e-10 );
    EXPECT_NEAR( -0.5, gradients[3].z(), 1.0e-10 );

    EXPECT_NEAR( 0.0, gradients[4].x(), 1.0e-10 );
    EXPECT_NEAR( 0.25, gradients[4].y(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[4].z(), 1.0e-10 );

    EXPECT_NEAR( 0.0, gradients[5].x(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[5].y(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[5].z(), 1.0e-10 );

    EXPECT_NEAR( 0.0, gradients[6].x(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[6].y(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[6].z(), 1.0e-10 );

    EXPECT_NEAR( 0.0, gradients[7].x(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[7].y(), 1.0e-10 );
    EXPECT_NEAR( 0.0, gradients[7].z(), 1.0e-10 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigHexGradientTools, GenerateJacobianForRotatedElement )
{
    // Try a more complex element
    std::array<cvf::Vec3d, 8> hexCorners = { cvf::Vec3d( -1.0, -1.0, -1.0 ),
                                             cvf::Vec3d( 2.0, -1.0, -1.0 ),
                                             cvf::Vec3d( 2.0, 3.0, -1.0 ),
                                             cvf::Vec3d( -1.0, 3.0, -1.0 ),
                                             cvf::Vec3d( -1.0, -1.0, 4.0 ),
                                             cvf::Vec3d( 2.0, -1.0, 4.0 ),
                                             cvf::Vec3d( 2.0, 3.0, 4.0 ),
                                             cvf::Vec3d( -1.0, 3.0, 4.0 ) };

    cvf::Vec3d corner0( -1.0, -1.0, -1.0 );

    cvf::Mat4d rot = cvf::Mat4d::fromRotation( { 1, 0, 0 }, cvf::PI_D / 2.0 );
    for ( auto& v : hexCorners )
        v.transformPoint( rot );

    cvf::Mat3d actualJacobian = caf::HexInterpolator::jacobi( hexCorners, corner0 );

    // Calculate expected jacobian
    cvf::Vec3d dx_rot = ( hexCorners[1] - hexCorners[0] );
    cvf::Vec3d dy_rot = ( hexCorners[3] - hexCorners[0] );
    cvf::Vec3d dz_rot = ( hexCorners[4] - hexCorners[0] );

    double di = 2.0;
    double dj = 2.0;
    double dk = 2.0;

    cvf::Mat3d expectedJacobian( dx_rot.x() / di,
                                 dx_rot.y() / di,
                                 dx_rot.z() / di,
                                 dy_rot.x() / dj,
                                 dy_rot.y() / dj,
                                 dy_rot.z() / dj,
                                 dz_rot.x() / dk,
                                 dz_rot.y() / dk,
                                 dz_rot.z() / dk );
    expectedJacobian.transpose();

    for ( int row = 0; row < 3; row++ )
    {
        for ( int col = 0; col < 3; col++ )
        {
            EXPECT_DOUBLE_EQ( expectedJacobian.rowCol( row, col ), actualJacobian.rowCol( row, col ) );
        }
    }
}
