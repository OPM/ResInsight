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

#include "RimSummaryPlotTools.h"

#include "RiaStdStringTools.h"
#include "Summary/RiaSummaryAddressModifier.h"

#include "RimEnsembleCurveSet.h"
#include "RimPlotAxisPropertiesInterface.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "Tools/RimPlotAxisTools.h"

#include "RiuPlotAxis.h"
#include "RiuPlotWidget.h"

#include <algorithm>
#include <variant>

namespace internal
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t countAxes( const std::vector<RimPlotAxisPropertiesInterface*>& axes, RiaDefines::PlotAxis axis )
{
    return std::count_if( axes.begin(), axes.end(), [axis]( const auto& ap ) { return ap->plotAxis().axis() == axis; } );
}

//--------------------------------------------------------------------------------------------------
/// Unit, vector and axis of a curve or ensemble curve set already present in the plot, used to
/// auto-assign a newly added curve or curve set to a matching or unused Y-axis.
//--------------------------------------------------------------------------------------------------
struct AxisCandidateInfo
{
    std::string unitName;
    QString     vectorAxisText;
    RiuPlotAxis axis;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
AxisCandidateInfo axisCandidateInfo( const RiaSummaryAddressModifier::CurveAddressProvider& provider )
{
    return std::visit(
        []( auto&& arg )
        { return AxisCandidateInfo{ arg->unitNameY(), RimPlotAxisTools::axisTextForAddress( arg->summaryAddressY() ), arg->axisY() }; },
        provider );
}

//--------------------------------------------------------------------------------------------------
/// Find the Y-axis to use for a curve or curve set, based on matching unit or summary vector of the other curves and curve sets.
/// Creates a new axis if required.
//--------------------------------------------------------------------------------------------------
RiuPlotAxis findOrCreateYPlotAxis( RimSummaryPlot*                                     plot,
                                   const std::vector<RimPlotAxisPropertiesInterface*>& axisProperties,
                                   const AxisCandidateInfo&                            incoming,
                                   const std::vector<AxisCandidateInfo>&               others )
{
    enum class AxisAssignmentStrategy
    {
        ALTERNATING,
        USE_MATCHING_UNIT,
        USE_MATCHING_VECTOR
    };

    auto strategy = AxisAssignmentStrategy::USE_MATCHING_UNIT;

    auto destinationUnit      = RiaStdStringTools::toUpper( incoming.unitName );
    bool anyOtherWithUnitText = std::any_of( others.begin(), others.end(), []( const auto& o ) { return !o.unitName.empty(); } );
    if ( destinationUnit.empty() || !anyOtherWithUnitText ) strategy = AxisAssignmentStrategy::USE_MATCHING_VECTOR;

    if ( strategy == AxisAssignmentStrategy::USE_MATCHING_VECTOR )
    {
        // Special handling if curve unit is matching. Try to match on summary vector name to avoid creation of new axis
        for ( const auto& o : others )
        {
            if ( incoming.vectorAxisText == o.vectorAxisText ) return o.axis;
        }
    }
    else if ( strategy == AxisAssignmentStrategy::USE_MATCHING_UNIT )
    {
        bool hasLeftOrRightAxis = std::any_of( axisProperties.begin(),
                                               axisProperties.end(),
                                               []( const auto& ap )
                                               {
                                                   return ap->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_LEFT ||
                                                          ap->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_RIGHT;
                                               } );
        if ( hasLeftOrRightAxis )
        {
            for ( const auto& o : others )
            {
                if ( RiaStdStringTools::toUpper( o.unitName ) == destinationUnit ) return o.axis;
            }
        }

        strategy = AxisAssignmentStrategy::ALTERNATING;
    }

    bool defaultLeftUsed = std::any_of( others.begin(), others.end(), []( const auto& o ) { return o.axis == RiuPlotAxis::defaultLeft(); } );
    if ( !defaultLeftUsed ) return RiuPlotAxis::defaultLeft();

    bool defaultRightUsed = std::any_of( others.begin(), others.end(), []( const auto& o ) { return o.axis == RiuPlotAxis::defaultRight(); } );
    if ( !defaultRightUsed ) return RiuPlotAxis::defaultRight();

    RiaDefines::PlotAxis plotAxisType = RiaDefines::PlotAxis::PLOT_AXIS_LEFT;
    if ( strategy == AxisAssignmentStrategy::ALTERNATING )
    {
        size_t axisCountLeft  = countAxes( axisProperties, RiaDefines::PlotAxis::PLOT_AXIS_LEFT );
        size_t axisCountRight = countAxes( axisProperties, RiaDefines::PlotAxis::PLOT_AXIS_RIGHT );

        if ( axisCountLeft > axisCountRight ) plotAxisType = RiaDefines::PlotAxis::PLOT_AXIS_RIGHT;
    }

    if ( plot->plotWidget() && plot->plotWidget()->isMultiAxisSupported() )
    {
        auto newPlotAxis = plot->plotWidget()->createNextPlotAxis( plotAxisType );
        plot->addNewAxisProperties( newPlotAxis, "New Axis" );

        return newPlotAxis;
    }

    // If we get here, we have no more axes to assign to, use left axis as fallback
    return RiuPlotAxis::defaultLeft();
}
} // namespace internal

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotTools::assignYPlotAxis( RimSummaryPlot*                                        plot,
                                           const std::vector<RimPlotAxisPropertiesInterface*>&    axisProperties,
                                           const RiaSummaryAddressModifier::CurveAddressProvider& curveProvider )
{
    std::vector<internal::AxisCandidateInfo> others;
    for ( const auto& provider : RiaSummaryAddressModifier::createAddressProviders( plot ) )
    {
        if ( provider != curveProvider ) others.push_back( internal::axisCandidateInfo( provider ) );
    }

    const auto axis = internal::findOrCreateYPlotAxis( plot, axisProperties, internal::axisCandidateInfo( curveProvider ), others );

    std::visit( [axis]( auto&& arg ) { arg->setLeftOrRightAxisY( axis ); }, curveProvider );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotTools::assignXPlotAxis( RimSummaryPlot*                                     plot,
                                           const std::vector<RimPlotAxisPropertiesInterface*>& axisProperties,
                                           RimSummaryCurve*                                    curve )
{
    RiuPlotAxis newPlotAxis = RimSummaryPlot::plotAxisForTime();

    if ( curve->axisTypeX() == RiaDefines::HorizontalAxisType::SUMMARY_VECTOR )
    {
        enum class AxisAssignmentStrategy
        {
            ALL_TOP,
            ALL_BOTTOM,
            ALTERNATING,
            USE_MATCHING_UNIT,
            USE_MATCHING_VECTOR
        };

        auto strategy = AxisAssignmentStrategy::USE_MATCHING_UNIT;

        auto destinationUnit = RiaStdStringTools::toUpper( curve->unitNameX() );
        if ( destinationUnit.empty() ) strategy = AxisAssignmentStrategy::USE_MATCHING_VECTOR;

        auto anyCurveWithUnitText = [plot, curve]
        {
            for ( auto c : plot->summaryCurves() )
            {
                if ( c == curve ) continue;

                if ( !c->unitNameX().empty() ) return true;
            }

            return false;
        };

        if ( !anyCurveWithUnitText() ) strategy = AxisAssignmentStrategy::USE_MATCHING_VECTOR;

        if ( strategy == AxisAssignmentStrategy::USE_MATCHING_VECTOR )
        {
            // Special handling if curve unit is matching. Try to match on summary vector name to avoid creation of new axis

            for ( auto c : plot->summaryCurves() )
            {
                if ( c == curve ) continue;

                auto incomingAxisText = RimPlotAxisTools::axisTextForAddress( curve->summaryAddressY() );
                auto currentAxisText  = RimPlotAxisTools::axisTextForAddress( c->summaryAddressY() );
                if ( incomingAxisText == currentAxisText )
                {
                    curve->setTopOrBottomAxisX( c->axisX() );
                    return;
                }
            }
        }
        else if ( strategy == AxisAssignmentStrategy::USE_MATCHING_UNIT )
        {
            bool isTopUsed    = false;
            bool isBottomUsed = false;

            for ( auto c : plot->summaryCurves() )
            {
                if ( c == curve ) continue;

                if ( c->axisX() == RiuPlotAxis::defaultTop() ) isTopUsed = true;
                if ( c->axisX() == RiuPlotAxis::defaultBottomForSummaryVectors() ) isBottomUsed = true;

                auto currentUnit = RiaStdStringTools::toUpper( c->unitNameX() );

                if ( currentUnit == destinationUnit )
                {
                    for ( RimPlotAxisPropertiesInterface* ap : axisProperties )
                    {
                        if ( ap->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_TOP ||
                             ap->plotAxis().axis() == RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM )
                        {
                            curve->setTopOrBottomAxisX( c->axisX() );

                            return;
                        }
                    }
                }
            }

            if ( !isTopUsed )
            {
                curve->setTopOrBottomAxisX( RiuPlotAxis::defaultTop() );
                return;
            }

            if ( !isBottomUsed )
            {
                curve->setTopOrBottomAxisX( RiuPlotAxis::defaultBottomForSummaryVectors() );
                return;
            }

            strategy = AxisAssignmentStrategy::ALTERNATING;
        }

        RiaDefines::PlotAxis plotAxisType = RiaDefines::PlotAxis::PLOT_AXIS_TOP;

        if ( strategy == AxisAssignmentStrategy::ALTERNATING )
        {
            size_t axisCountTop = internal::countAxes( axisProperties, RiaDefines::PlotAxis::PLOT_AXIS_TOP );
            size_t axisCountBot = internal::countAxes( axisProperties, RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM );

            if ( axisCountTop > axisCountBot ) plotAxisType = RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM;
        }
        else if ( strategy == AxisAssignmentStrategy::ALL_TOP )
        {
            plotAxisType = RiaDefines::PlotAxis::PLOT_AXIS_TOP;
        }
        else if ( strategy == AxisAssignmentStrategy::ALL_BOTTOM )
        {
            plotAxisType = RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM;
        }

        RiuPlotAxis newPlotAxis = RiuPlotAxis::defaultBottomForSummaryVectors();
        if ( plot->plotWidget() && plot->plotWidget()->isMultiAxisSupported() )
        {
            newPlotAxis = plot->plotWidget()->createNextPlotAxis( plotAxisType );
            plot->addNewAxisProperties( newPlotAxis, "New Axis" );
        }
    }

    curve->setTopOrBottomAxisX( newPlotAxis );
}
