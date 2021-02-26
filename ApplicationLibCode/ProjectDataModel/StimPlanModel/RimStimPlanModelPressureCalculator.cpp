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

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigGridBase.h"
#include "RigMainGrid.h"
#include "RigWellPath.h"
#include "RigWellPathGeometryTools.h"

#include "RimEclipseCase.h"
#include "RimModeledWellPath.h"
#include "RimPressureTable.h"
#include "RimPressureTableItem.h"
#include "RimStimPlanModel.h"
#include "RimStimPlanModelCalculator.h"
#include "RimStimPlanModelTemplate.h"
#include "RimStimPlanModelWellLogCalculator.h"

#include <limits>

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

    if ( curveProperty == RiaDefines::CurveProperty::INITIAL_PRESSURE )
    {
        bool hasMissingValues = std::find( values.begin(), values.end(), std::numeric_limits<double>::infinity() ) !=
                                values.end();
        if ( hasMissingValues )
        {
            if ( !interpolateInitialPressureByEquilibrationRegion( stimPlanModel,
                                                                   timeStep,
                                                                   measuredDepthValues,
                                                                   tvDepthValues,
                                                                   values ) )
            {
                RiaLogging::error( "Pressure interpolation by equilibration region failed." );
            }
        }
    }
    else if ( curveProperty == RiaDefines::CurveProperty::PRESSURE )
    {
        // Filter out the facies which does not have pressure depletion.
        std::map<int, double> faciesWithInitialPressure =
            stimPlanModel->stimPlanModelTemplate()->faciesWithInitialPressure();

        if ( !faciesWithInitialPressure.empty() )
        {
            std::vector<double> initialPressureValues;
            std::vector<double> initialPressureMeasuredDepthValues;
            std::vector<double> initialPressureTvDepthValues;

            if ( !extractValuesForProperty( RiaDefines::CurveProperty::INITIAL_PRESSURE,
                                            stimPlanModel,
                                            timeStep,
                                            initialPressureValues,
                                            initialPressureMeasuredDepthValues,
                                            initialPressureTvDepthValues,
                                            rkbDiff ) )
            {
                return false;
            }

            CAF_ASSERT( faciesValues.size() == initialPressureValues.size() );
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

std::set<int> RimStimPlanModelPressureCalculator::findUniqueValues( const std::vector<double>& values )
{
    std::set<int> res;
    for ( double v : values )
    {
        if ( !std::isinf( v ) )
        {
            res.insert( static_cast<int>( v ) );
        }
    }

    return res;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModelPressureCalculator::sortAndRemoveDuplicates( DepthValuePairVector& depthValuePairs )
{
    std::sort( depthValuePairs.begin(), depthValuePairs.end() );
    depthValuePairs.erase( unique( depthValuePairs.begin(), depthValuePairs.end() ), depthValuePairs.end() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelPressureCalculator::buildPressureTablesPerEqlNum( const RimStimPlanModel*    stimPlanModel,
                                                                       EqlNumToDepthValuePairMap& valuesPerEqlNum,
                                                                       const std::set<int>&       presentEqlNums )
{
    RimEclipseCase* eclipseCase = stimPlanModel->eclipseCaseForProperty( RiaDefines::CurveProperty::EQLNUM );

    // TODO: too naive??
    int                gridIndex = 0;
    const RigGridBase* grid      = eclipseCase->mainGrid()->gridByIndex( gridIndex );

    RigEclipseCaseData* caseData = eclipseCase->eclipseCaseData();

    RiaDefines::PorosityModelType porosityModel = RiaDefines::PorosityModelType::MATRIX_MODEL;
    const std::vector<double>&    eqlNumValues =
        RimStimPlanModelWellLogCalculator::loadResults( caseData,
                                                        porosityModel,
                                                        RiaDefines::ResultCatType::STATIC_NATIVE,
                                                        "EQLNUM" );
    const std::vector<double>& pressureValues =
        RimStimPlanModelWellLogCalculator::loadResults( caseData,
                                                        porosityModel,
                                                        RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                        "PRESSURE" );

    if ( eqlNumValues.size() != pressureValues.size() )
    {
        RiaLogging::error( "Unexpected result size for EQLNUM and PRESSURE found for pressure calculation." );
        return false;
    }

    auto   activeCellInfo = caseData->activeCellInfo( porosityModel );
    size_t cellCount      = activeCellInfo->reservoirActiveCellCount();

    if ( cellCount != pressureValues.size() )
    {
        RiaLogging::error( "Unexpected number of active cells in pressure calculation." );
        return false;
    }

    for ( size_t cellIndex = 0; cellIndex < cellCount; cellIndex++ )
    {
        size_t resultIdx = activeCellInfo->cellResultIndex( cellIndex );
        int    eqlNum    = static_cast<int>( eqlNumValues[resultIdx] );
        double pressure  = pressureValues[resultIdx];
        if ( presentEqlNums.count( eqlNum ) > 0 && !std::isinf( pressure ) )
        {
            cvf::Vec3d center = grid->cell( cellIndex ).center();
            valuesPerEqlNum[eqlNum].push_back( std::make_pair( -center.z(), pressure ) );
        }
    }

    // Sort and remove duplicates for each depth/value dataset
    for ( int eqlNum : presentEqlNums )
    {
        sortAndRemoveDuplicates( valuesPerEqlNum[eqlNum] );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModelPressureCalculator::interpolatePressure( const DepthValuePairVector& depthValuePairs,
                                                                double                      depth,
                                                                int                         eqlNum )
{
    std::vector<double> depths;
    for ( auto dvp : depthValuePairs )
    {
        depths.push_back( dvp.first );
    }

    // Find value before and after this depth in the static data
    auto [startIndex, endIndex] = findIndex( depth, depths );

    auto [startDepth, startValue] = depthValuePairs[startIndex];
    auto [endDepth, endValue]     = depthValuePairs[endIndex];

    // Interpolate a value for the given tvd
    std::vector<double> xs = { startDepth, depth, endDepth };
    std::vector<double> ys = { startValue, std::numeric_limits<double>::infinity(), endValue };
    RiaInterpolationTools::interpolateMissingValues( xs, ys );
    double value = ys[1];

    RiaLogging::info( QString( "Interpolating initial pressure from %1 depth/value pairs (EQLNUM: %2, TVD: %3)."
                               " Above: TVD: %4, P: %5. Below: TVD: %6, P: %7. Pressure: %8" )
                          .arg( depthValuePairs.size() )
                          .arg( eqlNum )
                          .arg( depth )
                          .arg( startDepth )
                          .arg( startValue )
                          .arg( endDepth )
                          .arg( endValue )
                          .arg( value ) );

    return value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModelPressureCalculator::interpolateInitialPressureByEquilibrationRegion(
    const RimStimPlanModel*    stimPlanModel,
    int                        timeStep,
    const std::vector<double>& measuredDepthValues,
    const std::vector<double>& tvDepthValues,
    std::vector<double>&       values ) const
{
    std::vector<double> eqlNumValues;
    std::vector<double> eqlNumMeasuredDepthsValues;
    std::vector<double> eqlNumTvDepthValues;
    double              rkbDiff = -1.0;
    if ( !stimPlanModel->calculator()->extractCurveData( RiaDefines::CurveProperty::EQLNUM,
                                                         timeStep,
                                                         eqlNumValues,
                                                         eqlNumMeasuredDepthsValues,
                                                         eqlNumTvDepthValues,
                                                         rkbDiff ) )
    {
        RiaLogging::error( "Failed to extract EQLNUM data in pressure calculation" );
        return false;
    }

    std::set<int> presentEqlNums = findUniqueValues( eqlNumValues );

    RiaLogging::info( QString( "Found %1 EQLNUM values." ).arg( presentEqlNums.size() ) );

    EqlNumToDepthValuePairMap valuesPerEqlNum;
    if ( !buildPressureTablesPerEqlNum( stimPlanModel, valuesPerEqlNum, presentEqlNums ) )
    {
        RiaLogging::error( "Failed to build EQLNUM pressure data in pressure calculation" );
        return false;
    }

    for ( size_t i = 0; i < values.size(); i++ )
    {
        if ( std::isinf( values[i] ) )
        {
            int                  eqlNum          = static_cast<int>( eqlNumValues[i] );
            DepthValuePairVector depthValuePairs = valuesPerEqlNum[eqlNum];
            if ( !depthValuePairs.empty() )
            {
                double depth = tvDepthValues[i];
                double value = interpolatePressure( depthValuePairs, depth, eqlNum );
                values[i]    = value;
            }
        }
    }

    return true;
}
