/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RimEnsembleCurveSetColorManager.h"

#include "RiaColorTables.h"

#include "RimCustomObjectiveFunction.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimObjectiveFunction.h"
#include "RimRegularLegendConfig.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"

#include "cvfScalarMapper.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
namespace caf
{
template <>
void AppEnum<RimEnsembleCurveSetColorManager::ColorMode>::setUp()
{
    addItem( RimEnsembleCurveSetColorManager::ColorMode::SINGLE_COLOR, "SINGLE_COLOR", "Single Color" );
    addItem( RimEnsembleCurveSetColorManager::ColorMode::SINGLE_COLOR_WITH_ALPHA, "SINGLE_COLOR_WITH_ALPHA", "Single Color with Transparency" );
    addItem( RimEnsembleCurveSetColorManager::ColorMode::BY_ENSEMBLE_PARAM, "BY_ENSEMBLE_PARAM", "By Ensemble Parameter" );
    addItem( RimEnsembleCurveSetColorManager::ColorMode::BY_OBJECTIVE_FUNCTION, "BY_OBJECTIVE_FUNCTION", "By Objective Function" );
    addItem( RimEnsembleCurveSetColorManager::ColorMode::BY_CUSTOM_OBJECTIVE_FUNCTION,
             "BY_CUSTOM_OBJECTIVE_FUNCTION",
             "By Custom Objective Function" );
    setDefault( RimEnsembleCurveSetColorManager::ColorMode::SINGLE_COLOR );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::map<RimRegularLegendConfig::ColorRangesType, cvf::Color3ubArray> RimEnsembleCurveSetColorManager::m_ensembleColorRanges(
    { { RimRegularLegendConfig::ColorRangesType::GREEN_RED,
        cvf::Color3ubArray( { cvf::Color3ub( 0x00, 0xff, 0x00 ), cvf::Color3ub( 0xff, 0x00, 0x00 ) } ) },
      { RimRegularLegendConfig::ColorRangesType::BLUE_MAGENTA,
        cvf::Color3ubArray( { cvf::Color3ub( 0x00, 0x00, 0xff ), cvf::Color3ub( 0xff, 0x00, 0xff ) } ) },
      { RimRegularLegendConfig::ColorRangesType::RED_LIGHT_DARK,
        cvf::Color3ubArray( { cvf::Color3ub( 0xff, 0xcc, 0xcc ), cvf::Color3ub( 0x99, 0x00, 0x00 ) } ) },
      { RimRegularLegendConfig::ColorRangesType::GREEN_LIGHT_DARK,
        cvf::Color3ubArray( { cvf::Color3ub( 0xcc, 0xff, 0xcc ), cvf::Color3ub( 0x00, 0x99, 0x00 ) } ) },
      { RimRegularLegendConfig::ColorRangesType::BLUE_LIGHT_DARK,
        cvf::Color3ubArray( { cvf::Color3ub( 0xcc, 0xcc, 0xff ), cvf::Color3ub( 0x00, 0x00, 0x99 ) } ) } } );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::map<RimRegularLegendConfig::ColorRangesType, cvf::Color3ubArray>& RimEnsembleCurveSetColorManager::EnsembleColorRanges()
{
    return m_ensembleColorRanges;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimRegularLegendConfig::ColorRangesType RimEnsembleCurveSetColorManager::DEFAULT_ENSEMBLE_COLOR_RANGE =
    RimRegularLegendConfig::ColorRangesType::GREEN_RED;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig::ColorRangesType RimEnsembleCurveSetColorManager::cycledEnsembleColorRange( int index )
{
    size_t modIndex = index % m_ensembleColorRanges.size();

    auto crIt = m_ensembleColorRanges.begin();
    for ( int i = 0; i < static_cast<int>( modIndex ); ++i )
        ++crIt;

    return crIt->first;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSetColorManager::initializeLegendConfig( RimRegularLegendConfig* legendConfig, const RigEnsembleParameter& ensembleParam )
{
    if ( ensembleParam.isText() )
    {
        std::set<QString> categories;

        for ( auto value : ensembleParam.values )
        {
            categories.insert( value.toString() );
        }

        std::vector<QString> categoryNames = std::vector<QString>( categories.begin(), categories.end() );
        legendConfig->setNamedCategories( categoryNames );
        legendConfig->setAutomaticRanges( 0, categoryNames.size() - 1, 0, categoryNames.size() - 1 );
    }
    else
    {
        double minValue = std::numeric_limits<double>::infinity();
        double maxValue = -std::numeric_limits<double>::infinity();

        for ( auto value : ensembleParam.values )
        {
            double nValue = value.toDouble();
            if ( nValue != std::numeric_limits<double>::infinity() )
            {
                if ( nValue < minValue ) minValue = nValue;
                if ( nValue > maxValue ) maxValue = nValue;
            }
        }

        legendConfig->setAutomaticRanges( minValue, maxValue, minValue, maxValue );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSetColorManager::initializeLegendConfig( RimRegularLegendConfig*                      legendConfig,
                                                              RimObjectiveFunction*                        objectiveFunction,
                                                              const std::vector<RimSummaryCase*>&          summaryCases,
                                                              const std::vector<RifEclipseSummaryAddress>& vectorSummaryAddresses,
                                                              const ObjectiveFunctionTimeConfig&           timeConfig )
{
    auto [minValue, maxValue] = objectiveFunction->minMaxValues( summaryCases, vectorSummaryAddresses, timeConfig );

    legendConfig->setAutomaticRanges( minValue, maxValue, minValue, maxValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleCurveSetColorManager::initializeLegendConfig( RimRegularLegendConfig*                     legendConfig,
                                                              caf::PdmPointer<RimCustomObjectiveFunction> customObjectiveFunction )
{
    double minValue = std::numeric_limits<double>::infinity();
    double maxValue = -std::numeric_limits<double>::infinity();

    for ( auto value : customObjectiveFunction->functionValueForAllCases() )
    {
        if ( value != std::numeric_limits<double>::infinity() )
        {
            if ( value < minValue ) minValue = value;
            if ( value > maxValue ) maxValue = value;
        }
    }

    legendConfig->setAutomaticRanges( minValue, maxValue, minValue, maxValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimEnsembleCurveSetColorManager::caseColor( const RimRegularLegendConfig* legendConfig,
                                                         const RimSummaryCase*         summaryCase,
                                                         const RigEnsembleParameter&   ensembleParam )
{
    if ( !summaryCase || !legendConfig )
    {
        return RiaColorTables::undefinedCellColor();
    }

    if ( ensembleParam.isText() )
    {
        QString tValue = summaryCase->hasCaseRealizationParameters()
                             ? summaryCase->caseRealizationParameters()->parameterValue( ensembleParam.name ).textValue()
                             : "";
        double  nValue = legendConfig->categoryValueFromCategoryName( tValue );
        if ( nValue != std::numeric_limits<double>::infinity() )
        {
            int iValue = static_cast<int>( nValue );
            return cvf::Color3f( legendConfig->scalarMapper()->mapToColor( iValue ) );
        }
    }
    else
    {
        double value = summaryCase->hasCaseRealizationParameters()
                           ? summaryCase->caseRealizationParameters()->parameterValue( ensembleParam.name ).numericValue()
                           : std::numeric_limits<double>::infinity();
        if ( value != std::numeric_limits<double>::infinity() )
        {
            return cvf::Color3f( legendConfig->scalarMapper()->mapToColor( value ) );
        }
    }
    return RiaColorTables::undefinedCellColor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimEnsembleCurveSetColorManager::caseColor( const RimRegularLegendConfig*         legendConfig,
                                                         RimSummaryCase*                       summaryCase,
                                                         RimObjectiveFunction*                 objectiveFunction,
                                                         std::vector<RifEclipseSummaryAddress> vectorSummaryAddresses,
                                                         const ObjectiveFunctionTimeConfig&    timeConfig )
{
    double value = objectiveFunction->value( summaryCase, vectorSummaryAddresses, timeConfig );
    if ( value != std::numeric_limits<double>::infinity() )
    {
        return cvf::Color3f( legendConfig->scalarMapper()->mapToColor( value ) );
    }
    return RiaColorTables::undefinedCellColor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimEnsembleCurveSetColorManager::caseColor( const RimRegularLegendConfig*               legendConfig,
                                                         RimSummaryCase*                             summaryCase,
                                                         caf::PdmPointer<RimCustomObjectiveFunction> customObjectiveFunction )
{
    double value = customObjectiveFunction->value( summaryCase );
    if ( value != std::numeric_limits<double>::infinity() )
    {
        return cvf::Color3f( legendConfig->scalarMapper()->mapToColor( value ) );
    }
    return RiaColorTables::undefinedCellColor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleCurveSetColorManager::hasSameColorForAllRealizationCurves( ColorMode colorMode )
{
    return ( colorMode == ColorMode::SINGLE_COLOR || colorMode == ColorMode::SINGLE_COLOR_WITH_ALPHA );
}
