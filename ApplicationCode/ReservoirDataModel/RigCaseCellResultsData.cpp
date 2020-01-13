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
#include "RiaLogging.h"

#include "RigAllenDiagramData.h"
#include "RigCaseCellResultCalculator.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseMultiPropertyStatCalc.h"
#include "RigEclipseNativeStatCalc.h"
#include "RigEclipseResultInfo.h"
#include "RigFormationNames.h"
#include "RigMainGrid.h"
#include "RigStatisticsDataCache.h"
#include "RigStatisticsMath.h"

#include "RimCompletionCellIntersectionCalc.h"
#include "RimEclipseCase.h"
#include "RimProject.h"

#include "RifReaderEclipseOutput.h"

#include "cafProgressInfo.h"
#include "cvfGeometryTools.h"

#include <QDateTime>

#include "RigEclipseAllenFaultsStatCalc.h"
#include <algorithm>
#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCaseCellResultsData::RigCaseCellResultsData( RigEclipseCaseData*           ownerCaseData,
                                                RiaDefines::PorosityModelType porosityModel )
    : m_activeCellInfo( nullptr )
    , m_porosityModel( porosityModel )
{
    CVF_ASSERT( ownerCaseData != nullptr );
    CVF_ASSERT( ownerCaseData->mainGrid() != nullptr );

    m_ownerCaseData = ownerCaseData;
    m_ownerMainGrid = ownerCaseData->mainGrid();

    m_allenDiagramData = new RigAllenDiagramData;
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
void RigCaseCellResultsData::minMaxCellScalarValues( const RigEclipseResultAddress& resVarAddr,
                                                     size_t                         timeStepIndex,
                                                     double&                        min,
                                                     double&                        max )
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
void RigCaseCellResultsData::posNegClosestToZero( const RigEclipseResultAddress& resVarAddr,
                                                  size_t                         timeStepIndex,
                                                  double&                        pos,
                                                  double&                        neg )
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
const std::vector<size_t>& RigCaseCellResultsData::cellScalarValuesHistogram( const RigEclipseResultAddress& resVarAddr,
                                                                              size_t timeStepIndex )
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
void RigCaseCellResultsData::p10p90CellScalarValues( const RigEclipseResultAddress& resVarAddr,
                                                     size_t                         timeStepIndex,
                                                     double&                        p10,
                                                     double&                        p90 )
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
void RigCaseCellResultsData::meanCellScalarValues( const RigEclipseResultAddress& resVarAddr,
                                                   size_t                         timeStepIndex,
                                                   double&                        meanValue )
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
void RigCaseCellResultsData::sumCellScalarValues( const RigEclipseResultAddress& resVarAddr,
                                                  size_t                         timeStepIndex,
                                                  double&                        sumValue )
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
void RigCaseCellResultsData::mobileVolumeWeightedMean( const RigEclipseResultAddress& resVarAddr,
                                                       size_t                         timeStepIndex,
                                                       double&                        meanValue )
{
    statistics( resVarAddr )->mobileVolumeWeightedMean( timeStepIndex, meanValue );
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
const std::vector<std::vector<double>>&
    RigCaseCellResultsData::cellScalarResults( const RigEclipseResultAddress& resVarAddr ) const
{
    size_t scalarResultIndex = findScalarResultIndexFromAddress( resVarAddr );

    CVF_TIGHT_ASSERT( scalarResultIndex < resultCount() );

    return m_cellScalarResults[scalarResultIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>>*
    RigCaseCellResultsData::modifiableCellScalarResultTimesteps( const RigEclipseResultAddress& resVarAddr )
{
    size_t scalarResultIndex = findScalarResultIndexFromAddress( resVarAddr );

    CVF_TIGHT_ASSERT( scalarResultIndex < resultCount() );

    return &( m_cellScalarResults[scalarResultIndex] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>* RigCaseCellResultsData::modifiableCellScalarResult( const RigEclipseResultAddress& resVarAddr,
                                                                         size_t                         timeStepIndex )
{
    size_t scalarResultIndex = findScalarResultIndexFromAddress( resVarAddr );

    CVF_TIGHT_ASSERT( scalarResultIndex < resultCount() );
    CVF_TIGHT_ASSERT( timeStepIndex < m_cellScalarResults[scalarResultIndex].size() );

    return &( m_cellScalarResults[scalarResultIndex][timeStepIndex] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigCaseCellResultsData::cellScalarResults( const RigEclipseResultAddress& resVarAddr,
                                                                      size_t timeStepIndex ) const
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
size_t RigCaseCellResultsData::findOrCreateScalarResultIndex( const RigEclipseResultAddress& resVarAddr,
                                                              bool                           needsToBeStored )
{
    size_t scalarResultIndex = this->findScalarResultIndexFromAddress( resVarAddr );

    // If the result exists, do nothing

    if ( scalarResultIndex != cvf::UNDEFINED_SIZE_T )
    {
        return scalarResultIndex;
    }

    // Create the new empty result with metadata

    scalarResultIndex = this->resultCount();
    m_cellScalarResults.push_back( std::vector<std::vector<double>>() );

    RigEclipseResultInfo resInfo( resVarAddr, needsToBeStored, false, scalarResultIndex );

    m_resultInfos.push_back( resInfo );

    // Create statistics calculator and add statistics cache object
    // Todo: Move to a "factory" method

    QString resultName = resVarAddr.m_resultName;

    cvf::ref<RigStatisticsCalculator> statisticsCalculator;

    if ( resultName == RiaDefines::combinedTransmissibilityResultName() )
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();

        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "TRANX" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "TRANY" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "TRANZ" ) );

        statisticsCalculator = calc;
    }
    else if ( resultName == RiaDefines::combinedMultResultName() )
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();

        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "MULTX" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "MULTX-" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "MULTY" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "MULTY-" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "MULTZ" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "MULTZ-" ) );

        statisticsCalculator = calc;
    }
    else if ( resultName == RiaDefines::combinedRiTranResultName() )
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::STATIC_NATIVE,
                                                                      RiaDefines::riTranXResultName() ) );
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::STATIC_NATIVE,
                                                                      RiaDefines::riTranYResultName() ) );
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::STATIC_NATIVE,
                                                                      RiaDefines::riTranZResultName() ) );
        statisticsCalculator = calc;
    }
    else if ( resultName == RiaDefines::combinedRiMultResultName() )
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::STATIC_NATIVE,
                                                                      RiaDefines::riMultXResultName() ) );
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::STATIC_NATIVE,
                                                                      RiaDefines::riMultYResultName() ) );
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::STATIC_NATIVE,
                                                                      RiaDefines::riMultZResultName() ) );
        statisticsCalculator = calc;
    }
    else if ( resultName == RiaDefines::combinedRiAreaNormTranResultName() )
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::STATIC_NATIVE,
                                                                      RiaDefines::riAreaNormTranXResultName() ) );
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::STATIC_NATIVE,
                                                                      RiaDefines::riAreaNormTranYResultName() ) );
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::STATIC_NATIVE,
                                                                      RiaDefines::riAreaNormTranZResultName() ) );
        statisticsCalculator = calc;
    }
    else if ( resultName == RiaDefines::combinedWaterFluxResultName() )
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "FLRWATI+" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "FLRWATJ+" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "FLRWATK+" ) );
        statisticsCalculator = calc;
    }
    else if ( resultName == RiaDefines::combinedOilFluxResultName() )
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "FLROILI+" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "FLROILJ+" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "FLROILK+" ) );
        statisticsCalculator = calc;
    }
    else if ( resultName == RiaDefines::combinedGasFluxResultName() )
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc = new RigEclipseMultiPropertyStatCalc();
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "FLRGASI+" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "FLRGASJ+" ) );
        calc->addNativeStatisticsCalculator( this, RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "FLRGASK+" ) );
        statisticsCalculator = calc;
    }
    else if ( resultName.endsWith( "IJK" ) )
    {
        cvf::ref<RigEclipseMultiPropertyStatCalc> calc     = new RigEclipseMultiPropertyStatCalc();
        QString                                   baseName = resultName.left( resultName.size() - 3 );
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::GENERATED,
                                                                      QString( "%1I" ).arg( baseName ) ) );
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::GENERATED,
                                                                      QString( "%1J" ).arg( baseName ) ) );
        calc->addNativeStatisticsCalculator( this,
                                             RigEclipseResultAddress( RiaDefines::GENERATED,
                                                                      QString( "%1K" ).arg( baseName ) ) );
        statisticsCalculator = calc;
    }
    else if ( resultName == RiaDefines::formationAllenResultName() ||
              resultName == RiaDefines::formationBinaryAllenResultName() )
    {
        cvf::ref<RigEclipseAllenFaultsStatCalc> calc = new RigEclipseAllenFaultsStatCalc( m_ownerMainGrid->nncData(),
                                                                                          resVarAddr );
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
        if ( it->resultType() == resType && !it->eclipseResultAddress().isTimeLapse() &&
             !it->eclipseResultAddress().hasDifferenceCase() )
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
    CVF_TIGHT_ASSERT( scalarResultIndex < m_cellScalarResults.size() );
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

    if ( !m_cellScalarResults[scalarResultIndex].size() ) return true;

    size_t firstTimeStepResultValueCount = m_cellScalarResults[scalarResultIndex][0].size();
    if ( firstTimeStepResultValueCount == m_ownerMainGrid->globalCellArray().size() ) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::hasFlowDiagUsableFluxes() const
{
    QStringList dynResVarNames = resultNames( RiaDefines::DYNAMIC_NATIVE );

    bool hasFlowFluxes = true;
    hasFlowFluxes      = dynResVarNames.contains( "FLRWATI+" );
    hasFlowFluxes = hasFlowFluxes && ( dynResVarNames.contains( "FLROILI+" ) || dynResVarNames.contains( "FLRGASI+" ) );

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

    std::vector<std::vector<double>>* dataValues = this->modifiableCellScalarResultTimesteps( resVarAddr );
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
        if ( !this->hasResultEntry( RigEclipseResultAddress( newResultName ) ) ) break;

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
    m_statisticsDataCache.clear();
}

//--------------------------------------------------------------------------------------------------
/// Removes all the actual numbers put into this object, and frees up the memory.
/// Does not touch the metadata in any way
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::freeAllocatedResultsData()
{
    for ( size_t resultIdx = 0; resultIdx < m_cellScalarResults.size(); ++resultIdx )
    {
        for ( size_t tsIdx = 0; tsIdx < m_cellScalarResults[resultIdx].size(); ++tsIdx )
        {
            // Using swap with an empty vector as that is the safest way to really get rid of the allocated data in a vector
            std::vector<double> empty;
            m_cellScalarResults[resultIdx][tsIdx].swap( empty );
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
bool RigCaseCellResultsData::updateResultName( RiaDefines::ResultCatType resultType,
                                               const QString&            oldName,
                                               const QString&            newName )
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
const std::vector<double>*
    RigCaseCellResultsData::getResultIndexableStaticResult( RigActiveCellInfo*      actCellInfo,
                                                            RigCaseCellResultsData* gridCellResults,
                                                            QString                 resultName,
                                                            std::vector<double>&    activeCellsResultsTempContainer )
{
    size_t                  resultCellCount    = actCellInfo->reservoirCellResultCount();
    size_t                  reservoirCellCount = actCellInfo->reservoirCellCount();
    RigEclipseResultAddress resVarAddr( RiaDefines::STATIC_NATIVE, resultName );

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
const std::vector<RigEclipseResultInfo>& RigCaseCellResultsData::infoForEachResultIndex()
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
        if ( ri.resultType() == RiaDefines::SOURSIMRL )
        {
            ri.setResultType( RiaDefines::REMOVED );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::createPlaceholderResultEntries()
{
    // SOIL
    {
        if ( !hasResultEntry( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "SOIL" ) ) )
        {
            if ( hasResultEntry( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "SWAT" ) ) ||
                 hasResultEntry( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "SGAS" ) ) )
            {
                size_t soilIndex = findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE,
                                                                                           "SOIL" ),
                                                                  false );
                this->setMustBeCalculated( soilIndex );
            }
        }
    }

    // Oil Volume
    if ( RiaApplication::enableDevelopmentFeatures() )
    {
        if ( hasResultEntry( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "SOIL" ) ) )
        {
            findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE,
                                                                    RiaDefines::riOilVolumeResultName() ),
                                           false );
        }
    }

    // Completion type
    {
        findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE,
                                                                RiaDefines::completionTypeResultName() ),
                                       false );
    }

    // Fault results
    {
        findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::ALLEN_DIAGRAMS,
                                                                RiaDefines::formationBinaryAllenResultName() ),
                                       false );

        findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::ALLEN_DIAGRAMS,
                                                                RiaDefines::formationAllenResultName() ),
                                       false );
    }

    // FLUX
    {
        if ( hasResultEntry( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "FLRWATI+" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "FLRWATJ+" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "FLRWATK+" ) ) )
        {
            findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE,
                                                                    RiaDefines::combinedWaterFluxResultName() ),
                                           false );
        }

        if ( hasResultEntry( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "FLROILI+" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "FLROILJ+" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "FLROILK+" ) ) )
        {
            findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE,
                                                                    RiaDefines::combinedOilFluxResultName() ),
                                           false );
        }

        if ( hasResultEntry( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "FLRGASI+" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "FLRGASJ+" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "FLRGASK+" ) ) )
        {
            findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE,
                                                                    RiaDefines::combinedGasFluxResultName() ),
                                           false );
        }
    }

    // TRANSXYZ
    {
        if ( hasCompleteTransmissibilityResults() )
        {
            addStaticScalarResult( RiaDefines::STATIC_NATIVE, RiaDefines::combinedTransmissibilityResultName(), false, 0 );
        }
    }
    // MULTXYZ
    {
        if ( hasResultEntry( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "MULTX" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "MULTX-" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "MULTY" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "MULTY-" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "MULTZ" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "MULTZ-" ) ) )
        {
            addStaticScalarResult( RiaDefines::STATIC_NATIVE, RiaDefines::combinedMultResultName(), false, 0 );
        }
    }

    // riTRANSXYZ and X,Y,Z
    {
        if ( hasResultEntry( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "PERMX" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "PERMY" ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "PERMZ" ) ) )
        {
            addStaticScalarResult( RiaDefines::STATIC_NATIVE, RiaDefines::riTranXResultName(), false, 0 );
            addStaticScalarResult( RiaDefines::STATIC_NATIVE, RiaDefines::riTranYResultName(), false, 0 );
            addStaticScalarResult( RiaDefines::STATIC_NATIVE, RiaDefines::riTranZResultName(), false, 0 );
            addStaticScalarResult( RiaDefines::STATIC_NATIVE, RiaDefines::combinedRiTranResultName(), false, 0 );
        }
    }

    // riMULTXYZ and X, Y, Z
    {
        if ( hasCompleteTransmissibilityResults() &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, RiaDefines::riTranXResultName() ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, RiaDefines::riTranYResultName() ) ) &&
             hasResultEntry( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, RiaDefines::riTranZResultName() ) ) )
        {
            addStaticScalarResult( RiaDefines::STATIC_NATIVE, RiaDefines::riMultXResultName(), false, 0 );
            addStaticScalarResult( RiaDefines::STATIC_NATIVE, RiaDefines::riMultYResultName(), false, 0 );
            addStaticScalarResult( RiaDefines::STATIC_NATIVE, RiaDefines::riMultZResultName(), false, 0 );
            addStaticScalarResult( RiaDefines::STATIC_NATIVE, RiaDefines::combinedRiMultResultName(), false, 0 );
        }
    }

    // riTRANSXYZbyArea and X, Y, Z
    {
        if ( hasResultEntry( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "TRANX" ) ) )
        {
            addStaticScalarResult( RiaDefines::STATIC_NATIVE, RiaDefines::riAreaNormTranXResultName(), false, 0 );
        }

        if ( hasResultEntry( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "TRANY" ) ) )
        {
            addStaticScalarResult( RiaDefines::STATIC_NATIVE, RiaDefines::riAreaNormTranYResultName(), false, 0 );
        }

        if ( hasResultEntry( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "TRANZ" ) ) )
        {
            addStaticScalarResult( RiaDefines::STATIC_NATIVE, RiaDefines::riAreaNormTranZResultName(), false, 0 );
        }

        if ( hasCompleteTransmissibilityResults() )
        {
            addStaticScalarResult( RiaDefines::STATIC_NATIVE, RiaDefines::combinedRiAreaNormTranResultName(), false, 0 );
        }
    }

    // Cell Volume
    {
        addStaticScalarResult( RiaDefines::STATIC_NATIVE, RiaDefines::riCellVolumeResultName(), false, 0 );
    }

    // Mobile Pore Volume
    {
        if ( hasResultEntry( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "PORV" ) ) )
        {
            addStaticScalarResult( RiaDefines::STATIC_NATIVE, RiaDefines::mobilePoreVolumeName(), false, 0 );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::hasCompleteTransmissibilityResults() const
{
    if ( hasResultEntry( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "TRANX" ) ) &&
         hasResultEntry( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "TRANY" ) ) &&
         hasResultEntry( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "TRANZ" ) ) )
    {
        return true;
    }

    return false;
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
void RigCaseCellResultsData::ensureKnownResultLoadedForTimeStep( const RigEclipseResultAddress& resultAddress,
                                                                 size_t                         timeStepIndex )
{
    CAF_ASSERT( resultAddress.m_resultCatType != RiaDefines::UNDEFINED );

    findOrLoadKnownScalarResultForTimeStep( resultAddress, timeStepIndex );
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
    else if ( resVarAddr.m_resultCatType == RiaDefines::UNDEFINED )
    {
        std::vector<RiaDefines::ResultCatType> searchOrder = {RiaDefines::STATIC_NATIVE,
                                                              RiaDefines::DYNAMIC_NATIVE,
                                                              RiaDefines::SOURSIMRL,
                                                              RiaDefines::GENERATED,
                                                              RiaDefines::INPUT_PROPERTY,
                                                              RiaDefines::FORMATION_NAMES};

        size_t scalarResultIndex = this->findOrLoadKnownScalarResultByResultTypeOrder( resVarAddr, searchOrder );

        return scalarResultIndex;
    }

    size_t scalarResultIndex = this->findScalarResultIndexFromAddress( resVarAddr );

    if ( scalarResultIndex == cvf::UNDEFINED_SIZE_T ) return cvf::UNDEFINED_SIZE_T;

    RiaDefines::ResultCatType type       = resVarAddr.m_resultCatType;
    QString                   resultName = resVarAddr.m_resultName;

    if ( resVarAddr.hasDifferenceCase() || resVarAddr.isTimeLapse() )
    {
        if ( !RigCaseCellResultCalculator::computeDifference( this->m_ownerCaseData, m_porosityModel, resVarAddr ) )
        {
            return cvf::UNDEFINED_SIZE_T;
        }

        return scalarResultIndex;
    }

    // Load dependency data sets

    if ( type == RiaDefines::STATIC_NATIVE )
    {
        if ( resultName == RiaDefines::combinedTransmissibilityResultName() )
        {
            this->findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "TRANX" ) );
            this->findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "TRANY" ) );
            this->findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "TRANZ" ) );
        }
        else if ( resultName == RiaDefines::combinedMultResultName() )
        {
            this->findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "MULTX" ) );
            this->findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "MULTX-" ) );
            this->findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "MULTY" ) );
            this->findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "MULTY-" ) );
            this->findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "MULTZ" ) );
            this->findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "MULTZ-" ) );
        }
        else if ( resultName == RiaDefines::combinedRiTranResultName() )
        {
            computeRiTransComponent( RiaDefines::riTranXResultName() );
            computeRiTransComponent( RiaDefines::riTranYResultName() );
            computeRiTransComponent( RiaDefines::riTranZResultName() );
            computeNncCombRiTrans();
        }
        else if ( resultName == RiaDefines::riTranXResultName() || resultName == RiaDefines::riTranYResultName() ||
                  resultName == RiaDefines::riTranZResultName() )
        {
            computeRiTransComponent( resultName );
        }
        else if ( resultName == RiaDefines::combinedRiMultResultName() )
        {
            computeRiMULTComponent( RiaDefines::riMultXResultName() );
            computeRiMULTComponent( RiaDefines::riMultYResultName() );
            computeRiMULTComponent( RiaDefines::riMultZResultName() );
            computeNncCombRiTrans();
            computeNncCombRiMULT();
        }
        else if ( resultName == RiaDefines::riMultXResultName() || resultName == RiaDefines::riMultYResultName() ||
                  resultName == RiaDefines::riMultZResultName() )
        {
            computeRiMULTComponent( resultName );
        }
        else if ( resultName == RiaDefines::combinedRiAreaNormTranResultName() )
        {
            computeRiTRANSbyAreaComponent( RiaDefines::riAreaNormTranXResultName() );
            computeRiTRANSbyAreaComponent( RiaDefines::riAreaNormTranYResultName() );
            computeRiTRANSbyAreaComponent( RiaDefines::riAreaNormTranZResultName() );
            computeNncCombRiTRANSbyArea();
        }
        else if ( resultName == RiaDefines::riAreaNormTranXResultName() ||
                  resultName == RiaDefines::riAreaNormTranYResultName() ||
                  resultName == RiaDefines::riAreaNormTranZResultName() )
        {
            computeRiTRANSbyAreaComponent( resultName );
        }
        else if ( resultName == RiaDefines::formationAllenResultName() ||
                  resultName == RiaDefines::formationBinaryAllenResultName() )
        {
            computeAllenResults( this, m_ownerMainGrid );
        }
    }
    else if ( type == RiaDefines::DYNAMIC_NATIVE )
    {
        if ( resultName == RiaDefines::combinedWaterFluxResultName() )
        {
            this->findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "FLRWATI+" ) );
            this->findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "FLRWATJ+" ) );
            this->findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "FLRWATK+" ) );
        }
        else if ( resultName == RiaDefines::combinedOilFluxResultName() )
        {
            this->findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "FLROILI+" ) );
            this->findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "FLROILJ+" ) );
            this->findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "FLROILK+" ) );
        }
        else if ( resultName == RiaDefines::combinedGasFluxResultName() )
        {
            this->findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "FLRGASI+" ) );
            this->findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "FLRGASJ+" ) );
            this->findOrLoadKnownScalarResult( RigEclipseResultAddress( type, "FLRGASK+" ) );
        }
    }

    if ( isDataPresent( scalarResultIndex ) )
    {
        return scalarResultIndex;
    }

    if ( resultName == "SOIL" )
    {
        if ( this->mustBeCalculated( scalarResultIndex ) )
        {
            // Trigger loading of SWAT, SGAS to establish time step count if no data has been loaded from file at this point
            findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "SWAT" ) );
            findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "SGAS" ) );

            m_cellScalarResults[scalarResultIndex].resize( this->maxTimeStepCount() );
            for ( size_t timeStepIdx = 0; timeStepIdx < this->maxTimeStepCount(); timeStepIdx++ )
            {
                std::vector<double>& values = m_cellScalarResults[scalarResultIndex][timeStepIdx];
                if ( values.size() == 0 )
                {
                    computeSOILForTimeStep( timeStepIdx );
                }
            }

            return scalarResultIndex;
        }
    }
    else if ( resultName == RiaDefines::completionTypeResultName() )
    {
        caf::ProgressInfo progressInfo( this->maxTimeStepCount(), "Calculate Completion Type Results" );
        m_cellScalarResults[scalarResultIndex].resize( this->maxTimeStepCount() );
        for ( size_t timeStepIdx = 0; timeStepIdx < this->maxTimeStepCount(); ++timeStepIdx )
        {
            computeCompletionTypeForTimeStep( timeStepIdx );
            progressInfo.incrementProgress();
        }
    }
    else if ( resultName == RiaDefines::mobilePoreVolumeName() )
    {
        computeMobilePV();
    }

    if ( type == RiaDefines::GENERATED )
    {
        return cvf::UNDEFINED_SIZE_T;
    }

    if ( m_readerInterface.notNull() )
    {
        // Add one more result to result container
        size_t timeStepCount = this->infoForEachResultIndex()[scalarResultIndex].timeStepInfos().size();

        bool resultLoadingSucess = true;

        size_t tempGridCellCount = m_ownerMainGrid->totalTemporaryGridCellCount();

        if ( type == RiaDefines::DYNAMIC_NATIVE && timeStepCount > 0 )
        {
            m_cellScalarResults[scalarResultIndex].resize( timeStepCount );

            size_t i;
            for ( i = 0; i < timeStepCount; i++ )
            {
                std::vector<double>& values = m_cellScalarResults[scalarResultIndex][i];
                if ( !m_readerInterface->dynamicResult( resultName, m_porosityModel, i, &values ) )
                {
                    resultLoadingSucess = false;
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
        else if ( type == RiaDefines::STATIC_NATIVE )
        {
            m_cellScalarResults[scalarResultIndex].resize( 1 );

            std::vector<double>& values = m_cellScalarResults[scalarResultIndex][0];
            if ( !m_readerInterface->staticResult( resultName, m_porosityModel, &values ) )
            {
                resultLoadingSucess = false;
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

        if ( !resultLoadingSucess )
        {
            // Remove last scalar result because loading of result failed
            m_cellScalarResults[scalarResultIndex].clear();
        }
    }

    if ( resultName == RiaDefines::riCellVolumeResultName() )
    {
        computeCellVolumes();
    }
    else if ( resultName == RiaDefines::riOilVolumeResultName() )
    {
        computeCellVolumes();
        computeOilVolumes();
    }

    // Handle SourSimRL reading

    if ( type == RiaDefines::SOURSIMRL )
    {
        RifReaderEclipseOutput* eclReader = dynamic_cast<RifReaderEclipseOutput*>( m_readerInterface.p() );
        if ( eclReader )
        {
            size_t timeStepCount = this->infoForEachResultIndex()[scalarResultIndex].timeStepInfos().size();

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
size_t RigCaseCellResultsData::findOrLoadKnownScalarResultByResultTypeOrder(
    const RigEclipseResultAddress&                resVarAddr,
    const std::vector<RiaDefines::ResultCatType>& resultCategorySearchOrder )
{
    std::set<RiaDefines::ResultCatType> otherResultTypesToSearch = {RiaDefines::STATIC_NATIVE,
                                                                    RiaDefines::DYNAMIC_NATIVE,
                                                                    RiaDefines::SOURSIMRL,
                                                                    RiaDefines::INPUT_PROPERTY,
                                                                    RiaDefines::GENERATED,
                                                                    RiaDefines::FORMATION_NAMES};

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
        resVarAddressWithType.m_resultCatType         = resultType;

        size_t scalarResultIndex = this->findOrLoadKnownScalarResult( resVarAddressWithType );

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
size_t RigCaseCellResultsData::findOrLoadKnownScalarResultForTimeStep( const RigEclipseResultAddress& resVarAddr,
                                                                       size_t                         timeStepIndex )
{
    RiaDefines::ResultCatType type       = resVarAddr.m_resultCatType;
    QString                   resultName = resVarAddr.m_resultName;

    // Special handling for SOIL
    if ( type == RiaDefines::DYNAMIC_NATIVE && resultName.toUpper() == "SOIL" )
    {
        size_t soilScalarResultIndex = this->findScalarResultIndexFromAddress( resVarAddr );

        if ( this->mustBeCalculated( soilScalarResultIndex ) )
        {
            m_cellScalarResults[soilScalarResultIndex].resize( this->maxTimeStepCount() );

            std::vector<double>& values = m_cellScalarResults[soilScalarResultIndex][timeStepIndex];
            if ( values.size() == 0 )
            {
                computeSOILForTimeStep( timeStepIndex );
            }

            return soilScalarResultIndex;
        }
    }
    else if ( type == RiaDefines::DYNAMIC_NATIVE && resultName == RiaDefines::completionTypeResultName() )
    {
        size_t completionTypeScalarResultIndex = this->findScalarResultIndexFromAddress( resVarAddr );
        computeCompletionTypeForTimeStep( timeStepIndex );
        return completionTypeScalarResultIndex;
    }

    size_t scalarResultIndex = this->findScalarResultIndexFromAddress( resVarAddr );
    if ( scalarResultIndex == cvf::UNDEFINED_SIZE_T ) return cvf::UNDEFINED_SIZE_T;

    if ( type == RiaDefines::GENERATED )
    {
        return cvf::UNDEFINED_SIZE_T;
    }

    if ( m_readerInterface.notNull() )
    {
        size_t timeStepCount = this->infoForEachResultIndex()[scalarResultIndex].timeStepInfos().size();

        bool resultLoadingSucess = true;

        if ( type == RiaDefines::DYNAMIC_NATIVE && timeStepCount > 0 )
        {
            m_cellScalarResults[scalarResultIndex].resize( timeStepCount );

            std::vector<double>& values = m_cellScalarResults[scalarResultIndex][timeStepIndex];
            if ( values.size() == 0 )
            {
                if ( !m_readerInterface->dynamicResult( resultName, m_porosityModel, timeStepIndex, &values ) )
                {
                    resultLoadingSucess = false;
                }
            }
        }
        else if ( type == RiaDefines::STATIC_NATIVE )
        {
            m_cellScalarResults[scalarResultIndex].resize( 1 );

            std::vector<double>& values = m_cellScalarResults[scalarResultIndex][0];
            if ( !m_readerInterface->staticResult( resultName, m_porosityModel, &values ) )
            {
                resultLoadingSucess = false;
            }
        }

        if ( !resultLoadingSucess )
        {
            // Error logging
            CVF_ASSERT( false );
        }
    }

    // Handle SourSimRL reading

    if ( type == RiaDefines::SOURSIMRL )
    {
        RifReaderEclipseOutput* eclReader = dynamic_cast<RifReaderEclipseOutput*>( m_readerInterface.p() );
        if ( eclReader )
        {
            size_t timeStepCount = this->infoForEachResultIndex()[scalarResultIndex].timeStepInfos().size();

            m_cellScalarResults[scalarResultIndex].resize( timeStepCount );

            std::vector<double>& values = m_cellScalarResults[scalarResultIndex][timeStepIndex];

            if ( values.size() == 0 )
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
    // Compute SGAS based on SWAT if the simulation contains no oil
    testAndComputeSgasForTimeStep( timeStepIndex );

    RigEclipseResultAddress SWATAddr( RiaDefines::DYNAMIC_NATIVE, "SWAT" );
    RigEclipseResultAddress SGASAddr( RiaDefines::DYNAMIC_NATIVE, "SGAS" );
    RigEclipseResultAddress SSOLAddr( RiaDefines::DYNAMIC_NATIVE, "SSOL" );

    size_t scalarIndexSWAT = findOrLoadKnownScalarResultForTimeStep( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE,
                                                                                              "SWAT" ),
                                                                     timeStepIndex );
    size_t scalarIndexSGAS = findOrLoadKnownScalarResultForTimeStep( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE,
                                                                                              "SGAS" ),
                                                                     timeStepIndex );
    size_t scalarIndexSSOL = findOrLoadKnownScalarResultForTimeStep( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE,
                                                                                              "SSOL" ),
                                                                     timeStepIndex );

    // Early exit if none of SWAT or SGAS is present
    if ( scalarIndexSWAT == cvf::UNDEFINED_SIZE_T && scalarIndexSGAS == cvf::UNDEFINED_SIZE_T )
    {
        return;
    }

    size_t soilResultValueCount = 0;
    size_t soilTimeStepCount    = 0;

    if ( scalarIndexSWAT != cvf::UNDEFINED_SIZE_T )
    {
        const std::vector<double>& swatForTimeStep = this->cellScalarResults( SWATAddr, timeStepIndex );
        if ( swatForTimeStep.size() > 0 )
        {
            soilResultValueCount = swatForTimeStep.size();
            soilTimeStepCount    = this->infoForEachResultIndex()[scalarIndexSWAT].timeStepInfos().size();
        }
    }

    if ( scalarIndexSGAS != cvf::UNDEFINED_SIZE_T )
    {
        const std::vector<double>& sgasForTimeStep = this->cellScalarResults( SGASAddr, timeStepIndex );
        if ( sgasForTimeStep.size() > 0 )
        {
            soilResultValueCount = qMax( soilResultValueCount, sgasForTimeStep.size() );

            size_t sgasTimeStepCount = this->infoForEachResultIndex()[scalarIndexSGAS].timeStepInfos().size();
            soilTimeStepCount        = qMax( soilTimeStepCount, sgasTimeStepCount );
        }
    }

    // Make sure memory is allocated for the new SOIL results
    RigEclipseResultAddress SOILAddr( RiaDefines::DYNAMIC_NATIVE, "SOIL" );
    size_t                  soilResultScalarIndex = this->findScalarResultIndexFromAddress( SOILAddr );
    m_cellScalarResults[soilResultScalarIndex].resize( soilTimeStepCount );

    if ( this->cellScalarResults( SOILAddr, timeStepIndex ).size() > 0 )
    {
        // Data is computed and allocated, nothing more to do
        return;
    }

    m_cellScalarResults[soilResultScalarIndex][timeStepIndex].resize( soilResultValueCount );

    const std::vector<double>* swatForTimeStep = nullptr;
    const std::vector<double>* sgasForTimeStep = nullptr;
    const std::vector<double>* ssolForTimeStep = nullptr;

    if ( scalarIndexSWAT != cvf::UNDEFINED_SIZE_T )
    {
        swatForTimeStep = &( this->cellScalarResults( SWATAddr, timeStepIndex ) );
        if ( swatForTimeStep->size() == 0 )
        {
            swatForTimeStep = nullptr;
        }
    }

    if ( scalarIndexSGAS != cvf::UNDEFINED_SIZE_T )
    {
        sgasForTimeStep = &( this->cellScalarResults( SGASAddr, timeStepIndex ) );
        if ( sgasForTimeStep->size() == 0 )
        {
            sgasForTimeStep = nullptr;
        }
    }

    if ( scalarIndexSSOL != cvf::UNDEFINED_SIZE_T )
    {
        ssolForTimeStep = &( this->cellScalarResults( SSOLAddr, timeStepIndex ) );
        if ( ssolForTimeStep->size() == 0 )
        {
            ssolForTimeStep = nullptr;
        }
    }

    std::vector<double>* soilForTimeStep = this->modifiableCellScalarResult( SOILAddr, timeStepIndex );

#pragma omp parallel for
    for ( int idx = 0; idx < static_cast<int>( soilResultValueCount ); idx++ )
    {
        double soilValue = 1.0;
        if ( sgasForTimeStep )
        {
            soilValue -= sgasForTimeStep->at( idx );
        }

        if ( swatForTimeStep )
        {
            soilValue -= swatForTimeStep->at( idx );
        }

        if ( ssolForTimeStep )
        {
            soilValue -= ssolForTimeStep->at( idx );
        }

        soilForTimeStep->at( idx ) = soilValue;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::testAndComputeSgasForTimeStep( size_t timeStepIndex )
{
    size_t scalarIndexSWAT = findOrLoadKnownScalarResultForTimeStep( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE,
                                                                                              "SWAT" ),
                                                                     timeStepIndex );
    if ( scalarIndexSWAT == cvf::UNDEFINED_SIZE_T )
    {
        return;
    }

    if ( m_readerInterface.isNull() ) return;

    std::set<RiaDefines::PhaseType> phases = m_readerInterface->availablePhases();
    if ( phases.count( RiaDefines::GAS_PHASE ) == 0 ) return;
    if ( phases.count( RiaDefines::OIL_PHASE ) > 0 ) return;

    // Simulation type is gas and water. No SGAS is present, compute SGAS based on SWAT

    size_t scalarIndexSGAS = this->findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE,
                                                                                           "SGAS" ),
                                                                  false );
    if ( m_cellScalarResults[scalarIndexSGAS].size() > timeStepIndex )
    {
        std::vector<double>& values = m_cellScalarResults[scalarIndexSGAS][timeStepIndex];
        if ( values.size() > 0 ) return;
    }

    size_t swatResultValueCount = 0;
    size_t swatTimeStepCount    = 0;

    {
        std::vector<double>& swatForTimeStep = m_cellScalarResults[scalarIndexSWAT][timeStepIndex];
        if ( swatForTimeStep.size() > 0 )
        {
            swatResultValueCount = swatForTimeStep.size();
            swatTimeStepCount    = this->infoForEachResultIndex()[scalarIndexSWAT].timeStepInfos().size();
        }
    }

    m_cellScalarResults[scalarIndexSGAS].resize( swatTimeStepCount );

    if ( m_cellScalarResults[scalarIndexSGAS][timeStepIndex].size() > 0 )
    {
        return;
    }

    m_cellScalarResults[scalarIndexSGAS][timeStepIndex].resize( swatResultValueCount );

    std::vector<double>* swatForTimeStep = nullptr;

    {
        swatForTimeStep = &( m_cellScalarResults[scalarIndexSWAT][timeStepIndex] );
        if ( swatForTimeStep->size() == 0 )
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

    size_t depthResultIndex = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "DEPTH" ) );
    size_t dxResultIndex    = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "DX" ) );
    size_t dyResultIndex    = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "DY" ) );
    size_t dzResultIndex    = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "DZ" ) );
    size_t topsResultIndex = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "TOPS" ) );
    size_t bottomResultIndex = findOrLoadKnownScalarResult(
        RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "BOTTOM" ) );

    bool computeDepth  = false;
    bool computeDx     = false;
    bool computeDy     = false;
    bool computeDz     = false;
    bool computeTops   = false;
    bool computeBottom = false;

    if ( depthResultIndex == cvf::UNDEFINED_SIZE_T )
    {
        depthResultIndex = this->addStaticScalarResult( RiaDefines::STATIC_NATIVE, "DEPTH", false, actCellCount );
        computeDepth     = true;
    }

    if ( dxResultIndex == cvf::UNDEFINED_SIZE_T )
    {
        dxResultIndex = this->addStaticScalarResult( RiaDefines::STATIC_NATIVE, "DX", false, actCellCount );
        computeDx     = true;
    }

    if ( dyResultIndex == cvf::UNDEFINED_SIZE_T )
    {
        dyResultIndex = this->addStaticScalarResult( RiaDefines::STATIC_NATIVE, "DY", false, actCellCount );
        computeDy     = true;
    }

    if ( dzResultIndex == cvf::UNDEFINED_SIZE_T )
    {
        dzResultIndex = this->addStaticScalarResult( RiaDefines::STATIC_NATIVE, "DZ", false, actCellCount );
        computeDz     = true;
    }

    if ( topsResultIndex == cvf::UNDEFINED_SIZE_T )
    {
        topsResultIndex = this->addStaticScalarResult( RiaDefines::STATIC_NATIVE, "TOPS", false, actCellCount );
        computeTops     = true;
    }

    if ( bottomResultIndex == cvf::UNDEFINED_SIZE_T )
    {
        bottomResultIndex = this->addStaticScalarResult( RiaDefines::STATIC_NATIVE, "BOTTOM", false, actCellCount );
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

    for ( size_t cellIdx = 0; cellIdx < m_ownerMainGrid->globalCellArray().size(); cellIdx++ )
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
            cvf::Vec3d cellWidth = cell.faceCenter( cvf::StructGridInterface::NEG_I ) -
                                   cell.faceCenter( cvf::StructGridInterface::POS_I );
            dx[0][resultIndex] = cellWidth.length();
        }

        if ( computeDy || isTemporaryGrid )
        {
            cvf::Vec3d cellWidth = cell.faceCenter( cvf::StructGridInterface::NEG_J ) -
                                   cell.faceCenter( cvf::StructGridInterface::POS_J );
            dy[0][resultIndex] = cellWidth.length();
        }

        if ( computeDz || isTemporaryGrid )
        {
            cvf::Vec3d cellWidth = cell.faceCenter( cvf::StructGridInterface::NEG_K ) -
                                   cell.faceCenter( cvf::StructGridInterface::POS_K );
            dz[0][resultIndex] = cellWidth.length();
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
typedef size_t ( *ResultIndexFunction )( const RigActiveCellInfo* activeCellinfo, size_t reservoirCellIndex );

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

    if ( riTransComponentResultName == RiaDefines::riTranXResultName() )
    {
        permCompName = "PERMX";
        faceId       = cvf::StructGridInterface::POS_I;
    }
    else if ( riTransComponentResultName == RiaDefines::riTranYResultName() )
    {
        permCompName = "PERMY";
        faceId       = cvf::StructGridInterface::POS_J;
    }
    else if ( riTransComponentResultName == RiaDefines::riTranZResultName() )
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

    size_t permResultIdx = findOrLoadKnownScalarResult(
        RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, permCompName ) );
    size_t ntgResultIdx = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "NTG" ) );

    bool hasNTGResults = ntgResultIdx != cvf::UNDEFINED_SIZE_T;

    // Get the result index of the output

    size_t riTransResultIdx = this->findScalarResultIndexFromAddress(
        RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, riTransComponentResultName ) );
    CVF_ASSERT( riTransResultIdx != cvf::UNDEFINED_SIZE_T );

    // Get the result count, to handle that one of them might be globally defined

    size_t permxResultValueCount = m_cellScalarResults[permResultIdx][0].size();
    size_t resultValueCount      = permxResultValueCount;
    if ( hasNTGResults )
    {
        size_t ntgResultValueCount = m_cellScalarResults[ntgResultIdx][0].size();
        resultValueCount           = CVF_MIN( permxResultValueCount, ntgResultValueCount );
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
        bool isPermUsingResIdx = this->isUsingGlobalActiveIndex(
            RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, permCompName ) );
        bool isTransUsingResIdx = this->isUsingGlobalActiveIndex(
            RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, riTransComponentResultName ) );
        bool isNtgUsingResIdx = false;
        if ( hasNTGResults )
        {
            isNtgUsingResIdx = this->isUsingGlobalActiveIndex(
                RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "NTG" ) );
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

    for ( size_t nativeResvCellIndex = 0; nativeResvCellIndex < m_ownerMainGrid->globalCellArray().size();
          nativeResvCellIndex++ )
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

            const RigFault* fault = grid->mainGrid()->findFaultFromCellIndexAndCellFace( nativeResvCellIndex, faceId );
            bool            isOnFault = fault;

            cvf::Vec3d faceAreaVec;
            cvf::Vec3d faceCenter;

            if ( isOnFault )
            {
                calculateConnectionGeometry( nativeCell, neighborCell, nodes, faceId, &faceAreaVec );
            }
            else
            {
                faceAreaVec = nativeCell.faceNormalWithAreaLenght( faceId );
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
                cvf::Vec3d centerToFace = neighborCell.faceCenter( cvf::StructGridInterface::oppositeFace( faceId ) ) -
                                          neighborCell.center();

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
    RigEclipseResultAddress riCombTransEclResAddr( RiaDefines::STATIC_NATIVE, RiaDefines::combinedRiTranResultName() );
    if ( m_ownerMainGrid->nncData()->staticConnectionScalarResult( riCombTransEclResAddr ) ) return;

    double cdarchy = darchysValue();

    // Get the needed result indices we depend on

    size_t permXResultIdx = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "PERMX" ) );
    size_t permYResultIdx = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "PERMY" ) );
    size_t permZResultIdx = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "PERMZ" ) );

    size_t ntgResultIdx = findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "NTG" ) );

    bool hasNTGResults = ntgResultIdx != cvf::UNDEFINED_SIZE_T;

    // Get all the actual result values

    std::vector<double>& permXResults       = m_cellScalarResults[permXResultIdx][0];
    std::vector<double>& permYResults       = m_cellScalarResults[permYResultIdx][0];
    std::vector<double>& permZResults       = m_cellScalarResults[permZResultIdx][0];
    std::vector<double>& riCombTransResults = m_ownerMainGrid->nncData()->makeStaticConnectionScalarResult(
        RiaDefines::propertyNameRiCombTrans() );
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
        bool isPermXUsingResIdx = this->isUsingGlobalActiveIndex(
            RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "PERMX" ) );
        bool isPermYUsingResIdx = this->isUsingGlobalActiveIndex(
            RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "PERMY" ) );
        bool isPermZUsingResIdx = this->isUsingGlobalActiveIndex(
            RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "PERMZ" ) );
        bool isNtgUsingResIdx = false;
        if ( hasNTGResults )
        {
            isNtgUsingResIdx = this->isUsingGlobalActiveIndex(
                RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, "NTG" ) );
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
    std::vector<RigConnection>& nncConnections = m_ownerMainGrid->nncData()->connections();
    for ( size_t connIdx = 0; connIdx < nncConnections.size(); connIdx++ )
    {
        size_t                             nativeResvCellIndex = nncConnections[connIdx].m_c1GlobIdx;
        size_t                             neighborResvCellIdx = nncConnections[connIdx].m_c2GlobIdx;
        cvf::StructGridInterface::FaceType faceId              = nncConnections[connIdx].m_c1Face;

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

        cvf::Vec3d faceAreaVec = cvf::Vec3d::ZERO;
        cvf::Vec3d faceCenter  = cvf::Vec3d::ZERO;

        // Polygon center
        const std::vector<cvf::Vec3d>& realPolygon = nncConnections[connIdx].m_polygon;
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

            halfCellTrans = halfCellTransmissibility( perm, ntg, centerToFace, faceAreaVec );
        }

        // Neighbor cell half cell transm
        {
            cvf::Vec3d centerToFace = neighborCell.faceCenter( cvf::StructGridInterface::oppositeFace( faceId ) ) -
                                      neighborCell.center();

            double perm = ( *permResults )[neighborCellPermResIdx];

            double ntg = 1.0;
            if ( hasNTGResults && faceId != cvf::StructGridInterface::POS_K )
            {
                size_t ntgResIdx = ( *ntgIdxFunc )( activeCellInfo, neighborResvCellIdx );
                ntg              = ( *ntgResults )[ntgResIdx];
            }

            neighborHalfCellTrans = halfCellTransmissibility( perm, ntg, centerToFace, -faceAreaVec );
        }

        double newtranTemp          = newtran( cdarchy, 1.0, halfCellTrans, neighborHalfCellTrans );
        riCombTransResults[connIdx] = newtranTemp;
    }
}

double riMult( double transResults, double riTransResults )
{
    if ( transResults == HUGE_VAL || riTransResults == HUGE_VAL ) return HUGE_VAL;

    // To make 0.0 values give 1.0 in mult value
    if ( cvf::Math::abs( riTransResults ) < 1e-12 )
    {
        if ( cvf::Math::abs( transResults ) < 1e-12 )
        {
            return 1.0;
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

    if ( riMultCompName == RiaDefines::riMultXResultName() )
    {
        riTransCompName = RiaDefines::riTranXResultName();
        transCompName   = "TRANX";
    }
    else if ( riMultCompName == RiaDefines::riMultYResultName() )
    {
        riTransCompName = RiaDefines::riTranYResultName();
        transCompName   = "TRANY";
    }
    else if ( riMultCompName == RiaDefines::riMultZResultName() )
    {
        riTransCompName = RiaDefines::riTranZResultName();
        transCompName   = "TRANZ";
    }
    else
    {
        CVF_ASSERT( false );
    }

    // Get the needed result indices we depend on

    size_t transResultIdx = findOrLoadKnownScalarResult(
        RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, transCompName ) );
    size_t riTransResultIdx = findOrLoadKnownScalarResult(
        RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, riTransCompName ) );

    // Get the result index of the output

    size_t riMultResultIdx = this->findScalarResultIndexFromAddress(
        RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, riMultCompName ) );
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
    auto riCombMultEclResAddr  = RigEclipseResultAddress( RiaDefines::STATIC_NATIVE,
                                                         RiaDefines::combinedRiMultResultName() );
    auto riCombTransEclResAddr = RigEclipseResultAddress( RiaDefines::STATIC_NATIVE,
                                                          RiaDefines::combinedRiTranResultName() );
    auto combTransEclResAddr   = RigEclipseResultAddress( RiaDefines::STATIC_NATIVE,
                                                        RiaDefines::combinedTransmissibilityResultName() );

    if ( m_ownerMainGrid->nncData()->staticConnectionScalarResult( riCombMultEclResAddr ) ) return;

    std::vector<double>& riMultResults = m_ownerMainGrid->nncData()->makeStaticConnectionScalarResult(
        RiaDefines::propertyNameRiCombMult() );

    const std::vector<double>* riTransResults = m_ownerMainGrid->nncData()->staticConnectionScalarResult(
        riCombTransEclResAddr );

    const std::vector<double>* transResults = m_ownerMainGrid->nncData()->staticConnectionScalarResult(
        combTransEclResAddr );

    m_ownerMainGrid->nncData()->setEclResultAddress( RiaDefines::propertyNameRiCombMult(), riCombMultEclResAddr );

    for ( size_t nncConIdx = 0; nncConIdx < riMultResults.size(); ++nncConIdx )
    {
        riMultResults[nncConIdx] = riMult( ( *transResults )[nncConIdx], ( *riTransResults )[nncConIdx] );
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

    if ( riTransByAreaCompResultName == RiaDefines::riAreaNormTranXResultName() )
    {
        transCompName = "TRANX";
        faceId        = cvf::StructGridInterface::POS_I;
    }
    else if ( riTransByAreaCompResultName == RiaDefines::riAreaNormTranYResultName() )
    {
        transCompName = "TRANY";
        faceId        = cvf::StructGridInterface::POS_J;
    }
    else if ( riTransByAreaCompResultName == RiaDefines::riAreaNormTranZResultName() )
    {
        transCompName = "TRANZ";
        faceId        = cvf::StructGridInterface::POS_K;
    }
    else
    {
        CVF_ASSERT( false );
    }

    // Get the needed result indices we depend on

    size_t tranCompScResIdx = findOrLoadKnownScalarResult(
        RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, transCompName ) );

    // Get the result index of the output

    size_t riTranByAreaScResIdx = this->findScalarResultIndexFromAddress(
        RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, riTransByAreaCompResultName ) );
    CVF_ASSERT( riTranByAreaScResIdx != cvf::UNDEFINED_SIZE_T );

    // Get the result count, to handle that one of them might be globally defined

    size_t resultValueCount = m_cellScalarResults[tranCompScResIdx][0].size();

    // Get all the actual result values

    const std::vector<double>& transResults         = m_cellScalarResults[tranCompScResIdx][0];
    std::vector<double>&       riTransByAreaResults = m_cellScalarResults[riTranByAreaScResIdx][0];

    // Set up output container to correct number of results

    riTransByAreaResults.resize( resultValueCount );

    // Prepare how to index the result values:

    bool isUsingResIdx = this->isUsingGlobalActiveIndex(
        RigEclipseResultAddress( RiaDefines::STATIC_NATIVE, transCompName ) );

    // Set up result index function pointers

    ResultIndexFunction resValIdxFunc = isUsingResIdx ? &reservoirActiveCellIndex : &directReservoirCellIndex;

    const RigActiveCellInfo*       activeCellInfo = this->activeCellInfo();
    const std::vector<cvf::Vec3d>& nodes          = m_ownerMainGrid->nodes();

    for ( size_t nativeResvCellIndex = 0; nativeResvCellIndex < m_ownerMainGrid->globalCellArray().size();
          nativeResvCellIndex++ )
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

            const RigFault* fault = grid->mainGrid()->findFaultFromCellIndexAndCellFace( nativeResvCellIndex, faceId );
            bool            isOnFault = fault;

            cvf::Vec3d faceAreaVec;

            if ( isOnFault )
            {
                calculateConnectionGeometry( nativeCell, neighborCell, nodes, faceId, &faceAreaVec );
            }
            else
            {
                faceAreaVec = nativeCell.faceNormalWithAreaLenght( faceId );
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
    auto riCombTransByAreaEclResAddr = RigEclipseResultAddress( RiaDefines::STATIC_NATIVE,
                                                                RiaDefines::combinedRiAreaNormTranResultName() );
    auto combTransEclResAddr         = RigEclipseResultAddress( RiaDefines::STATIC_NATIVE,
                                                        RiaDefines::combinedTransmissibilityResultName() );

    if ( m_ownerMainGrid->nncData()->staticConnectionScalarResult( riCombTransByAreaEclResAddr ) ) return;

    std::vector<double>& riAreaNormTransResults = m_ownerMainGrid->nncData()->makeStaticConnectionScalarResult(
        RiaDefines::propertyNameRiCombTransByArea() );

    m_ownerMainGrid->nncData()->setEclResultAddress( RiaDefines::propertyNameRiCombTransByArea(),
                                                     riCombTransByAreaEclResAddr );

    const std::vector<double>* transResults = m_ownerMainGrid->nncData()->staticConnectionScalarResult(
        combTransEclResAddr );

    const std::vector<RigConnection>& connections = m_ownerMainGrid->nncData()->connections();

    for ( size_t nncConIdx = 0; nncConIdx < riAreaNormTransResults.size(); ++nncConIdx )
    {
        const std::vector<cvf::Vec3d>& realPolygon   = connections[nncConIdx].m_polygon;
        cvf::Vec3d                     faceAreaVec   = cvf::GeometryTools::polygonAreaNormal3D( realPolygon );
        double                         areaOfOverlap = faceAreaVec.length();

        riAreaNormTransResults[nncConIdx] = ( *transResults )[nncConIdx] / areaOfOverlap;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeCompletionTypeForTimeStep( size_t timeStep )
{
    size_t completionTypeResultIndex = this->findScalarResultIndexFromAddress(
        RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, RiaDefines::completionTypeResultName() ) );

    if ( m_cellScalarResults[completionTypeResultIndex].size() < this->maxTimeStepCount() )
    {
        m_cellScalarResults[completionTypeResultIndex].resize( this->maxTimeStepCount() );
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
    size_t cellVolIdx = this->findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE,
                                                                                      RiaDefines::riCellVolumeResultName() ),
                                                             false );

    if ( m_cellScalarResults[cellVolIdx].empty() )
    {
        m_cellScalarResults[cellVolIdx].resize( 1 );
    }
    std::vector<double>& cellVolumeResults = m_cellScalarResults[cellVolIdx][0];

    size_t cellResultCount = m_activeCellInfo->reservoirCellResultCount();
    cellVolumeResults.resize( cellResultCount, std::numeric_limits<double>::infinity() );

#pragma omp parallel for
    for ( int nativeResvCellIndex = 0;
          nativeResvCellIndex < static_cast<int>( m_ownerMainGrid->globalCellArray().size() );
          nativeResvCellIndex++ )
    {
        size_t resultIndex = activeCellInfo()->cellResultIndex( nativeResvCellIndex );
        if ( resultIndex != cvf::UNDEFINED_SIZE_T )
        {
            const RigCell& cell = m_ownerMainGrid->globalCellArray()[nativeResvCellIndex];
            if ( !cell.subGrid() )
            {
                cellVolumeResults[resultIndex] = cell.volume();
            }
        }
    }

    // Clear oil volume so it will have to be recalculated.
    clearScalarResult( RiaDefines::DYNAMIC_NATIVE, RiaDefines::riOilVolumeResultName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeOilVolumes()
{
    size_t cellVolIdx = this->findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE,
                                                                                      RiaDefines::riCellVolumeResultName() ),
                                                             false );
    const std::vector<double>& cellVolumeResults = m_cellScalarResults[cellVolIdx][0];

    size_t soilIdx = this->findOrLoadKnownScalarResult( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE, "SOIL" ) );
    size_t oilVolIdx = this->findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::DYNAMIC_NATIVE,
                                                                                     RiaDefines::riOilVolumeResultName() ),
                                                            false );
    m_cellScalarResults[oilVolIdx].resize( this->maxTimeStepCount() );

    size_t cellResultCount = m_activeCellInfo->reservoirCellResultCount();
    for ( size_t timeStepIdx = 0; timeStepIdx < this->maxTimeStepCount(); timeStepIdx++ )
    {
        const std::vector<double>& soilResults      = m_cellScalarResults[soilIdx][timeStepIdx];
        std::vector<double>&       oilVolumeResults = m_cellScalarResults[oilVolIdx][timeStepIdx];
        oilVolumeResults.resize( cellResultCount, 0u );

#pragma omp parallel for
        for ( int nativeResvCellIndex = 0;
              nativeResvCellIndex < static_cast<int>( m_ownerMainGrid->globalCellArray().size() );
              nativeResvCellIndex++ )
        {
            size_t resultIndex = activeCellInfo()->cellResultIndex( nativeResvCellIndex );
            if ( resultIndex != cvf::UNDEFINED_SIZE_T )
            {
                CVF_ASSERT( soilResults.at( resultIndex ) <= 1.01 );
                oilVolumeResults[resultIndex] = std::max( 0.0,
                                                          soilResults.at( resultIndex ) *
                                                              cellVolumeResults.at( resultIndex ) );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::computeMobilePV()
{
    std::vector<double> porvDataTemp;
    std::vector<double> swcrDataTemp;
    std::vector<double> multpvDataTemp;

    const std::vector<double>* porvResults   = nullptr;
    const std::vector<double>* swcrResults   = nullptr;
    const std::vector<double>* multpvResults = nullptr;

    porvResults = RigCaseCellResultsData::getResultIndexableStaticResult( this->activeCellInfo(),
                                                                          this,
                                                                          "PORV",
                                                                          porvDataTemp );
    if ( !porvResults || porvResults->empty() )
    {
        RiaLogging::error( "Assumed PORV, but not data was found." );
        return;
    }

    swcrResults   = RigCaseCellResultsData::getResultIndexableStaticResult( this->activeCellInfo(),
                                                                          this,
                                                                          "SWCR",
                                                                          swcrDataTemp );
    multpvResults = RigCaseCellResultsData::getResultIndexableStaticResult( this->activeCellInfo(),
                                                                            this,
                                                                            "MULTPV",
                                                                            multpvDataTemp );

    size_t mobPVIdx = this->findOrCreateScalarResultIndex( RigEclipseResultAddress( RiaDefines::STATIC_NATIVE,
                                                                                    RiaDefines::mobilePoreVolumeName() ),
                                                           false );

    std::vector<double>& mobPVResults = m_cellScalarResults[mobPVIdx][0];

    // Set up output container to correct number of results
    mobPVResults.resize( porvResults->size() );

    if ( multpvResults && swcrResults )
    {
        for ( size_t vIdx = 0; vIdx < porvResults->size(); ++vIdx )
        {
            mobPVResults[vIdx] = ( *multpvResults )[vIdx] * ( *porvResults )[vIdx] * ( 1.0 - ( *swcrResults )[vIdx] );
        }
    }
    else if ( !multpvResults && swcrResults )
    {
        for ( size_t vIdx = 0; vIdx < porvResults->size(); ++vIdx )
        {
            mobPVResults[vIdx] = ( *porvResults )[vIdx] * ( 1.0 - ( *swcrResults )[vIdx] );
        }
    }
    else if ( !swcrResults && multpvResults )
    {
        for ( size_t vIdx = 0; vIdx < porvResults->size(); ++vIdx )
        {
            mobPVResults[vIdx] = ( *multpvResults )[vIdx] * ( *porvResults )[vIdx];
        }
    }
    else
    {
        mobPVResults.assign( porvResults->begin(), porvResults->end() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::setReaderInterface( RifReaderInterface* readerInterface )
{
    m_readerInterface = readerInterface;
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

    size_t totalGlobCellCount = m_ownerMainGrid->globalCellArray().size();
    this->addStaticScalarResult( RiaDefines::FORMATION_NAMES,
                                 RiaDefines::activeFormationNamesResultName(),
                                 false,
                                 totalGlobCellCount );

    std::vector<double>* fnData =
        this->modifiableCellScalarResult( RigEclipseResultAddress( RiaDefines::FORMATION_NAMES,
                                                                   RiaDefines::activeFormationNamesResultName() ),
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

    computeAllenResults( this, m_ownerMainGrid );
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
RigAllenDiagramData* RigCaseCellResultsData::allenDiagramData()
{
    return m_allenDiagramData.p();
}

//--------------------------------------------------------------------------------------------------
///  If we have any results on any time step, assume we have loaded results
//--------------------------------------------------------------------------------------------------
bool RigCaseCellResultsData::isDataPresent( size_t scalarResultIndex ) const
{
    if ( scalarResultIndex >= this->resultCount() )
    {
        return false;
    }

    const std::vector<std::vector<double>>& data = m_cellScalarResults[scalarResultIndex];

    for ( size_t tsIdx = 0; tsIdx < data.size(); ++tsIdx )
    {
        if ( data[tsIdx].size() )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCaseCellResultsData::assignValuesToTemporaryLgrs( const QString&       resultName,
                                                          std::vector<double>& valuesForAllReservoirCells )
{
    CVF_ASSERT( m_activeCellInfo );

    static std::vector<QString> excludedProperties = {
        "MOBPROV",  "PORV",     "FIPOIL",   "FIPGAS",   "FIPWAT",   "FLROILI+", "FLROILJ+", "FLROILK+", "FLRGASI+",
        "FLRGASJ+", "FLRGASK+", "FLRWATI+", "FLRWATJ+", "FLRWATK+", "FLOOILI+", "FLOWATI+", "FLOGASI+", "FLOOILJ+",
        "FLOWATJ+", "FLOGASJ+", "FLOOILK+", "FLOWATK+", "FLOGASK+", "SFIPGAS",  "SFIPOIL",  "SFIPWAT",  "AREAX",
        "AREAY",    "AREAZ",    "DIFFX",    "DIFFY",    "DIFFZ",    "DZNET",    "HEATTX",   "HEATTY",   "HEATTZ",
        "LX",       "LY",       "LZ",       "MINPVV",   "TRANX",    "TRANY",    "TRANZ",
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
void RigCaseCellResultsData::computeAllenResults( RigCaseCellResultsData* cellResultsData, RigMainGrid* mainGrid )
{
    CVF_ASSERT( mainGrid );
    CVF_ASSERT( cellResultsData );

    auto fnNamesResAddr   = RigEclipseResultAddress( RiaDefines::FORMATION_NAMES,
                                                   RiaDefines::activeFormationNamesResultName() );
    bool hasFormationData = cellResultsData->hasResultEntry( fnNamesResAddr );

    if ( hasFormationData )
    {
        auto fnAllenResultResAddr = RigEclipseResultAddress( RiaDefines::ALLEN_DIAGRAMS,
                                                             RiaDefines::formationAllenResultName() );
        auto fnBinAllenResAddr    = RigEclipseResultAddress( RiaDefines::ALLEN_DIAGRAMS,
                                                          RiaDefines::formationBinaryAllenResultName() );

        // Create and retreive nnc result arrays

        std::vector<double>& fnAllenNncResults = mainGrid->nncData()->makeStaticConnectionScalarResult(
            RiaDefines::formationAllenResultName() );
        std::vector<double>& fnBinAllenNncResults = mainGrid->nncData()->makeStaticConnectionScalarResult(
            RiaDefines::formationBinaryAllenResultName() );

        // Associate them with eclipse result address

        mainGrid->nncData()->setEclResultAddress( RiaDefines::formationAllenResultName(), fnAllenResultResAddr );
        mainGrid->nncData()->setEclResultAddress( RiaDefines::formationBinaryAllenResultName(), fnBinAllenResAddr );

        const std::vector<double>& fnData = cellResultsData->cellScalarResults( fnNamesResAddr, 0 );

        // Add a result entry for the special allen grid data (used only for the grid cells without nnc coverage)

        cellResultsData->addStaticScalarResult( RiaDefines::ALLEN_DIAGRAMS,
                                                RiaDefines::formationAllenResultName(),
                                                false,
                                                fnData.size() );
        cellResultsData->addStaticScalarResult( RiaDefines::ALLEN_DIAGRAMS,
                                                RiaDefines::formationBinaryAllenResultName(),
                                                false,
                                                fnData.size() );

        std::vector<double>* alData    = cellResultsData->modifiableCellScalarResult( fnAllenResultResAddr, 0 );
        std::vector<double>* binAlData = cellResultsData->modifiableCellScalarResult( fnBinAllenResAddr, 0 );

        ( *alData ) = fnData;

        for ( double& val : ( *binAlData ) )
        {
            val = 0.0;
        }

        size_t formationCount = 0;
        if ( cellResultsData->activeFormationNames() )
        {
            formationCount = cellResultsData->activeFormationNames()->formationNames().size();
        }

        const std::vector<RigConnection>& nncConnections = mainGrid->nncData()->connections();

        std::map<std::pair<int, int>, int> formationCombinationToCategory;
        for ( size_t i = 0; i < nncConnections.size(); i++ )
        {
            const auto& c = nncConnections[i];

            size_t globCellIdx1 = c.m_c1GlobIdx;
            size_t globCellIdx2 = c.m_c2GlobIdx;

            int formation1 = (int)( fnData[globCellIdx1] );
            int formation2 = (int)( fnData[globCellIdx2] );

            int category = -1;
            if ( formation1 != formation2 )
            {
                if ( formation1 < formation2 )
                {
                    std::swap( formation1, formation2 );
                }

                auto formationCombination = std::make_pair( formation1, formation2 );

                auto existingCategory = formationCombinationToCategory.find( formationCombination );
                if ( existingCategory != formationCombinationToCategory.end() )
                {
                    category = existingCategory->second;
                }
                else
                {
                    category = static_cast<int>( formationCombinationToCategory.size() + formationCount );

                    formationCombinationToCategory[formationCombination] = category;
                }
                fnBinAllenNncResults[i] = 1.0;
            }
            else
            {
                category                = formation1;
                fnBinAllenNncResults[i] = 0.0;
            }

            fnAllenNncResults[i] = category;
        }

        cellResultsData->allenDiagramData()->setFormationCombinationToCategorymap( formationCombinationToCategory );
    }
    else
    {
#if 0
        for ( size_t i = 0; i < mainGrid->nncData()->connections().size(); i++ )
        {
            const auto& c = mainGrid->nncData()->connections()[i];

            size_t globCellIdx1 = c.m_c1GlobIdx;
            size_t globCellIdx2 = c.m_c2GlobIdx;

            size_t i1, j1, k1;
            mainGrid->ijkFromCellIndex( globCellIdx1, &i1, &j1, &k1 );

            size_t i2, j2, k2;
            mainGrid->ijkFromCellIndex( globCellIdx2, &i2, &j2, &k2 );

            double binaryValue = 0.0;
            if ( k1 != k2 )
            {
                binaryValue = 1.0;
            }

            fnAllenNncResults[i]        = k1;
            allAllenFormationResults[i] = k1;
            fnBinAllenNncResults[i]     = binaryValue;
        }
#endif
    }
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
    else if ( resVarAddr.m_resultCatType == RiaDefines::UNDEFINED )
    {
        RigEclipseResultAddress resVarAddressWithType = resVarAddr;

        resVarAddressWithType.m_resultCatType = RiaDefines::STATIC_NATIVE;

        size_t scalarResultIndex = this->findScalarResultIndexFromAddress( resVarAddressWithType );

        if ( scalarResultIndex == cvf::UNDEFINED_SIZE_T )
        {
            resVarAddressWithType.m_resultCatType = RiaDefines::DYNAMIC_NATIVE;
            scalarResultIndex                     = this->findScalarResultIndexFromAddress( resVarAddressWithType );
        }

        if ( scalarResultIndex == cvf::UNDEFINED_SIZE_T )
        {
            resVarAddressWithType.m_resultCatType = RiaDefines::SOURSIMRL;
            scalarResultIndex                     = this->findScalarResultIndexFromAddress( resVarAddressWithType );
        }

        if ( scalarResultIndex == cvf::UNDEFINED_SIZE_T )
        {
            resVarAddressWithType.m_resultCatType = RiaDefines::GENERATED;
            scalarResultIndex                     = this->findScalarResultIndexFromAddress( resVarAddressWithType );
        }

        if ( scalarResultIndex == cvf::UNDEFINED_SIZE_T )
        {
            resVarAddressWithType.m_resultCatType = RiaDefines::INPUT_PROPERTY;
            scalarResultIndex                     = this->findScalarResultIndexFromAddress( resVarAddressWithType );
        }

        if ( scalarResultIndex == cvf::UNDEFINED_SIZE_T )
        {
            resVarAddressWithType.m_resultCatType = RiaDefines::FORMATION_NAMES;
            scalarResultIndex                     = this->findScalarResultIndexFromAddress( resVarAddressWithType );
        }

        return scalarResultIndex;
    }
    else
    {
        std::vector<RigEclipseResultInfo>::const_iterator it;
        for ( it = m_resultInfos.begin(); it != m_resultInfos.end(); ++it )
        {
            if ( it->eclipseResultAddress() == resVarAddr )
            {
                return it->gridScalarResultIndex();
            }
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
    std::vector<RigEclipseResultAddress> resAddresses = mainCaseResultsData->results( poroModel )->existingResults();
    std::vector<RigEclipseTimeStepInfo>  timeStepInfos =
        mainCaseResultsData->results( poroModel )->timeStepInfos( resAddresses[0] );

    const std::vector<RigEclipseResultInfo> resultInfos =
        mainCaseResultsData->results( poroModel )->infoForEachResultIndex();

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

                std::vector<std::vector<double>>* dataValues = cellResultsStorage->modifiableCellScalarResultTimesteps(
                    resVarAddr );
                dataValues->resize( timeStepInfos.size() );
            }
        }

        cellResultsStorage->createPlaceholderResultEntries();
    }
}
