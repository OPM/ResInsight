/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RigCaseCellResultsData.h"

#include "RiaApplication.h"
#include "RiaDefines.h"
#include "RiaEclipseUnitTools.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaResultNames.h"

#include "RifReaderEclipseOutput.h"

#include "RigAllanDiagramData.h"
#include "RigAllanUtil.h"
#include "RigCaseCellResultCalculator.h"
#include "RigCellVolumeResultCalculator.h"
#include "RigCellsWithNncsCalculator.h"
#include "RigEclipseAllanFaultsStatCalc.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseMultiPropertyStatCalc.h"
#include "RigEclipseNativeStatCalc.h"
#include "RigEclipseResultInfo.h"
#include "RigFaultDistanceResultCalculator.h"
#include "RigFormationNames.h"
#include "RigIndexIjkResultCalculator.h"
#include "RigMainGrid.h"
#include "RigMobilePoreVolumeResultCalculator.h"
#include "RigOilVolumeResultCalculator.h"
#include "RigSoilResultCalculator.h"
#include "RigStatisticsDataCache.h"
#include "RigStatisticsMath.h"

#include "RimCompletionCellIntersectionCalc.h"
#include "RimEclipseCase.h"
#include "RimGridCalculation.h"
#include "RimGridCalculationCollection.h"
#include "RimProject.h"

#include "cafAssert.h"
#include "cafProgressInfo.h"
#include "cvfGeometryTools.h"

#include <QDateTime>

#include <algorithm>
#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCaseCellResultsData::RigCaseCellResultsData( RigEclipseCaseData* ownerCaseData, RiaDefines::PorosityModelType porosityModel )
    : m_activeCellInfo( nullptr )
    , m_porosityModel( porosityModel )
{
    CVF_ASSERT( ownerCaseData != nullptr );
    CVF_ASSERT( ownerCaseData->mainGrid() != nullptr );

    m_ownerCaseData = ownerCaseData;
    m_ownerMainGrid = ownerCaseData->mainGrid();

    m_allanDiagramData = new RigAllanDiagramData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::setMainGrid( RigMainGrid* ownerGrid )
{
    m_ownerMainGrid = ownerGrid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::setActiveCellInfo( RigActiveCellInfo* activeCellInfo )
{
    m_activeCellInfo = activeCellInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::minMaxCellScalarValues( const RigEclipseResultAddress& resVarAddr, double& min, double& max )
{
    statistics( resVarAddr )->minMaxCellScalarValues( min, max );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::minMaxCellScalarValues( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex, double& min, double& max )
{
    statistics( resVarAddr )->minMaxCellScalarValues( timeStepIndex, min, max );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::posNegClosestToZero( const RigEclipseResultAddress& resVarAddr, double& pos, double& neg )
{
    statistics( resVarAddr )->posNegClosestToZero( pos, neg );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::posNegClosestToZero( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex, double& pos, double& neg )
{
    statistics( resVarAddr )->posNegClosestToZero( timeStepIndex, pos, neg );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RigCaseCellResultsData::cellScalarValuesHistogram( const RigEclipseResultAddress& resVarAddr )
{
    return statistics( resVarAddr )->cellScalarValuesHistogram();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RigCaseCellResultsData::cellScalarValuesHistogram( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex )
{
    return statistics( resVarAddr )->cellScalarValuesHistogram( timeStepIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::p10p90CellScalarValues( const RigEclipseResultAddress& resVarAddr, double& p10, double& p90 )
{
    statistics( resVarAddr )->p10p90CellScalarValues( p10, p90 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::p10p90CellScalarValues( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex, double& p10, double& p90 )
{
    statistics( resVarAddr )->p10p90CellScalarValues( timeStepIndex, p10, p90 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::meanCellScalarValues( const RigEclipseResultAddress& resVarAddr, double& meanValue )
{
    statistics( resVarAddr )->meanCellScalarValues( meanValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::meanCellScalarValues( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex, double& meanValue )
{
    statistics( resVarAddr )->meanCellScalarValues( timeStepIndex, meanValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<int>& RigCaseCellResultsData::uniqueCellScalarValues( const RigEclipseResultAddress& resVarAddr )
{
    return statistics( resVarAddr )->uniqueCellScalarValues();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::sumCellScalarValues( const RigEclipseResultAddress& resVarAddr, double& sumValue )
{
    statistics( resVarAddr )->sumCellScalarValues( sumValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::sumCellScalarValues( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex, double& sumValue )
{
    statistics( resVarAddr )->sumCellScalarValues( timeStepIndex, sumValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::mobileVolumeWeightedMean( const RigEclipseResultAddress& resVarAddr, double& meanValue )
{
    statistics( resVarAddr )->mobileVolumeWeightedMean( meanValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::mobileVolumeWeightedMean( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex, double& meanValue )
{
    statistics( resVarAddr )->mobileVolumeWeightedMean( timeStepIndex, meanValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, size_t> RigCaseCellResultsData::resultValueCount() const
{
    std::map<std::string, size_t> memoryUse;

    for ( size_t i = 0; i < m_cellScalarResults.size(); i++ )
    {
        if ( allocatedValueCount( i ) > 0 )
        {
            memoryUse[m_resultInfos[i].resultName().toStdString()] = allocatedValueCount( i );
        }
    }

    return memoryUse;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::resultCount() const
{
    return m_cellScalarResults.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::timeStepCount( const RigEclipseResultAddress& resVarAddr ) const
{
    size_t scalarResultIndex = findScalarResultIndexFromAddress( resVarAddr );
    CVF_TIGHT_ASSERT( scalarResultIndex < resultCount() );

    return m_cellScalarResults[scalarResultIndex].size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::vector<double>>& RigCaseCellResultsData::cellScalarResults( const RigEclipseResultAddress& resVarAddr ) const
{
    size_t scalarResultIndex = findScalarResultIndexFromAddress( resVarAddr );

    CVF_TIGHT_ASSERT( scalarResultIndex < resultCount() );

    return m_cellScalarResults[scalarResultIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>>* RigCaseCellResultsData::modifiableCellScalarResultTimesteps( const RigEclipseResultAddress& resVarAddr )
{
    size_t scalarResultIndex = findScalarResultIndexFromAddress( resVarAddr );

    CVF_TIGHT_ASSERT( scalarResultIndex < resultCount() );

    return &( m_cellScalarResults[scalarResultIndex] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>* RigCaseCellResultsData::modifiableCellScalarResult( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex )
{
    size_t scalarResultIndex = findScalarResultIndexFromAddress( resVarAddr );

    CVF_TIGHT_ASSERT( scalarResultIndex < resultCount() );
    CVF_TIGHT_ASSERT( timeStepIndex < m_cellScalarResults[scalarResultIndex].size() );

    return &( m_cellScalarResults[scalarResultIndex][timeStepIndex] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigCaseCellResultsData::cellScalarResults( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex ) const
{
    size_t scalarResultIndex = findScalarResultIndexFromAddress( resVarAddr );
    CVF_TIGHT_ASSERT( scalarResultIndex < resultCount() );
    CVF_TIGHT_ASSERT( timeStepIndex < m_cellScalarResults[scalarResultIndex].size() );

    return m_cellScalarResults[scalarResultIndex][timeStepIndex];
}

//--------------------------------------------------------------------------------------------------
/// Adds an empty scalar set, and returns the scalarResultIndex to it.
/// if resultName already exists, it just returns the scalarResultIndex to the existing result.
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::findOrCreateScalarResultIndex( const RigEclipseResultAddress& resVarAddr, bool needsToBeStored )
{
    size_t scalarResultIndex = findScalarResultIndexFromAddress( resVarAddr );

    // If the result exists, do nothing

    if ( scalarResultIndex != cvf::UNDEFINED_SIZE_T )
    {
        return scalarResultIndex;
    }

    // Create the new empty result with metadata

    scalarResultIndex = resultCount();
    m_cellScalarResults.push_back( std::vector<std::vector<double>>() );

    RigEclipseResultInfo resInfo( resVarAddr, needsToBeStored, false, scalarResultIndex );

    m_resultInfos.push_back( resInfo );
    m_addressToResultIndexMap[resVarAddr] = scalarResultIndex;

    // Create statistics calculator and add statistics cache object
    // Todo: Move to a "factory" method

    QString resultName = resVarAddr.resultName();

    cvf::ref<RigStatisticsCalculator> statisticsCalculator;

    if ( resultName == RiaResultNames::combinedTransmissibilityResultName() )
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();

        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANX" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANY" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANZ" ) );

        statisticsCalculator = calc;
    }
    else if ( resultName == RiaResultNames::combinedMultResultName() )
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();

        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "MULTX" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "MULTX-" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "MULTY" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "MULTY-" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "MULTZ" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "MULTZ-" ) );

        statisticsCalculator = calc;
    }
    else if ( resultName == RiaResultNames::combinedRiTranResultName() )
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                      RiaResultNames::riTranXResultName() ) );
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                      RiaResultNames::riTranYResultName() ) );
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                      RiaResultNames::riTranZResultName() ) );
        statisticsCalculator = calc;
    }
    else if ( resultName == RiaResultNames::combinedRiMultResultName() )
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                      RiaResultNames::riMultXResultName() ) );
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                      RiaResultNames::riMultYResultName() ) );
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                      RiaResultNames::riMultZResultName() ) );
        statisticsCalculator = calc;
    }
    else if ( resultName == RiaResultNames::combinedRiAreaNormTranResultName() )
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                      RiaResultNames::riAreaNormTranXResultName() ) );
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                      RiaResultNames::riAreaNormTranYResultName() ) );
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                      RiaResultNames::riAreaNormTranZResultName() ) );
        statisticsCalculator = calc;
    }
    else if ( resultName == RiaResultNames::combinedWaterFluxResultName() )
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRWATI+" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRWATJ+" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRWATK+" ) );
        statisticsCalculator = calc;
    }
    else if ( resultName == RiaResultNames::combinedOilFluxResultName() )
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLROILI+" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLROILJ+" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLROILK+" ) );
        statisticsCalculator = calc;
    }
    else if ( resultName == RiaResultNames::combinedGasFluxResultName() )
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRGASI+" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRGASJ+" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRGASK+" ) );
        statisticsCalculator = calc;
    }
    else if ( resultName.endsWith( "IJK" ) )
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc     = new RigEclipseMultiPropertyStatCalc();
        QString                                   baseName = resultName.left( resultName.size() - 3 );
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::ResultCatType::GENERATED, QString( "%1I" ).arg( baseName ) ) );
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::ResultCatType::GENERATED, QString( "%1J" ).arg( baseName ) ) );
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::ResultCatType::GENERATED, QString( "%1K" ).arg( baseName ) ) );
        statisticsCalculator = calc;
    }
    else if ( resultName == RiaResultNames::formationAllanResultName() || resultName == RiaResultNames::formationBinaryAllanResultName() )
    {
        cvf::ref<RigEclipseAllanFaultsStatCalc> calc = new RigEclipseAllanFaultsStatCalc( m_ownerMainGrid->nncData(), resVarAddr );
        statisticsCalculator                         = calc;
    }
    else
    {
        statisticsCalculator = new RigEclipseNativeStatCalc( this, resVarAddr );
    }

    cvf::ref<RigStatisticsDataCache> dataCache = new RigStatisticsDataCache( statisticsCalculator.p() );
    m_statisticsDataCache.push_back( dataCache.p() );

    return scalarResultIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RigCaseCellResultsData::resultNames( RiaDefines::ResultCatType resType ) const
{
    QStringList varList;

    std::vector<RigEclipseResultInfo>::const_iterator it;
    for ( it = m_resultInfos.begin(); it != m_resultInfos.end(); ++it )
    {
        auto resultAddress = it->eclipseResultAddress();
        if ( resultAddress.isDeltaTimeStepActive() || resultAddress.isDeltaCaseActive() || resultAddress.isDivideByCellFaceAreaActive() )
            continue;

        if ( it->resultType() == resType )
        {
            varList.push_back( it->resultName() );
        }
    }

    return varList;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigActiveCellInfo* RigCaseCellResultsData::activeCellInfo()
{
    return m_activeCellInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigActiveCellInfo* RigCaseCellResultsData::activeCellInfo() const
{
    return m_activeCellInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::recalculateStatistics( const RigEclipseResultAddress& resVarAddr )
{
    size_t scalarResultIndex = findScalarResultIndexFromAddress( resVarAddr );
    if ( scalarResultIndex < m_cellScalarResults.size() )
    {
        m_statisticsDataCache[scalarResultIndex]->clearAllStatistics();
    }
}

//--------------------------------------------------------------------------------------------------
/// Returns whether the result data in question is addressed by Active Cell Index
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::isUsingGlobalActiveIndex( const RigEclipseResultAddress& resVarAddr ) const
{
    size_t scalarResultIndex = findScalarResultIndexFromAddress( resVarAddr );
    CVF_TIGHT_ASSERT( scalarResultIndex < m_cellScalarResults.size() );

    if ( m_cellScalarResults[scalarResultIndex].empty() ) return true;

    size_t firstTimeStepResultValueCount = m_cellScalarResults[scalarResultIndex][0].size();
    return firstTimeStepResultValueCount != m_ownerMainGrid->globalCellArray().size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::hasFlowDiagUsableFluxes() const
{
    QStringList dynResVarNames = resultNames( RiaDefines::ResultCatType::DYNAMIC_NATIVE );

    bool hasFlowFluxes = true;
    hasFlowFluxes      = dynResVarNames.contains( "FLRWATI+" );
    hasFlowFluxes      = hasFlowFluxes && ( dynResVarNames.contains( "FLROILI+" ) || dynResVarNames.contains( "FLRGASI+" ) );

    return hasFlowFluxes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RigCaseCellResultsData::allTimeStepDatesFromEclipseReader() const
{
    const RifReaderEclipseOutput* rifReaderOutput = dynamic_cast<const RifReaderEclipseOutput*>( m_readerInterface.p() );
    if ( rifReaderOutput )
    {
        return rifReaderOutput->allTimeSteps();
    }
    else
    {
        return std::vector<QDateTime>();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RigCaseCellResultsData::timeStepDates( const RigEclipseResultAddress& resVarAddr ) const
{
    if ( findScalarResultIndexFromAddress( resVarAddr ) < m_resultInfos.size() )
    {
        return m_resultInfos[findScalarResultIndexFromAddress( resVarAddr )].dates();
    }
    else
        return std::vector<QDateTime>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RigCaseCellResultsData::timeStepDates() const
{
    RigEclipseResultAddress scalarResWithMostTimeSteps;
    maxTimeStepCount( &scalarResWithMostTimeSteps );

    return timeStepDates( scalarResWithMostTimeSteps );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigCaseCellResultsData::daysSinceSimulationStart() const
{
    RigEclipseResultAddress scalarResWithMostTimeSteps;
    maxTimeStepCount( &scalarResWithMostTimeSteps );

    return daysSinceSimulationStart( scalarResWithMostTimeSteps );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigCaseCellResultsData::daysSinceSimulationStart( const RigEclipseResultAddress& resVarAddr ) const
{
    if ( findScalarResultIndexFromAddress( resVarAddr ) < m_resultInfos.size() )
    {
        return m_resultInfos[findScalarResultIndexFromAddress( resVarAddr )].daysSinceSimulationStarts();
    }
    else
    {
        return std::vector<double>();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigCaseCellResultsData::reportStepNumber( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex ) const
{
    if ( findScalarResultIndexFromAddress( resVarAddr ) < m_resultInfos.size() &&
         m_resultInfos[findScalarResultIndexFromAddress( resVarAddr )].timeStepInfos().size() > timeStepIndex )
        return m_resultInfos[findScalarResultIndexFromAddress( resVarAddr )].timeStepInfos()[timeStepIndex].m_reportNumber;
    else
        return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigEclipseTimeStepInfo> RigCaseCellResultsData::timeStepInfos( const RigEclipseResultAddress& resVarAddr ) const
{
    if ( findScalarResultIndexFromAddress( resVarAddr ) < m_resultInfos.size() )
        return m_resultInfos[findScalarResultIndexFromAddress( resVarAddr )].timeStepInfos();
    else
        return std::vector<RigEclipseTimeStepInfo>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::setTimeStepInfos( const RigEclipseResultAddress&             resVarAddr,
                                               const std::vector<RigEclipseTimeStepInfo>& timeStepInfos )
{
    CVF_ASSERT( findScalarResultIndexFromAddress( resVarAddr ) < m_resultInfos.size() );

    m_resultInfos[findScalarResultIndexFromAddress( resVarAddr )].setTimeStepInfos( timeStepInfos );

    std::vector<std::vector<double>>* dataValues = modifiableCellScalarResultTimesteps( resVarAddr );
    dataValues->resize( timeStepInfos.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::maxTimeStepCount( RigEclipseResultAddress* resultAddressWithMostTimeSteps ) const
{
    size_t                  maxTsCount = 0;
    RigEclipseResultAddress scalarResultIndexWithMaxTsCount;

    for ( size_t i = 0; i < m_resultInfos.size(); i++ )
    {
        if ( m_resultInfos[i].timeStepInfos().size() > maxTsCount )
        {
            maxTsCount = m_resultInfos[i].timeStepInfos().size();

            if ( resultAddressWithMostTimeSteps )
            {
                *resultAddressWithMostTimeSteps = m_resultInfos[i].eclipseResultAddress();
            }
        }
    }

    return maxTsCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigCaseCellResultsData::makeResultNameUnique( const QString& resultNameProposal ) const
{
    QString newResultName = resultNameProposal;
    int     nameNum       = 1;
    int     stringLength  = newResultName.size();
    while ( true )
    {
        if ( !hasResultEntry( RigEclipseResultAddress( newResultName ) ) ) break;

        newResultName.truncate( stringLength );
        newResultName += "_" + QString::number( nameNum );
        ++nameNum;
    }

    return newResultName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::clearScalarResult( RiaDefines::ResultCatType type, const QString& resultName )
{
    clearScalarResult( RigEclipseResultAddress( type, resultName ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::clearScalarResult( const RigEclipseResultAddress& resultAddress )
{
    size_t scalarResultIndex = findScalarResultIndexFromAddress( resultAddress );

    if ( scalarResultIndex == cvf::UNDEFINED_SIZE_T ) return;

    for ( size_t tsIdx = 0; tsIdx < m_cellScalarResults[scalarResultIndex].size(); ++tsIdx )
    {
        std::vector<double> empty;
        m_cellScalarResults[scalarResultIndex][tsIdx].swap( empty );
    }

    recalculateStatistics( resultAddress );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::clearAllResults()
{
    m_cellScalarResults.clear();
    m_resultInfos.clear();
    m_addressToResultIndexMap.clear();
    m_statisticsDataCache.clear();
}

//--------------------------------------------------------------------------------------------------
/// Removes all the actual numbers put into this object, and frees up the memory.
/// Does not touch the metadata in any way
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::freeAllocatedResultsData( std::vector<RiaDefines::ResultCatType> categoriesToExclude,
                                                       std::optional<size_t>                  timeStepIndexToRelease )
{
    for ( size_t resultIdx = 0; resultIdx < m_cellScalarResults.size(); ++resultIdx )
    {
        if ( resultIdx < m_resultInfos.size() )
        {
            auto resultCategory = m_resultInfos[resultIdx].eclipseResultAddress().resultCatType();
            if ( std::find( categoriesToExclude.begin(), categoriesToExclude.end(), resultCategory ) != categoriesToExclude.end() )
            {
                // Keep generated results for these categories
                continue;
            }
        }

        for ( size_t index = 0; index < m_cellScalarResults[resultIdx].size(); index++ )
        {
            if ( timeStepIndexToRelease && index != *timeStepIndexToRelease )
            {
                // Keep generated results for these time steps
                continue;
            }

            auto& dataForTimeStep = m_cellScalarResults[resultIdx][index];

            if ( !dataForTimeStep.empty() )
            {
                // Using swap with an empty vector as that is the safest way to really get rid of the allocated data in a
                // vector
                std::vector<double> empty;
                dataForTimeStep.swap( empty );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::isResultLoaded( const RigEclipseResultAddress& resultAddr ) const
{
    size_t scalarResultIndex = findScalarResultIndexFromAddress( resultAddr );

    CVF_TIGHT_ASSERT( scalarResultIndex != cvf::UNDEFINED_SIZE_T );
    if ( scalarResultIndex != cvf::UNDEFINED_SIZE_T )
    {
        return isDataPresent( scalarResultIndex );
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// Make sure we have a result with given type and name, and make sure one "timestep" result vector
// for the static result values are allocated
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::addStaticScalarResult( RiaDefines::ResultCatType type,
                                                      const QString&            resultName,
                                                      bool                      needsToBeStored,
                                                      size_t                    resultValueCount )
{
    size_t resultIdx = findOrCreateScalarResultIndex( RigEclipseResultAddress( type, resultName ), needsToBeStored );

    m_cellScalarResults[resultIdx].resize( 1, std::vector<double>() );
    m_cellScalarResults[resultIdx][0].resize( resultValueCount, HUGE_VAL );

    return resultIdx;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseResultAddress RigCaseCellResultsData::defaultResult() const
{
    auto allResults = existingResults();

    if ( maxTimeStepCount() > 0 )
    {
        auto prefs = RiaPreferences::current();
        if ( prefs->loadAndShowSoil ) return RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::soil() );

        auto dynamicResult = std::find_if( allResults.begin(),
                                           allResults.end(),
                                           []( const RigEclipseResultAddress& adr )
                                           { return adr.resultCatType() == RiaDefines::ResultCatType::DYNAMIC_NATIVE; } );

        if ( dynamicResult != allResults.end() ) return *dynamicResult;
    }

    // If any input property exists, use that
    auto inputProperty = std::find_if( allResults.begin(),
                                       allResults.end(),
                                       []( const RigEclipseResultAddress& adr )
                                       { return adr.resultCatType() == RiaDefines::ResultCatType::INPUT_PROPERTY; } );

    if ( inputProperty != allResults.end() ) return *inputProperty;

    // If any static property exists, use that
    auto staticResult = std::find_if( allResults.begin(),
                                      allResults.end(),
                                      []( const RigEclipseResultAddress& adr )
                                      { return adr.resultCatType() == RiaDefines::ResultCatType::STATIC_NATIVE; } );

    if ( staticResult != allResults.end() ) return *staticResult;

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::updateResultName( RiaDefines::ResultCatType resultType, const QString& oldName, const QString& newName )
{
    bool anyNameUpdated = false;

    for ( auto& it : m_resultInfos )
    {
        if ( it.resultType() == resultType && it.resultName() == oldName )
        {
            anyNameUpdated = true;
            it.setResultName( newName );
        }
    }

    return anyNameUpdated;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>* RigCaseCellResultsData::getResultIndexableStaticResult( RigActiveCellInfo*      actCellInfo,
                                                                                   RigCaseCellResultsData* gridCellResults,
                                                                                   QString                 resultName,
                                                                                   std::vector<double>&    activeCellsResultsTempContainer )
{
    size_t                  resultCellCount    = actCellInfo->reservoirActiveCellCount();
    size_t                  reservoirCellCount = actCellInfo->reservoirCellCount();
    RigEclipseResultAddress resVarAddr( RiaDefines::ResultCatType::STATIC_NATIVE, resultName );

    size_t scalarResultIndexPorv = gridCellResults->findOrLoadKnownScalarResult( resVarAddr );

    if ( scalarResultIndexPorv == cvf::UNDEFINED_SIZE_T ) return nullptr;

    const std::vector<double>* porvResults = &( gridCellResults->cellScalarResults( resVarAddr, 0 ) );

    if ( !gridCellResults->isUsingGlobalActiveIndex( resVarAddr ) )
    {
        // PORV is given for all cells

        activeCellsResultsTempContainer.resize( resultCellCount, HUGE_VAL );

        for ( size_t globalCellIndex = 0; globalCellIndex < reservoirCellCount; globalCellIndex++ )
        {
            size_t resultIdx = actCellInfo->cellResultIndex( globalCellIndex );
            if ( resultIdx != cvf::UNDEFINED_SIZE_T )
            {
                activeCellsResultsTempContainer[resultIdx] = porvResults->at( globalCellIndex );
            }
        }
        return &activeCellsResultsTempContainer;
    }
    else
    {
        return porvResults;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RigEclipseResultInfo>& RigCaseCellResultsData::infoForEachResultIndex() const
{
    return m_resultInfos;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::mustBeCalculated( size_t scalarResultIndex ) const
{
    std::vector<RigEclipseResultInfo>::const_iterator it;
    for ( it = m_resultInfos.begin(); it != m_resultInfos.end(); ++it )
    {
        if ( it->gridScalarResultIndex() == scalarResultIndex )
        {
            return it->mustBeCalculated();
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::setMustBeCalculated( size_t scalarResultIndex )
{
    std::vector<RigEclipseResultInfo>::iterator it;
    for ( it = m_resultInfos.begin(); it != m_resultInfos.end(); ++it )
    {
        if ( it->gridScalarResultIndex() == scalarResultIndex )
        {
            it->setMustBeCalculated( true );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::eraseAllSourSimData()
{
    for ( size_t i = 0; i < m_resultInfos.size(); i++ )
    {
        RigEclipseResultInfo& ri = m_resultInfos[i];
        if ( ri.resultType() == RiaDefines::ResultCatType::SOURSIMRL )
        {
            ri.setResultType( RiaDefines::ResultCatType::REMOVED );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::setRemovedTagOnGeneratedResult( const RigEclipseResultAddress& resultAddress )
{
    CAF_ASSERT( resultAddress.resultCatType() == RiaDefines::ResultCatType::GENERATED );

    for ( auto& it : m_resultInfos )
    {
        if ( it.resultType() == RiaDefines::ResultCatType::GENERATED && it.resultName() == resultAddress.resultName() )
        {
            it.setResultType( RiaDefines::ResultCatType::REMOVED );
            return;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::createPlaceholderResultEntries()
{
    bool needsToBeStored = false;
    // SOIL
    {
        if ( !hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::soil() ) ) )
        {
            if ( hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::swat() ) ) ||
                 hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::sgas() ) ) )
            {
                size_t soilIndex = findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                                           RiaResultNames::soil() ),
                                                                  needsToBeStored );
                setMustBeCalculated( soilIndex );
            }
        }
    }

    // Oil Volume
    if ( RiaApplication::enableDevelopmentFeatures() )
    {
        if ( hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::soil() ) ) )
        {
            findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                    RiaResultNames::riOilVolumeResultName() ),
                                           needsToBeStored );
        }
    }

    // Completion type
    {
        findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                RiaResultNames::completionTypeResultName() ),
                                       needsToBeStored );
    }

    // Fault results
    {
        findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::ALLAN_DIAGRAMS,
                                                                RiaResultNames::formationBinaryAllanResultName() ),
                                       needsToBeStored );

        findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::ALLAN_DIAGRAMS,
                                                                RiaResultNames::formationAllanResultName() ),
                                       needsToBeStored );
    }

    // FLUX
    {
        if ( hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRWATI+" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRWATJ+" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRWATK+" ) ) )
        {
            findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                    RiaResultNames::combinedWaterFluxResultName() ),
                                           needsToBeStored );
        }

        if ( hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLROILI+" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLROILJ+" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLROILK+" ) ) )
        {
            findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                    RiaResultNames::combinedOilFluxResultName() ),
                                           needsToBeStored );
        }

        if ( hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRGASI+" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRGASJ+" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRGASK+" ) ) )
        {
            findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                    RiaResultNames::combinedGasFluxResultName() ),
                                           needsToBeStored );
        }
    }

    // TRANSXYZ
    {
        if ( hasCompleteTransmissibilityResults() )
        {
            addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE,
                                   RiaResultNames::combinedTransmissibilityResultName(),
                                   needsToBeStored,
                                   0 );
        }
    }
    // MULTXYZ
    {
        if ( hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "MULTX" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "MULTX-" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "MULTY" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "MULTY-" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "MULTZ" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "MULTZ-" ) ) )
        {
            addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::combinedMultResultName(), needsToBeStored, 0 );
        }
    }

    // riTRANSXYZ and X,Y,Z
    {
        if ( hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PERMX" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PERMY" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PERMZ" ) ) )
        {
            addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::riTranXResultName(), needsToBeStored, 0 );
            addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::riTranYResultName(), needsToBeStored, 0 );
            addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::riTranZResultName(), needsToBeStored, 0 );
            addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::combinedRiTranResultName(), needsToBeStored, 0 );
        }
    }

    // riMULTXYZ and X, Y, Z
    {
        if ( hasCompleteTransmissibilityResults() &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::riTranXResultName() ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::riTranYResultName() ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::riTranZResultName() ) ) )
        {
            addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::riMultXResultName(), needsToBeStored, 0 );
            addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::riMultYResultName(), needsToBeStored, 0 );
            addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::riMultZResultName(), needsToBeStored, 0 );
            addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::combinedRiMultResultName(), needsToBeStored, 0 );
        }
    }

    // riTRANS X,Y,Z byArea
    {
        if ( hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANX" ) ) )
        {
            addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::riAreaNormTranXResultName(), needsToBeStored, 0 );
        }

        if ( hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANY" ) ) )
        {
            addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::riAreaNormTranYResultName(), needsToBeStored, 0 );
        }

        if ( hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANZ" ) ) )
        {
            addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::riAreaNormTranZResultName(), needsToBeStored, 0 );
        }
    }
    // riTRANSXYZbyArea
    {
        if ( hasCompleteTransmissibilityResults() )
        {
            addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE,
                                   RiaResultNames::combinedRiAreaNormTranResultName(),
                                   needsToBeStored,
                                   0 );
        }
    }

    // Cell Volume
    {
        if ( !hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::riCellVolumeResultName() ) ) )
        {
            addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::riCellVolumeResultName(), needsToBeStored, 0 );
        }
    }

    // Mobile Pore Volume
    {
        if ( hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PORV" ) ) )
        {
            if ( !hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::mobilePoreVolumeName() ) ) )
            {
                addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::mobilePoreVolumeName(), needsToBeStored, 0 );
            }
        }
    }

    // I/J/K indexes
    {
        findOrCreateScalarResultIndex( RiaResultNames::staticIntegerAddress( RiaResultNames::indexIResultName() ), needsToBeStored );
        findOrCreateScalarResultIndex( RiaResultNames::staticIntegerAddress( RiaResultNames::indexJResultName() ), needsToBeStored );
        findOrCreateScalarResultIndex( RiaResultNames::staticIntegerAddress( RiaResultNames::indexKResultName() ), needsToBeStored );
    }

    // Fault distance
    {
        findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::faultDistanceName() ),
                                       needsToBeStored );
    }

    // NNC cells, 1 for cells with NNC and 0 for other cells
    {
        findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                RiaDefines::ResultDataType::INTEGER,
                                                                RiaResultNames::riNncCells() ),
                                       needsToBeStored );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::hasCompleteTransmissibilityResults() const
{
    return hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANX" ) ) &&
           hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANY" ) ) &&
           hasResultEntry( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANZ" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigEclipseResultAddress> RigCaseCellResultsData::existingResults() const
{
    std::vector<RigEclipseResultAddress> addresses;
    for ( const auto& ri : m_resultInfos )
    {
        addresses.emplace_back( ri.eclipseResultAddress() );
    }

    return addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigEclipseResultInfo* RigCaseCellResultsData::resultInfo( const RigEclipseResultAddress& resVarAddr ) const
{
    size_t index = findScalarResultIndexFromAddress( resVarAddr );
    if ( index < m_resultInfos.size() )
    {
        return &( m_resultInfos[index] );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::ensureKnownResultLoaded( const RigEclipseResultAddress& resultAddress )
{
    size_t resultIndex = findOrLoadKnownScalarResult( resultAddress );

    return ( resultIndex != cvf::UNDEFINED_SIZE_T );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::findAndLoadResultByName( const QString&                                resultName,
                                                      const std::vector<RiaDefines::ResultCatType>& resultCategorySearchOrder )
{
    RigEclipseResultAddress adr( resultName );

    size_t resultIndex = findOrLoadKnownScalarResultByResultTypeOrder( adr, resultCategorySearchOrder );

    return ( resultIndex != cvf::UNDEFINED_SIZE_T );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::hasResultEntry( const RigEclipseResultAddress& resultAddress ) const
{
    size_t resultIndex = findScalarResultIndexFromAddress( resultAddress );

    return ( resultIndex != cvf::UNDEFINED_SIZE_T );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::createResultEntry( const RigEclipseResultAddress& resultAddress, bool needsToBeStored )
{
    findOrCreateScalarResultIndex( resultAddress, needsToBeStored );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::findOrLoadKnownScalarResult( const RigEclipseResultAddress& resVarAddr )
{
    if ( !resVarAddr.isValid() )
    {
        return cvf::UNDEFINED_SIZE_T;
    }
    else if ( resVarAddr.resultCatType() == RiaDefines::ResultCatType::UNDEFINED )
    {
        std::vector<RiaDefines::ResultCatType> searchOrder = { RiaDefines::ResultCatType::STATIC_NATIVE,
                                                               RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                               RiaDefines::ResultCatType::SOURSIMRL,
                                                               RiaDefines::ResultCatType::GENERATED,
                                                               RiaDefines::ResultCatType::INPUT_PROPERTY,
                                                               RiaDefines::ResultCatType::FORMATION_NAMES };

        size_t scalarResultIndex = findOrLoadKnownScalarResultByResultTypeOrder( resVarAddr, searchOrder );

        return scalarResultIndex;
    }

    size_t scalarResultIndex = findScalarResultIndexFromAddress( resVarAddr );

    if ( scalarResultIndex == cvf::UNDEFINED_SIZE_T ) return cvf::UNDEFINED_SIZE_T;

    RiaDefines::ResultCatType type       = resVarAddr.resultCatType();
    QString                   resultName = resVarAddr.resultName();

    if ( resVarAddr.isDeltaCaseActive() || resVarAddr.isDeltaTimeStepActive() )
    {
        if ( !RigCaseCellResultCalculator::computeDifference( m_ownerCaseData, m_porosityModel, resVarAddr ) )
        {
            return cvf::UNDEFINED_SIZE_T;
        }

        return scalarResultIndex;
    }

    if ( resVarAddr.isDivideByCellFaceAreaActive() )
    {
        if ( !isDataPresent( scalarResultIndex ) )
        {
            if ( !RigCaseCellResultCalculator::computeDivideByCellFaceArea( m_ownerMainGrid, m_ownerCaseData, m_porosityModel, resVarAddr ) )

            {
                return cvf::UNDEFINED_SIZE_T;
            }
        }

        return scalarResultIndex;
    }

    // Load dependency data sets

    if ( type == RiaDefines::ResultCatType::STATIC_NATIVE )
    {
        if ( resultName == RiaResultNames::combinedTransmissibilityResultName() )
        {
            findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "TRANX" ) );
            findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "TRANY" ) );
            findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "TRANZ" ) );
        }
        else if ( resultName == RiaResultNames::combinedMultResultName() )
        {
            findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "MULTX" ) );
            findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "MULTX-" ) );
            findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "MULTY" ) );
            findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "MULTY-" ) );
            findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "MULTZ" ) );
            findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "MULTZ-" ) );
        }
        else if ( resultName == RiaResultNames::combinedRiTranResultName() )
        {
            computeRiTransComponent( RiaResultNames::riTranXResultName() );
            computeRiTransComponent( RiaResultNames::riTranYResultName() );
            computeRiTransComponent( RiaResultNames::riTranZResultName() );
            computeNncCombRiTrans();
        }
        else if ( resultName == RiaResultNames::riTranXResultName() || resultName == RiaResultNames::riTranYResultName() ||
                  resultName == RiaResultNames::riTranZResultName() )
        {
            computeRiTransComponent( resultName );
        }
        else if ( resultName == RiaResultNames::combinedRiMultResultName() )
        {
            computeRiMULTComponent( RiaResultNames::riMultXResultName() );
            computeRiMULTComponent( RiaResultNames::riMultYResultName() );
            computeRiMULTComponent( RiaResultNames::riMultZResultName() );
            computeNncCombRiTrans();
            computeNncCombRiMULT();
        }
        else if ( resultName == RiaResultNames::riMultXResultName() || resultName == RiaResultNames::riMultYResultName() ||
                  resultName == RiaResultNames::riMultZResultName() )
        {
            computeRiMULTComponent( resultName );
        }
        else if ( resultName == RiaResultNames::combinedRiAreaNormTranResultName() )
        {
            computeRiTRANSbyAreaComponent( RiaResultNames::riAreaNormTranXResultName() );
            computeRiTRANSbyAreaComponent( RiaResultNames::riAreaNormTranYResultName() );
            computeRiTRANSbyAreaComponent( RiaResultNames::riAreaNormTranZResultName() );
            computeNncCombRiTRANSbyArea();
        }
        else if ( resultName == RiaResultNames::riAreaNormTranXResultName() || resultName == RiaResultNames::riAreaNormTranYResultName() ||
                  resultName == RiaResultNames::riAreaNormTranZResultName() )
        {
            computeRiTRANSbyAreaComponent( resultName );
        }
        else if ( resultName == RiaResultNames::formationAllanResultName() || resultName == RiaResultNames::formationBinaryAllanResultName() )
        {
            bool includeInactiveCells = false;
            if ( m_readerInterface.notNull() )
            {
                includeInactiveCells = m_readerInterface->includeInactiveCellsInFaultGeometry();
            }
            RigAllanUtil::computeAllanResults( this, m_ownerMainGrid, includeInactiveCells );
        }
        else if ( resultName == RiaResultNames::indexIResultName() || resultName == RiaResultNames::indexJResultName() ||
                  resultName == RiaResultNames::indexKResultName() )
        {
            computeIndexResults();
        }
        else if ( resultName == RiaResultNames::faultDistanceName() )
        {
            computeFaultDistance();
        }
        else if ( resultName == RiaResultNames::riNncCells() )
        {
            computeNncsCells();
        }
    }
    else if ( type == RiaDefines::ResultCatType::DYNAMIC_NATIVE )
    {
        if ( resultName == RiaResultNames::combinedWaterFluxResultName() )
        {
            findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "FLRWATI+" ) );
            findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "FLRWATJ+" ) );
            findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "FLRWATK+" ) );
        }
        else if ( resultName == RiaResultNames::combinedOilFluxResultName() )
        {
            findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "FLROILI+" ) );
            findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "FLROILJ+" ) );
            findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "FLROILK+" ) );
        }
        else if ( resultName == RiaResultNames::combinedGasFluxResultName() )
        {
            findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "FLRGASI+" ) );
            findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "FLRGASJ+" ) );
            findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "FLRGASK+" ) );
        }
    }

    if ( isDataPresent( scalarResultIndex ) )
    {
        return scalarResultIndex;
    }

    if ( resultName == RiaResultNames::soil() )
    {
        if ( mustBeCalculated( scalarResultIndex ) )
        {
            // Trigger loading of SWAT, SGAS to establish time step count if no data has been loaded from file at
            // this point
            findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::swat() ) );
            findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::sgas() ) );

            m_cellScalarResults[scalarResultIndex].resize( maxTimeStepCount() );
            for ( size_t timeStepIdx = 0; timeStepIdx < maxTimeStepCount(); timeStepIdx++ )
            {
                std::vector<double>& values = m_cellScalarResults[scalarResultIndex][timeStepIdx];
                if ( values.empty() )
                {
                    computeSOILForTimeStep( timeStepIdx );
                }
            }

            return scalarResultIndex;
        }
    }
    else if ( resultName == RiaResultNames::completionTypeResultName() )
    {
        caf::ProgressInfo progressInfo( maxTimeStepCount(), "Calculate Completion Type Results" );
        m_cellScalarResults[scalarResultIndex].resize( maxTimeStepCount() );
        for ( size_t timeStepIdx = 0; timeStepIdx < maxTimeStepCount(); ++timeStepIdx )
        {
            computeCompletionTypeForTimeStep( timeStepIdx );
            progressInfo.incrementProgress();
        }
    }
    else if ( resultName == RiaResultNames::mobilePoreVolumeName() )
    {
        computeMobilePV();
    }

    if ( type == RiaDefines::ResultCatType::GENERATED )
    {
        return cvf::UNDEFINED_SIZE_T;
    }

    if ( m_readerInterface.notNull() )
    {
        // Add one more result to result container
        size_t timeStepCount = infoForEachResultIndex()[scalarResultIndex].timeStepInfos().size();

        bool resultLoadingSuccess = true;

        size_t tempGridCellCount = m_ownerMainGrid->totalTemporaryGridCellCount();

        if ( type == RiaDefines::ResultCatType::DYNAMIC_NATIVE && timeStepCount > 0 )
        {
            m_cellScalarResults[scalarResultIndex].resize( timeStepCount );

            size_t i;
            for ( i = 0; i < timeStepCount; i++ )
            {
                std::vector<double>& values = m_cellScalarResults[scalarResultIndex][i];
                if ( !m_readerInterface->dynamicResult( resultName, m_porosityModel, i, &values ) )
                {
                    resultLoadingSuccess = false;
                }
                else if ( tempGridCellCount > 0 )
                {
                    if ( !values.empty() )
                    {
                        values.resize( values.size() + tempGridCellCount, std::numeric_limits<double>::infinity() );

                        assignValuesToTemporaryLgrs( resultName, values );
                    }
                }
            }
        }
        else if ( type == RiaDefines::ResultCatType::STATIC_NATIVE )
        {
            m_cellScalarResults[scalarResultIndex].resize( 1 );

            std::vector<double>& values = m_cellScalarResults[scalarResultIndex][0];
            if ( !m_readerInterface->staticResult( resultName, m_porosityModel, &values ) )
            {
                resultLoadingSuccess = false;
            }
            else if ( tempGridCellCount > 0 )
            {
                if ( !values.empty() )
                {
                    values.resize( values.size() + tempGridCellCount, std::numeric_limits<double>::infinity() );

                    assignValuesToTemporaryLgrs( resultName, values );
                }
            }
        }

        if ( !resultLoadingSuccess )
        {
            // Remove last scalar result because loading of result failed
            m_cellScalarResults[scalarResultIndex].clear();
        }
    }

    if ( resultName == RiaResultNames::riCellVolumeResultName() )
    {
        computeCellVolumes();
    }
    else if ( resultName == RiaResultNames::riOilVolumeResultName() )
    {
        computeCellVolumes();
        computeOilVolumes();
    }

    // Allan results
    if ( resultName == RiaResultNames::formationAllanResultName() || resultName == RiaResultNames::formationBinaryAllanResultName() )
    {
        bool includeInactiveCells = false;
        if ( m_readerInterface.notNull() )
        {
            includeInactiveCells = m_readerInterface->includeInactiveCellsInFaultGeometry();
        }

        RigAllanUtil::computeAllanResults( this, m_ownerMainGrid, includeInactiveCells );
    }

    // Handle SourSimRL reading

    if ( type == RiaDefines::ResultCatType::SOURSIMRL )
    {
        RifReaderEclipseOutput* eclReader = dynamic_cast<RifReaderEclipseOutput*>( m_readerInterface.p() );
        if ( eclReader )
        {
            size_t timeStepCount = infoForEachResultIndex()[scalarResultIndex].timeStepInfos().size();

            m_cellScalarResults[scalarResultIndex].resize( timeStepCount );

            size_t i;
            for ( i = 0; i < timeStepCount; i++ )
            {
                std::vector<double>& values = m_cellScalarResults[scalarResultIndex][i];
                eclReader->sourSimRlResult( resultName, i, &values );
            }
        }
    }

    return scalarResultIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::findOrLoadKnownScalarResultByResultTypeOrder( const RigEclipseResultAddress& resVarAddr,
                                                                             const std::vector<RiaDefines::ResultCatType>& resultCategorySearchOrder )
{
    std::set<RiaDefines::ResultCatType> otherResultTypesToSearch = { RiaDefines::ResultCatType::STATIC_NATIVE,
                                                                     RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                                     RiaDefines::ResultCatType::SOURSIMRL,
                                                                     RiaDefines::ResultCatType::INPUT_PROPERTY,
                                                                     RiaDefines::ResultCatType::GENERATED,
                                                                     RiaDefines::ResultCatType::FORMATION_NAMES };

    for ( const auto& resultType : resultCategorySearchOrder )
    {
        otherResultTypesToSearch.erase( resultType );
    }

    std::vector<RiaDefines::ResultCatType> resultTypesOrdered = resultCategorySearchOrder;

    for ( const auto& resultType : otherResultTypesToSearch )
    {
        resultTypesOrdered.push_back( resultType );
    }

    for ( const auto& resultType : resultTypesOrdered )
    {
        RigEclipseResultAddress resVarAddressWithType = resVarAddr;
        resVarAddressWithType.setResultCatType( resultType );

        size_t scalarResultIndex = findOrLoadKnownScalarResult( resVarAddressWithType );

        if ( scalarResultIndex != cvf::UNDEFINED_SIZE_T )
        {
            return scalarResultIndex;
        }
    }

    return cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// This method is intended to be used for multicase cross statistical calculations, when
/// we need process one timestep at a time, freeing memory as we go.
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::findOrLoadKnownScalarResultForTimeStep( const RigEclipseResultAddress& resVarAddr, size_t timeStepIndex )
{
    RiaDefines::ResultCatType type       = resVarAddr.resultCatType();
    QString                   resultName = resVarAddr.resultName();

    // Special handling for SOIL
    if ( type == RiaDefines::ResultCatType::DYNAMIC_NATIVE && resultName.toUpper() == RiaResultNames::soil() )
    {
        size_t soilScalarResultIndex = findScalarResultIndexFromAddress( resVarAddr );

        if ( mustBeCalculated( soilScalarResultIndex ) )
        {
            m_cellScalarResults[soilScalarResultIndex].resize( maxTimeStepCount() );

            std::vector<double>& values = m_cellScalarResults[soilScalarResultIndex][timeStepIndex];
            if ( values.empty() )
            {
                computeSOILForTimeStep( timeStepIndex );
            }

            return soilScalarResultIndex;
        }
    }
    else if ( type == RiaDefines::ResultCatType::DYNAMIC_NATIVE && resultName == RiaResultNames::completionTypeResultName() )
    {
        size_t completionTypeScalarResultIndex = findScalarResultIndexFromAddress( resVarAddr );
        computeCompletionTypeForTimeStep( timeStepIndex );
        return completionTypeScalarResultIndex;
    }

    size_t scalarResultIndex = findScalarResultIndexFromAddress( resVarAddr );
    if ( scalarResultIndex == cvf::UNDEFINED_SIZE_T ) return cvf::UNDEFINED_SIZE_T;

    if ( type == RiaDefines::ResultCatType::GENERATED )
    {
        return cvf::UNDEFINED_SIZE_T;
    }

    if ( m_readerInterface.notNull() )
    {
        size_t timeStepCount = infoForEachResultIndex()[scalarResultIndex].timeStepInfos().size();

        bool resultLoadingSuccess = true;

        if ( type == RiaDefines::ResultCatType::DYNAMIC_NATIVE && timeStepCount > 0 )
        {
            m_cellScalarResults[scalarResultIndex].resize( timeStepCount );

            std::vector<double>& values = m_cellScalarResults[scalarResultIndex][timeStepIndex];
            if ( values.empty() )
            {
                if ( !m_readerInterface->dynamicResult( resultName, m_porosityModel, timeStepIndex, &values ) )
                {
                    resultLoadingSuccess = false;
                }
            }
        }
        else if ( type == RiaDefines::ResultCatType::STATIC_NATIVE )
        {
            m_cellScalarResults[scalarResultIndex].resize( 1 );

            std::vector<double>& values = m_cellScalarResults[scalarResultIndex][0];
            if ( values.empty() )
            {
                if ( !m_readerInterface->staticResult( resultName, m_porosityModel, &values ) )
                {
                    resultLoadingSuccess = false;
                }
            }
        }

        if ( !resultLoadingSuccess )
        {
            // Error logging
            CVF_ASSERT( false );
        }
    }

    // Handle SourSimRL reading

    if ( type == RiaDefines::ResultCatType::SOURSIMRL )
    {
        RifReaderEclipseOutput* eclReader = dynamic_cast<RifReaderEclipseOutput*>( m_readerInterface.p() );
        if ( eclReader )
        {
            size_t timeStepCount = infoForEachResultIndex()[scalarResultIndex].timeStepInfos().size();

            m_cellScalarResults[scalarResultIndex].resize( timeStepCount );

            std::vector<double>& values = m_cellScalarResults[scalarResultIndex][timeStepIndex];

            if ( values.empty() )
            {
                eclReader->sourSimRlResult( resultName, timeStepIndex, &values );
            }
        }
    }

    return scalarResultIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeSOILForTimeStep( size_t timeStepIndex )
{
    RigEclipseResultAddress SOILAddr( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::soil() );
    RigSoilResultCalculator calculator( *this );
    calculator.calculate( SOILAddr, timeStepIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::testAndComputeSgasForTimeStep( size_t timeStepIndex )
{
    size_t scalarIndexSWAT =
        findOrLoadKnownScalarResultForTimeStep( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::swat() ),
                                                timeStepIndex );
    if ( scalarIndexSWAT == cvf::UNDEFINED_SIZE_T )
    {
        return;
    }

    if ( m_readerInterface.isNull() ) return;

    std::set<RiaDefines::PhaseType> phases = m_readerInterface->availablePhases();
    if ( phases.count( RiaDefines::PhaseType::GAS_PHASE ) == 0 ) return;
    if ( phases.count( RiaDefines::PhaseType::OIL_PHASE ) > 0 ) return;

    // Simulation type is gas and water. No SGAS is present, compute SGAS based on SWAT

    size_t scalarIndexSGAS =
        findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::sgas() ), false );
    if ( m_cellScalarResults[scalarIndexSGAS].size() > timeStepIndex )
    {
        std::vector<double>& values = m_cellScalarResults[scalarIndexSGAS][timeStepIndex];
        if ( !values.empty() ) return;
    }

    size_t swatResultValueCount = 0;
    size_t swatTimeStepCount    = 0;

    {
        std::vector<double>& swatForTimeStep = m_cellScalarResults[scalarIndexSWAT][timeStepIndex];
        if ( !swatForTimeStep.empty() )
        {
            swatResultValueCount = swatForTimeStep.size();
            swatTimeStepCount    = infoForEachResultIndex()[scalarIndexSWAT].timeStepInfos().size();
        }
    }

    m_cellScalarResults[scalarIndexSGAS].resize( swatTimeStepCount );

    if ( !m_cellScalarResults[scalarIndexSGAS][timeStepIndex].empty() )
    {
        return;
    }

    m_cellScalarResults[scalarIndexSGAS][timeStepIndex].resize( swatResultValueCount );

    std::vector<double>* swatForTimeStep = nullptr;

    {
        swatForTimeStep = &( m_cellScalarResults[scalarIndexSWAT][timeStepIndex] );
        if ( swatForTimeStep->empty() )
        {
            swatForTimeStep = nullptr;
        }
    }

    std::vector<double>& sgasForTimeStep = m_cellScalarResults[scalarIndexSGAS][timeStepIndex];

#pragma omp parallel for
    for ( int idx = 0; idx < static_cast<int>( swatResultValueCount ); idx++ )
    {
        double sgasValue = 1.0;

        if ( swatForTimeStep )
        {
            sgasValue -= swatForTimeStep->at( idx );
        }

        sgasForTimeStep[idx] = sgasValue;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeDepthRelatedResults()
{
    size_t actCellCount = activeCellInfo()->reservoirActiveCellCount();
    if ( actCellCount == 0 ) return;

    size_t depthResultIndex  = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "DEPTH" ) );
    size_t dxResultIndex     = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "DX" ) );
    size_t dyResultIndex     = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "DY" ) );
    size_t dzResultIndex     = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "DZ" ) );
    size_t topsResultIndex   = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TOPS" ) );
    size_t bottomResultIndex = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "BOTTOM" ) );

    bool computeDepth  = false;
    bool computeDx     = false;
    bool computeDy     = false;
    bool computeDz     = false;
    bool computeTops   = false;
    bool computeBottom = false;

    if ( depthResultIndex == cvf::UNDEFINED_SIZE_T )
    {
        depthResultIndex = addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, "DEPTH", false, actCellCount );
        computeDepth     = true;
    }

    if ( dxResultIndex == cvf::UNDEFINED_SIZE_T )
    {
        dxResultIndex = addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, "DX", false, actCellCount );
        computeDx     = true;
    }

    if ( dyResultIndex == cvf::UNDEFINED_SIZE_T )
    {
        dyResultIndex = addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, "DY", false, actCellCount );
        computeDy     = true;
    }

    if ( dzResultIndex == cvf::UNDEFINED_SIZE_T )
    {
        dzResultIndex = addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, "DZ", false, actCellCount );
        computeDz     = true;
    }

    if ( topsResultIndex == cvf::UNDEFINED_SIZE_T )
    {
        topsResultIndex = addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, "TOPS", false, actCellCount );
        computeTops     = true;
    }

    if ( bottomResultIndex == cvf::UNDEFINED_SIZE_T )
    {
        bottomResultIndex = addStaticScalarResult( RiaDefines::ResultCatType::STATIC_NATIVE, "BOTTOM", false, actCellCount );
        computeBottom     = true;
    }

    std::vector<std::vector<double>>& depth  = m_cellScalarResults[depthResultIndex];
    std::vector<std::vector<double>>& dx     = m_cellScalarResults[dxResultIndex];
    std::vector<std::vector<double>>& dy     = m_cellScalarResults[dyResultIndex];
    std::vector<std::vector<double>>& dz     = m_cellScalarResults[dzResultIndex];
    std::vector<std::vector<double>>& tops   = m_cellScalarResults[topsResultIndex];
    std::vector<std::vector<double>>& bottom = m_cellScalarResults[bottomResultIndex];

    // Make sure the size is at least active cells
    {
        if ( depth[0].size() < actCellCount )
        {
            depth[0].resize( actCellCount, std::numeric_limits<double>::max() );
            computeDepth = true;
        }

        if ( dx[0].size() < actCellCount )
        {
            dx[0].resize( actCellCount, std::numeric_limits<double>::max() );
            computeDx = true;
        }

        if ( dy[0].size() < actCellCount )
        {
            dy[0].resize( actCellCount, std::numeric_limits<double>::max() );
            computeDy = true;
        }

        if ( dz[0].size() < actCellCount )
        {
            dz[0].resize( actCellCount, std::numeric_limits<double>::max() );
            computeDz = true;
        }

        if ( tops[0].size() < actCellCount )
        {
            tops[0].resize( actCellCount, std::numeric_limits<double>::max() );
            computeTops = true;
        }

        if ( bottom[0].size() < actCellCount )
        {
            bottom[0].resize( actCellCount, std::numeric_limits<double>::max() );
            computeBottom = true;
        }
    }

#pragma omp parallel for
    for ( long cellIdx = 0; cellIdx < static_cast<long>( m_ownerMainGrid->globalCellArray().size() ); cellIdx++ )
    {
        const RigCell& cell = m_ownerMainGrid->globalCellArray()[cellIdx];

        size_t resultIndex = activeCellInfo()->cellResultIndex( cellIdx );
        if ( resultIndex == cvf::UNDEFINED_SIZE_T ) continue;

        bool isTemporaryGrid = cell.hostGrid()->isTempGrid();

        if ( computeDepth || isTemporaryGrid )
        {
            depth[0][resultIndex] = cvf::Math::abs( cell.center().z() );
        }

        if ( computeDx || isTemporaryGrid )
        {
            cvf::Vec3d cellWidth = cell.faceCenter( cvf::StructGridInterface::NEG_I ) - cell.faceCenter( cvf::StructGridInterface::POS_I );
            dx[0][resultIndex]   = cellWidth.length();
        }

        if ( computeDy || isTemporaryGrid )
        {
            cvf::Vec3d cellWidth = cell.faceCenter( cvf::StructGridInterface::NEG_J ) - cell.faceCenter( cvf::StructGridInterface::POS_J );
            dy[0][resultIndex]   = cellWidth.length();
        }

        if ( computeDz || isTemporaryGrid )
        {
            cvf::Vec3d cellWidth = cell.faceCenter( cvf::StructGridInterface::NEG_K ) - cell.faceCenter( cvf::StructGridInterface::POS_K );
            dz[0][resultIndex]   = cellWidth.length();
        }

        if ( computeTops || isTemporaryGrid )
        {
            tops[0][resultIndex] = cvf::Math::abs( cell.faceCenter( cvf::StructGridInterface::NEG_K ).z() );
        }

        if ( computeBottom || isTemporaryGrid )
        {
            bottom[0][resultIndex] = cvf::Math::abs( cell.faceCenter( cvf::StructGridInterface::POS_K ).z() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeIndexResults()
{
    RigEclipseResultAddress     addr( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::indexIResultName() );
    RigIndexIjkResultCalculator calculator( *this );
    calculator.calculate( addr, 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeFaultDistance()
{
    RigEclipseResultAddress          addr( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::faultDistanceName() );
    RigFaultDistanceResultCalculator calculator( *this );
    calculator.calculate( addr, 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeNncsCells()
{
    RigEclipseResultAddress    addr( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::riNncCells() );
    RigCellsWithNncsCalculator calculator( *this );
    calculator.calculate( addr, 0 );
}

namespace RigTransmissibilityCalcTools
{
void calculateConnectionGeometry( const RigCell&                     c1,
                                  const RigCell&                     c2,
                                  const std::vector<cvf::Vec3d>&     nodes,
                                  cvf::StructGridInterface::FaceType faceId,
                                  cvf::Vec3d*                        faceAreaVec )
{
    CVF_TIGHT_ASSERT( faceAreaVec );

    *faceAreaVec = cvf::Vec3d::ZERO;

    std::vector<size_t>     polygon;
    std::vector<cvf::Vec3d> intersections;
    std::array<size_t, 4>   face1;
    std::array<size_t, 4>   face2;
    c1.faceIndices( faceId, &face1 );
    c2.faceIndices( cvf::StructGridInterface::oppositeFace( faceId ), &face2 );

    bool foundOverlap = cvf::GeometryTools::calculateOverlapPolygonOfTwoQuads( &polygon,
                                                                               &intersections,
                                                                               (cvf::EdgeIntersectStorage<size_t>*)nullptr,
                                                                               cvf::wrapArrayConst( &nodes ),
                                                                               face1.data(),
                                                                               face2.data(),
                                                                               1e-6 );

    if ( foundOverlap )
    {
        std::vector<cvf::Vec3d> realPolygon;

        for ( size_t pIdx = 0; pIdx < polygon.size(); ++pIdx )
        {
            if ( polygon[pIdx] < nodes.size() )
                realPolygon.push_back( nodes[polygon[pIdx]] );
            else
                realPolygon.push_back( intersections[polygon[pIdx] - nodes.size()] );
        }

        // Polygon area vector

        *faceAreaVec = cvf::GeometryTools::polygonAreaNormal3D( realPolygon );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double halfCellTransmissibility( double perm, double ntg, const cvf::Vec3d& centerToFace, const cvf::Vec3d& faceAreaVec )
{
    return perm * ntg * ( faceAreaVec * centerToFace ) / ( centerToFace * centerToFace );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double newtran( double cdarchy, double mult, double halfCellTrans, double neighborHalfCellTrans )
{
    if ( cvf::Math::abs( halfCellTrans ) < 1e-15 || cvf::Math::abs( neighborHalfCellTrans ) < 1e-15 )
    {
        return 0.0;
    }

    double result = cdarchy * mult / ( ( 1 / halfCellTrans ) + ( 1 / neighborHalfCellTrans ) );
    CVF_TIGHT_ASSERT( result == result );
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
using ResultIndexFunction = size_t ( * )( const RigActiveCellInfo*, size_t );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

size_t directReservoirCellIndex( const RigActiveCellInfo* activeCellinfo, size_t reservoirCellIndex )
{
    return reservoirCellIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

size_t reservoirActiveCellIndex( const RigActiveCellInfo* activeCellinfo, size_t reservoirCellIndex )
{
    return activeCellinfo->cellResultIndex( reservoirCellIndex );
}
} // namespace RigTransmissibilityCalcTools

using namespace RigTransmissibilityCalcTools;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeRiTransComponent( const QString& riTransComponentResultName )
{
    // Set up which component to compute

    cvf::StructGridInterface::FaceType faceId = cvf::StructGridInterface::NO_FACE;
    QString                            permCompName;

    if ( riTransComponentResultName == RiaResultNames::riTranXResultName() )
    {
        permCompName = "PERMX";
        faceId       = cvf::StructGridInterface::POS_I;
    }
    else if ( riTransComponentResultName == RiaResultNames::riTranYResultName() )
    {
        permCompName = "PERMY";
        faceId       = cvf::StructGridInterface::POS_J;
    }
    else if ( riTransComponentResultName == RiaResultNames::riTranZResultName() )
    {
        permCompName = "PERMZ";
        faceId       = cvf::StructGridInterface::POS_K;
    }
    else
    {
        CVF_ASSERT( false );
    }

    double cdarchy = darchysValue();

    // Get the needed result indices we depend on

    size_t permResultIdx = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, permCompName ) );
    size_t ntgResultIdx  = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "NTG" ) );

    bool hasNTGResults = ntgResultIdx != cvf::UNDEFINED_SIZE_T;

    // Get the result index of the output

    size_t riTransResultIdx =
        findScalarResultIndexFromAddress( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, riTransComponentResultName ) );
    CVF_ASSERT( riTransResultIdx != cvf::UNDEFINED_SIZE_T );

    // Get the result count, to handle that one of them might be globally defined

    size_t permxResultValueCount = m_cellScalarResults[permResultIdx][0].size();
    size_t resultValueCount      = permxResultValueCount;
    if ( hasNTGResults )
    {
        size_t ntgResultValueCount = m_cellScalarResults[ntgResultIdx][0].size();
        resultValueCount           = std::min( permxResultValueCount, ntgResultValueCount );
    }

    // Get all the actual result values

    const std::vector<double>& permResults    = m_cellScalarResults[permResultIdx][0];
    std::vector<double>&       riTransResults = m_cellScalarResults[riTransResultIdx][0];
    std::vector<double>*       ntgResults     = nullptr;
    if ( hasNTGResults )
    {
        ntgResults = &( m_cellScalarResults[ntgResultIdx][0] );
    }

    // Set up output container to correct number of results

    riTransResults.resize( resultValueCount );

    // Prepare how to index the result values:
    ResultIndexFunction riTranIdxFunc = nullptr;
    ResultIndexFunction permIdxFunc   = nullptr;
    ResultIndexFunction ntgIdxFunc    = nullptr;
    {
        bool isPermUsingResIdx = isUsingGlobalActiveIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, permCompName ) );
        bool isTransUsingResIdx =
            isUsingGlobalActiveIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, riTransComponentResultName ) );
        bool isNtgUsingResIdx = false;
        if ( hasNTGResults )
        {
            isNtgUsingResIdx = isUsingGlobalActiveIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "NTG" ) );
        }

        // Set up result index function pointers

        riTranIdxFunc = isTransUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;
        permIdxFunc   = isPermUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;
        if ( hasNTGResults )
        {
            ntgIdxFunc = isNtgUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;
        }
    }

    const RigActiveCellInfo*       activeCellInfo        = this->activeCellInfo();
    const std::vector<cvf::Vec3d>& nodes                 = m_ownerMainGrid->nodes();
    bool                           isFaceNormalsOutwards = m_ownerMainGrid->isFaceNormalsOutwards();

    for ( size_t nativeResvCellIndex = 0; nativeResvCellIndex < m_ownerMainGrid->globalCellArray().size(); nativeResvCellIndex++ )
    {
        // Do nothing if we are only dealing with active cells, and this cell is not active:
        size_t tranResIdx = ( *riTranIdxFunc )( activeCellInfo, nativeResvCellIndex );

        if ( tranResIdx == cvf::UNDEFINED_SIZE_T ) continue;

        const RigCell& nativeCell = m_ownerMainGrid->globalCellArray()[nativeResvCellIndex];
        RigGridBase*   grid       = nativeCell.hostGrid();

        size_t gridLocalNativeCellIndex = nativeCell.gridLocalCellIndex();

        size_t i, j, k, gridLocalNeighborCellIdx;

        grid->ijkFromCellIndex( gridLocalNativeCellIndex, &i, &j, &k );

        if ( grid->cellIJKNeighbor( i, j, k, faceId, &gridLocalNeighborCellIdx ) )
        {
            size_t         neighborResvCellIdx = grid->reservoirCellIndex( gridLocalNeighborCellIdx );
            const RigCell& neighborCell        = m_ownerMainGrid->globalCellArray()[neighborResvCellIdx];

            // Do nothing if neighbor cell has no results
            size_t neighborCellPermResIdx = ( *permIdxFunc )( activeCellInfo, neighborResvCellIdx );
            if ( neighborCellPermResIdx == cvf::UNDEFINED_SIZE_T ) continue;

            // Connection geometry

            const RigFault* fault     = grid->mainGrid()->findFaultFromCellIndexAndCellFace( nativeResvCellIndex, faceId );
            bool            isOnFault = fault;

            cvf::Vec3d faceAreaVec;
            cvf::Vec3d faceCenter;

            if ( isOnFault )
            {
                calculateConnectionGeometry( nativeCell, neighborCell, nodes, faceId, &faceAreaVec );
            }
            else
            {
                faceAreaVec = nativeCell.faceNormalWithAreaLength( faceId );
            }

            if ( !isFaceNormalsOutwards ) faceAreaVec = -faceAreaVec;

            double halfCellTrans         = 0;
            double neighborHalfCellTrans = 0;

            // Native cell half cell transm
            {
                cvf::Vec3d centerToFace = nativeCell.faceCenter( faceId ) - nativeCell.center();

                size_t permResIdx = ( *permIdxFunc )( activeCellInfo, nativeResvCellIndex );
                double perm       = permResults[permResIdx];

                double ntg = 1.0;
                if ( hasNTGResults && faceId != cvf::StructGridInterface::POS_K )
                {
                    size_t ntgResIdx = ( *ntgIdxFunc )( activeCellInfo, nativeResvCellIndex );
                    ntg              = ( *ntgResults )[ntgResIdx];
                }

                halfCellTrans = halfCellTransmissibility( perm, ntg, centerToFace, faceAreaVec );
            }

            // Neighbor cell half cell transm
            {
                cvf::Vec3d centerToFace = neighborCell.faceCenter( cvf::StructGridInterface::oppositeFace( faceId ) ) - neighborCell.center();

                double perm = permResults[neighborCellPermResIdx];

                double ntg = 1.0;
                if ( hasNTGResults && faceId != cvf::StructGridInterface::POS_K )
                {
                    size_t ntgResIdx = ( *ntgIdxFunc )( activeCellInfo, neighborResvCellIdx );
                    ntg              = ( *ntgResults )[ntgResIdx];
                }

                neighborHalfCellTrans = halfCellTransmissibility( perm, ntg, centerToFace, -faceAreaVec );
            }

            riTransResults[tranResIdx] = newtran( cdarchy, 1.0, halfCellTrans, neighborHalfCellTrans );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeNncCombRiTrans()
{
    RigEclipseResultAddress riCombTransEclResAddr( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::combinedRiTranResultName() );
    if ( m_ownerMainGrid->nncData()->staticConnectionScalarResult( riCombTransEclResAddr ) ) return;

    double cdarchy = darchysValue();

    // Get the needed result indices we depend on

    size_t permXResultIdx = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PERMX" ) );
    size_t permYResultIdx = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PERMY" ) );
    size_t permZResultIdx = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PERMZ" ) );

    size_t ntgResultIdx = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "NTG" ) );

    bool hasNTGResults = ntgResultIdx != cvf::UNDEFINED_SIZE_T;

    // Get all the actual result values

    std::vector<double>& permXResults = m_cellScalarResults[permXResultIdx][0];
    std::vector<double>& permYResults = m_cellScalarResults[permYResultIdx][0];
    std::vector<double>& permZResults = m_cellScalarResults[permZResultIdx][0];
    std::vector<double>& riCombTransResults =
        m_ownerMainGrid->nncData()->makeStaticConnectionScalarResult( RiaDefines::propertyNameRiCombTrans() );
    m_ownerMainGrid->nncData()->setEclResultAddress( RiaDefines::propertyNameRiCombTrans(), riCombTransEclResAddr );

    std::vector<double>* ntgResults = nullptr;
    if ( hasNTGResults )
    {
        ntgResults = &( m_cellScalarResults[ntgResultIdx][0] );
    }

    // Prepare how to index the result values:
    ResultIndexFunction permXIdxFunc = nullptr;
    ResultIndexFunction permYIdxFunc = nullptr;
    ResultIndexFunction permZIdxFunc = nullptr;
    ResultIndexFunction ntgIdxFunc   = nullptr;
    {
        bool isPermXUsingResIdx = isUsingGlobalActiveIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PERMX" ) );
        bool isPermYUsingResIdx = isUsingGlobalActiveIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PERMY" ) );
        bool isPermZUsingResIdx = isUsingGlobalActiveIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "PERMZ" ) );
        bool isNtgUsingResIdx   = false;
        if ( hasNTGResults )
        {
            isNtgUsingResIdx = isUsingGlobalActiveIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "NTG" ) );
        }

        // Set up result index function pointers

        permXIdxFunc = isPermXUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;
        permYIdxFunc = isPermYUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;
        permZIdxFunc = isPermZUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;
        if ( hasNTGResults )
        {
            ntgIdxFunc = isNtgUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;
        }
    }

    const RigActiveCellInfo* activeCellInfo        = this->activeCellInfo();
    bool                     isFaceNormalsOutwards = m_ownerMainGrid->isFaceNormalsOutwards();

    // NNC calculation
    const RigConnectionContainer& nncConnections = m_ownerMainGrid->nncData()->allConnections();
    for ( size_t connIdx = 0; connIdx < nncConnections.size(); connIdx++ )
    {
        size_t                             nativeResvCellIndex = nncConnections[connIdx].c1GlobIdx();
        size_t                             neighborResvCellIdx = nncConnections[connIdx].c2GlobIdx();
        cvf::StructGridInterface::FaceType faceId = static_cast<cvf::StructGridInterface::FaceType>( nncConnections[connIdx].face() );

        ResultIndexFunction  permIdxFunc = nullptr;
        std::vector<double>* permResults = nullptr;

        switch ( faceId )
        {
            case cvf::StructGridInterface::POS_I:
            case cvf::StructGridInterface::NEG_I:
                permIdxFunc = permXIdxFunc;
                permResults = &permXResults;
                break;
            case cvf::StructGridInterface::POS_J:
            case cvf::StructGridInterface::NEG_J:
                permIdxFunc = permYIdxFunc;
                permResults = &permYResults;
                break;
            case cvf::StructGridInterface::POS_K:
            case cvf::StructGridInterface::NEG_K:
                permIdxFunc = permZIdxFunc;
                permResults = &permZResults;
                break;
            default:
                break;
        }

        if ( !permIdxFunc ) continue;

        // Do nothing if we are only dealing with active cells, and this cell is not active:
        size_t nativeCellPermResIdx = ( *permIdxFunc )( activeCellInfo, nativeResvCellIndex );
        if ( nativeCellPermResIdx == cvf::UNDEFINED_SIZE_T ) continue;

        // Do nothing if neighbor cell has no results
        size_t neighborCellPermResIdx = ( *permIdxFunc )( activeCellInfo, neighborResvCellIdx );
        if ( neighborCellPermResIdx == cvf::UNDEFINED_SIZE_T ) continue;

        const RigCell& nativeCell   = m_ownerMainGrid->globalCellArray()[nativeResvCellIndex];
        const RigCell& neighborCell = m_ownerMainGrid->globalCellArray()[neighborResvCellIdx];

        // Connection geometry

        cvf::Vec3f faceAreaVec = cvf::Vec3f::ZERO;
        cvf::Vec3f faceCenter  = cvf::Vec3f::ZERO;

        // Polygon center
        const std::vector<cvf::Vec3f>& realPolygon = nncConnections[connIdx].polygon();
        for ( size_t pIdx = 0; pIdx < realPolygon.size(); ++pIdx )
        {
            faceCenter += realPolygon[pIdx];
        }

        faceCenter *= 1.0 / realPolygon.size();

        // Polygon area vector

        faceAreaVec = cvf::GeometryTools::polygonAreaNormal3D( realPolygon );

        if ( !isFaceNormalsOutwards ) faceAreaVec = -faceAreaVec;

        double halfCellTrans         = 0;
        double neighborHalfCellTrans = 0;

        // Native cell half cell transm
        {
            cvf::Vec3d centerToFace = nativeCell.faceCenter( faceId ) - nativeCell.center();

            double perm = ( *permResults )[nativeCellPermResIdx];

            double ntg = 1.0;
            if ( hasNTGResults && faceId != cvf::StructGridInterface::POS_K )
            {
                size_t ntgResIdx = ( *ntgIdxFunc )( activeCellInfo, nativeResvCellIndex );
                ntg              = ( *ntgResults )[ntgResIdx];
            }

            halfCellTrans = halfCellTransmissibility( perm, ntg, centerToFace, cvf::Vec3d( faceAreaVec ) );
        }

        // Neighbor cell half cell transm
        {
            cvf::Vec3d centerToFace = neighborCell.faceCenter( cvf::StructGridInterface::oppositeFace( faceId ) ) - neighborCell.center();

            double perm = ( *permResults )[neighborCellPermResIdx];

            double ntg = 1.0;
            if ( hasNTGResults && faceId != cvf::StructGridInterface::POS_K )
            {
                size_t ntgResIdx = ( *ntgIdxFunc )( activeCellInfo, neighborResvCellIdx );
                ntg              = ( *ntgResults )[ntgResIdx];
            }

            neighborHalfCellTrans = halfCellTransmissibility( perm, ntg, centerToFace, -cvf::Vec3d( faceAreaVec ) );
        }

        double newtranTemp          = newtran( cdarchy, 1.0, halfCellTrans, neighborHalfCellTrans );
        riCombTransResults[connIdx] = newtranTemp;
    }
}

double riMult( double transResults, double riTransResults )
{
    if ( transResults == HUGE_VAL || riTransResults == HUGE_VAL ) return HUGE_VAL;

    const double epsilon = 1e-9;

    if ( cvf::Math::abs( riTransResults ) < epsilon )
    {
        if ( cvf::Math::abs( transResults ) < epsilon )
        {
            return 0.0;
        }

        return HUGE_VAL;
    }

    double result = transResults / riTransResults;

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeRiMULTComponent( const QString& riMultCompName )
{
    // Set up which component to compute

    QString riTransCompName;
    QString transCompName;

    if ( riMultCompName == RiaResultNames::riMultXResultName() )
    {
        riTransCompName = RiaResultNames::riTranXResultName();
        transCompName   = "TRANX";
    }
    else if ( riMultCompName == RiaResultNames::riMultYResultName() )
    {
        riTransCompName = RiaResultNames::riTranYResultName();
        transCompName   = "TRANY";
    }
    else if ( riMultCompName == RiaResultNames::riMultZResultName() )
    {
        riTransCompName = RiaResultNames::riTranZResultName();
        transCompName   = "TRANZ";
    }
    else
    {
        CVF_ASSERT( false );
    }

    // Get the needed result indices we depend on

    size_t transResultIdx = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, transCompName ) );
    size_t riTransResultIdx =
        findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, riTransCompName ) );

    // Get the result index of the output

    size_t riMultResultIdx =
        findScalarResultIndexFromAddress( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, riMultCompName ) );
    CVF_ASSERT( riMultResultIdx != cvf::UNDEFINED_SIZE_T );

    // Get the result count, to handle that one of them might be globally defined

    CVF_ASSERT( m_cellScalarResults[riTransResultIdx][0].size() == m_cellScalarResults[transResultIdx][0].size() );

    size_t resultValueCount = m_cellScalarResults[transResultIdx][0].size();

    // Get all the actual result values

    const std::vector<double>& riTransResults = m_cellScalarResults[riTransResultIdx][0];
    const std::vector<double>& transResults   = m_cellScalarResults[transResultIdx][0];
    std::vector<double>&       riMultResults  = m_cellScalarResults[riMultResultIdx][0];

    // Set up output container to correct number of results

    riMultResults.resize( resultValueCount );

    for ( size_t vIdx = 0; vIdx < transResults.size(); ++vIdx )
    {
        riMultResults[vIdx] = riMult( transResults[vIdx], riTransResults[vIdx] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeNncCombRiMULT()
{
    auto riCombMultEclResAddr = RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::combinedRiMultResultName() );
    auto riCombTransEclResAddr =
        RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::combinedRiTranResultName() );
    auto combTransEclResAddr =
        RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::combinedTransmissibilityResultName() );

    if ( m_ownerMainGrid->nncData()->staticConnectionScalarResult( riCombMultEclResAddr ) ) return;

    const std::vector<double>* riTransResults = m_ownerMainGrid->nncData()->staticConnectionScalarResult( riCombTransEclResAddr );

    const std::vector<double>* transResults = m_ownerMainGrid->nncData()->staticConnectionScalarResult( combTransEclResAddr );

    if ( riTransResults && transResults && ( riTransResults->size() == transResults->size() ) )
    {
        std::vector<double>& riMultResults =
            m_ownerMainGrid->nncData()->makeStaticConnectionScalarResult( RiaDefines::propertyNameRiCombMult() );

        m_ownerMainGrid->nncData()->setEclResultAddress( RiaDefines::propertyNameRiCombMult(), riCombMultEclResAddr );

        for ( size_t nncConIdx = 0; nncConIdx < riMultResults.size(); ++nncConIdx )
        {
            riMultResults[nncConIdx] = riMult( ( *transResults )[nncConIdx], ( *riTransResults )[nncConIdx] );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeRiTRANSbyAreaComponent( const QString& riTransByAreaCompResultName )
{
    // Set up which component to compute

    cvf::StructGridInterface::FaceType faceId = cvf::StructGridInterface::NO_FACE;
    QString                            transCompName;

    if ( riTransByAreaCompResultName == RiaResultNames::riAreaNormTranXResultName() )
    {
        transCompName = "TRANX";
        faceId        = cvf::StructGridInterface::POS_I;
    }
    else if ( riTransByAreaCompResultName == RiaResultNames::riAreaNormTranYResultName() )
    {
        transCompName = "TRANY";
        faceId        = cvf::StructGridInterface::POS_J;
    }
    else if ( riTransByAreaCompResultName == RiaResultNames::riAreaNormTranZResultName() )
    {
        transCompName = "TRANZ";
        faceId        = cvf::StructGridInterface::POS_K;
    }
    else
    {
        CVF_ASSERT( false );
    }

    // Get the needed result indices we depend on

    size_t tranCompScResIdx = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, transCompName ) );

    // Get the result index of the output

    size_t riTranByAreaScResIdx =
        findScalarResultIndexFromAddress( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, riTransByAreaCompResultName ) );

    CVF_ASSERT( riTranByAreaScResIdx != cvf::UNDEFINED_SIZE_T );

    // Get the result count, to handle that one of them might be globally defined

    size_t resultValueCount = m_cellScalarResults[tranCompScResIdx][0].size();

    // Get all the actual result values

    const std::vector<double>& transResults         = m_cellScalarResults[tranCompScResIdx][0];
    std::vector<double>&       riTransByAreaResults = m_cellScalarResults[riTranByAreaScResIdx][0];

    // Set up output container to correct number of results

    riTransByAreaResults.resize( resultValueCount );

    // Prepare how to index the result values:

    bool isUsingResIdx = isUsingGlobalActiveIndex( RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, transCompName ) );

    // Set up result index function pointers

    ResultIndexFunction resValIdxFunc = isUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;

    const RigActiveCellInfo*       activeCellInfo = this->activeCellInfo();
    const std::vector<cvf::Vec3d>& nodes          = m_ownerMainGrid->nodes();

    for ( size_t nativeResvCellIndex = 0; nativeResvCellIndex < m_ownerMainGrid->globalCellArray().size(); nativeResvCellIndex++ )
    {
        // Do nothing if we are only dealing with active cells, and this cell is not active:
        size_t nativeCellResValIdx = ( *resValIdxFunc )( activeCellInfo, nativeResvCellIndex );

        if ( nativeCellResValIdx == cvf::UNDEFINED_SIZE_T ) continue;

        const RigCell& nativeCell = m_ownerMainGrid->globalCellArray()[nativeResvCellIndex];
        RigGridBase*   grid       = nativeCell.hostGrid();

        size_t gridLocalNativeCellIndex = nativeCell.gridLocalCellIndex();

        size_t i, j, k, gridLocalNeighborCellIdx;

        grid->ijkFromCellIndex( gridLocalNativeCellIndex, &i, &j, &k );

        if ( grid->cellIJKNeighbor( i, j, k, faceId, &gridLocalNeighborCellIdx ) )
        {
            size_t         neighborResvCellIdx = grid->reservoirCellIndex( gridLocalNeighborCellIdx );
            const RigCell& neighborCell        = m_ownerMainGrid->globalCellArray()[neighborResvCellIdx];

            // Connection geometry

            const RigFault* fault     = grid->mainGrid()->findFaultFromCellIndexAndCellFace( nativeResvCellIndex, faceId );
            bool            isOnFault = fault;

            cvf::Vec3d faceAreaVec;

            if ( isOnFault )
            {
                calculateConnectionGeometry( nativeCell, neighborCell, nodes, faceId, &faceAreaVec );
            }
            else
            {
                faceAreaVec = nativeCell.faceNormalWithAreaLength( faceId );
            }

            double areaOfOverlap  = faceAreaVec.length();
            double transCompValue = transResults[nativeCellResValIdx];

            riTransByAreaResults[nativeCellResValIdx] = transCompValue / areaOfOverlap;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeNncCombRiTRANSbyArea()
{
    auto riCombTransByAreaEclResAddr =
        RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::combinedRiAreaNormTranResultName() );
    auto combTransEclResAddr =
        RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::combinedTransmissibilityResultName() );

    if ( m_ownerMainGrid->nncData()->staticConnectionScalarResult( riCombTransByAreaEclResAddr ) ) return;

    const std::vector<double>* transResults = m_ownerMainGrid->nncData()->staticConnectionScalarResult( combTransEclResAddr );

    if ( !transResults ) return;

    std::vector<double>& riAreaNormTransResults =
        m_ownerMainGrid->nncData()->makeStaticConnectionScalarResult( RiaDefines::propertyNameRiCombTransByArea() );

    m_ownerMainGrid->nncData()->setEclResultAddress( RiaDefines::propertyNameRiCombTransByArea(), riCombTransByAreaEclResAddr );

    if ( transResults->size() != riAreaNormTransResults.size() ) return;

    const RigConnectionContainer& connections = m_ownerMainGrid->nncData()->allConnections();

    for ( size_t nncConIdx = 0; nncConIdx < riAreaNormTransResults.size(); ++nncConIdx )
    {
        const std::vector<cvf::Vec3f>& realPolygon   = connections[nncConIdx].polygon();
        cvf::Vec3f                     faceAreaVec   = cvf::GeometryTools::polygonAreaNormal3D( realPolygon );
        double                         areaOfOverlap = faceAreaVec.length();

        riAreaNormTransResults[nncConIdx] = ( *transResults )[nncConIdx] / areaOfOverlap;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeCompletionTypeForTimeStep( size_t timeStep )
{
    size_t completionTypeResultIndex = findScalarResultIndexFromAddress(
        RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::completionTypeResultName() ) );

    if ( m_cellScalarResults[completionTypeResultIndex].size() < maxTimeStepCount() )
    {
        m_cellScalarResults[completionTypeResultIndex].resize( maxTimeStepCount() );
    }

    std::vector<double>& completionTypeResult = m_cellScalarResults[completionTypeResultIndex][timeStep];

    size_t resultValues = m_ownerMainGrid->globalCellArray().size();

    if ( completionTypeResult.size() == resultValues )
    {
        return;
    }

    completionTypeResult.resize( resultValues );
    std::fill( completionTypeResult.begin(), completionTypeResult.end(), HUGE_VAL );

    RimEclipseCase* eclipseCase = m_ownerCaseData->ownerCase();

    if ( !eclipseCase ) return;

    // If permeabilities are generated by calculations, make sure that generated data is calculated
    // See RicExportFractureCompletionsImpl::generateCompdatValues()
    for ( const QString propertyName : { "PERMX", "PERMY", "PERMZ" } )
    {
        for ( auto userCalculation : RimProject::current()->gridCalculationCollection()->calculations() )
        {
            if ( auto gridCalculation = dynamic_cast<RimGridCalculation*>( userCalculation ) )
            {
                auto outputCases = gridCalculation->outputEclipseCases();
                if ( std::find( outputCases.begin(), outputCases.end(), eclipseCase ) == outputCases.end() ) continue;
            }

            QString generatedPropertyName = RimUserDefinedCalculation::findLeftHandSide( userCalculation->expression() );
            if ( generatedPropertyName == propertyName )
            {
                userCalculation->calculate();
            }
        }
    }

    RimCompletionCellIntersectionCalc::calculateCompletionTypeResult( eclipseCase, completionTypeResult, timeStep );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigCaseCellResultsData::darchysValue()
{
    return RiaEclipseUnitTools::darcysConstant( m_ownerCaseData->unitsType() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeCellVolumes()
{
    RigEclipseResultAddress       addr( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::riCellVolumeResultName() );
    RigCellVolumeResultCalculator calculator( *this );
    calculator.calculate( addr, 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeOilVolumes()
{
    RigEclipseResultAddress      addr( RiaDefines::ResultCatType::DYNAMIC_NATIVE, RiaResultNames::riOilVolumeResultName() );
    RigOilVolumeResultCalculator calculator( *this );
    // Computes for all time steps
    calculator.calculate( addr, -1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeMobilePV()
{
    RigEclipseResultAddress             addr( RiaDefines::ResultCatType::STATIC_NATIVE, RiaResultNames::mobilePoreVolumeName() );
    RigMobilePoreVolumeResultCalculator calculator( *this );
    calculator.calculate( addr, 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::setReaderInterface( RifReaderInterface* readerInterface )
{
    m_readerInterface = readerInterface;

    if ( m_ownerMainGrid )
    {
        m_readerInterface->updateFromGridCount( m_ownerMainGrid->gridCount() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RifReaderInterface* RigCaseCellResultsData::readerInterface() const
{
    return m_readerInterface.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::setHdf5Filename( const QString& hdf5SourSimFilename )
{
    RifReaderEclipseOutput* rifReaderOutput = dynamic_cast<RifReaderEclipseOutput*>( m_readerInterface.p() );
    if ( rifReaderOutput )
    {
        rifReaderOutput->setHdf5FileName( hdf5SourSimFilename );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::setActiveFormationNames( RigFormationNames* activeFormationNames )
{
    m_activeFormationNamesData = activeFormationNames;

    if ( !activeFormationNames )
    {
        clearScalarResult(
            RigEclipseResultAddress( RiaDefines::ResultCatType::FORMATION_NAMES, RiaResultNames::activeFormationNamesResultName() ) );
        return;
    }

    size_t totalGlobCellCount = m_ownerMainGrid->globalCellArray().size();
    addStaticScalarResult( RiaDefines::ResultCatType::FORMATION_NAMES, RiaResultNames::activeFormationNamesResultName(), false, totalGlobCellCount );

    std::vector<double>* fnData = modifiableCellScalarResult( RigEclipseResultAddress( RiaDefines::ResultCatType::FORMATION_NAMES,
                                                                                       RiaResultNames::activeFormationNamesResultName() ),
                                                              0 );

    if ( m_activeFormationNamesData.isNull() )
    {
        for ( size_t cIdx = 0; cIdx < totalGlobCellCount; ++cIdx )
        {
            fnData->at( cIdx ) = HUGE_VAL;
        }
    }
    else
    {
        size_t localCellCount = m_ownerMainGrid->cellCount();
        for ( size_t cIdx = 0; cIdx < localCellCount; ++cIdx )
        {
            size_t i( cvf::UNDEFINED_SIZE_T ), j( cvf::UNDEFINED_SIZE_T ), k( cvf::UNDEFINED_SIZE_T );

            if ( !m_ownerMainGrid->ijkFromCellIndex( cIdx, &i, &j, &k ) ) continue;

            int formNameIdx = activeFormationNames->formationIndexFromKLayerIdx( k );
            if ( formNameIdx != -1 )
            {
                fnData->at( cIdx ) = formNameIdx;
            }
            else
            {
                fnData->at( cIdx ) = HUGE_VAL;
            }
        }

        for ( size_t cIdx = localCellCount; cIdx < totalGlobCellCount; ++cIdx )
        {
            size_t mgrdCellIdx = m_ownerMainGrid->globalCellArray()[cIdx].mainGridCellIndex();

            size_t i( cvf::UNDEFINED_SIZE_T ), j( cvf::UNDEFINED_SIZE_T ), k( cvf::UNDEFINED_SIZE_T );

            if ( !m_ownerMainGrid->ijkFromCellIndex( mgrdCellIdx, &i, &j, &k ) ) continue;

            int formNameIdx = activeFormationNames->formationIndexFromKLayerIdx( k );
            if ( formNameIdx != -1 )
            {
                fnData->at( cIdx ) = formNameIdx;
            }
            else
            {
                fnData->at( cIdx ) = HUGE_VAL;
            }
        }
    }

    // As the Allan formation diagram is depending on formation results, we need to clear the data set
    // Will be recomputed when required
    auto fnNamesResAddr = RigEclipseResultAddress( RiaDefines::ResultCatType::ALLAN_DIAGRAMS, RiaResultNames::formationAllanResultName() );
    clearScalarResult( fnNamesResAddr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigFormationNames* RigCaseCellResultsData::activeFormationNames() const
{
    return m_activeFormationNamesData.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigAllanDiagramData* RigCaseCellResultsData::allanDiagramData()
{
    return m_allanDiagramData.p();
}

//--------------------------------------------------------------------------------------------------
///  If we have any results on any time step, assume we have loaded results
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::isDataPresent( size_t scalarResultIndex ) const
{
    return allocatedValueCount( scalarResultIndex ) > 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::allocatedValueCount( size_t scalarResultIndex ) const
{
    if ( scalarResultIndex >= resultCount() ) return 0;

    const std::vector<std::vector<double>>& valuesAllTimeSteps = m_cellScalarResults[scalarResultIndex];

    size_t valueCount = 0;
    for ( const auto& values : valuesAllTimeSteps )
    {
        valueCount += values.size();
    }

    return valueCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::assignValuesToTemporaryLgrs( const QString& resultName, std::vector<double>& valuesForAllReservoirCells )
{
    CVF_ASSERT( m_activeCellInfo );

    static std::vector<QString> excludedProperties = {
        "MOBPROV",  "PORV",     "FIPOIL",   "FIPGAS",   "FIPWAT",   "FLROILI+", "FLROILJ+", "FLROILK+", "FLRGASI+", "FLRGASJ+", "FLRGASK+",
        "FLRWATI+", "FLRWATJ+", "FLRWATK+", "FLOOILI+", "FLOWATI+", "FLOGASI+", "FLOOILJ+", "FLOWATJ+", "FLOGASJ+", "FLOOILK+", "FLOWATK+",
        "FLOGASK+", "SFIPGAS",  "SFIPOIL",  "SFIPWAT",  "AREAX",    "AREAY",    "AREAZ",    "DIFFX",    "DIFFY",    "DIFFZ",    "DZNET",
        "HEATTX",   "HEATTY",   "HEATTZ",   "LX",       "LY",       "LZ",       "MINPVV",   "TRANX",    "TRANY",    "TRANZ",
    };

    if ( std::find( excludedProperties.begin(), excludedProperties.end(), resultName ) != excludedProperties.end() )
    {
        return;
    }

    bool invalidCellsDetected = false;
    for ( size_t gridIdx = 0; gridIdx < m_ownerMainGrid->gridCount(); gridIdx++ )
    {
        const auto& grid = m_ownerMainGrid->gridByIndex( gridIdx );
        if ( grid->isTempGrid() )
        {
            for ( size_t localCellIdx = 0; localCellIdx < grid->cellCount(); localCellIdx++ )
            {
                const RigCell& cell = grid->cell( localCellIdx );

                size_t mainGridCellIndex  = cell.mainGridCellIndex();
                size_t reservoirCellIndex = grid->reservoirCellIndex( localCellIdx );

                size_t mainGridCellResultIndex = m_activeCellInfo->cellResultIndex( mainGridCellIndex );
                size_t cellResultIndex         = m_activeCellInfo->cellResultIndex( reservoirCellIndex );

                if ( mainGridCellResultIndex != cvf::UNDEFINED_SIZE_T && cellResultIndex != cvf::UNDEFINED_SIZE_T )
                {
                    double mainGridValue = valuesForAllReservoirCells[mainGridCellResultIndex];

                    valuesForAllReservoirCells[cellResultIndex] = mainGridValue;
                }
                else
                {
                    invalidCellsDetected = true;
                }
            }
        }
    }

    if ( invalidCellsDetected )
    {
        RiaLogging::warning( "Detected invalid/undefined cells when assigning result values to temporary LGRs" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigStatisticsDataCache* RigCaseCellResultsData::statistics( const RigEclipseResultAddress& resVarAddr )
{
    size_t scalarResultIndex = findScalarResultIndexFromAddress( resVarAddr );
    CAF_ASSERT( scalarResultIndex < m_statisticsDataCache.size() );
    return m_statisticsDataCache[scalarResultIndex].p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigCaseCellResultsData::findScalarResultIndexFromAddress( const RigEclipseResultAddress& resVarAddr ) const
{
    if ( !resVarAddr.isValid() )
    {
        return cvf::UNDEFINED_SIZE_T;
    }
    else if ( resVarAddr.resultCatType() == RiaDefines::ResultCatType::UNDEFINED )
    {
        RigEclipseResultAddress resVarAddressWithType = resVarAddr;

        resVarAddressWithType.setResultCatType( RiaDefines::ResultCatType::STATIC_NATIVE );

        size_t scalarResultIndex = findScalarResultIndexFromAddress( resVarAddressWithType );

        if ( scalarResultIndex == cvf::UNDEFINED_SIZE_T )
        {
            resVarAddressWithType.setResultCatType( RiaDefines::ResultCatType::DYNAMIC_NATIVE );
            scalarResultIndex = findScalarResultIndexFromAddress( resVarAddressWithType );
        }

        if ( scalarResultIndex == cvf::UNDEFINED_SIZE_T )
        {
            resVarAddressWithType.setResultCatType( RiaDefines::ResultCatType::SOURSIMRL );
            scalarResultIndex = findScalarResultIndexFromAddress( resVarAddressWithType );
        }

        if ( scalarResultIndex == cvf::UNDEFINED_SIZE_T )
        {
            resVarAddressWithType.setResultCatType( RiaDefines::ResultCatType::GENERATED );
            scalarResultIndex = findScalarResultIndexFromAddress( resVarAddressWithType );
        }

        if ( scalarResultIndex == cvf::UNDEFINED_SIZE_T )
        {
            resVarAddressWithType.setResultCatType( RiaDefines::ResultCatType::INPUT_PROPERTY );
            scalarResultIndex = findScalarResultIndexFromAddress( resVarAddressWithType );
        }

        if ( scalarResultIndex == cvf::UNDEFINED_SIZE_T )
        {
            resVarAddressWithType.setResultCatType( RiaDefines::ResultCatType::FORMATION_NAMES );
            scalarResultIndex = findScalarResultIndexFromAddress( resVarAddressWithType );
        }

        return scalarResultIndex;
    }
    else
    {
        auto index = m_addressToResultIndexMap.find( resVarAddr );
        if ( index != m_addressToResultIndexMap.end() )
        {
            return index->second;
        }

        return cvf::UNDEFINED_SIZE_T;
    }
}

#include "RimEclipseResultCase.h"

//--------------------------------------------------------------------------------------------------
/// Copy result meta data from main case to all other cases in grid case group
/// This code was originally part of RimStatisticsCaseEvaluator, but moved here to be a general solution
/// for all cases
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::copyResultsMetaDataFromMainCase( RigEclipseCaseData*           mainCaseResultsData,
                                                              RiaDefines::PorosityModelType poroModel,
                                                              std::vector<RimEclipseCase*>  destinationCases )
{
    std::vector<RigEclipseResultAddress> resAddresses  = mainCaseResultsData->results( poroModel )->existingResults();
    std::vector<RigEclipseTimeStepInfo>  timeStepInfos = mainCaseResultsData->results( poroModel )->timeStepInfos( resAddresses[0] );

    const std::vector<RigEclipseResultInfo> resultInfos = mainCaseResultsData->results( poroModel )->infoForEachResultIndex();

    for ( size_t i = 0; i < destinationCases.size(); i++ )
    {
        RimEclipseResultCase* rimReservoir = dynamic_cast<RimEclipseResultCase*>( destinationCases[i] );

        if ( !rimReservoir ) continue; // Input reservoir
        if ( mainCaseResultsData == rimReservoir->eclipseCaseData() ) continue; // Do not copy ontop of itself

        RigCaseCellResultsData* cellResultsStorage = rimReservoir->results( poroModel );

        for ( size_t resIdx = 0; resIdx < resultInfos.size(); resIdx++ )
        {
            RigEclipseResultAddress resVarAddr = resultInfos[resIdx].eclipseResultAddress();

            bool needsToBeStored  = resultInfos[resIdx].needsToBeStored();
            bool mustBeCalculated = resultInfos[resIdx].mustBeCalculated();

            size_t scalarResultIndex = cellResultsStorage->findScalarResultIndexFromAddress( resVarAddr );

            if ( scalarResultIndex == cvf::UNDEFINED_SIZE_T )
            {
                cellResultsStorage->createResultEntry( resVarAddr, needsToBeStored );

                if ( mustBeCalculated )
                {
                    scalarResultIndex = cellResultsStorage->findScalarResultIndexFromAddress( resVarAddr );
                    cellResultsStorage->setMustBeCalculated( scalarResultIndex );
                }

                cellResultsStorage->setTimeStepInfos( resVarAddr, timeStepInfos );

                std::vector<std::vector<double>>* dataValues = cellResultsStorage->modifiableCellScalarResultTimesteps( resVarAddr );
                dataValues->resize( timeStepInfos.size() );
            }
        }

        cellResultsStorage->createPlaceholderResultEntries();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::setStatisticsDataCacheNumBins( const RigEclipseResultAddress& resultAddress, size_t numBins )
{
    statistics( resultAddress )->setNumBins( numBins );
}
