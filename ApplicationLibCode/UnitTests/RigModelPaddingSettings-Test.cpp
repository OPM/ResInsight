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

#include "RigModelPaddingSettings.h"

//--------------------------------------------------------------------------------------------------
/// Test default values
//--------------------------------------------------------------------------------------------------
TEST( RigModelPaddingSettings, DefaultValues )
{
    RigModelPaddingSettings settings;

    EXPECT_FALSE( settings.isEnabled() );
    EXPECT_EQ( 0, settings.nzUpper() );
    EXPECT_DOUBLE_EQ( 0.0, settings.topUpper() );
    EXPECT_DOUBLE_EQ( 0.0, settings.upperPorosity() );
    EXPECT_EQ( 1, settings.upperEquilnum() );
    EXPECT_EQ( 0, settings.nzLower() );
    EXPECT_DOUBLE_EQ( 0.0, settings.bottomLower() );
    EXPECT_DOUBLE_EQ( 0.1, settings.minLayerThickness() );
    EXPECT_FALSE( settings.fillGaps() );
    EXPECT_FALSE( settings.monotonicZcorn() );
    EXPECT_FALSE( settings.verticalPillars() );
}

//--------------------------------------------------------------------------------------------------
/// Test setters and getters
//--------------------------------------------------------------------------------------------------
TEST( RigModelPaddingSettings, SettersAndGetters )
{
    RigModelPaddingSettings settings;

    settings.setEnabled( true );
    settings.setNzUpper( 5 );
    settings.setTopUpper( 1000.0 );
    settings.setUpperPorosity( 0.25 );
    settings.setUpperEquilnum( 2 );
    settings.setNzLower( 3 );
    settings.setBottomLower( 3500.0 );
    settings.setMinLayerThickness( 0.5 );
    settings.setFillGaps( true );
    settings.setMonotonicZcorn( true );
    settings.setVerticalPillars( true );

    EXPECT_TRUE( settings.isEnabled() );
    EXPECT_EQ( 5, settings.nzUpper() );
    EXPECT_DOUBLE_EQ( 1000.0, settings.topUpper() );
    EXPECT_DOUBLE_EQ( 0.25, settings.upperPorosity() );
    EXPECT_EQ( 2, settings.upperEquilnum() );
    EXPECT_EQ( 3, settings.nzLower() );
    EXPECT_DOUBLE_EQ( 3500.0, settings.bottomLower() );
    EXPECT_DOUBLE_EQ( 0.5, settings.minLayerThickness() );
    EXPECT_TRUE( settings.fillGaps() );
    EXPECT_TRUE( settings.monotonicZcorn() );
    EXPECT_TRUE( settings.verticalPillars() );
}

//--------------------------------------------------------------------------------------------------
/// Test validation when disabled
//--------------------------------------------------------------------------------------------------
TEST( RigModelPaddingSettings, Validation_WhenDisabled )
{
    RigModelPaddingSettings settings;
    settings.setEnabled( false );

    QString errors = settings.validate();

    EXPECT_TRUE( errors.isEmpty() ) << "Validation should pass when padding is disabled";
}

//--------------------------------------------------------------------------------------------------
/// Test validation with valid upper padding settings
//--------------------------------------------------------------------------------------------------
TEST( RigModelPaddingSettings, Validation_ValidUpperPadding )
{
    RigModelPaddingSettings settings;
    settings.setEnabled( true );
    settings.setNzUpper( 5 );
    settings.setTopUpper( 1000.0 );
    settings.setUpperPorosity( 0.25 );
    settings.setUpperEquilnum( 1 );
    settings.setMinLayerThickness( 0.1 );

    QString errors = settings.validate();

    EXPECT_TRUE( errors.isEmpty() ) << "Validation should pass with valid upper padding: " << errors.toStdString();
}

//--------------------------------------------------------------------------------------------------
/// Test validation with valid lower padding settings
//--------------------------------------------------------------------------------------------------
TEST( RigModelPaddingSettings, Validation_ValidLowerPadding )
{
    RigModelPaddingSettings settings;
    settings.setEnabled( true );
    settings.setNzLower( 3 );
    settings.setBottomLower( 3500.0 );
    settings.setMinLayerThickness( 0.1 );

    QString errors = settings.validate();

    EXPECT_TRUE( errors.isEmpty() ) << "Validation should pass with valid lower padding: " << errors.toStdString();
}

//--------------------------------------------------------------------------------------------------
/// Test validation with valid both upper and lower padding
//--------------------------------------------------------------------------------------------------
TEST( RigModelPaddingSettings, Validation_ValidBothPadding )
{
    RigModelPaddingSettings settings;
    settings.setEnabled( true );
    settings.setNzUpper( 2 );
    settings.setTopUpper( 1000.0 );
    settings.setUpperPorosity( 0.3 );
    settings.setUpperEquilnum( 1 );
    settings.setNzLower( 3 );
    settings.setBottomLower( 3500.0 );
    settings.setMinLayerThickness( 0.5 );

    QString errors = settings.validate();

    EXPECT_TRUE( errors.isEmpty() ) << "Validation should pass with valid both padding: " << errors.toStdString();
}

//--------------------------------------------------------------------------------------------------
/// Test validation fails when no layers specified
//--------------------------------------------------------------------------------------------------
TEST( RigModelPaddingSettings, Validation_NoLayersSpecified )
{
    RigModelPaddingSettings settings;
    settings.setEnabled( true );
    settings.setNzUpper( 0 );
    settings.setNzLower( 0 );

    QString errors = settings.validate();

    EXPECT_FALSE( errors.isEmpty() );
    EXPECT_TRUE( errors.contains( "At least one padding direction" ) );
}

//--------------------------------------------------------------------------------------------------
/// Test validation fails with negative upper layers
//--------------------------------------------------------------------------------------------------
TEST( RigModelPaddingSettings, Validation_NegativeUpperLayers )
{
    RigModelPaddingSettings settings;
    settings.setEnabled( true );
    settings.setNzUpper( -1 );
    settings.setNzLower( 5 );

    QString errors = settings.validate();

    EXPECT_FALSE( errors.isEmpty() );
    EXPECT_TRUE( errors.contains( "cannot be negative" ) );
}

//--------------------------------------------------------------------------------------------------
/// Test validation fails with negative lower layers
//--------------------------------------------------------------------------------------------------
TEST( RigModelPaddingSettings, Validation_NegativeLowerLayers )
{
    RigModelPaddingSettings settings;
    settings.setEnabled( true );
    settings.setNzUpper( 5 );
    settings.setNzLower( -2 );

    QString errors = settings.validate();

    EXPECT_FALSE( errors.isEmpty() );
    EXPECT_TRUE( errors.contains( "cannot be negative" ) );
}

//--------------------------------------------------------------------------------------------------
/// Test validation fails with invalid porosity (< 0)
//--------------------------------------------------------------------------------------------------
TEST( RigModelPaddingSettings, Validation_InvalidPorosityNegative )
{
    RigModelPaddingSettings settings;
    settings.setEnabled( true );
    settings.setNzUpper( 5 );
    settings.setUpperPorosity( -0.1 );

    QString errors = settings.validate();

    EXPECT_FALSE( errors.isEmpty() );
    EXPECT_TRUE( errors.contains( "porosity" ) );
}

//--------------------------------------------------------------------------------------------------
/// Test validation fails with invalid porosity (> 1)
//--------------------------------------------------------------------------------------------------
TEST( RigModelPaddingSettings, Validation_InvalidPorosityTooHigh )
{
    RigModelPaddingSettings settings;
    settings.setEnabled( true );
    settings.setNzUpper( 5 );
    settings.setUpperPorosity( 1.5 );

    QString errors = settings.validate();

    EXPECT_FALSE( errors.isEmpty() );
    EXPECT_TRUE( errors.contains( "porosity" ) );
}

//--------------------------------------------------------------------------------------------------
/// Test validation fails with invalid EQUILNUM
//--------------------------------------------------------------------------------------------------
TEST( RigModelPaddingSettings, Validation_InvalidEquilnum )
{
    RigModelPaddingSettings settings;
    settings.setEnabled( true );
    settings.setNzUpper( 5 );
    settings.setUpperPorosity( 0.25 );
    settings.setUpperEquilnum( 0 );

    QString errors = settings.validate();

    EXPECT_FALSE( errors.isEmpty() );
    EXPECT_TRUE( errors.contains( "EQUILNUM" ) );
}

//--------------------------------------------------------------------------------------------------
/// Test validation fails with non-positive layer thickness
//--------------------------------------------------------------------------------------------------
TEST( RigModelPaddingSettings, Validation_NonPositiveLayerThickness )
{
    RigModelPaddingSettings settings;
    settings.setEnabled( true );
    settings.setNzUpper( 5 );
    settings.setUpperPorosity( 0.25 );
    settings.setMinLayerThickness( 0.0 );

    QString errors = settings.validate();

    EXPECT_FALSE( errors.isEmpty() );
    EXPECT_TRUE( errors.contains( "thickness" ) );
}

//--------------------------------------------------------------------------------------------------
/// Test validation with edge case porosity values (0 and 1)
//--------------------------------------------------------------------------------------------------
TEST( RigModelPaddingSettings, Validation_EdgeCasePorosity )
{
    RigModelPaddingSettings settings;
    settings.setEnabled( true );
    settings.setNzUpper( 5 );
    settings.setMinLayerThickness( 0.1 );

    // Test porosity = 0 (valid)
    settings.setUpperPorosity( 0.0 );
    QString errors = settings.validate();
    EXPECT_TRUE( errors.isEmpty() ) << "Porosity 0.0 should be valid: " << errors.toStdString();

    // Test porosity = 1 (valid)
    settings.setUpperPorosity( 1.0 );
    errors = settings.validate();
    EXPECT_TRUE( errors.isEmpty() ) << "Porosity 1.0 should be valid: " << errors.toStdString();
}
