/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RiaTestDataDirectory.h"

#include "Well/RigWellPath.h"

#include "RimStimPlanFractureTemplate.h"
#include "RimWellPath.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"

#include <cmath>

static const QString STIMPLAN_TEMPLATE_TEST_DATA_DIRECTORY   = QString( "%1/RimStimPlanFractureTemplate/" ).arg( TEST_DATA_DIR );
static const QString STIMPLAN_XML_READER_TEST_DATA_DIRECTORY = QString( "%1/RifStimPlanXmlReader/" ).arg( TEST_DATA_DIR );

//--------------------------------------------------------------------------------------------------
/// Gives access to the protected settings needed to load a template from file
//--------------------------------------------------------------------------------------------------
class TestStimPlanFractureTemplate : public RimStimPlanFractureTemplate
{
public:
    explicit TestStimPlanFractureTemplate( const QString& fileName )
    {
        setFileName( fileName );
        loadDataAndUpdate();
        m_fractureWidthType = WIDTH_FROM_FRACTURE;
        m_permeabilityType  = CONDUCTIVITY_FROM_FRACTURE;
    }

    void useBetaFromFile() { m_betaFactorType = BETA_FACTOR_FROM_FRACTURE; }
    void setPerforationLengthValue( double length ) { m_perforationLength = length; }
    void setOrientation( FracOrientationEnum orientation ) { m_orientationType = orientation; }
};

//--------------------------------------------------------------------------------------------------
/// Width, conductivity and beta at the well position are taken from the fracture grid, and the
/// beta factor is converted from file units to Forchheimer units
//--------------------------------------------------------------------------------------------------
TEST( RimStimPlanFractureTemplateTest, IntersectionDataAtWellCenterFromFile )
{
    TestStimPlanFractureTemplate tmpl( STIMPLAN_TEMPLATE_TEST_DATA_DIRECTORY + "uniform_width_beta.xml" );
    tmpl.useBetaFromFile();

    RimWellPathFracture fracture;
    fracture.setFractureTemplate( &tmpl );

    ASSERT_TRUE( fracture.fractureGrid() != nullptr );

    // Width 0.01 m and conductivity 300 md-m gives permeability 30000 mD
    EXPECT_NEAR( 0.01, tmpl.computeFractureWidth( &fracture ), 1.0e-12 );
    EXPECT_NEAR( 30000.0, tmpl.computeEffectivePermeability( &fracture ), 1.0e-6 );
    EXPECT_NEAR( 300.0, tmpl.computeKh( &fracture ), 1.0e-9 );

    // 2.0265e8 [/m] divided by 1.01325e8 gives 2.0 Forchheimer units
    EXPECT_NEAR( 2.0, tmpl.getOrComputeBetaFactor( &fracture ), 1.0e-9 );
}

TEST( RimStimPlanFractureTemplateTest, BetaFactorIsUserDefinedByDefault )
{
    TestStimPlanFractureTemplate tmpl( STIMPLAN_TEMPLATE_TEST_DATA_DIRECTORY + "uniform_width_beta.xml" );

    RimWellPathFracture fracture;
    fracture.setFractureTemplate( &tmpl );

    EXPECT_NEAR( 0.006083236, tmpl.getOrComputeBetaFactor( &fracture ), 1.0e-12 );
}

//--------------------------------------------------------------------------------------------------
/// Without a fracture instance there is no intersection data: width is zero and beta is undefined
//--------------------------------------------------------------------------------------------------
TEST( RimStimPlanFractureTemplateTest, NoFractureInstanceGivesNoIntersectionData )
{
    TestStimPlanFractureTemplate tmpl( STIMPLAN_TEMPLATE_TEST_DATA_DIRECTORY + "uniform_width_beta.xml" );
    tmpl.useBetaFromFile();

    EXPECT_DOUBLE_EQ( 0.0, tmpl.computeFractureWidth( nullptr ) );
    EXPECT_TRUE( std::isinf( tmpl.getOrComputeBetaFactor( nullptr ) ) );
}

//--------------------------------------------------------------------------------------------------
/// A file without width values has no width data, and the conductivity at the well is used directly
//--------------------------------------------------------------------------------------------------
TEST( RimStimPlanFractureTemplateTest, FileWithoutWidthUsesConductivityDirectly )
{
    TestStimPlanFractureTemplate tmpl( STIMPLAN_XML_READER_TEST_DATA_DIRECTORY + "small_fracture.xml" );

    RimWellPathFracture fracture;
    fracture.setFractureTemplate( &tmpl );

    ASSERT_TRUE( fracture.fractureGrid() != nullptr );

    // The conductivity at the well center is 301.7 md-m in the file
    EXPECT_NEAR( 301.7, tmpl.computeKh( &fracture ), 1.0e-9 );
}

//--------------------------------------------------------------------------------------------------
/// For fractures along the well path, the values are averaged along the part of the well path
/// that intersects the fracture
//--------------------------------------------------------------------------------------------------
TEST( RimStimPlanFractureTemplateTest, IntersectionDataAlongWellPath )
{
    // Horizontal well along the X axis at depth 2015 m, with measured depth equal to the X coordinate + 500
    std::vector<cvf::Vec3d> wellPathPoints;
    std::vector<double>     measuredDepths;
    for ( int i = 0; i <= 20; i++ )
    {
        const double x = -500.0 + i * 50.0;
        wellPathPoints.push_back( cvf::Vec3d( x, 0.0, -2015.0 ) );
        measuredDepths.push_back( x + 500.0 );
    }

    RimWellPath wellPath;
    wellPath.setWellPathGeometry( new RigWellPath( wellPathPoints, measuredDepths ) );

    TestStimPlanFractureTemplate tmpl( STIMPLAN_TEMPLATE_TEST_DATA_DIRECTORY + "uniform_width_beta.xml" );
    tmpl.useBetaFromFile();
    tmpl.setOrientation( RimFractureTemplate::ALONG_WELL_PATH );
    tmpl.setPerforationLengthValue( 80.0 );

    auto fracture = new RimWellPathFracture;
    wellPath.fractureCollection()->addFracture( fracture );
    fracture->setFractureUnit( RiaDefines::EclipseUnitSystem::UNITS_METRIC );
    fracture->setFractureTemplate( &tmpl );
    fracture->setMeasuredDepth( 500.0 );

    ASSERT_TRUE( fracture->fractureGrid() != nullptr );

    EXPECT_NEAR( 0.01, tmpl.computeFractureWidth( fracture ), 1.0e-9 );
    EXPECT_NEAR( 30000.0, tmpl.computeEffectivePermeability( fracture ), 1.0e-3 );
    EXPECT_NEAR( 300.0, tmpl.computeKh( fracture ), 1.0e-6 );
    EXPECT_NEAR( 2.0, tmpl.getOrComputeBetaFactor( fracture ), 1.0e-9 );
}
