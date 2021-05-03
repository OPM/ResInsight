/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RigEnsembleFractureStatisticsCalculator.h"

#include "RiaDefines.h"
#include "RiaWeightedMeanCalculator.h"

#include "RigFractureCell.h"
#include "RigFractureGrid.h"
#include "RigHistogramData.h"
#include "RigStatisticsMath.h"
#include "RigStimPlanFractureDefinition.h"
#include "RigTransmissibilityEquations.h"

#include "RimEnsembleFractureStatistics.h"
#include "RimStimPlanFractureTemplate.h"

#include "cafAppEnum.h"

#include "cvfObject.h"

#include <limits>

namespace caf
{
template <>
void caf::AppEnum<RigEnsembleFractureStatisticsCalculator::PropertyType>::setUp()
{
    addItem( RigEnsembleFractureStatisticsCalculator::PropertyType::HEIGHT, "HEIGHT", "Height" );
    addItem( RigEnsembleFractureStatisticsCalculator::PropertyType::AREA, "AREA", "Area" );
    addItem( RigEnsembleFractureStatisticsCalculator::PropertyType::WIDTH, "WIDTH", "Width" );
    addItem( RigEnsembleFractureStatisticsCalculator::PropertyType::XF, "XF", "Xf" );
    addItem( RigEnsembleFractureStatisticsCalculator::PropertyType::KFWF, "KFWF", "KfWf" );
    addItem( RigEnsembleFractureStatisticsCalculator::PropertyType::PERMEABILITY, "PERMEABILITY", "Permeability" );
    setDefault( RigEnsembleFractureStatisticsCalculator::PropertyType::HEIGHT );
}
}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigHistogramData RigEnsembleFractureStatisticsCalculator::createStatisticsData( RimEnsembleFractureStatistics* esf,
                                                                                PropertyType propertyType )
{
    std::vector<cvf::ref<RigStimPlanFractureDefinition>> defs = esf->readFractureDefinitions();

    std::vector<double> samples;
    if ( propertyType == PropertyType::HEIGHT )
    {
        samples = calculateGridStatistics( defs, &RigEnsembleFractureStatisticsCalculator::calculateHeight );
    }
    else if ( propertyType == PropertyType::AREA )
    {
        samples = calculateGridStatistics( defs, &RigEnsembleFractureStatisticsCalculator::calculateArea );
    }
    else if ( propertyType == PropertyType::WIDTH )
    {
        samples =
            calculateAreaWeightedStatistics( defs, &RigEnsembleFractureStatisticsCalculator::calculateAreaWeightedWidth );
    }
    else if ( propertyType == PropertyType::PERMEABILITY )
    {
        samples =
            calculateAreaWeightedStatistics( defs,
                                             &RigEnsembleFractureStatisticsCalculator::calculateAreaWeightedPermeability );
    }
    else if ( propertyType == PropertyType::XF )
    {
        samples = calculateGridStatistics( defs, &RigEnsembleFractureStatisticsCalculator::calculateXf );
    }
    else if ( propertyType == PropertyType::KFWF )
    {
        samples = calculateGridStatistics( defs, &RigEnsembleFractureStatisticsCalculator::calculateKfWf );
    }

    RigHistogramData histogramData;

    double sum;
    double range;
    double dev;
    RigStatisticsMath::calculateBasicStatistics( samples,
                                                 &histogramData.min,
                                                 &histogramData.max,
                                                 &sum,
                                                 &range,
                                                 &histogramData.mean,
                                                 &dev );

    double p50;
    double mean;
    RigStatisticsMath::calculateStatisticsCurves( samples, &histogramData.p10, &p50, &histogramData.p90, &mean );

    std::vector<size_t>    histogram;
    RigHistogramCalculator histogramCalculator( histogramData.min, histogramData.max, 20, &histogram );
    for ( auto s : samples )
        histogramCalculator.addValue( s );

    histogramData.histogram = histogram;

    return histogramData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigEnsembleFractureStatisticsCalculator::calculateGridStatistics(
    const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& defs,
    double( func )( cvf::cref<RigFractureGrid> ) )
{
    std::vector<double> samples;
    if ( defs.empty() ) return samples;

    // TODO: heuristic to find conductivity name?
    QString conductivityResultName = defs[0]->conductivityResultNames()[0];

    std::vector<cvf::cref<RigFractureGrid>> grids =
        RimEnsembleFractureStatistics::createFractureGrids( defs,
                                                            RiaDefines::EclipseUnitSystem::UNITS_METRIC,
                                                            conductivityResultName,
                                                            RimEnsembleFractureStatistics::MeshAlignmentType::PERFORATION_DEPTH );

    for ( auto grid : grids )
    {
        double result = func( grid );
        samples.push_back( result );
    }

    return samples;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigEnsembleFractureStatisticsCalculator::calculateHeight( cvf::cref<RigFractureGrid> fractureGrid )
{
    double longestRange = 0.0;

    for ( size_t i = 0; i < fractureGrid->iCellCount(); i++ )
    {
        double currentAggregatedDistanceY = 0.0;
        for ( size_t j = 0; j < fractureGrid->jCellCount(); j++ )
        {
            size_t idx          = fractureGrid->getGlobalIndexFromIJ( i, j );
            auto   fractureCell = fractureGrid->cellFromIndex( idx );

            double conductivityValue = fractureCell.getConductivityValue();
            if ( conductivityValue > 0.0 )
            {
                currentAggregatedDistanceY += fractureCell.cellSizeZ();
            }
            else
            {
                longestRange               = std::max( longestRange, currentAggregatedDistanceY );
                currentAggregatedDistanceY = 0.0;
            }
        }

        longestRange = std::max( longestRange, currentAggregatedDistanceY );
    }

    return longestRange;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigEnsembleFractureStatisticsCalculator::calculateArea( cvf::cref<RigFractureGrid> fractureGrid )
{
    double sum = 0.0;
    for ( auto fractureCell : fractureGrid->fractureCells() )
    {
        double value = fractureCell.getConductivityValue();
        if ( !std::isinf( value ) && value > 0.0 )
        {
            sum += fractureCell.area();
        }
    }

    return sum;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigEnsembleFractureStatisticsCalculator::calculateKfWf( cvf::cref<RigFractureGrid> fractureGrid )
{
    RiaWeightedMeanCalculator<double> calc;
    for ( auto fractureCell : fractureGrid->fractureCells() )
    {
        double value = fractureCell.getConductivityValue();
        if ( !std::isinf( value ) && value > 0.0 )
        {
            double area = fractureCell.area();
            calc.addValueAndWeight( value, area );
        }
    }

    return calc.weightedMean();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigEnsembleFractureStatisticsCalculator::calculateAreaWeightedStatistics(
    const std::vector<cvf::ref<RigStimPlanFractureDefinition>>& defs,
    double( func )( cvf::cref<RigFractureGrid>, cvf::cref<RigFractureGrid> ) )
{
    std::vector<double> samples;
    if ( defs.empty() ) return samples;

    // TODO: heuristic to find conductivity name?
    QString conductivityResultName = defs[0]->conductivityResultNames()[0];

    std::vector<cvf::cref<RigFractureGrid>> grids =
        RimEnsembleFractureStatistics::createFractureGrids( defs,
                                                            RiaDefines::EclipseUnitSystem::UNITS_METRIC,
                                                            conductivityResultName,
                                                            RimEnsembleFractureStatistics::MeshAlignmentType::PERFORATION_DEPTH );

    QString widthResultName = RimStimPlanFractureTemplate::widthParameterNameAndUnit( defs[0] ).first;
    std::vector<cvf::cref<RigFractureGrid>> widthGrids =
        RimEnsembleFractureStatistics::createFractureGrids( defs,
                                                            RiaDefines::EclipseUnitSystem::UNITS_METRIC,
                                                            widthResultName,
                                                            RimEnsembleFractureStatistics::MeshAlignmentType::PERFORATION_DEPTH );

    CAF_ASSERT( grids.size() == widthGrids.size() );

    for ( size_t i = 0; i < grids.size(); i++ )
    {
        double result = func( grids[i], widthGrids[i] );
        samples.push_back( result );
    }

    return samples;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigEnsembleFractureStatisticsCalculator::calculateAreaWeightedWidth( cvf::cref<RigFractureGrid> conductivityGrid,
                                                                            cvf::cref<RigFractureGrid> widthGrid )
{
    RiaWeightedMeanCalculator<double>   calc;
    const std::vector<RigFractureCell>& conductivityCells = conductivityGrid->fractureCells();
    const std::vector<RigFractureCell>& widthCells        = widthGrid->fractureCells();
    CAF_ASSERT( conductivityCells.size() == widthCells.size() );

    for ( size_t i = 0; i < conductivityCells.size(); i++ )
    {
        double value = conductivityCells[i].getConductivityValue();
        if ( !std::isinf( value ) && value > 0.0 )
        {
            double cellArea = conductivityCells[i].area();
            // TODO: conductivity is misleading here
            double widthValue = widthCells[i].getConductivityValue();
            calc.addValueAndWeight( widthValue, cellArea );
        }
    }

    return calc.weightedMean();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigEnsembleFractureStatisticsCalculator::calculateAreaWeightedPermeability( cvf::cref<RigFractureGrid> conductivityGrid,
                                                                                   cvf::cref<RigFractureGrid> widthGrid )
{
    RiaWeightedMeanCalculator<double>   calc;
    const std::vector<RigFractureCell>& conductivityCells = conductivityGrid->fractureCells();
    const std::vector<RigFractureCell>& widthCells        = widthGrid->fractureCells();
    CAF_ASSERT( conductivityCells.size() == widthCells.size() );

    for ( size_t i = 0; i < conductivityCells.size(); i++ )
    {
        double conductivity = conductivityCells[i].getConductivityValue();
        if ( !std::isinf( conductivity ) && conductivity > 0.0 )
        {
            double cellArea = conductivityCells[i].area();
            // TODO: conductivity is misleading here
            double width        = widthCells[i].getConductivityValue();
            double permeability = RigTransmissibilityEquations::permeability( conductivity, width );
            calc.addValueAndWeight( permeability, cellArea );
        }
    }

    return calc.weightedMean();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigEnsembleFractureStatisticsCalculator::calculateXf( cvf::cref<RigFractureGrid> fractureGrid )
{
    double height = calculateHeight( fractureGrid );
    double area   = calculateArea( fractureGrid );

    if ( height > 0.0 )
    {
        double length     = area / height;
        double halfLength = length / 2.0;
        return halfLength;
    }

    return 0.0;
}
