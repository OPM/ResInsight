#include "gtest/gtest.h"

#include "RifEclipseSummaryAddress.h"

#include "RimCurveStyleRule.h"
#include "RimCurveStyleRules.h"
#include "RimCurveStyleSettings.h"
#include "RimPlotCurveAppearance.h"
#include "RimSavedCurveAppearance.h"
#include "RimSummaryCurve.h"

using D = RimCurveStyleSettings::Dimension;

//--------------------------------------------------------------------------------------------------
/// Rules for the selected dimensions are returned ordered by dimension, independent of creation order
//--------------------------------------------------------------------------------------------------
TEST( RimCurveStyleRulesTest, RulesForOrderedByDimension )
{
    RimCurveStyleRules rules;
    rules.settings()->setColorDimension( D::WELL );
    rules.settings()->setSymbolDimension( D::CASE );

    auto wellRule = rules.findOrCreateRule( D::WELL, "A-1" );
    wellRule->setCustom( true );
    rules.findOrCreateRule( D::WELL, "B-2" )->setCustom( true );
    rules.findOrCreateRule( D::VECTOR, "WOPR" )->setCustom( true );
    rules.findOrCreateRule( D::CASE, "auto case" );
    auto caseRule = rules.findOrCreateRule( D::CASE, "case" );
    caseRule->setCustom( true );

    EXPECT_EQ( wellRule, rules.findOrCreateRule( D::WELL, "A-1" ) );

    const auto matching = rules.rulesFor( RifEclipseSummaryAddress::wellAddress( "WOPR", "A-1" ), "case" );

    ASSERT_EQ( 2u, matching.size() );
    EXPECT_EQ( caseRule, matching[0] );
    EXPECT_EQ( wellRule, matching[1] );

    EXPECT_TRUE( rules.rulesFor( RifEclipseSummaryAddress::wellAddress( "WOPR", "A-1" ), "auto case" ).size() == 1 );
}

//--------------------------------------------------------------------------------------------------
/// Registered names keep their index when new names are added
//--------------------------------------------------------------------------------------------------
TEST( RimCurveStyleRulesTest, ItemIndicesStableWhenNamesAdded )
{
    RimCurveStyleRules rules;

    rules.registerItemNames( D::CASE, { "B", "D" } );
    auto indices = rules.itemIndices( D::CASE );
    EXPECT_EQ( 0, indices["B"] );
    EXPECT_EQ( 1, indices["D"] );

    rules.registerItemNames( D::CASE, { "A", "B", "C", "D" } );
    indices = rules.itemIndices( D::CASE );
    ASSERT_EQ( 4u, indices.size() );
    EXPECT_EQ( 0, indices["B"] );
    EXPECT_EQ( 1, indices["D"] );
    EXPECT_EQ( 2, indices["A"] );
    EXPECT_EQ( 3, indices["C"] );

    EXPECT_TRUE( rules.itemIndices( D::WELL ).empty() );
    EXPECT_TRUE( rules.itemIndices( D::VECTOR ).empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimCurveStyleRulesTest, ControlsOnlyCurvesWithValueForDimension )
{
    RimCurveStyleRules rules;
    rules.settings()->setColorDimension( D::WELL );
    rules.settings()->setSymbolDimension( D::NONE );

    const auto wellAddress  = RifEclipseSummaryAddress::wellAddress( "WOPR", "A-1" );
    const auto fieldAddress = RifEclipseSummaryAddress::fieldAddress( "FOPT" );

    EXPECT_FALSE( rules.controlsColor( wellAddress, "case" ) );

    rules.setUseGlobalAppearance( true );

    EXPECT_TRUE( rules.controlsColor( wellAddress, "case" ) );
    EXPECT_FALSE( rules.controlsColor( fieldAddress, "case" ) );
    EXPECT_FALSE( rules.controlsSymbol( wellAddress, "case" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimCurveStyleRulesTest, CopyAppearance )
{
    RimPlotCurveAppearance source;
    source.setColor( cvf::Color3f::RED );
    source.setSymbol( RiuPlotCurveSymbol::SYMBOL_DIAMOND );
    source.setLineThickness( 4 );
    source.setSymbolLabel( "label" );

    RimPlotCurveAppearance target;
    target.copyFrom( source );

    EXPECT_TRUE( target.color() == cvf::Color3f::RED );
    EXPECT_EQ( RiuPlotCurveSymbol::SYMBOL_DIAMOND, target.symbol() );
    EXPECT_EQ( 4, target.lineThickness() );
    EXPECT_EQ( "label", target.symbolLabel() );
}

//--------------------------------------------------------------------------------------------------
/// The saved appearance is restored, and not applied after the curve is deleted
//--------------------------------------------------------------------------------------------------
TEST( RimCurveStyleRulesTest, SavedAppearanceRestore )
{
    auto curve = new RimSummaryCurve;
    curve->setColor( cvf::Color3f::RED );

    RimSavedCurveAppearance saved;
    saved.setCurve( curve );

    curve->setColor( cvf::Color3f::BLUE );
    EXPECT_EQ( curve, saved.restore() );
    EXPECT_TRUE( curve->color() == cvf::Color3f::RED );

    delete curve;
    EXPECT_EQ( nullptr, saved.restore() );
}
