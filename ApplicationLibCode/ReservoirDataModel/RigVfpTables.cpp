/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RigVfpTables.h"

#include "RiaEclipseUnitTools.h"

#include "cafAppEnum.h"

#include "opm/input/eclipse/Schedule/VFPInjTable.hpp"
#include "opm/input/eclipse/Schedule/VFPProdTable.hpp"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VfpPlotData RigVfpTables::populatePlotData( const Opm::VFPInjTable&                 table,
                                            RimVfpDefines::InterpolatedVariableType interpolatedVariable,
                                            RimVfpDefines::FlowingPhaseType         flowingPhase )
{
    VfpPlotData plotData;

    QString xAxisTitle = axisTitle( RimVfpDefines::ProductionVariableType::FLOW_RATE, flowingPhase );
    plotData.setXAxisTitle( xAxisTitle );

    QString yAxisTitle = QString( "%1 %2" ).arg( caf::AppEnum<RimVfpDefines::InterpolatedVariableType>::uiText( interpolatedVariable ),
                                                 getDisplayUnitWithBracket( RimVfpDefines::ProductionVariableType::THP ) );
    plotData.setYAxisTitle( yAxisTitle );

    std::vector<double> thpValues = table.getTHPAxis();

    for ( size_t thp = 0; thp < thpValues.size(); thp++ )
    {
        size_t              numValues = table.getFloAxis().size();
        std::vector<double> xVals     = table.getFloAxis();
        std::vector<double> yVals( numValues, 0.0 );
        for ( size_t y = 0; y < numValues; y++ )
        {
            yVals[y] = table( thp, y );
            if ( interpolatedVariable == RimVfpDefines::InterpolatedVariableType::BHP_THP_DIFF )
            {
                yVals[y] -= thpValues[thp];
            }
        }

        double  value = convertToDisplayUnit( thpValues[thp], RimVfpDefines::ProductionVariableType::THP );
        QString unit  = getDisplayUnit( RimVfpDefines::ProductionVariableType::THP );
        QString title = QString( "%1 [%2]: %3" )
                            .arg( caf::AppEnum<RimVfpDefines::ProductionVariableType>::uiText( RimVfpDefines::ProductionVariableType::THP ) )
                            .arg( unit )
                            .arg( value );

        convertToDisplayUnit( yVals, RimVfpDefines::ProductionVariableType::THP );
        convertToDisplayUnit( xVals, RimVfpDefines::ProductionVariableType::FLOW_RATE );

        plotData.appendCurve( title, xVals, yVals );
    }

    return plotData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VfpPlotData RigVfpTables::populatePlotData( const Opm::VFPProdTable&                table,
                                            RimVfpDefines::ProductionVariableType   primaryVariable,
                                            RimVfpDefines::ProductionVariableType   familyVariable,
                                            RimVfpDefines::InterpolatedVariableType interpolatedVariable,
                                            RimVfpDefines::FlowingPhaseType         flowingPhase,
                                            const VfpTableSelection&                tableSelection ) const
{
    VfpPlotData plotData;

    QString xAxisTitle = axisTitle( primaryVariable, flowingPhase );
    plotData.setXAxisTitle( xAxisTitle );

    QString yAxisTitle = QString( "%1 %2" ).arg( caf::AppEnum<RimVfpDefines::InterpolatedVariableType>::uiText( interpolatedVariable ),
                                                 getDisplayUnitWithBracket( RimVfpDefines::ProductionVariableType::THP ) );
    plotData.setYAxisTitle( yAxisTitle );

    size_t numFamilyValues = getProductionTableData( table, familyVariable ).size();
    for ( size_t familyIdx = 0; familyIdx < numFamilyValues; familyIdx++ )
    {
        std::vector<double> primaryAxisValues    = getProductionTableData( table, primaryVariable );
        std::vector<double> familyVariableValues = getProductionTableData( table, familyVariable );
        std::vector<double> thpValues            = getProductionTableData( table, RimVfpDefines::ProductionVariableType::THP );

        size_t              numValues = primaryAxisValues.size();
        std::vector<double> yVals( numValues, 0.0 );

        for ( size_t y = 0; y < numValues; y++ )
        {
            size_t wfr_idx =
                getVariableIndex( table, RimVfpDefines::ProductionVariableType::WATER_CUT, primaryVariable, y, familyVariable, familyIdx, tableSelection );
            size_t gfr_idx = getVariableIndex( table,
                                               RimVfpDefines::ProductionVariableType::GAS_LIQUID_RATIO,
                                               primaryVariable,
                                               y,
                                               familyVariable,
                                               familyIdx,
                                               tableSelection );
            size_t alq_idx = getVariableIndex( table,
                                               RimVfpDefines::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY,
                                               primaryVariable,
                                               y,
                                               familyVariable,
                                               familyIdx,
                                               tableSelection );
            size_t flo_idx =
                getVariableIndex( table, RimVfpDefines::ProductionVariableType::FLOW_RATE, primaryVariable, y, familyVariable, familyIdx, tableSelection );
            size_t thp_idx =
                getVariableIndex( table, RimVfpDefines::ProductionVariableType::THP, primaryVariable, y, familyVariable, familyIdx, tableSelection );

            yVals[y] = table( thp_idx, wfr_idx, gfr_idx, alq_idx, flo_idx );
            if ( interpolatedVariable == RimVfpDefines::InterpolatedVariableType::BHP_THP_DIFF )
            {
                yVals[y] -= thpValues[thp_idx];
            }
        }

        double  familyValue = convertToDisplayUnit( familyVariableValues[familyIdx], familyVariable );
        QString familyUnit  = getDisplayUnit( familyVariable );
        QString familyTitle = QString( "%1: %2 %3" )
                                  .arg( caf::AppEnum<RimVfpDefines::ProductionVariableType>::uiText( familyVariable ) )
                                  .arg( familyValue )
                                  .arg( familyUnit );

        convertToDisplayUnit( yVals, RimVfpDefines::ProductionVariableType::THP );
        convertToDisplayUnit( primaryAxisValues, primaryVariable );

        plotData.appendCurve( familyTitle, primaryAxisValues, yVals );
    }

    return plotData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VfpPlotData RigVfpTables::populatePlotData( int                                     tableIndex,
                                            RimVfpDefines::ProductionVariableType   primaryVariable,
                                            RimVfpDefines::ProductionVariableType   familyVariable,
                                            RimVfpDefines::InterpolatedVariableType interpolatedVariable,
                                            RimVfpDefines::FlowingPhaseType         flowingPhase,
                                            const VfpTableSelection&                tableSelection ) const
{
    auto prodTable = productionTable( tableIndex );
    if ( prodTable.has_value() )
    {
        return populatePlotData( *prodTable, primaryVariable, familyVariable, interpolatedVariable, flowingPhase, tableSelection );
    };

    auto injContainer = injectionTable( tableIndex );
    if ( injContainer.has_value() )
    {
        return populatePlotData( *injContainer, interpolatedVariable, flowingPhase );
    };

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VfpPlotData RigVfpTables::populatePlotData( int                                     tableIndex,
                                            RimVfpDefines::ProductionVariableType   primaryVariable,
                                            RimVfpDefines::ProductionVariableType   familyVariable,
                                            RimVfpDefines::InterpolatedVariableType interpolatedVariable,
                                            RimVfpDefines::FlowingPhaseType         flowingPhase,
                                            const VfpValueSelection&                valueSelection ) const
{
    auto prodTable = productionTable( tableIndex );
    if ( prodTable.has_value() )
    {
        return populatePlotData( *prodTable, primaryVariable, familyVariable, interpolatedVariable, flowingPhase, valueSelection );
    }

    auto injContainer = injectionTable( tableIndex );
    if ( injContainer.has_value() )
    {
        return populatePlotData( *injContainer, interpolatedVariable, flowingPhase );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VfpPlotData RigVfpTables::populatePlotData( const Opm::VFPProdTable&                table,
                                            RimVfpDefines::ProductionVariableType   primaryVariable,
                                            RimVfpDefines::ProductionVariableType   familyVariable,
                                            RimVfpDefines::InterpolatedVariableType interpolatedVariable,
                                            RimVfpDefines::FlowingPhaseType         flowingPhase,
                                            const VfpValueSelection&                valueSelection ) const
{
    VfpPlotData plotData;

    QString xAxisTitle = axisTitle( primaryVariable, flowingPhase );
    plotData.setXAxisTitle( xAxisTitle );

    QString yAxisTitle = QString( "%1 %2" ).arg( caf::AppEnum<RimVfpDefines::InterpolatedVariableType>::uiText( interpolatedVariable ),
                                                 getDisplayUnitWithBracket( RimVfpDefines::ProductionVariableType::THP ) );
    plotData.setYAxisTitle( yAxisTitle );

    std::vector<double> familyFilterValues = valueSelection.familyValues;

    size_t numFamilyValues = getProductionTableData( table, familyVariable ).size();
    for ( size_t familyIdx = 0; familyIdx < numFamilyValues; familyIdx++ )
    {
        std::vector<double> primaryAxisValues    = getProductionTableData( table, primaryVariable );
        std::vector<double> familyVariableValues = getProductionTableData( table, familyVariable );
        std::vector<double> thpValues            = getProductionTableData( table, RimVfpDefines::ProductionVariableType::THP );

        // Skip if the family value is not in the filter
        auto currentFamilyValue = familyVariableValues[familyIdx];
        auto it                 = std::find( familyFilterValues.begin(), familyFilterValues.end(), currentFamilyValue );
        if ( it == familyFilterValues.end() ) continue;

        size_t              numValues = primaryAxisValues.size();
        std::vector<double> yVals( numValues, 0.0 );

        for ( size_t y = 0; y < numValues; y++ )
        {
            auto currentPrimaryValue = primaryAxisValues[y];

            size_t wfr_idx = getVariableIndexForValue( table,
                                                       RimVfpDefines::ProductionVariableType::WATER_CUT,
                                                       primaryVariable,
                                                       currentPrimaryValue,
                                                       familyVariable,
                                                       currentFamilyValue,
                                                       valueSelection );
            size_t gfr_idx = getVariableIndexForValue( table,
                                                       RimVfpDefines::ProductionVariableType::GAS_LIQUID_RATIO,
                                                       primaryVariable,
                                                       currentPrimaryValue,
                                                       familyVariable,
                                                       currentFamilyValue,
                                                       valueSelection );
            size_t alq_idx = getVariableIndexForValue( table,
                                                       RimVfpDefines::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY,
                                                       primaryVariable,
                                                       currentPrimaryValue,
                                                       familyVariable,
                                                       currentFamilyValue,
                                                       valueSelection );
            size_t flo_idx = getVariableIndexForValue( table,
                                                       RimVfpDefines::ProductionVariableType::FLOW_RATE,
                                                       primaryVariable,
                                                       currentPrimaryValue,
                                                       familyVariable,
                                                       currentFamilyValue,
                                                       valueSelection );
            size_t thp_idx = getVariableIndexForValue( table,
                                                       RimVfpDefines::ProductionVariableType::THP,
                                                       primaryVariable,
                                                       currentPrimaryValue,
                                                       familyVariable,
                                                       currentFamilyValue,
                                                       valueSelection );

            yVals[y] = table( thp_idx, wfr_idx, gfr_idx, alq_idx, flo_idx );
            if ( interpolatedVariable == RimVfpDefines::InterpolatedVariableType::BHP_THP_DIFF )
            {
                yVals[y] -= thpValues[thp_idx];
            }
        }

        double  familyValue = convertToDisplayUnit( currentFamilyValue, familyVariable );
        QString familyUnit  = getDisplayUnit( familyVariable );
        QString familyTitle = QString( "%1: %2 %3" )
                                  .arg( caf::AppEnum<RimVfpDefines::ProductionVariableType>::uiText( familyVariable ) )
                                  .arg( familyValue )
                                  .arg( familyUnit );

        convertToDisplayUnit( yVals, RimVfpDefines::ProductionVariableType::THP );
        convertToDisplayUnit( primaryAxisValues, primaryVariable );

        plotData.appendCurve( familyTitle, primaryAxisValues, yVals );
    }

    return plotData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigVfpTables::asciiDataForTable( int                                     tableNumber,
                                         RimVfpDefines::ProductionVariableType   primaryVariable,
                                         RimVfpDefines::ProductionVariableType   familyVariable,
                                         RimVfpDefines::InterpolatedVariableType interpolatedVariable,
                                         RimVfpDefines::FlowingPhaseType         flowingPhase,
                                         const VfpTableSelection&                tableSelection ) const
{
    VfpPlotData plotData;
    auto        prodTable = productionTable( tableNumber );
    if ( prodTable.has_value() )
    {
        plotData = populatePlotData( *prodTable, primaryVariable, familyVariable, interpolatedVariable, flowingPhase, tableSelection );
    }
    else
    {
        auto injTable = injectionTable( tableNumber );
        if ( injTable.has_value() )
        {
            plotData = populatePlotData( *injTable, interpolatedVariable, flowingPhase );
        }
    }

    return textForPlotData( plotData );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RigVfpTables::findClosestIndices( const std::vector<double>& sourceValues, const std::vector<double>& valuesToMatch )
{
    std::vector<int> result( sourceValues.size(), -1 );

    // Returns the indices of the closest values in valuesToMatch for each value in sourceValues.
    for ( size_t i = 0; i < sourceValues.size(); ++i )
    {
        double minDistance  = std::numeric_limits<double>::max();
        int    closestIndex = -1;

        for ( size_t j = 0; j < valuesToMatch.size(); ++j )
        {
            double distance = std::abs( sourceValues[i] - valuesToMatch[j] );
            if ( distance < minDistance )
            {
                minDistance  = distance;
                closestIndex = static_cast<int>( j );
            }
        }

        if ( closestIndex < static_cast<int>( valuesToMatch.size() ) )
        {
            result[i] = closestIndex;
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RigVfpTables::uniqueClosestIndices( const std::vector<double>& sourceValues, const std::vector<double>& valuesToMatch )
{
    // Find the closest value in valuesForMatch for each value in sourceValues
    std::vector<double> distances( sourceValues.size(), std::numeric_limits<double>::max() );

    auto closestIndices = findClosestIndices( sourceValues, valuesToMatch );

    for ( size_t i = 0; i < sourceValues.size(); i++ )
    {
        if ( closestIndices[i] >= 0 )
        {
            distances[i] = std::abs( sourceValues[i] - valuesToMatch[closestIndices[i]] );
        }
    }

    while ( std::any_of( distances.begin(), distances.end(), []( double val ) { return val != std::numeric_limits<double>::max(); } ) )
    {
        // find the index of the smallest value in minDistance
        auto minDistanceIt = std::min_element( distances.begin(), distances.end() );
        if ( minDistanceIt == distances.end() )
        {
            break;
        }

        auto minDistanceIndex = std::distance( distances.begin(), minDistanceIt );
        auto matchingIndex    = closestIndices[minDistanceIndex];

        if ( matchingIndex > -1 )
        {
            // Remove all references to the matching index
            for ( size_t i = 0; i < sourceValues.size(); i++ )
            {
                if ( i == static_cast<size_t>( minDistanceIndex ) )
                {
                    distances[i] = std::numeric_limits<double>::max();
                }
                else if ( closestIndices[i] == matchingIndex )
                {
                    distances[i]      = std::numeric_limits<double>::max();
                    closestIndices[i] = -1;
                }
            }
        }
    }

    return closestIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigVfpTables::axisTitle( RimVfpDefines::ProductionVariableType variableType, RimVfpDefines::FlowingPhaseType flowingPhase )
{
    QString title;

    if ( flowingPhase == RimVfpDefines::FlowingPhaseType::GAS )
    {
        title = "Gas ";
    }
    else
    {
        title = "Liquid ";
    }
    title += QString( "%1 %2" ).arg( caf::AppEnum<RimVfpDefines::ProductionVariableType>::uiText( variableType ),
                                     getDisplayUnitWithBracket( variableType ) );

    return title;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigVfpTables::getDisplayUnit( RimVfpDefines::ProductionVariableType variableType )
{
    if ( variableType == RimVfpDefines::ProductionVariableType::THP ) return "Bar";

    if ( variableType == RimVfpDefines::ProductionVariableType::FLOW_RATE ) return "Sm3/day";

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigVfpTables::getDisplayUnitWithBracket( RimVfpDefines::ProductionVariableType variableType )
{
    QString unit = getDisplayUnit( variableType );
    if ( !unit.isEmpty() ) return QString( "[%1]" ).arg( unit );

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigVfpTables::convertToDisplayUnit( double value, RimVfpDefines::ProductionVariableType variableType )
{
    if ( variableType == RimVfpDefines::ProductionVariableType::THP )
    {
        return RiaEclipseUnitTools::pascalToBar( value );
    }

    if ( variableType == RimVfpDefines::ProductionVariableType::FLOW_RATE )
    {
        // Convert to m3/sec to m3/day
        return value * static_cast<double>( 24 * 60 * 60 );
    }

    return value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigVfpTables::convertToDisplayUnit( std::vector<double>& values, RimVfpDefines::ProductionVariableType variableType )
{
    for ( double& value : values )
        value = convertToDisplayUnit( value, variableType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigVfpTables::textForPlotData( const VfpPlotData& plotData )
{
    QString dataText;

    if ( plotData.size() > 0 )
    {
        // The curves should have same dimensions
        const size_t curveSize = plotData.curveSize( 0 );

        // Generate the headers for the columns
        // First column is the primary variable
        QString columnTitleLine( plotData.xAxisTitle() );

        // Then one column per "family"
        for ( size_t s = 0; s < plotData.size(); s++ )
        {
            columnTitleLine.append( QString( "\t%1" ).arg( plotData.curveTitle( s ) ) );
        }
        columnTitleLine.append( "\n" );

        dataText.append( columnTitleLine );

        // Add the rows: one row per primary variable value
        for ( size_t idx = 0; idx < curveSize; idx++ )
        {
            QString line;

            // First item on each line is the primary variable
            line.append( QString( "%1" ).arg( plotData.xData( 0 )[idx] ) );

            for ( size_t s = 0; s < plotData.size(); s++ )
            {
                line.append( QString( "\t%1" ).arg( plotData.yData( s )[idx] ) );
            }
            dataText.append( line );
            dataText.append( "\n" );
        }
    }

    return dataText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigVfpTables::getProductionTableData( const Opm::VFPProdTable& table, RimVfpDefines::ProductionVariableType variableType ) const
{
    std::vector<double> xVals;
    if ( variableType == RimVfpDefines::ProductionVariableType::WATER_CUT )
    {
        xVals = table.getWFRAxis();
    }
    else if ( variableType == RimVfpDefines::ProductionVariableType::GAS_LIQUID_RATIO )
    {
        xVals = table.getGFRAxis();
    }
    else if ( variableType == RimVfpDefines::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY )
    {
        xVals = table.getALQAxis();

        // Convert ALQ values to raw values. No conversion of this data type is done in convertToDisplayUnit. As convertToDisplayUnit is a
        // static function, it will require a lot of refactoring to get the unit system information communicated to this function.
        auto alq_dim = Opm::VFPProdTable::ALQDimension( table.getALQType(), m_unitSystem );
        for ( double& value : xVals )
        {
            value = alq_dim.convertSiToRaw( value );
        }
    }
    else if ( variableType == RimVfpDefines::ProductionVariableType::FLOW_RATE )
    {
        // Values are converted in convertToDisplayUnit
        xVals = table.getFloAxis();
    }
    else if ( variableType == RimVfpDefines::ProductionVariableType::THP )
    {
        // Values are converted in convertToDisplayUnit
        xVals = table.getTHPAxis();
    }

    return xVals;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigVfpTables::getProductionTableData( int tableIndex, RimVfpDefines::ProductionVariableType variableType ) const
{
    auto prodTable = productionTable( tableIndex );
    if ( prodTable.has_value() )
    {
        return getProductionTableData( *prodTable, variableType );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigVfpTables::getVariableIndex( const Opm::VFPProdTable&              table,
                                       RimVfpDefines::ProductionVariableType targetVariable,
                                       RimVfpDefines::ProductionVariableType primaryVariable,
                                       size_t                                primaryValue,
                                       RimVfpDefines::ProductionVariableType familyVariable,
                                       size_t                                familyValue,
                                       const VfpTableSelection&              tableSelection ) const
{
    if ( targetVariable == primaryVariable ) return primaryValue;
    if ( targetVariable == familyVariable ) return familyValue;
    if ( targetVariable == RimVfpDefines::ProductionVariableType::WATER_CUT ) return tableSelection.waterCutIdx;
    if ( targetVariable == RimVfpDefines::ProductionVariableType::GAS_LIQUID_RATIO ) return tableSelection.gasLiquidRatioIdx;
    if ( targetVariable == RimVfpDefines::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY )
        return tableSelection.articifialLiftQuantityIdx;
    if ( targetVariable == RimVfpDefines::ProductionVariableType::FLOW_RATE ) return tableSelection.flowRateIdx;
    if ( targetVariable == RimVfpDefines::ProductionVariableType::THP ) return tableSelection.thpIdx;

    return getProductionTableData( table, targetVariable ).size() - 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigVfpTables::getVariableIndexForValue( const Opm::VFPProdTable&              table,
                                               RimVfpDefines::ProductionVariableType targetVariable,
                                               RimVfpDefines::ProductionVariableType primaryVariable,
                                               double                                primaryValue,
                                               RimVfpDefines::ProductionVariableType familyVariable,
                                               double                                familyValue,
                                               const VfpValueSelection&              valueSelection ) const
{
    auto findClosestIndex = []( const std::vector<double>& values, const double value )
    {
        auto it = std::lower_bound( values.begin(), values.end(), value );
        if ( it == values.begin() )
        {
            return (size_t)0;
        }
        if ( it == values.end() )
        {
            return values.size() - 1;
        }
        if ( *it == value )
        {
            return (size_t)std::distance( values.begin(), it );
        }

        auto prev = it - 1;
        if ( std::abs( *prev - value ) < std::abs( *it - value ) )
        {
            return (size_t)std::distance( values.begin(), prev );
        }

        return (size_t)std::distance( values.begin(), it );
    };

    if ( targetVariable == primaryVariable )
    {
        const auto values = getProductionTableData( table, targetVariable );
        return findClosestIndex( values, primaryValue );
    }

    if ( targetVariable == familyVariable )
    {
        const auto values = getProductionTableData( table, targetVariable );
        return findClosestIndex( values, familyValue );
    }

    auto findClosestIndexForVariable =
        [&]( RimVfpDefines::ProductionVariableType targetVariable, const double selectedValue, const Opm::VFPProdTable& table )
    {
        const auto values = getProductionTableData( table, targetVariable );
        return findClosestIndex( values, selectedValue );
    };

    switch ( targetVariable )
    {
        case RimVfpDefines::ProductionVariableType::WATER_CUT:
        {
            return findClosestIndexForVariable( targetVariable, valueSelection.waterCutValue, table );
        }
        case RimVfpDefines::ProductionVariableType::GAS_LIQUID_RATIO:
        {
            return findClosestIndexForVariable( targetVariable, valueSelection.gasLiquidRatioValue, table );
        }
        case RimVfpDefines::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY:
        {
            return findClosestIndexForVariable( targetVariable, valueSelection.artificialLiftQuantityValue, table );
        }
        case RimVfpDefines::ProductionVariableType::FLOW_RATE:
        {
            return findClosestIndexForVariable( targetVariable, valueSelection.flowRateValue, table );
        }
        case RimVfpDefines::ProductionVariableType::THP:
        {
            return findClosestIndexForVariable( targetVariable, valueSelection.thpValue, table );
        }
        default:
            break;
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<Opm::VFPInjTable> RigVfpTables::injectionTable( int tableNumber ) const
{
    for ( const auto& table : m_injectionTables )
    {
        if ( table.getTableNum() == tableNumber )
        {
            return table;
        }
    }

    return std::nullopt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<Opm::VFPProdTable> RigVfpTables::productionTable( int tableNumber ) const
{
    for ( const auto& table : m_productionTables )
    {
        if ( table.getTableNum() == tableNumber )
        {
            return table;
        }
    }

    return std::nullopt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpDefines::FlowingPhaseType RigVfpTables::getFlowingPhaseType( const Opm::VFPProdTable& table )
{
    switch ( table.getFloType() )
    {
        case Opm::VFPProdTable::FLO_TYPE::FLO_OIL:
            return RimVfpDefines::FlowingPhaseType::OIL;
        case Opm::VFPProdTable::FLO_TYPE::FLO_GAS:
            return RimVfpDefines::FlowingPhaseType::GAS;
        case Opm::VFPProdTable::FLO_TYPE::FLO_LIQ:
            return RimVfpDefines::FlowingPhaseType::LIQUID;
        default:
            return RimVfpDefines::FlowingPhaseType::INVALID;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpDefines::FlowingPhaseType RigVfpTables::getFlowingPhaseType( const Opm::VFPInjTable& table )
{
    switch ( table.getFloType() )
    {
        case Opm::VFPInjTable::FLO_TYPE::FLO_OIL:
            return RimVfpDefines::FlowingPhaseType::OIL;
        case Opm::VFPInjTable::FLO_TYPE::FLO_GAS:
            return RimVfpDefines::FlowingPhaseType::GAS;
        case Opm::VFPInjTable::FLO_TYPE::FLO_WAT:
            return RimVfpDefines::FlowingPhaseType::WATER;
        default:
            return RimVfpDefines::FlowingPhaseType::INVALID;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpDefines::FlowingWaterFractionType RigVfpTables::getFlowingWaterFractionType( const Opm::VFPProdTable& table )
{
    switch ( table.getWFRType() )
    {
        case Opm::VFPProdTable::WFR_TYPE::WFR_WOR:
            return RimVfpDefines::FlowingWaterFractionType::WOR;
        case Opm::VFPProdTable::WFR_TYPE::WFR_WCT:
            return RimVfpDefines::FlowingWaterFractionType::WCT;
        case Opm::VFPProdTable::WFR_TYPE::WFR_WGR:
            return RimVfpDefines::FlowingWaterFractionType::WGR;
        default:
            return RimVfpDefines::FlowingWaterFractionType::INVALID;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpDefines::FlowingGasFractionType RigVfpTables::getFlowingGasFractionType( const Opm::VFPProdTable& table )
{
    switch ( table.getGFRType() )
    {
        case Opm::VFPProdTable::GFR_TYPE::GFR_GOR:
            return RimVfpDefines::FlowingGasFractionType::GOR;
        case Opm::VFPProdTable::GFR_TYPE::GFR_GLR:
            return RimVfpDefines::FlowingGasFractionType::GLR;
        case Opm::VFPProdTable::GFR_TYPE::GFR_OGR:
            return RimVfpDefines::FlowingGasFractionType::OGR;
        default:
            return RimVfpDefines::FlowingGasFractionType::INVALID;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigVfpTables::setUnitSystem( const Opm::UnitSystem& unitSystem )
{
    m_unitSystem = unitSystem;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigVfpTables::addInjectionTable( const Opm::VFPInjTable& table )
{
    m_injectionTables.push_back( table );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigVfpTables::addProductionTable( const Opm::VFPProdTable& table )
{
    m_productionTables.push_back( table );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RigVfpTables::injectionTableNumbers() const
{
    std::vector<int> tableNumbers;

    for ( const auto& table : m_injectionTables )
    {
        tableNumbers.push_back( table.getTableNum() );
    }

    return tableNumbers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RigVfpTables::productionTableNumbers() const
{
    std::vector<int> tableNumbers;

    for ( const auto& table : m_productionTables )
    {
        tableNumbers.push_back( table.getTableNum() );
    }

    return tableNumbers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VfpTableInitialData RigVfpTables::getTableInitialData( int tableIndex ) const
{
    auto prodTable = productionTable( tableIndex );
    if ( prodTable.has_value() )
    {
        VfpTableInitialData initialData{};
        initialData.isProductionTable = true;
        initialData.tableNumber       = prodTable->getTableNum();
        initialData.datumDepth        = prodTable->getDatumDepth();
        initialData.flowingPhase      = getFlowingPhaseType( *prodTable );
        initialData.waterFraction     = getFlowingWaterFractionType( *prodTable );
        initialData.gasFraction       = getFlowingGasFractionType( *prodTable );

        return initialData;
    }

    auto injTable = injectionTable( tableIndex );
    if ( injTable.has_value() )
    {
        VfpTableInitialData initialData{};
        initialData.isProductionTable = false;
        initialData.tableNumber       = injTable->getTableNum();
        initialData.datumDepth        = injTable->getDatumDepth();
        initialData.flowingPhase      = getFlowingPhaseType( *injTable );
        return initialData;
    }

    return {};
}
