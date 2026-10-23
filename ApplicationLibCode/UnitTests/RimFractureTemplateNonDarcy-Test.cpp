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

#include "RiaDefines.h"
#include "RiaFractureDefines.h"

#include "RimEllipseFractureTemplate.h"

#include <cmath>
#include <limits>

//--------------------------------------------------------------------------------------------------
/// Exposes protected settings of the template, so that the values reported by
/// wellFractureIntersectionData() can be verified through the public compute functions
//--------------------------------------------------------------------------------------------------
class TestEllipseFractureTemplate : public RimEllipseFractureTemplate
{
public:
    explicit TestEllipseFractureTemplate( RiaDefines::EclipseUnitSystem unit )
    {
        setUnitSystem( unit );
        setDefaultValuesFromUnit();
    }

    void useFractureData()
    {
        m_fractureWidthType    = WIDTH_FROM_FRACTURE;
        m_permeabilityType     = CONDUCTIVITY_FROM_FRACTURE;
        m_betaFactorType       = USER_DEFINED_BETA_FACTOR;
        m_relativePermeability = 1.0;
    }

    void useUserDefinedWidth( double width )
    {
        m_fractureWidthType = USER_DEFINED_WIDTH;
        m_fractureWidth     = width;
    }

    void useComputedDFactor( double inertialCoefficient )
    {
        m_nonDarcyFlowType    = NON_DARCY_COMPUTED;
        m_inertialCoefficient = inertialCoefficient;
    }

    void useUserDefinedDFactor( double dFactor )
    {
        m_nonDarcyFlowType   = NON_DARCY_USER_DEFINED;
        m_userDefinedDFactor = dFactor;
    }

    void   setRelativePermeability( double value ) { m_relativePermeability = value; }
    void   setOrientation( FracOrientationEnum orientation ) { m_orientationType = orientation; }
    double wellDiameterValue() const { return m_wellDiameter; }
    double gasDensity() const { return m_relativeGasDensity; }
    double gasViscosityValue() const { return m_gasViscosity; }
};

//--------------------------------------------------------------------------------------------------
/// Ellipse templates report the user entered width and permeability as intersection data
//--------------------------------------------------------------------------------------------------
TEST( RimFractureTemplateNonDarcy, EllipseIntersectionDataMetric )
{
    TestEllipseFractureTemplate tmpl( RiaDefines::EclipseUnitSystem::UNITS_METRIC );
    tmpl.useFractureData();

    // Metric defaults: width 0.01 m, permeability 100000 mD
    EXPECT_DOUBLE_EQ( 0.01, tmpl.computeFractureWidth( nullptr ) );
    EXPECT_DOUBLE_EQ( 100000.0, tmpl.computeEffectivePermeability( nullptr ) );
    EXPECT_NEAR( 1000.0, tmpl.computeKh( nullptr ), 1.0e-9 );
}

TEST( RimFractureTemplateNonDarcy, EllipseIntersectionDataField )
{
    TestEllipseFractureTemplate tmpl( RiaDefines::EclipseUnitSystem::UNITS_FIELD );
    tmpl.useFractureData();

    // Field defaults: width 0.5 inches (reported in feet), permeability 80000 mD
    EXPECT_NEAR( 0.5 / 12.0, tmpl.computeFractureWidth( nullptr ), 1.0e-12 );
    EXPECT_DOUBLE_EQ( 80000.0, tmpl.computeEffectivePermeability( nullptr ) );
    EXPECT_NEAR( 80000.0 * 0.5 / 12.0, tmpl.computeKh( nullptr ), 1.0e-9 );
}

TEST( RimFractureTemplateNonDarcy, RelativePermeabilityScalesEffectivePermeability )
{
    TestEllipseFractureTemplate tmpl( RiaDefines::EclipseUnitSystem::UNITS_METRIC );
    tmpl.useFractureData();
    tmpl.setRelativePermeability( 0.25 );

    EXPECT_DOUBLE_EQ( 25000.0, tmpl.computeEffectivePermeability( nullptr ) );
    EXPECT_NEAR( 250.0, tmpl.computeKh( nullptr ), 1.0e-9 );
}

TEST( RimFractureTemplateNonDarcy, UserDefinedWidthOverridesIntersectionWidth )
{
    TestEllipseFractureTemplate tmpl( RiaDefines::EclipseUnitSystem::UNITS_METRIC );
    tmpl.useFractureData();
    tmpl.useUserDefinedWidth( 0.05 );

    EXPECT_DOUBLE_EQ( 0.05, tmpl.computeFractureWidth( nullptr ) );
    EXPECT_NEAR( 100000.0 * 0.05, tmpl.computeKh( nullptr ), 1.0e-9 );
}

//--------------------------------------------------------------------------------------------------
/// D = alpha * beta * Ke / h * 1 / rw * gamma / mu, using the intersection data of the template
//--------------------------------------------------------------------------------------------------
TEST( RimFractureTemplateNonDarcy, ComputedDFactorMatchesEquation )
{
    TestEllipseFractureTemplate tmpl( RiaDefines::EclipseUnitSystem::UNITS_METRIC );
    tmpl.useFractureData();
    tmpl.useComputedDFactor( 0.5 );

    const double alpha  = RiaDefines::nonDarcyFlowAlpha( RiaDefines::EclipseUnitSystem::UNITS_METRIC );
    const double radius = tmpl.wellDiameterValue() / 2.0;
    const double ke     = 100000.0;
    const double h      = 0.01;

    const double expected = alpha * 0.5 * ke * tmpl.gasDensity() / ( h * radius * tmpl.gasViscosityValue() );

    EXPECT_NEAR( expected, tmpl.computeDFactor( nullptr ), expected * 1.0e-12 );
    EXPECT_NEAR( radius, tmpl.computeWellRadiusForDFactorCalculation( nullptr ), 1.0e-12 );
}

TEST( RimFractureTemplateNonDarcy, ComputedDFactorUsesUnitDependentAlpha )
{
    TestEllipseFractureTemplate metric( RiaDefines::EclipseUnitSystem::UNITS_METRIC );
    metric.useFractureData();
    metric.useComputedDFactor( 1.0 );
    metric.useUserDefinedWidth( 0.01 );

    TestEllipseFractureTemplate field( RiaDefines::EclipseUnitSystem::UNITS_FIELD );
    field.useFractureData();
    field.useComputedDFactor( 1.0 );
    field.useUserDefinedWidth( 0.01 );

    // Make everything but alpha, permeability and well radius equal between the two templates
    const double ratio = ( field.computeDFactor( nullptr ) / metric.computeDFactor( nullptr ) );

    // The field well diameter is given in inches, but the radius used in the D-factor must be in feet
    EXPECT_NEAR( field.wellDiameterValue() / 2.0 / 12.0, field.computeWellRadiusForDFactorCalculation( nullptr ), 1.0e-12 );

    const double alphaRatio = RiaDefines::nonDarcyFlowAlpha( RiaDefines::EclipseUnitSystem::UNITS_FIELD ) /
                              RiaDefines::nonDarcyFlowAlpha( RiaDefines::EclipseUnitSystem::UNITS_METRIC );
    const double permRatio = field.computeEffectivePermeability( nullptr ) / metric.computeEffectivePermeability( nullptr );
    const double radiusRatio =
        metric.computeWellRadiusForDFactorCalculation( nullptr ) / field.computeWellRadiusForDFactorCalculation( nullptr );

    EXPECT_NEAR( alphaRatio * permRatio * radiusRatio, ratio, ratio * 1.0e-12 );
}

TEST( RimFractureTemplateNonDarcy, DFactorScaleFactorAndAlongWellPathCorrection )
{
    TestEllipseFractureTemplate tmpl( RiaDefines::EclipseUnitSystem::UNITS_METRIC );
    tmpl.useFractureData();
    tmpl.useComputedDFactor( 0.5 );

    const double base = tmpl.computeDFactor( nullptr );

    tmpl.setScaleFactors( 1.0, 1.0, 3.0, 1.0 );
    EXPECT_NEAR( 3.0 * base, tmpl.computeDFactor( nullptr ), base * 1.0e-12 );

    // Linear inflow correction factor of 1.2 for fractures along the well path
    tmpl.setScaleFactors( 1.0, 1.0, 1.0, 1.0 );
    tmpl.setOrientation( RimFractureTemplate::ALONG_WELL_PATH );
    EXPECT_NEAR( 1.2 * base, tmpl.computeDFactor( nullptr ), base * 1.0e-12 );
}

TEST( RimFractureTemplateNonDarcy, UserDefinedDFactorIsScaledButNotComputed )
{
    TestEllipseFractureTemplate tmpl( RiaDefines::EclipseUnitSystem::UNITS_METRIC );
    tmpl.useFractureData();
    tmpl.useUserDefinedDFactor( 0.125 );

    EXPECT_DOUBLE_EQ( 0.125, tmpl.computeDFactor( nullptr ) );

    tmpl.setScaleFactors( 1.0, 1.0, 2.0, 1.0 );
    EXPECT_DOUBLE_EQ( 0.25, tmpl.computeDFactor( nullptr ) );
}

TEST( RimFractureTemplateNonDarcy, ZeroWidthGivesInfiniteDFactor )
{
    TestEllipseFractureTemplate tmpl( RiaDefines::EclipseUnitSystem::UNITS_METRIC );
    tmpl.useFractureData();
    tmpl.useComputedDFactor( 0.5 );
    tmpl.useUserDefinedWidth( 0.0 );

    EXPECT_EQ( std::numeric_limits<double>::infinity(), tmpl.computeDFactor( nullptr ) );
}
