/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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
#include "RimStimPlanModelPressureCalculator.h"

#include "RiaDefines.h"
#include "RiaInterpolationTools.h"
#include "RiaLogging.h"
#include "RiaStimPlanModelDefines.h"

#include "RigWellPath.h"
#include "RigWellPathGeometryTools.h"

#include "RimModeledWellPath.h"
#include "RimPressureTable.h"
#include "RimPressureTableItem.h"
#include "RimStimPlanModel.h"
#include "RimStimPlanModelCalculator.h"
#include "RimStimPlanModelTemplate.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelPressureCalculator::RimStimPlanModelPressureCalculator( RimStimPlanModelCalculator* stimPlanModelCalculator )
    : RimStimPlanModelWellLogCalculator( stimPlanModelCalculator )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelPressureCalculator::isMatching( RiaDefines::CurveProperty curveProperty ) const
{
    std::vector<RiaDefines::CurveProperty> matching = {
        RiaDefines::CurveProperty::INITIAL_PRESSURE,
        RiaDefines::CurveProperty::PRESSURE,
    };

    return std::find( matching.begin(), matching.end(), curveProperty ) != matching.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t findFirstIndex( double depth, const std::vector<double>& depths )
{
    if ( !depths.empty() )
    {
        for ( size_t i = 0; i < depths.size(); i++ )
            if ( depths[i] > depth ) return i - 1;
    }

    // Not found
    return depths.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t findLastIndex( double depth, const std::vector<double>& depths, size_t firstIndex )
{
    if ( !depths.empty() )
    {
        for ( size_t i = firstIndex; i < depths.size(); i++ )
            if ( depths[i] >= depth ) return i;
    }

    // Not found
    return depths.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<size_t, size_t> findIndex( double depth, const std::vector<double>& depths )
{
    size_t firstIndex = findFirstIndex( depth, depths );
    size_t lastIndex  = findLastIndex( depth, depths, firstIndex );
    return std::make_pair( firstIndex, lastIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelPressureCalculator::extractValuesForProperty( RiaDefines::CurveProperty curveProperty,
                                                                   const RimStimPlanModel*   stimPlanModel,
                                                                   int                       timeStep,
                                                                   std::vector<double>&      values,
                                                                   std::vector<double>&      measuredDepthValues,
                                                                   std::vector<double>&      tvDepthValues,
                                                                   double&                   rkbDiff ) const
{
    // Get depth from the static eclipse case
    std::vector<double> staticTvDepthValues;
    std::vector<double> staticMeasuredDepthValues;
    std::vector<double> faciesValues;

    {
        std::vector<double> dummyValues;
        double              dummyRkbDiff;
        if ( !RimStimPlanModelWellLogCalculator::extractValuesForProperty( RiaDefines::CurveProperty::FACIES,
                                                                           stimPlanModel,
                                                                           timeStep,
                                                                           faciesValues,
                                                                           staticMeasuredDepthValues,
                                                                           staticTvDepthValues,
                                                                           dummyRkbDiff ) )
        {
            return false;
        }
    }

    if ( stimPlanModel->stimPlanModelTemplate()->usePressureTableForProperty( curveProperty ) )
    {
        if ( !extractPressureDataFromTable( curveProperty, stimPlanModel, values, measuredDepthValues, tvDepthValues ) )
        {
            RiaLogging::error( "Unable to extract pressure data from table" );
            return false;
        }
    }
    else
    {
        // Extract the property we care about
        RimStimPlanModelWellLogCalculator::extractValuesForProperty( curveProperty,
                                                                     stimPlanModel,
                                                                     timeStep,
                                                                     values,
                                                                     measuredDepthValues,
                                                                     tvDepthValues,
                                                                     rkbDiff );
    }

    if ( staticTvDepthValues.size() != tvDepthValues.size() )
    {
        // Populate the original tvds/mds with interpolated values
        std::vector<double> tvds;
        std::vector<double> mds;
        std::vector<double> results;

        double prevEndIndex = 0;
        for ( size_t i = 0; i < staticTvDepthValues.size(); i++ )
        {
            double tvd   = staticTvDepthValues[i];
            double md    = staticMeasuredDepthValues[i];
            double value = std::numeric_limits<double>::infinity();

            // Find value before and after this depth in the static data
            auto [startIndex, endIndex] = findIndex( md, measuredDepthValues );

            if ( startIndex < values.size() && endIndex < values.size() )
            {
                double prevValue = values[startIndex];
                double nextValue = values[endIndex];

                if ( startIndex == endIndex )
                {
                    const double delta  = 0.001;
                    double       prevMd = staticMeasuredDepthValues[i - 1];
                    double       diffMd = std::fabs( prevMd - md );
                    if ( startIndex > prevEndIndex && diffMd > delta )
                    {
                        // Avoid skipping datapoints in the original data:
                        // this can happen when multiple point have same measured depth.
                        // Need to get the "stair step" look of the pressure data after interpolation.
                        value = values[prevEndIndex];
                    }
                    else
                    {
                        // Exact match: not need to interpolate
                        value = prevValue;
                    }
                }
                else if ( !std::isinf( prevValue ) && !std::isinf( nextValue ) )
                {
                    // Interpolate a value for the given md
                    std::vector<double> xs = { measuredDepthValues[startIndex], md, measuredDepthValues[endIndex] };
                    std::vector<double> ys = { prevValue, std::numeric_limits<double>::infinity(), values[endIndex] };
                    RiaInterpolationTools::interpolateMissingValues( xs, ys );
                    value = ys[1];
                }

                prevEndIndex = endIndex;
            }
            else
            {
                // The last point is added without interpolation
                value = values.back();
            }

            results.push_back( value );
            tvds.push_back( tvd );
            mds.push_back( md );
        }

        tvDepthValues       = tvds;
        measuredDepthValues = mds;
        values              = results;
    }

    // Filter out the facies which does not have pressure depletion.
    std::map<int, double> faciesWithInitialPressure = stimPlanModel->stimPlanModelTemplate()->faciesWithInitialPressure();

    if ( curveProperty == RiaDefines::CurveProperty::PRESSURE && !faciesWithInitialPressure.empty() )
    {
        std::vector<double> initialPressureValues;
        std::vector<double> initialPressureMeasuredDepthValues;
        std::vector<double> initialPressureTvDepthValues;

        if ( !stimPlanModel->calculator()->extractCurveData( RiaDefines::CurveProperty::INITIAL_PRESSURE,
                                                             timeStep,
                                                             initialPressureValues,
                                                             initialPressureMeasuredDepthValues,
                                                             initialPressureTvDepthValues,
                                                             rkbDiff ) )
        {
            return false;
        }

        for ( size_t i = 0; i < faciesValues.size(); i++ )
        {
            // Use the values from initial pressure curve
            int    faciesValue     = static_cast<int>( faciesValues[i] );
            double currentPressure = values[i];
            double initialPressure = initialPressureValues[i];
            auto   faciesConfig    = faciesWithInitialPressure.find( faciesValue );
            if ( faciesConfig != faciesWithInitialPressure.end() && !std::isinf( currentPressure ) &&
                 !std::isinf( initialPressure ) )
            {
                double fraction = faciesConfig->second;
                double value    = initialPressure - ( initialPressure - currentPressure ) * ( 1.0 - fraction );

                values[i] = value;
            }
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelPressureCalculator::extractPressureDataFromTable( RiaDefines::CurveProperty curveProperty,
                                                                       const RimStimPlanModel*   stimPlanModel,
                                                                       std::vector<double>&      values,
                                                                       std::vector<double>&      measuredDepthValues,
                                                                       std::vector<double>&      tvDepthValues ) const
{
    RimStimPlanModelTemplate* stimPlanModelTemplate = stimPlanModel->stimPlanModelTemplate();
    if ( !stimPlanModelTemplate ) return false;

    RimPressureTable* pressureTable = stimPlanModelTemplate->pressureTable();
    if ( !pressureTable ) return false;

    std::vector<RimPressureTableItem*> items = pressureTable->items();
    if ( items.empty() ) return false;

    if ( !stimPlanModel->thicknessDirectionWellPath() )
    {
        return false;
    }

    RigWellPath* wellPathGeometry = stimPlanModel->thicknessDirectionWellPath()->wellPathGeometry();
    if ( !wellPathGeometry )
    {
        RiaLogging::error( "No well path geometry found for pressure data table." );
        return false;
    }

    // Convert table data into a "fake" well log extraction
    for ( RimPressureTableItem* item : items )
    {
        if ( curveProperty == RiaDefines::CurveProperty::INITIAL_PRESSURE )
            values.push_back( item->initialPressure() );
        else
        {
            values.push_back( item->pressure() );
        }

        tvDepthValues.push_back( item->depth() );
    }

    // Interpolate MDs from the tvd data from the table and well path geometry
    const std::vector<double>& mdValuesOfWellPath  = wellPathGeometry->measuredDepths();
    const std::vector<double>& tvdValuesOfWellPath = wellPathGeometry->trueVerticalDepths();

    measuredDepthValues =
        RigWellPathGeometryTools::interpolateMdFromTvd( mdValuesOfWellPath, tvdValuesOfWellPath, tvDepthValues );
    CVF_ASSERT( measuredDepthValues.size() == tvDepthValues.size() );

    return true;
}
