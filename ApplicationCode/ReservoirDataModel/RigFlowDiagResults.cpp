/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RigFlowDiagResults.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigFlowDiagSolverInterface.h"
#include "RigFlowDiagStatCalc.h"
#include "RigMainGrid.h"

#include "RigFlowDiagResultFrames.h"
#include "RigNumberOfFloodedPoreVolumesCalculator.h"
#include "RigStatisticsDataCache.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimFlowDiagSolution.h"

#include <cmath> // Needed for HUGE_VAL on Linux

namespace caf
{
template <>
void RigFlowDiagResults::CellFilterEnum::setUp()
{
    addItem( RigFlowDiagResults::CELLS_ACTIVE, "CELLS_ACTIVE", "All Active Cells" );
    addItem( RigFlowDiagResults::CELLS_VISIBLE, "CELLS_VISIBLE", "Visible Cells" );
    addItem( RigFlowDiagResults::CELLS_COMMUNICATION, "CELLS_COMMUNICATION", "Injector Producer Communication" );
    addItem( RigFlowDiagResults::CELLS_FLOODED, "CELLS_FLOODED", "Flooded by Injector" );
    addItem( RigFlowDiagResults::CELLS_DRAINED, "CELLS_DRAINED", "Drained by Producer" );
    setDefault( RigFlowDiagResults::CELLS_ACTIVE );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFlowDiagResults::RigFlowDiagResults( RimFlowDiagSolution* flowSolution, size_t timeStepCount )
    : m_flowDiagSolution( flowSolution )
{
    m_timeStepCount = timeStepCount;
    m_hasAtemptedNativeResults.resize( timeStepCount );
    m_injProdPairFluxCommunicationTimesteps.resize( timeStepCount );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFlowDiagResults::~RigFlowDiagResults()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>* RigFlowDiagResults::resultValues( const RigFlowDiagResultAddress& resVarAddr,
                                                             size_t                          timeStepIndex )
{
    CVF_ASSERT( m_timeStepCount != cvf::UNDEFINED_SIZE_T ); // Forgotten to call init

    return findOrCalculateResult( resVarAddr, timeStepIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigActiveCellInfo* RigFlowDiagResults::activeCellInfo( const RigFlowDiagResultAddress& resVarAddr )
{
    RimEclipseResultCase* eclCase;
    m_flowDiagSolution->firstAncestorOrThisOfType( eclCase );

    return eclCase->eclipseCaseData()->activeCellInfo( RiaDefines::MATRIX_MODEL ); // Todo: base on resVarAddr member
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>* RigFlowDiagResults::findOrCalculateResult( const RigFlowDiagResultAddress& resVarAddr,
                                                                      size_t                          timeStepIndex )
{
    std::vector<double>* frameData = findScalarResultFrame( resVarAddr, timeStepIndex );

    if ( frameData ) return frameData;

    frameData = calculateDerivedResult( resVarAddr, timeStepIndex );

    if ( frameData ) return frameData;

    // We need to access the native data from the opm solver

    if ( !solverInterface() ) return nullptr;

    calculateNativeResultsIfNotPreviouslyAttempted( timeStepIndex, resVarAddr.phaseSelection );

    return findScalarResultFrame( resVarAddr, timeStepIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::calculateNativeResultsIfNotPreviouslyAttempted( size_t timeStepIndex,
                                                                         RigFlowDiagResultAddress::PhaseSelection phaseSelection )
{
    if ( timeStepIndex >= m_hasAtemptedNativeResults.size() ) return;

    auto it = m_hasAtemptedNativeResults[timeStepIndex].find( phaseSelection );
    if ( it == m_hasAtemptedNativeResults[timeStepIndex].end() || !it->second )
    {
        RigFlowDiagTimeStepResult nativeTimestepResults =
            solverInterface()->calculate( timeStepIndex,
                                          phaseSelection,
                                          m_flowDiagSolution->allInjectorTracerActiveCellIndices( timeStepIndex ),
                                          m_flowDiagSolution->allProducerTracerActiveCellIndices( timeStepIndex ) );

        std::map<RigFlowDiagResultAddress, std::vector<double>>& nativeResults = nativeTimestepResults.nativeResults();

        for ( auto& resIt : nativeResults )
        {
            RigFlowDiagResultFrames* nativeResFrames = findScalarResult( resIt.first );
            if ( !nativeResFrames ) nativeResFrames = createScalarResult( resIt.first );

            nativeResFrames->frameData( timeStepIndex ).swap( resIt.second );
        }

        m_injProdPairFluxCommunicationTimesteps[timeStepIndex][phaseSelection].swap(
            nativeTimestepResults.injProdWellPairFluxes() );

        m_hasAtemptedNativeResults[timeStepIndex][phaseSelection] = true;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>* RigFlowDiagResults::findScalarResultFrame( const RigFlowDiagResultAddress& resVarAddr,
                                                                size_t                          timeStepIndex )
{
    RigFlowDiagResultFrames* resFrames = findScalarResult( resVarAddr );

    if ( resFrames )
    {
        std::vector<double>& frame = resFrames->frameData( timeStepIndex );
        if ( frame.size() ) return ( &frame );
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFlowDiagSolverInterface* RigFlowDiagResults::solverInterface()
{
    RimEclipseResultCase* eclCase;
    m_flowDiagSolution->firstAncestorOrThisOfType( eclCase );

    return eclCase->flowDiagSolverInterface();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFlowDiagResultFrames* RigFlowDiagResults::createScalarResult( const RigFlowDiagResultAddress& resVarAddr )
{
    cvf::ref<RigFlowDiagResultFrames> newFrameSet = new RigFlowDiagResultFrames( m_timeStepCount );
    m_resultSets[resVarAddr]                      = newFrameSet;

    return newFrameSet.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFlowDiagResultFrames* RigFlowDiagResults::findScalarResult( const RigFlowDiagResultAddress& resVarAddr )
{
    decltype( m_resultSets )::iterator it = m_resultSets.find( resVarAddr );

    if ( it == m_resultSets.end() ) return nullptr;

    return it->second.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>* RigFlowDiagResults::calculateDerivedResult( const RigFlowDiagResultAddress& resVarAddr,
                                                                 size_t                          timeStepIndex )
{
    if ( resVarAddr.isNativeResult() ) return nullptr;

    if ( resVarAddr.variableName == RIG_FLD_TOF_RESNAME )
    {
        return calculateAverageTOFResult( resVarAddr, timeStepIndex );
    }
    else if ( resVarAddr.variableName == RIG_FLD_CELL_FRACTION_RESNAME )
    {
        return calculateSumOfFractionsResult( resVarAddr, timeStepIndex );
    }
    else if ( resVarAddr.variableName == RIG_FLD_COMMUNICATION_RESNAME )
    {
        return calculateCommunicationResult( resVarAddr, timeStepIndex );
    }
    else if ( resVarAddr.variableName == RIG_FLD_MAX_FRACTION_TRACER_RESNAME )
    {
        return calculateTracerWithMaxFractionResult( resVarAddr, timeStepIndex );
    }
    else if ( resVarAddr.variableName == RIG_NUM_FLOODED_PV )
    {
        calculateNumFloodedPV( resVarAddr );
        return findScalarResultFrame( resVarAddr, timeStepIndex );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>* RigFlowDiagResults::calculateAverageTOFResult( const RigFlowDiagResultAddress& resVarAddr,
                                                                    size_t                          timeStepIndex )
{
    std::vector<const std::vector<double>*> injectorTOFs =
        findResultsForSelectedTracers( resVarAddr, timeStepIndex, RIG_FLD_TOF_RESNAME, RimFlowDiagSolution::INJECTOR );
    std::vector<const std::vector<double>*> injectorFractions =
        findResultsForSelectedTracers( resVarAddr, timeStepIndex, RIG_FLD_CELL_FRACTION_RESNAME, RimFlowDiagSolution::INJECTOR );

    std::vector<const std::vector<double>*> producerTOFs =
        findResultsForSelectedTracers( resVarAddr, timeStepIndex, RIG_FLD_TOF_RESNAME, RimFlowDiagSolution::PRODUCER );
    std::vector<const std::vector<double>*> producerFractions =
        findResultsForSelectedTracers( resVarAddr, timeStepIndex, RIG_FLD_CELL_FRACTION_RESNAME, RimFlowDiagSolution::PRODUCER );
    size_t activeCellCount = this->activeCellInfo( resVarAddr )->reservoirActiveCellCount();

    std::vector<double> injectorTotalFractions;
    std::vector<double> injectorFractMultTof;
    calculateSumOfFractionAndFractionMultTOF( activeCellCount,
                                              injectorFractions,
                                              injectorTOFs,
                                              &injectorTotalFractions,
                                              &injectorFractMultTof );

    std::vector<double> producerTotalFractions;
    std::vector<double> producerFractMultTof;
    calculateSumOfFractionAndFractionMultTOF( activeCellCount,
                                              producerFractions,
                                              producerTOFs,
                                              &producerTotalFractions,
                                              &producerFractMultTof );

    RigFlowDiagResultFrames* averageTofFrames = this->createScalarResult( resVarAddr );
    std::vector<double>&     averageTof       = averageTofFrames->frameData( timeStepIndex );
    averageTof.resize( activeCellCount, HUGE_VAL );

    for ( size_t acIdx = 0; acIdx < activeCellCount; ++acIdx )
    {
        if ( injectorTotalFractions[acIdx] == 0.0 && producerTotalFractions[acIdx] == 0.0 )
        {
            averageTof[acIdx] = HUGE_VAL;
        }
        else
        {
            double retVal = 0.0;
            if ( injectorTotalFractions[acIdx] != 0.0 )
                retVal += ( 1.0 / injectorTotalFractions[acIdx] ) * injectorFractMultTof[acIdx];
            if ( producerTotalFractions[acIdx] != 0.0 )
                retVal += ( 1.0 / producerTotalFractions[acIdx] ) * producerFractMultTof[acIdx];
            averageTof[acIdx] = retVal;
        }
    }

    /// Test to remove all averaging
    // if (injectorTOFs.size()) averageTof = (*injectorTOFs[0]);

    return &averageTof;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::calculateSumOfFractionAndFractionMultTOF( size_t activeCellCount,
                                                                   const std::vector<const std::vector<double>*>& fractions,
                                                                   const std::vector<const std::vector<double>*>& TOFs,
                                                                   std::vector<double>* sumOfFractions,
                                                                   std::vector<double>* fractionMultTOF )
{
    sumOfFractions->resize( activeCellCount, 0.0 );
    fractionMultTOF->resize( activeCellCount, 0.0 );

    for ( size_t iIdx = 0; iIdx < fractions.size(); ++iIdx )
    {
        const std::vector<double>* frInj  = fractions[iIdx];
        const std::vector<double>* tofInj = TOFs[iIdx];

        if ( !( frInj && tofInj ) ) continue;

        for ( size_t acIdx = 0; acIdx < activeCellCount; ++acIdx )
        {
            if ( ( *frInj )[acIdx] == HUGE_VAL ) continue;

            ( *sumOfFractions )[acIdx] += ( *frInj )[acIdx];
            ( *fractionMultTOF )[acIdx] += ( *frInj )[acIdx] * ( *tofInj )[acIdx];
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>* RigFlowDiagResults::calculateSumOfFractionsResult( const RigFlowDiagResultAddress& resVarAddr,
                                                                        size_t                          timeStepIndex )
{
    std::vector<const std::vector<double>*> fractions = findResultsForSelectedTracers( resVarAddr,
                                                                                       timeStepIndex,
                                                                                       RIG_FLD_CELL_FRACTION_RESNAME,
                                                                                       RimFlowDiagSolution::UNDEFINED );

    RigFlowDiagResultFrames* sumOfFractionsFrames = this->createScalarResult( resVarAddr );
    std::vector<double>&     sumOfFractions       = sumOfFractionsFrames->frameData( timeStepIndex );

    size_t activeCellCount = this->activeCellInfo( resVarAddr )->reservoirActiveCellCount();

    calculateSumOfFractions( fractions, activeCellCount, &sumOfFractions );

    return &sumOfFractions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>* RigFlowDiagResults::calculateTracerWithMaxFractionResult( const RigFlowDiagResultAddress& resVarAddr,
                                                                               size_t timeStepIndex )
{
    std::vector<std::pair<std::string, const std::vector<double>*>> fractions =
        findNamedResultsForSelectedTracers( resVarAddr,
                                            timeStepIndex,
                                            RIG_FLD_CELL_FRACTION_RESNAME,
                                            RimFlowDiagSolution::UNDEFINED );

    std::vector<int> resultTracerIdxToGlobalTracerIdx;
    {
        resultTracerIdxToGlobalTracerIdx.resize( fractions.size(), -1 );

        std::vector<QString> allTracerNames = m_flowDiagSolution->tracerNames();
        int                  selTracerIdx   = 0;
        for ( const auto& trNameFractionPair : fractions )
        {
            for ( size_t globIdx = 0; globIdx < allTracerNames.size(); ++globIdx )
            {
                if ( allTracerNames[globIdx].toStdString() == trNameFractionPair.first )
                {
                    resultTracerIdxToGlobalTracerIdx[selTracerIdx] = static_cast<int>( globIdx );
                    break;
                }
            }

            ++selTracerIdx;
        }
    }

    size_t activeCellCount = this->activeCellInfo( resVarAddr )->reservoirActiveCellCount();

    RigFlowDiagResultFrames* maxFractionTracerIdxFrames = this->createScalarResult( resVarAddr );
    std::vector<double>&     maxFractionTracerIdx       = maxFractionTracerIdxFrames->frameData( timeStepIndex );
    {
        maxFractionTracerIdx.resize( activeCellCount, HUGE_VAL );

        std::vector<double> maxFraction;
        maxFraction.resize( activeCellCount, -HUGE_VAL );

        for ( size_t frIdx = 0; frIdx < fractions.size(); ++frIdx )
        {
            const std::vector<double>* fr = fractions[frIdx].second;

            if ( !fr ) continue;

            for ( size_t acIdx = 0; acIdx < activeCellCount; ++acIdx )
            {
                if ( ( *fr )[acIdx] == HUGE_VAL ) continue;

                if ( maxFraction[acIdx] < ( *fr )[acIdx] )
                {
                    maxFraction[acIdx]          = ( *fr )[acIdx];
                    maxFractionTracerIdx[acIdx] = resultTracerIdxToGlobalTracerIdx[frIdx];
                }
            }
        }
    }

    return &maxFractionTracerIdx;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>* RigFlowDiagResults::calculateCommunicationResult( const RigFlowDiagResultAddress& resVarAddr,
                                                                       size_t                          timeStepIndex )
{
    std::vector<const std::vector<double>*> injectorFractions =
        findResultsForSelectedTracers( resVarAddr, timeStepIndex, RIG_FLD_CELL_FRACTION_RESNAME, RimFlowDiagSolution::INJECTOR );
    std::vector<const std::vector<double>*> producerFractions =
        findResultsForSelectedTracers( resVarAddr, timeStepIndex, RIG_FLD_CELL_FRACTION_RESNAME, RimFlowDiagSolution::PRODUCER );
    size_t activeCellCount = this->activeCellInfo( resVarAddr )->reservoirActiveCellCount();

    std::vector<double> sumOfInjectorFractions;
    calculateSumOfFractions( injectorFractions, activeCellCount, &sumOfInjectorFractions );

    std::vector<double> sumOfProducerFractions;
    calculateSumOfFractions( producerFractions, activeCellCount, &sumOfProducerFractions );

    RigFlowDiagResultFrames* commFrames = this->createScalarResult( resVarAddr );
    std::vector<double>&     commPI     = commFrames->frameData( timeStepIndex );
    commPI.resize( activeCellCount, HUGE_VAL );

    for ( size_t acIdx = 0; acIdx < activeCellCount; ++acIdx )
    {
        if ( ( sumOfInjectorFractions )[acIdx] == HUGE_VAL ) continue;
        if ( ( sumOfProducerFractions )[acIdx] == HUGE_VAL ) continue;

        ( commPI )[acIdx] = ( sumOfInjectorFractions )[acIdx] * ( sumOfProducerFractions )[acIdx];
    }

    return &commPI;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::calculateNumFloodedPV( const RigFlowDiagResultAddress& resVarAddr )
{
    RimEclipseCase* eclipseCase;
    m_flowDiagSolution->firstAncestorOrThisOfTypeAsserted( eclipseCase );
    std::vector<QString> tracerNames;
    for ( const std::string& tracerName : resVarAddr.selectedTracerNames )
    {
        tracerNames.push_back( QString::fromUtf8( tracerName.c_str() ) );
    }
    RigNumberOfFloodedPoreVolumesCalculator calc( eclipseCase, tracerNames );

    RigFlowDiagResultFrames* frames = this->createScalarResult( resVarAddr );
    for ( size_t frameIdx = 0; frameIdx < m_timeStepCount; ++frameIdx )
    {
        std::vector<double>& frame = frames->frameData( frameIdx );

        frame.swap( calc.numberOfFloodedPorevolumes()[frameIdx] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const std::vector<double>*>
    RigFlowDiagResults::findResultsForSelectedTracers( const RigFlowDiagResultAddress&       resVarAddr,
                                                       size_t                                timeStepIndex,
                                                       const std::string&                    nativeResultName,
                                                       RimFlowDiagSolution::TracerStatusType wantedTracerType )
{
    std::vector<const std::vector<double>*> selectedTracersResults;

    for ( const std::string& tracerName : resVarAddr.selectedTracerNames )
    {
        RimFlowDiagSolution::TracerStatusType tracerType =
            m_flowDiagSolution->tracerStatusInTimeStep( QString::fromStdString( tracerName ), timeStepIndex );

        if ( tracerType != RimFlowDiagSolution::CLOSED &&
             ( tracerType == wantedTracerType || wantedTracerType == RimFlowDiagSolution::UNDEFINED ) )
        {
            selectedTracersResults.push_back(
                findOrCalculateResult( RigFlowDiagResultAddress( nativeResultName, resVarAddr.phaseSelection, tracerName ),
                                       timeStepIndex ) );
        }
    }

    return selectedTracersResults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<std::string, const std::vector<double>*>>
    RigFlowDiagResults::findNamedResultsForSelectedTracers( const RigFlowDiagResultAddress&       resVarAddr,
                                                            size_t                                timeStepIndex,
                                                            const std::string&                    nativeResultName,
                                                            RimFlowDiagSolution::TracerStatusType wantedTracerType )
{
    std::vector<std::pair<std::string, const std::vector<double>*>> selectedTracersResults;

    for ( const std::string& tracerName : resVarAddr.selectedTracerNames )
    {
        RimFlowDiagSolution::TracerStatusType tracerType =
            m_flowDiagSolution->tracerStatusInTimeStep( QString::fromStdString( tracerName ), timeStepIndex );

        if ( tracerType != RimFlowDiagSolution::CLOSED &&
             ( tracerType == wantedTracerType || wantedTracerType == RimFlowDiagSolution::UNDEFINED ) )
        {
            selectedTracersResults.push_back(
                std::make_pair( tracerName,
                                findOrCalculateResult( RigFlowDiagResultAddress( nativeResultName,
                                                                                 resVarAddr.phaseSelection,
                                                                                 tracerName ),
                                                       timeStepIndex ) ) );
        }
    }

    return selectedTracersResults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigStatisticsDataCache* RigFlowDiagResults::statistics( const RigFlowDiagResultAddress& resVarAddr )
{
    RigStatisticsDataCache* statCache = m_resultStatistics[resVarAddr].p();
    if ( !statCache )
    {
        RigFlowDiagStatCalc* calculator = new RigFlowDiagStatCalc( this, resVarAddr );
        statCache                       = new RigStatisticsDataCache( calculator );
        m_resultStatistics[resVarAddr]  = statCache;
    }

    return statCache;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::calculateSumOfFractions( const std::vector<const std::vector<double>*>& fractions,
                                                  size_t                                         activeCellCount,
                                                  std::vector<double>*                           sumOfFractions )
{
    sumOfFractions->resize( activeCellCount, HUGE_VAL );

    for ( size_t iIdx = 0; iIdx < fractions.size(); ++iIdx )
    {
        const std::vector<double>* fraction = fractions[iIdx];

        if ( !( fraction ) ) continue;

        for ( size_t acIdx = 0; acIdx < activeCellCount; ++acIdx )
        {
            if ( ( *fraction )[acIdx] == HUGE_VAL ) continue;

            if ( ( *sumOfFractions )[acIdx] == HUGE_VAL ) ( *sumOfFractions )[acIdx] = 0.0;

            ( *sumOfFractions )[acIdx] += ( *fraction )[acIdx];
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::minMaxScalarValues( const RigFlowDiagResultAddress& resVarAddr,
                                             int                             timeStepIndex,
                                             double*                         localMin,
                                             double*                         localMax )
{
    this->statistics( resVarAddr )->minMaxCellScalarValues( timeStepIndex, *localMin, *localMax );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::minMaxScalarValues( const RigFlowDiagResultAddress& resVarAddr, double* globalMin, double* globalMax )
{
    this->statistics( resVarAddr )->minMaxCellScalarValues( *globalMin, *globalMax );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::posNegClosestToZero( const RigFlowDiagResultAddress& resVarAddr,
                                              int                             timeStepIndex,
                                              double*                         localPosClosestToZero,
                                              double*                         localNegClosestToZero )
{
    this->statistics( resVarAddr )->posNegClosestToZero( timeStepIndex, *localPosClosestToZero, *localNegClosestToZero );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::posNegClosestToZero( const RigFlowDiagResultAddress& resVarAddr,
                                              double*                         globalPosClosestToZero,
                                              double*                         globalNegClosestToZero )
{
    this->statistics( resVarAddr )->posNegClosestToZero( *globalPosClosestToZero, *globalNegClosestToZero );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::meanScalarValue( const RigFlowDiagResultAddress& resVarAddr, double* meanValue )
{
    CVF_ASSERT( meanValue );

    this->statistics( resVarAddr )->meanCellScalarValues( *meanValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::meanScalarValue( const RigFlowDiagResultAddress& resVarAddr, int timeStepIndex, double* meanValue )
{
    this->statistics( resVarAddr )->meanCellScalarValues( timeStepIndex, *meanValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::p10p90ScalarValues( const RigFlowDiagResultAddress& resVarAddr, double* p10, double* p90 )
{
    this->statistics( resVarAddr )->p10p90CellScalarValues( *p10, *p90 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::p10p90ScalarValues( const RigFlowDiagResultAddress& resVarAddr,
                                             int                             timeStepIndex,
                                             double*                         p10,
                                             double*                         p90 )
{
    this->statistics( resVarAddr )->p10p90CellScalarValues( timeStepIndex, *p10, *p90 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::sumScalarValue( const RigFlowDiagResultAddress& resVarAddr, double* sum )
{
    CVF_ASSERT( sum );

    this->statistics( resVarAddr )->sumCellScalarValues( *sum );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::sumScalarValue( const RigFlowDiagResultAddress& resVarAddr, int timeStepIndex, double* sum )
{
    CVF_ASSERT( sum );

    this->statistics( resVarAddr )->sumCellScalarValues( timeStepIndex, *sum );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RigFlowDiagResults::scalarValuesHistogram( const RigFlowDiagResultAddress& resVarAddr )
{
    return this->statistics( resVarAddr )->cellScalarValuesHistogram();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RigFlowDiagResults::scalarValuesHistogram( const RigFlowDiagResultAddress& resVarAddr,
                                                                      int                             timeStepIndex )
{
    return this->statistics( resVarAddr )->cellScalarValuesHistogram( timeStepIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<int>& RigFlowDiagResults::uniqueCellScalarValues( const RigFlowDiagResultAddress& resVarAddr )
{
    return this->statistics( resVarAddr )->uniqueCellScalarValues();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<int>& RigFlowDiagResults::uniqueCellScalarValues( const RigFlowDiagResultAddress& resVarAddr,
                                                                    int                             timeStepIndex )
{
    return this->statistics( resVarAddr )->uniqueCellScalarValues( timeStepIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::mobileVolumeWeightedMean( const RigFlowDiagResultAddress& resVarAddr, int timeStepIndex, double* mean )
{
    this->statistics( resVarAddr )->mobileVolumeWeightedMean( timeStepIndex, *mean );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RigFlowDiagResults::injectorProducerPairFluxes( const std::string& injTracername,
                                                                          const std::string& prodTracerName,
                                                                          int                timeStepIndex )
{
    calculateNativeResultsIfNotPreviouslyAttempted( timeStepIndex, RigFlowDiagResultAddress::PHASE_ALL );

    auto commPair = m_injProdPairFluxCommunicationTimesteps[timeStepIndex][RigFlowDiagResultAddress::PHASE_ALL].find(
        std::make_pair( injTracername, prodTracerName ) );
    if ( commPair != m_injProdPairFluxCommunicationTimesteps[timeStepIndex][RigFlowDiagResultAddress::PHASE_ALL].end() )
    {
        return commPair->second;
    }
    else
    {
        return std::make_pair( 0.0, 0.0 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigFlowDiagResults::maxAbsPairFlux( int timeStepIndex )
{
    calculateNativeResultsIfNotPreviouslyAttempted( timeStepIndex, RigFlowDiagResultAddress::PHASE_ALL );
    double maxFlux = 0.0;

    if ( (size_t)timeStepIndex < m_injProdPairFluxCommunicationTimesteps.size() )
    {
        for ( const auto& commPair :
              m_injProdPairFluxCommunicationTimesteps[timeStepIndex][RigFlowDiagResultAddress::PHASE_ALL] )
        {
            if ( fabs( commPair.second.first ) > maxFlux ) maxFlux = fabs( commPair.second.first );
            if ( fabs( commPair.second.second ) > maxFlux ) maxFlux = fabs( commPair.second.second );
        }
    }

    return maxFlux;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RigFlowDiagResults::calculatedTimeSteps( RigFlowDiagResultAddress::PhaseSelection phaseSelection )
{
    std::vector<int> timestepIndices;
    for ( size_t tsIdx = 0; tsIdx < m_timeStepCount; ++tsIdx )
    {
        auto it = m_hasAtemptedNativeResults[tsIdx].find( phaseSelection );
        if ( it != m_hasAtemptedNativeResults[tsIdx].end() && it->second )
        {
            timestepIndices.push_back( static_cast<int>( tsIdx ) );
        }
    }

    return timestepIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFlowDiagSolverInterface::FlowCharacteristicsResultFrame
    RigFlowDiagResults::flowCharacteristicsResults( int                         timeStepIndex,
                                                    CellFilter                  cellSelection,
                                                    const std::vector<QString>& tracerNames,
                                                    double                      max_pv_fraction,
                                                    double                      minCommunication,
                                                    int                         maxTof )
{
    std::set<std::string> injectorNames;
    std::set<std::string> producerNames;

    for ( const QString& tracerName : tracerNames )
    {
        RimFlowDiagSolution::TracerStatusType status =
            m_flowDiagSolution->tracerStatusInTimeStep( tracerName, timeStepIndex );
        if ( status == RimFlowDiagSolution::INJECTOR )
        {
            injectorNames.insert( tracerName.toStdString() );
        }
        else if ( status == RimFlowDiagSolution::PRODUCER )
        {
            producerNames.insert( tracerName.toStdString() );
        }
    }

    RigFlowDiagResultAddress injectorAddress( RIG_FLD_TOF_RESNAME, RigFlowDiagResultAddress::PHASE_ALL, injectorNames );
    RigFlowDiagResultAddress producerAddress( RIG_FLD_TOF_RESNAME, RigFlowDiagResultAddress::PHASE_ALL, producerNames );

    const std::vector<double>* allInjectorResults = resultValues( injectorAddress, timeStepIndex );
    const std::vector<double>* allProducerResults = resultValues( producerAddress, timeStepIndex );

    std::vector<double> injectorResults;
    std::vector<double> producerResults;
    std::vector<size_t> selectedCellIndices;

    if ( cellSelection == CELLS_COMMUNICATION )
    {
        std::set<std::string> allTracers;
        allTracers.insert( injectorNames.begin(), injectorNames.end() );
        allTracers.insert( producerNames.begin(), producerNames.end() );

        RigFlowDiagResultAddress   communicationAddress( RIG_FLD_COMMUNICATION_RESNAME,
                                                       RigFlowDiagResultAddress::PHASE_ALL,
                                                       allTracers );
        const std::vector<double>* communicationResult = resultValues( communicationAddress, timeStepIndex );

        for ( size_t i = 0; i < communicationResult->size(); ++i )
        {
            if ( communicationResult->at( i ) != HUGE_VAL && communicationResult->at( i ) >= minCommunication )
            {
                selectedCellIndices.push_back( i );
                if ( allInjectorResults != nullptr ) injectorResults.push_back( allInjectorResults->at( i ) );
                if ( allProducerResults != nullptr ) producerResults.push_back( allProducerResults->at( i ) );
            }
        }
    }
    else if ( cellSelection == CELLS_FLOODED )
    {
        if ( allInjectorResults != nullptr )
        {
            for ( size_t i = 0; i < allInjectorResults->size(); ++i )
            {
                if ( allInjectorResults->at( i ) != HUGE_VAL && allInjectorResults->at( i ) <= maxTof )
                {
                    selectedCellIndices.push_back( i );
                    injectorResults.push_back( allInjectorResults->at( i ) );
                    if ( allProducerResults != nullptr )
                    {
                        producerResults.push_back( allProducerResults->at( i ) );
                    }
                    else
                    {
                        producerResults.push_back( 0 );
                    }
                }
            }
        }
    }
    else if ( cellSelection == CELLS_DRAINED )
    {
        if ( allProducerResults != nullptr )
        {
            for ( size_t i = 0; i < allProducerResults->size(); ++i )
            {
                if ( allProducerResults->at( i ) != HUGE_VAL && allProducerResults->at( i ) <= maxTof )
                {
                    selectedCellIndices.push_back( i );
                    producerResults.push_back( allProducerResults->at( i ) );
                    if ( allInjectorResults != nullptr )
                    {
                        injectorResults.push_back( allInjectorResults->at( i ) );
                    }
                    else
                    {
                        injectorResults.push_back( 0 );
                    }
                }
            }
        }
    }
    else
    {
        if ( allInjectorResults != nullptr ) injectorResults = *allInjectorResults;
        if ( allProducerResults != nullptr ) producerResults = *allProducerResults;

        for ( size_t i = 0; i < injectorResults.size(); ++i )
        {
            selectedCellIndices.push_back( i );
        }
    }

    return solverInterface()->calculateFlowCharacteristics( &injectorResults,
                                                            &producerResults,
                                                            selectedCellIndices,
                                                            max_pv_fraction );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFlowDiagSolverInterface::FlowCharacteristicsResultFrame
    RigFlowDiagResults::flowCharacteristicsResults( int                      timeStepIndex,
                                                    const std::vector<char>& visibleActiveCells,
                                                    double                   max_pv_fraction )
{
    std::vector<QString> tracerNames = m_flowDiagSolution->tracerNames();

    std::set<std::string> injectorNames;
    std::set<std::string> producerNames;

    for ( const QString& tracerName : tracerNames )
    {
        RimFlowDiagSolution::TracerStatusType status =
            m_flowDiagSolution->tracerStatusInTimeStep( tracerName, timeStepIndex );
        if ( status == RimFlowDiagSolution::INJECTOR )
        {
            injectorNames.insert( tracerName.toStdString() );
        }
        else if ( status == RimFlowDiagSolution::PRODUCER )
        {
            producerNames.insert( tracerName.toStdString() );
        }
    }

    RigFlowDiagResultAddress injectorAddress( RIG_FLD_TOF_RESNAME, RigFlowDiagResultAddress::PHASE_ALL, injectorNames );
    RigFlowDiagResultAddress producerAddress( RIG_FLD_TOF_RESNAME, RigFlowDiagResultAddress::PHASE_ALL, producerNames );

    const std::vector<double>* allInjectorResults = resultValues( injectorAddress, timeStepIndex );
    const std::vector<double>* allProducerResults = resultValues( producerAddress, timeStepIndex );

    std::vector<size_t> selectedCellIndices;
    std::vector<double> injectorResults;
    std::vector<double> producerResults;

    for ( size_t i = 0; i < visibleActiveCells.size(); ++i )
    {
        if ( visibleActiveCells[i] )
        {
            selectedCellIndices.push_back( i );
            injectorResults.push_back( allInjectorResults->at( i ) );
            producerResults.push_back( allProducerResults->at( i ) );
        }
    }

    return solverInterface()->calculateFlowCharacteristics( &injectorResults,
                                                            &producerResults,
                                                            selectedCellIndices,
                                                            max_pv_fraction );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFlowDiagSolution* RigFlowDiagResults::flowDiagSolution()
{
    {
        return m_flowDiagSolution;
    }
}
