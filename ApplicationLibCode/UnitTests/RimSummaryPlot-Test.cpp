/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "RifEclipseSummaryAddress.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimMockSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryPlot.h"

#include "RiuPlotAxis.h"

//--------------------------------------------------------------------------------------------------
/// Regression tests for #14728: a newly added curve or ensemble curve set with a value range/unit
/// differing from the curves already present in the plot should automatically be assigned to an
/// unused secondary Y-axis, instead of always landing on the same axis as the existing curves.
//--------------------------------------------------------------------------------------------------
namespace
{
RimSummaryCurve* createCurve( const std::string& unitName, const RifEclipseSummaryAddress& address )
{
    auto* summaryCase = static_cast<RimMockSummaryCase*>( createMockCase( 0 ) );
    summaryCase->addVector( address, { 0, 100 }, { 1.0, 2.0 }, unitName );

    auto* curve = new RimSummaryCurve();
    curve->setSummaryCaseY( summaryCase );
    curve->setSummaryAddressY( address );

    return curve;
}

// RimEnsembleCurveSet::unitNameY() looks up the unit by querying the first summary case in the
// ensemble that has a reader, so every mock case added to the ensemble must know about every
// address used by any curve set in the same ensemble (mirroring how real ensembles share the same
// set of summary vectors across all realizations).
RimEnsembleCurveSet* createEnsembleCurveSet( RimSummaryEnsemble* ensemble, const std::string& unitName, const RifEclipseSummaryAddress& address )
{
    for ( auto existingCase : ensemble->allSummaryCases() )
    {
        static_cast<RimMockSummaryCase*>( existingCase )->addVector( address, { 0, 100 }, { 1.0, 2.0 }, unitName );
    }

    auto* summaryCase = static_cast<RimMockSummaryCase*>( createMockCase( 0 ) );
    summaryCase->addVector( address, { 0, 100 }, { 1.0, 2.0 }, unitName );
    ensemble->addCase( summaryCase, false );

    auto* curveSet = new RimEnsembleCurveSet();
    curveSet->setSummaryEnsemble( ensemble );
    curveSet->setSummaryAddressY( address );

    return curveSet;
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// The very first curve added to a plot has nothing to match against, so it must land on the default
/// (left) axis.
//--------------------------------------------------------------------------------------------------
TEST( RimSummaryPlot, AssignPlotAxis_FirstCurveUsesDefaultLeftAxis )
{
    RimSummaryPlot plot;

    auto* curve = createCurve( "SM3", RifEclipseSummaryAddress::fieldAddress( "FOPT" ) );
    plot.addCurveAndUpdate( curve );

    EXPECT_EQ( RiuPlotAxis::defaultLeft(), curve->axisY() );
}

//--------------------------------------------------------------------------------------------------
/// Two curves sharing the same unit are assumed to have comparable value ranges, so the second curve
/// should reuse the first curve's axis instead of creating a new one.
//--------------------------------------------------------------------------------------------------
TEST( RimSummaryPlot, AssignPlotAxis_MatchingUnitCurveReusesSameAxis )
{
    RimSummaryPlot plot;

    auto* curveA = createCurve( "SM3", RifEclipseSummaryAddress::fieldAddress( "FOPT" ) );
    plot.addCurveAndUpdate( curveA );

    auto* curveB = createCurve( "SM3", RifEclipseSummaryAddress::wellAddress( "WOPT", "W-1" ) );
    plot.addCurveAndUpdate( curveB );

    EXPECT_EQ( curveA->axisY(), curveB->axisY() );
}

//--------------------------------------------------------------------------------------------------
/// A curve with a different unit than the curves already in the plot (e.g. a rate vector added
/// alongside a cumulative vector) must be automatically assigned to the unused secondary (right)
/// axis rather than being added to the same axis as the existing curve.
//--------------------------------------------------------------------------------------------------
TEST( RimSummaryPlot, AssignPlotAxis_DifferingUnitCurveUsesSecondaryAxis )
{
    RimSummaryPlot plot;

    auto* fopt = createCurve( "SM3", RifEclipseSummaryAddress::fieldAddress( "FOPT" ) );
    plot.addCurveAndUpdate( fopt );

    auto* fopr = createCurve( "SM3/DAY", RifEclipseSummaryAddress::fieldAddress( "FOPR" ) );
    plot.addCurveAndUpdate( fopr );

    EXPECT_EQ( RiuPlotAxis::defaultLeft(), fopt->axisY() );
    EXPECT_EQ( RiuPlotAxis::defaultRight(), fopr->axisY() );
    EXPECT_NE( fopt->axisY(), fopr->axisY() );
}

//--------------------------------------------------------------------------------------------------
/// Same as AssignPlotAxis_FirstCurveUsesDefaultLeftAxis, but for an ensemble curve set. Prior to the
/// fix for #14728, ensemble curve sets had no automatic axis assignment logic at all and were always
/// forced onto the default left axis regardless of their unit.
//--------------------------------------------------------------------------------------------------
TEST( RimSummaryPlot, AssignPlotAxis_FirstEnsembleCurveSetUsesDefaultLeftAxis )
{
    RimSummaryPlot     plot;
    RimSummaryEnsemble ensemble;
    ensemble.setAsEnsemble( true );

    auto* curveSet = createEnsembleCurveSet( &ensemble, "SM3", RifEclipseSummaryAddress::fieldAddress( "FOPT" ) );
    plot.ensembleCurveSetCollection()->addCurveSet( curveSet );

    EXPECT_EQ( RiuPlotAxis::defaultLeft(), curveSet->axisY() );
}

//--------------------------------------------------------------------------------------------------
/// Two ensemble curve sets with differing units (e.g. FOPT and FOPR) must be automatically assigned
/// to different axes.
//--------------------------------------------------------------------------------------------------
TEST( RimSummaryPlot, AssignPlotAxis_DifferingUnitEnsembleCurveSetsUseDifferentAxes )
{
    RimSummaryPlot     plot;
    RimSummaryEnsemble ensemble;
    ensemble.setAsEnsemble( true );

    auto* foptCurveSet = createEnsembleCurveSet( &ensemble, "SM3", RifEclipseSummaryAddress::fieldAddress( "FOPT" ) );
    plot.ensembleCurveSetCollection()->addCurveSet( foptCurveSet );

    auto* foprCurveSet = createEnsembleCurveSet( &ensemble, "SM3/DAY", RifEclipseSummaryAddress::fieldAddress( "FOPR" ) );
    plot.ensembleCurveSetCollection()->addCurveSet( foprCurveSet );

    EXPECT_EQ( RiuPlotAxis::defaultLeft(), foptCurveSet->axisY() );
    EXPECT_EQ( RiuPlotAxis::defaultRight(), foprCurveSet->axisY() );
    EXPECT_NE( foptCurveSet->axisY(), foprCurveSet->axisY() );
}

//--------------------------------------------------------------------------------------------------
/// Two ensemble curve sets sharing the same unit are assumed to have comparable value ranges, and
/// should reuse the same axis to avoid creating unnecessary additional axes.
//--------------------------------------------------------------------------------------------------
TEST( RimSummaryPlot, AssignPlotAxis_MatchingUnitEnsembleCurveSetsReuseSameAxis )
{
    RimSummaryPlot     plot;
    RimSummaryEnsemble ensemble;
    ensemble.setAsEnsemble( true );

    auto* curveSetA = createEnsembleCurveSet( &ensemble, "SM3", RifEclipseSummaryAddress::fieldAddress( "FOPT" ) );
    plot.ensembleCurveSetCollection()->addCurveSet( curveSetA );

    auto* curveSetB = createEnsembleCurveSet( &ensemble, "SM3", RifEclipseSummaryAddress::fieldAddress( "FGPT" ) );
    plot.ensembleCurveSetCollection()->addCurveSet( curveSetB );

    EXPECT_EQ( curveSetA->axisY(), curveSetB->axisY() );
}

//--------------------------------------------------------------------------------------------------
/// A single curve and an ensemble curve set with differing units must be assigned to different axes,
/// verifying that curves and ensemble curve sets are compared against each other and not just within
/// their own kind.
//--------------------------------------------------------------------------------------------------
TEST( RimSummaryPlot, AssignPlotAxis_SingleCurveAndEnsembleCurveSetWithDifferingUnitsUseDifferentAxes )
{
    RimSummaryPlot     plot;
    RimSummaryEnsemble ensemble;
    ensemble.setAsEnsemble( true );

    auto* singleCurve = createCurve( "SM3", RifEclipseSummaryAddress::fieldAddress( "FOPT" ) );
    plot.addCurveAndUpdate( singleCurve );

    auto* curveSet = createEnsembleCurveSet( &ensemble, "SM3/DAY", RifEclipseSummaryAddress::fieldAddress( "FOPR" ) );
    plot.ensembleCurveSetCollection()->addCurveSet( curveSet );

    EXPECT_EQ( RiuPlotAxis::defaultLeft(), singleCurve->axisY() );
    EXPECT_EQ( RiuPlotAxis::defaultRight(), curveSet->axisY() );
}

//--------------------------------------------------------------------------------------------------
/// Same as above, but with the ensemble curve set added first and the single curve with a differing
/// unit added second.
//--------------------------------------------------------------------------------------------------
TEST( RimSummaryPlot, AssignPlotAxis_EnsembleCurveSetAndSingleCurveWithDifferingUnitsUseDifferentAxes )
{
    RimSummaryPlot     plot;
    RimSummaryEnsemble ensemble;
    ensemble.setAsEnsemble( true );

    auto* curveSet = createEnsembleCurveSet( &ensemble, "SM3", RifEclipseSummaryAddress::fieldAddress( "FOPT" ) );
    plot.ensembleCurveSetCollection()->addCurveSet( curveSet );

    auto* singleCurve = createCurve( "SM3/DAY", RifEclipseSummaryAddress::fieldAddress( "FOPR" ) );
    plot.addCurveAndUpdate( singleCurve );

    EXPECT_EQ( RiuPlotAxis::defaultLeft(), curveSet->axisY() );
    EXPECT_EQ( RiuPlotAxis::defaultRight(), singleCurve->axisY() );
}
