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

#include "RimHistogramCalculator.h"

#include "RigEclipseMultiPropertyStatCalc.h"
#include "RigEclipseNativeVisibleCellsStatCalc.h"
#include "RigFemNativeVisibleCellsStatCalc.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFlowDiagResults.h"
#include "RigFlowDiagVisibleCellsStatCalc.h"
#include "RigGeoMechCaseData.h"

#include "RimEclipseCellColors.h"
#include "RimEclipseContourMapProjection.h"
#include "RimEclipseContourMapView.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechContourMapProjection.h"
#include "RimGeoMechContourMapView.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"

namespace caf
{
template <>
void caf::AppEnum<RimHistogramCalculator::StatisticsTimeRangeType>::setUp()
{
    addItem( RimHistogramCalculator::StatisticsTimeRangeType::ALL_TIMESTEPS, "ALL_TIMESTEPS", "All Time Steps" );
    addItem( RimHistogramCalculator::StatisticsTimeRangeType::CURRENT_TIMESTEP, "CURRENT_TIMESTEP", "Current Time Step" );
    setDefault( RimHistogramCalculator::StatisticsTimeRangeType::ALL_TIMESTEPS );
}
} // namespace caf
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

namespace caf
{
template <>
void caf::AppEnum<RimHistogramCalculator::StatisticsCellRangeType>::setUp()
{
    addItem( RimHistogramCalculator::StatisticsCellRangeType::ALL_CELLS, "ALL_CELLS", "All Active Cells" );
    addItem( RimHistogramCalculator::StatisticsCellRangeType::VISIBLE_CELLS, "VISIBLE_CELLS", "Visible Cells" );
    setDefault( RimHistogramCalculator::StatisticsCellRangeType::ALL_CELLS );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramCalculator::RimHistogramCalculator()
    : m_isVisCellStatUpToDate( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCalculator::invalidateVisibleCellsCache()
{
    m_isVisCellStatUpToDate = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramData RimHistogramCalculator::histogramData( RimEclipseContourMapView* contourMap )
{
    RimHistogramData histData;

    if ( contourMap )
    {
        bool isResultsInfoRelevant = contourMap->contourMapProjection()->numberOfValidCells() > 0u;

        if ( isResultsInfoRelevant )
        {
            histData.min  = contourMap->contourMapProjection()->minValue();
            histData.max  = contourMap->contourMapProjection()->maxValue();
            histData.mean = contourMap->contourMapProjection()->meanValue();
            histData.sum  = contourMap->contourMapProjection()->sumAllValues();
        }
    }
    return histData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramData RimHistogramCalculator::histogramData( RimGeoMechContourMapView* contourMap )
{
    RimHistogramData histData;

    if ( contourMap )
    {
        bool isResultsInfoRelevant = contourMap->contourMapProjection()->numberOfValidCells() > 0u;

        if ( isResultsInfoRelevant )
        {
            histData.min  = contourMap->contourMapProjection()->minValue();
            histData.max  = contourMap->contourMapProjection()->maxValue();
            histData.mean = contourMap->contourMapProjection()->meanValue();
            histData.sum  = contourMap->contourMapProjection()->sumAllValues();
        }
    }
    return histData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramData RimHistogramCalculator::histogramData( RimEclipseView*         eclipseView,
                                                        StatisticsCellRangeType cellRange,
                                                        StatisticsTimeRangeType timeRange )
{
    if ( eclipseView )
    {
        RimEclipseResultDefinition* eclResultDefinition = eclipseView->cellResult();
        bool isResultsInfoRelevant = eclipseView->hasUserRequestedAnimation() && eclResultDefinition->hasResult();

        if ( isResultsInfoRelevant )
        {
            RigEclipseResultAddress eclResAddr = eclResultDefinition->eclipseResultAddress();

            if ( eclResAddr.isValid() )
            {
                int currentTimeStep = eclipseView->currentTimeStep();
                if ( eclipseView->cellResult()->hasStaticResult() )
                {
                    currentTimeStep = 0;
                }

                return histogramData( eclipseView, eclResultDefinition, cellRange, timeRange, currentTimeStep );
            }
        }
    }

    //
    RimHistogramData data;
    return data;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramData RimHistogramCalculator::histogramData( RimEclipseView*             eclipseView,
                                                        RimEclipseResultDefinition* eclResultDefinition,
                                                        StatisticsCellRangeType     cellRange,
                                                        StatisticsTimeRangeType     timeRange,
                                                        int                         timeStep )
{
    CVF_ASSERT( eclResultDefinition );

    RimHistogramData histData;

    eclResultDefinition->loadResult();

    if ( eclResultDefinition->isFlowDiagOrInjectionFlooding() )
    {
        // All timesteps is ignored
        if ( timeRange == StatisticsTimeRangeType::CURRENT_TIMESTEP || timeRange == StatisticsTimeRangeType::ALL_TIMESTEPS )
        {
            if ( cellRange == StatisticsCellRangeType::ALL_CELLS )
            {
                RigFlowDiagResults*      fldResults = eclResultDefinition->flowDiagSolution()->flowDiagResults();
                RigFlowDiagResultAddress resAddr    = eclResultDefinition->flowDiagResAddress();

                fldResults->minMaxScalarValues( resAddr, timeStep, &histData.min, &histData.max );
                fldResults->p10p90ScalarValues( resAddr, timeStep, &histData.p10, &histData.p90 );
                fldResults->meanScalarValue( resAddr, timeStep, &histData.mean );
                fldResults->sumScalarValue( resAddr, timeStep, &histData.sum );
                fldResults->mobileVolumeWeightedMean( resAddr, timeStep, &histData.weightedMean );

                histData.histogram = &( fldResults->scalarValuesHistogram( resAddr, timeStep ) );
            }
            else if ( cellRange == StatisticsCellRangeType::VISIBLE_CELLS )
            {
                CVF_ASSERT( eclipseView );

                updateVisCellStatsIfNeeded( eclipseView, eclResultDefinition );

                m_visibleCellStatistics->meanCellScalarValues( timeStep, histData.mean );
                m_visibleCellStatistics->minMaxCellScalarValues( timeStep, histData.min, histData.max );
                m_visibleCellStatistics->p10p90CellScalarValues( timeStep, histData.p10, histData.p90 );
                m_visibleCellStatistics->sumCellScalarValues( timeStep, histData.sum );
                m_visibleCellStatistics->mobileVolumeWeightedMean( timeStep, histData.weightedMean );

                histData.histogram = &( m_visibleCellStatistics->cellScalarValuesHistogram( timeStep ) );
            }
        }
    }
    else if ( cellRange == StatisticsCellRangeType::ALL_CELLS )
    {
        RigEclipseResultAddress eclResAddr  = eclResultDefinition->eclipseResultAddress();
        RigCaseCellResultsData* cellResults = eclResultDefinition->currentGridCellResults();
        if ( timeRange == StatisticsTimeRangeType::ALL_TIMESTEPS )
        {
            cellResults->minMaxCellScalarValues( eclResAddr, histData.min, histData.max );
            cellResults->p10p90CellScalarValues( eclResAddr, histData.p10, histData.p90 );
            cellResults->meanCellScalarValues( eclResAddr, histData.mean );
            cellResults->sumCellScalarValues( eclResAddr, histData.sum );
            cellResults->mobileVolumeWeightedMean( eclResAddr, histData.weightedMean );
            histData.histogram = &( cellResults->cellScalarValuesHistogram( eclResAddr ) );
        }
        else if ( timeRange == StatisticsTimeRangeType::CURRENT_TIMESTEP )
        {
            cellResults->minMaxCellScalarValues( eclResAddr, timeStep, histData.min, histData.max );
            cellResults->p10p90CellScalarValues( eclResAddr, timeStep, histData.p10, histData.p90 );
            cellResults->meanCellScalarValues( eclResAddr, timeStep, histData.mean );
            cellResults->sumCellScalarValues( eclResAddr, timeStep, histData.sum );
            cellResults->mobileVolumeWeightedMean( eclResAddr, timeStep, histData.weightedMean );
            histData.histogram = &( cellResults->cellScalarValuesHistogram( eclResAddr, timeStep ) );
        }
    }
    else if ( cellRange == StatisticsCellRangeType::VISIBLE_CELLS )
    {
        CVF_ASSERT( eclipseView );
        updateVisCellStatsIfNeeded( eclipseView, eclResultDefinition );
        if ( timeRange == StatisticsTimeRangeType::ALL_TIMESTEPS )
        {
            // TODO: Only valid if we have no dynamic property filter
            m_visibleCellStatistics->meanCellScalarValues( histData.mean );
            m_visibleCellStatistics->minMaxCellScalarValues( histData.min, histData.max );
            m_visibleCellStatistics->p10p90CellScalarValues( histData.p10, histData.p90 );
            m_visibleCellStatistics->sumCellScalarValues( histData.sum );
            m_visibleCellStatistics->mobileVolumeWeightedMean( histData.weightedMean );

            histData.histogram = &( m_visibleCellStatistics->cellScalarValuesHistogram() );
        }
        else if ( timeRange == StatisticsTimeRangeType::CURRENT_TIMESTEP )
        {
            m_visibleCellStatistics->meanCellScalarValues( timeStep, histData.mean );
            m_visibleCellStatistics->minMaxCellScalarValues( timeStep, histData.min, histData.max );
            m_visibleCellStatistics->p10p90CellScalarValues( timeStep, histData.p10, histData.p90 );
            m_visibleCellStatistics->sumCellScalarValues( timeStep, histData.sum );
            m_visibleCellStatistics->mobileVolumeWeightedMean( timeStep, histData.weightedMean );

            histData.histogram = &( m_visibleCellStatistics->cellScalarValuesHistogram( timeStep ) );
        }
    }
    return histData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramData RimHistogramCalculator::histogramData( RimGeoMechView*         geoMechView,
                                                        StatisticsCellRangeType cellRange,
                                                        StatisticsTimeRangeType timeRange )
{
    RimHistogramData histData;

    if ( geoMechView )
    {
        RimGeoMechCase*     geoMechCase           = geoMechView->geoMechCase();
        RigGeoMechCaseData* caseData              = geoMechCase ? geoMechCase->geoMechData() : nullptr;
        bool                isResultsInfoRelevant = caseData && geoMechView->hasUserRequestedAnimation() &&
                                     geoMechView->cellResultResultDefinition()->hasResult();

        if ( isResultsInfoRelevant )
        {
            RigFemResultAddress resAddress = geoMechView->cellResultResultDefinition()->resultAddress();
            if ( cellRange == StatisticsCellRangeType::ALL_CELLS )
            {
                if ( timeRange == StatisticsTimeRangeType::ALL_TIMESTEPS )
                {
                    caseData->femPartResults()->meanScalarValue( resAddress, &histData.mean );
                    caseData->femPartResults()->minMaxScalarValues( resAddress, &histData.min, &histData.max );
                    caseData->femPartResults()->p10p90ScalarValues( resAddress, &histData.p10, &histData.p90 );
                    caseData->femPartResults()->sumScalarValue( resAddress, &histData.sum );

                    histData.histogram = &( caseData->femPartResults()->scalarValuesHistogram( resAddress ) );
                }
                else if ( timeRange == StatisticsTimeRangeType::CURRENT_TIMESTEP )
                {
                    int timeStepIdx = geoMechView->currentTimeStep();
                    caseData->femPartResults()->meanScalarValue( resAddress, timeStepIdx, &histData.mean );
                    caseData->femPartResults()->minMaxScalarValues( resAddress, timeStepIdx, &histData.min, &histData.max );
                    caseData->femPartResults()->p10p90ScalarValues( resAddress, timeStepIdx, &histData.p10, &histData.p90 );
                    caseData->femPartResults()->sumScalarValue( resAddress, timeStepIdx, &histData.sum );

                    histData.histogram = &( caseData->femPartResults()->scalarValuesHistogram( resAddress, timeStepIdx ) );
                }
            }
            else if ( cellRange == StatisticsCellRangeType::VISIBLE_CELLS )
            {
                this->updateVisCellStatsIfNeeded( geoMechView );

                if ( timeRange == StatisticsTimeRangeType::ALL_TIMESTEPS )
                {
                    // TODO: Only valid if we have no dynamic property filter
                    m_visibleCellStatistics->meanCellScalarValues( histData.mean );
                    m_visibleCellStatistics->minMaxCellScalarValues( histData.min, histData.max );
                    m_visibleCellStatistics->p10p90CellScalarValues( histData.p10, histData.p90 );
                    m_visibleCellStatistics->sumCellScalarValues( histData.sum );

                    histData.histogram = &( m_visibleCellStatistics->cellScalarValuesHistogram() );
                }
                else if ( timeRange == StatisticsTimeRangeType::CURRENT_TIMESTEP )
                {
                    int timeStepIdx = geoMechView->currentTimeStep();
                    m_visibleCellStatistics->meanCellScalarValues( timeStepIdx, histData.mean );
                    m_visibleCellStatistics->minMaxCellScalarValues( timeStepIdx, histData.min, histData.max );
                    m_visibleCellStatistics->p10p90CellScalarValues( timeStepIdx, histData.p10, histData.p90 );
                    m_visibleCellStatistics->sumCellScalarValues( timeStepIdx, histData.sum );

                    histData.histogram = &( m_visibleCellStatistics->cellScalarValuesHistogram( timeStepIdx ) );
                }
            }
        }
    }
    return histData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCalculator::updateVisCellStatsIfNeeded( RimGeoMechView* geoMechView )
{
    if ( m_isVisCellStatUpToDate ) return;

    cvf::ref<RigStatisticsCalculator> calc;
    if ( geoMechView )
    {
        RigFemResultAddress resAddress = geoMechView->cellResultResultDefinition()->resultAddress();
        calc = new RigFemNativeVisibleCellsStatCalc( geoMechView->geoMechCase()->geoMechData(),
                                                     resAddress,
                                                     geoMechView->currentTotalCellVisibility().p() );
    }

    m_visibleCellStatistics = new RigStatisticsDataCache( calc.p() );
    m_isVisCellStatUpToDate = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCalculator::updateVisCellStatsIfNeeded( RimEclipseView*             eclipseView,
                                                         RimEclipseResultDefinition* eclResultDefinition )
{
    CVF_ASSERT( eclipseView );
    CVF_ASSERT( eclResultDefinition );

    if ( m_isVisCellStatUpToDate ) return;

    eclipseView->cellResult()->loadResult();

    cvf::ref<RigStatisticsCalculator> calc;

    if ( eclResultDefinition->isFlowDiagOrInjectionFlooding() )
    {
        RigFlowDiagResultAddress resAddr    = eclResultDefinition->flowDiagResAddress();
        RigFlowDiagResults*      fldResults = eclResultDefinition->flowDiagSolution()->flowDiagResults();
        calc = new RigFlowDiagVisibleCellsStatCalc( fldResults, resAddr, eclipseView->currentTotalCellVisibility().p() );
    }
    else
    {
        RigEclipseResultAddress resAddr = eclResultDefinition->eclipseResultAddress();

        QString resultName = resAddr.m_resultName;

        std::vector<RigEclipseResultAddress> addresses = sourcesForMultiPropertyResults( resultName );
        if ( addresses.size() )
        {
            cvf::ref<RigEclipseMultiPropertyStatCalc> multicalc = new RigEclipseMultiPropertyStatCalc();

            for ( RigEclipseResultAddress& compResAddr : addresses )
            {
                cvf::ref<RigEclipseNativeVisibleCellsStatCalc> singleCalc =
                    new RigEclipseNativeVisibleCellsStatCalc( eclResultDefinition->currentGridCellResults(),
                                                              compResAddr,
                                                              eclipseView->currentTotalCellVisibility().p() );
                multicalc->addStatisticsCalculator( singleCalc.p() );
            }

            calc = multicalc;
        }
        else
        {
            calc = new RigEclipseNativeVisibleCellsStatCalc( eclResultDefinition->currentGridCellResults(),
                                                             resAddr,
                                                             eclipseView->currentTotalCellVisibility().p() );
        }
    }

    m_visibleCellStatistics = new RigStatisticsDataCache( calc.p() );
    m_isVisCellStatUpToDate = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigEclipseResultAddress> RimHistogramCalculator::sourcesForMultiPropertyResults( const QString& resultName )
{
    static const std::map<QString, std::vector<RigEclipseResultAddress>> resultsWithMultiPropertySource =
        { { RiaDefines::combinedTransmissibilityResultName(),
            { RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANX" ),
              RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANY" ),
              RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "TRANZ" ) } },
          { RiaDefines::combinedMultResultName(),
            { RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "MULTX" ),
              RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "MULTX-" ),
              RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "MULTY" ),
              RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "MULTY-" ),
              RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "MULTZ" ),
              RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, "MULTZ-" ) } },
          { RiaDefines::combinedRiTranResultName(),
            { RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaDefines::riTranXResultName() ),
              RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaDefines::riTranYResultName() ),
              RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaDefines::riTranZResultName() ) } },
          { RiaDefines::combinedRiMultResultName(),
            { RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaDefines::riMultXResultName() ),
              RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaDefines::riMultYResultName() ),
              RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaDefines::riMultZResultName() ) } },
          { RiaDefines::combinedRiAreaNormTranResultName(),
            { RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaDefines::riAreaNormTranXResultName() ),
              RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaDefines::riAreaNormTranYResultName() ),
              RigEclipseResultAddress( RiaDefines::ResultCatType::STATIC_NATIVE, RiaDefines::riAreaNormTranZResultName() ) } },
          { RiaDefines::combinedWaterFluxResultName(),
            { RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRWATI+" ),
              RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRWATJ+" ),
              RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRWATK+" ) } },
          { RiaDefines::combinedOilFluxResultName(),
            { RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLROILI+" ),
              RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLROILJ+" ),
              RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLROILK+" ) } },
          { RiaDefines::combinedGasFluxResultName(),
            { RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRGASI+" ),
              RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRGASJ+" ),
              RigEclipseResultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, "FLRGASK+" ) } } };

    auto resNameResultAddrsPairIt = resultsWithMultiPropertySource.find( resultName );

    if ( resNameResultAddrsPairIt != resultsWithMultiPropertySource.end() )
    {
        return resNameResultAddrsPairIt->second;
    }
    else if ( resultName.endsWith( "IJK" ) )
    {
        std::vector<RigEclipseResultAddress> resultAddrs;

        QString     baseName = resultName.left( resultName.size() - 3 );
        QStringList endings  = { "I", "J", "K" };

        for ( QString ending : endings )
        {
            resultAddrs.emplace_back( RigEclipseResultAddress( RiaDefines::ResultCatType::GENERATED, baseName + ending ) );
        }

        return resultAddrs;
    }
    else
    {
        return std::vector<RigEclipseResultAddress>();
    }
}
