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

#include "RigFlowDiagSolverInterface.h"

#include "RiaLogging.h"
#include "RiaResultNames.h"

#include "RifEclipseOutputFileTools.h"
#include "RifReaderEclipseOutput.h"
#include "RifReaderInterface.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigFlowDiagInterfaceTools.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimFlowDiagSolution.h"

#include "opm/flowdiagnostics/DerivedQuantities.hpp"

#include "opm/utility/ECLPropertyUnitConversion.hpp"
#include "opm/utility/ECLPvtCurveCollection.hpp"
#include "opm/utility/ECLSaturationFunc.hpp"

#include "cafProgressInfo.h"

#include "cvfTrace.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFlowDiagTimeStepResult::RigFlowDiagTimeStepResult( size_t activeCellCount )
    : m_activeCellCount( activeCellCount )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFlowDiagTimeStepResult::setTracerTOF( const std::string&                       tracerName,
                                              RigFlowDiagResultAddress::PhaseSelection phaseSelection,
                                              const std::map<int, double>&             cellValues )
{
    std::set<std::string> tracers;
    tracers.insert( tracerName );

    RigFlowDiagResultAddress resAddr( RigFlowDiagDefines::tofResultName().toStdString(), phaseSelection, tracers );

    addResult( resAddr, cellValues );

    std::vector<double>& activeCellValues = m_nativeResults[resAddr];
    for ( double& val : activeCellValues )
    {
        val = val * 1.15741e-5; // days pr second. Converting to days
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFlowDiagTimeStepResult::setTracerFraction( const std::string&                       tracerName,
                                                   RigFlowDiagResultAddress::PhaseSelection phaseSelection,
                                                   const std::map<int, double>&             cellValues )
{
    std::set<std::string> tracers;
    tracers.insert( tracerName );

    addResult( RigFlowDiagResultAddress( RigFlowDiagDefines::cellFractionResultName().toStdString(), phaseSelection, tracers ), cellValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFlowDiagTimeStepResult::setInjProdWellPairFlux( const std::string&               injectorTracerName,
                                                        const std::string&               producerTracerName,
                                                        const std::pair<double, double>& injProdFluxes )
{
    m_injProdWellPairFluxes[std::make_pair( injectorTracerName, producerTracerName )] = injProdFluxes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFlowDiagTimeStepResult::addResult( const RigFlowDiagResultAddress& resAddr, const std::map<int, double>& cellValues )
{
    std::vector<double>& activeCellValues = m_nativeResults[resAddr];

    CVF_ASSERT( activeCellValues.empty() );

    activeCellValues.resize( m_activeCellCount, HUGE_VAL );

    for ( const auto& pairIt : cellValues )
    {
        activeCellValues[pairIt.first] = pairIt.second;
    }
}

class RigOpmFlowDiagStaticData
{
public:
    RigOpmFlowDiagStaticData( const ecl_grid_type* mainGrid, const std::wstring& initFilename, RiaDefines::EclipseUnitSystem caseUnitSystem )
        : m_initData( initFilename )
    {
        try
        {
            if ( mainGrid )
            {
                m_eclGraph   = std::make_unique<Opm::ECLGraph>( Opm::ECLGraph::load( mainGrid, m_initData ) );
                m_poreVolume = m_eclGraph->poreVolume();
            }

            m_hasUnifiedRestartFile = false;

            m_eclSaturationFunc = std::make_unique<Opm::ECLSaturationFunc>( m_initData );
        }
        catch ( ... )
        {
            RiaLogging::warning( "Exception during initialization of relative permeability plotting functionality. "
                                 "Functionality will not be available." );
        }

        try
        {
            m_eclPvtCurveCollection = std::make_unique<Opm::ECLPVT::ECLPvtCurveCollection>( m_initData );
        }
        catch ( ... )
        {
            RiaLogging::warning( "Unsupported PVT table format. Could not initialize PVT plotting functionality." );
        }

        // Try and set output unit system to the same system as the eclipse case system
        std::unique_ptr<const Opm::ECLUnits::UnitSystem> eclUnitSystem;
        if ( caseUnitSystem == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
            eclUnitSystem = Opm::ECLUnits::metricUnitConventions();
        else if ( caseUnitSystem == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
            eclUnitSystem = Opm::ECLUnits::fieldUnitConventions();
        else if ( caseUnitSystem == RiaDefines::EclipseUnitSystem::UNITS_LAB )
            eclUnitSystem = Opm::ECLUnits::labUnitConventions();

        if ( eclUnitSystem )
        {
            if ( m_eclSaturationFunc )
            {
                m_eclSaturationFunc->setOutputUnits( eclUnitSystem->clone() );
            }
            if ( m_eclPvtCurveCollection )
            {
                m_eclPvtCurveCollection->setOutputUnits( eclUnitSystem->clone() );
            }
        }
    }

public:
    Opm::ECLInitFileData                           m_initData;
    std::unique_ptr<Opm::ECLGraph>                 m_eclGraph;
    std::vector<double>                            m_poreVolume;
    std::unique_ptr<Opm::FlowDiagnostics::Toolbox> m_fldToolbox;
    bool                                           m_hasUnifiedRestartFile;
    std::vector<Opm::ECLRestartData>               m_singleRestartDataTimeSteps;
    std::unique_ptr<Opm::ECLRestartData>           m_unifiedRestartData;

    std::unique_ptr<Opm::ECLSaturationFunc>             m_eclSaturationFunc;
    std::unique_ptr<Opm::ECLPVT::ECLPvtCurveCollection> m_eclPvtCurveCollection;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFlowDiagSolverInterface::RigFlowDiagSolverInterface( RimEclipseResultCase* eclipseCase )
    : m_eclipseCase( eclipseCase )
    , m_pvtCurveErrorCount( 0 )
    , m_relpermCurveErrorCount( 0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFlowDiagSolverInterface::~RigFlowDiagSolverInterface() = default;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string removeCrossFlowEnding( std::string tracerName )
{
    return RimFlowDiagSolution::removeCrossFlowEnding( QString::fromStdString( tracerName ) ).toStdString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool hasCrossFlowEnding( std::string tracerName )
{
    return RimFlowDiagSolution::hasCrossFlowEnding( QString::fromStdString( tracerName ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string addCrossFlowEnding( std::string tracerName )
{
    return RimFlowDiagSolution::addCrossFlowEnding( QString::fromStdString( tracerName ) ).toStdString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFlowDiagTimeStepResult RigFlowDiagSolverInterface::calculate( size_t                                   timeStepIndex,
                                                                 RigFlowDiagResultAddress::PhaseSelection phaseSelection,
                                                                 std::map<std::string, std::vector<int>>  injectorTracers,
                                                                 std::map<std::string, std::vector<int>>  producerTracers )
{
    using namespace Opm::FlowDiagnostics;

    RigFlowDiagTimeStepResult result(
        m_eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )->reservoirActiveCellCount() );

    caf::ProgressInfo progressInfo( 8, "Calculating Flow Diagnostics" );

    try
    {
        progressInfo.setProgressDescription( "Grid access" );

        if ( !ensureStaticDataObjectInstanceCreated() || !m_opmFlowDiagStaticData->m_eclGraph )
        {
            return result;
        }

        progressInfo.incrementProgress();
        progressInfo.setProgressDescription( "Calculating Connectivities" );

        CVF_ASSERT( m_opmFlowDiagStaticData != nullptr );
        const Opm::FlowDiagnostics::ConnectivityGraph connGraph =
            Opm::FlowDiagnostics::ConnectivityGraph{ static_cast<int>( m_opmFlowDiagStaticData->m_eclGraph->numCells() ),
                                                     m_opmFlowDiagStaticData->m_eclGraph->neighbours() };

        progressInfo.incrementProgress();
        progressInfo.setProgressDescription( "Initialize Solver" );

        // Create the Toolbox.

        m_opmFlowDiagStaticData->m_fldToolbox = std::make_unique<Opm::FlowDiagnostics::Toolbox>( connGraph );

        // Look for unified restart file
        QStringList m_filesWithSameBaseName;

        QString gridFileName = m_eclipseCase->gridFileName();
        if ( !RifEclipseOutputFileTools::findSiblingFilesWithSameBaseName( gridFileName, &m_filesWithSameBaseName ) ) return result;

        QString firstRestartFileName = RifEclipseOutputFileTools::firstFileNameOfType( m_filesWithSameBaseName, ECL_UNIFIED_RESTART_FILE );
        if ( !firstRestartFileName.isEmpty() )
        {
            m_opmFlowDiagStaticData->m_unifiedRestartData =
                std::make_unique<Opm::ECLRestartData>( Opm::ECLRestartData( firstRestartFileName.toStdString() ) );
            m_opmFlowDiagStaticData->m_hasUnifiedRestartFile = true;
        }
        else
        {
            QStringList restartFileNames = RifEclipseOutputFileTools::filterFileNamesOfType( m_filesWithSameBaseName, ECL_RESTART_FILE );

            size_t restartFileCount = static_cast<size_t>( restartFileNames.size() );
            size_t maxTimeStepCount =
                m_eclipseCase->eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->maxTimeStepCount();

            if ( restartFileCount <= timeStepIndex && restartFileCount != maxTimeStepCount )
            {
                RiaLogging::errorInMessageBox( nullptr,
                                               "ResInsight",
                                               "Flow Diagnostics: Could not find all the restart files. Results will "
                                               "not be "
                                               "loaded." );
                return result;
            }

            restartFileNames.sort(); // To make sure they are sorted in increasing *.X000N order. Hack. Should probably
                                     // be actual time stored on file.
            m_opmFlowDiagStaticData->m_hasUnifiedRestartFile = false;

            for ( const auto& restartFileName : restartFileNames )
            {
                m_opmFlowDiagStaticData->m_singleRestartDataTimeSteps.push_back( Opm::ECLRestartData( restartFileName.toStdString() ) );
            }
        }
    }
    catch ( const std::exception& e )
    {
        RiaLogging::errorInMessageBox( nullptr, "ResInsight", "Flow Diagnostics Exception: " + QString( e.what() ) );
        return result;
    }

    progressInfo.setProgress( 3 );
    progressInfo.setProgressDescription( "Assigning Flux Field" );

    assignPhaseCorrecedPORV( phaseSelection, timeStepIndex );

    Opm::ECLRestartData* currentRestartData = nullptr;

    if ( !m_opmFlowDiagStaticData->m_hasUnifiedRestartFile )
    {
        currentRestartData = &( m_opmFlowDiagStaticData->m_singleRestartDataTimeSteps[timeStepIndex] );
    }
    else
    {
        currentRestartData = m_opmFlowDiagStaticData->m_unifiedRestartData.get();
    }

    CVF_ASSERT( currentRestartData );

    RigEclipseResultAddress addrToMaxTimeStepCountResult;
    m_eclipseCase->eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->maxTimeStepCount( &addrToMaxTimeStepCountResult );

    int reportStepNumber = m_eclipseCase->eclipseCaseData()
                               ->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
                               ->reportStepNumber( addrToMaxTimeStepCountResult, timeStepIndex );

    if ( !currentRestartData->selectReportStep( reportStepNumber ) )
    {
        RiaLogging::errorInMessageBox( nullptr,
                                       "ResInsight",
                                       "Flow Diagnostics: Could not find the requested timestep in the result file. "
                                       "Results "
                                       "will not be loaded." );
        return result;
    }

    // Set up flow Toolbox with timestep data
    std::map<Opm::FlowDiagnostics::CellSetID, Opm::FlowDiagnostics::CellSetValues> WellInFluxPrCell;

    try
    {
        if ( m_eclipseCase->eclipseCaseData()->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->hasFlowDiagUsableFluxes() )
        {
            Opm::FlowDiagnostics::ConnectionValues connectionsVals =
                RigFlowDiagInterfaceTools::extractFluxFieldFromRestartFile( *( m_opmFlowDiagStaticData->m_eclGraph ),
                                                                            *currentRestartData,
                                                                            phaseSelection );
            m_opmFlowDiagStaticData->m_fldToolbox->assignConnectionFlux( connectionsVals );
        }
        else
        {
            Opm::ECLInitFileData                   init( getInitFileName() );
            Opm::FlowDiagnostics::ConnectionValues connectionVals =
                RigFlowDiagInterfaceTools::calculateFluxField( ( *m_opmFlowDiagStaticData->m_eclGraph ), init, *currentRestartData, phaseSelection );
            m_opmFlowDiagStaticData->m_fldToolbox->assignConnectionFlux( connectionVals );
        }

        progressInfo.incrementProgress();

        Opm::ECLWellSolution wsol = Opm::ECLWellSolution{ -1.0, false };

        std::vector<std::string> gridNames = m_opmFlowDiagStaticData->m_eclGraph->activeGrids();

        const std::vector<Opm::ECLWellSolution::WellData> well_fluxes = wsol.solution( *currentRestartData, gridNames );

        WellInFluxPrCell = RigFlowDiagInterfaceTools::extractWellFlows( *( m_opmFlowDiagStaticData->m_eclGraph ), well_fluxes );

        m_opmFlowDiagStaticData->m_fldToolbox->assignInflowFlux( WellInFluxPrCell );
    }
    catch ( const std::exception& e )
    {
        RiaLogging::errorInMessageBox( nullptr, "ResInsight", "Flow Diagnostics Exception: " + QString( e.what() ) );
        return result;
    }

    progressInfo.incrementProgress();
    progressInfo.setProgressDescription( "Injector Solution" );

    try
    {
        // Injection Solution
        std::set<std::string>             injectorCrossFlowTracers;
        std::vector<CellSet>              injectorCellSets;
        std::unique_ptr<Toolbox::Forward> injectorSolution;
        {
            for ( const auto& tIt : injectorTracers )
            {
                std::string tracerName = tIt.first;
                if ( hasCrossFlowEnding( tracerName ) )
                {
                    tracerName = removeCrossFlowEnding( tracerName );
                    injectorCrossFlowTracers.insert( tracerName );
                }
                injectorCellSets.push_back( CellSet( CellSetID( tracerName ), tIt.second ) );
            }

            injectorSolution =
                std::make_unique<Toolbox::Forward>( m_opmFlowDiagStaticData->m_fldToolbox->computeInjectionDiagnostics( injectorCellSets ) );

            for ( const CellSetID& tracerId : injectorSolution->fd.startPoints() )
            {
                std::string tracername = tracerId.to_string();
                if ( injectorCrossFlowTracers.count( tracername ) ) tracername = addCrossFlowEnding( tracername );

                CellSetValues tofVals = injectorSolution->fd.timeOfFlight( tracerId );
                result.setTracerTOF( tracername, phaseSelection, tofVals );
                CellSetValues fracVals = injectorSolution->fd.concentration( tracerId );
                result.setTracerFraction( tracername, phaseSelection, fracVals );
            }
        }

        progressInfo.incrementProgress();
        progressInfo.setProgressDescription( "Producer Solution" );

        // Producer Solution
        std::set<std::string>             producerCrossFlowTracers;
        std::vector<CellSet>              prodjCellSets;
        std::unique_ptr<Toolbox::Reverse> producerSolution;
        {
            for ( const auto& tIt : producerTracers )
            {
                std::string tracerName = tIt.first;
                if ( hasCrossFlowEnding( tracerName ) )
                {
                    tracerName = removeCrossFlowEnding( tracerName );
                    producerCrossFlowTracers.insert( tracerName );
                }
                prodjCellSets.push_back( CellSet( CellSetID( tracerName ), tIt.second ) );
            }

            producerSolution =
                std::make_unique<Toolbox::Reverse>( m_opmFlowDiagStaticData->m_fldToolbox->computeProductionDiagnostics( prodjCellSets ) );

            for ( const CellSetID& tracerId : producerSolution->fd.startPoints() )
            {
                std::string tracername = tracerId.to_string();
                if ( producerCrossFlowTracers.count( tracername ) ) tracername = addCrossFlowEnding( tracername );

                CellSetValues tofVals = producerSolution->fd.timeOfFlight( tracerId );
                result.setTracerTOF( tracername, phaseSelection, tofVals );
                CellSetValues fracVals = producerSolution->fd.concentration( tracerId );
                result.setTracerFraction( tracername, phaseSelection, fracVals );
            }
        }

        progressInfo.incrementProgress();
        progressInfo.setProgressDescription( "Well pair fluxes" );

        int producerTracerCount = static_cast<int>( prodjCellSets.size() );

#pragma omp parallel for
        for ( int pIdx = 0; pIdx < producerTracerCount; ++pIdx )
        {
            const auto& prodCellSet = prodjCellSets[pIdx];

            std::string prodTracerName = prodCellSet.id().to_string();
            CellSetID   prodID( prodTracerName );

            std::string uiProducerTracerName = prodTracerName;
            if ( producerCrossFlowTracers.count( prodTracerName ) )
            {
                uiProducerTracerName = addCrossFlowEnding( prodTracerName );
            }

            for ( const auto& injCellSet : injectorCellSets )
            {
                std::string injTracerName = injCellSet.id().to_string();
                CellSetID   injID( injTracerName );

                std::pair<double, double> fluxPair =
                    injectorProducerPairFlux( *( injectorSolution.get() ), *( producerSolution.get() ), injID, prodID, WellInFluxPrCell );
                std::string uiInjectorTracerName = injTracerName;

                if ( injectorCrossFlowTracers.count( injTracerName ) )
                {
                    uiInjectorTracerName = addCrossFlowEnding( injTracerName );
                }

#pragma omp critical( critical_section_RigFlowDiagSolverInterface_calculate )
                {
                    result.setInjProdWellPairFlux( uiInjectorTracerName, uiProducerTracerName, fluxPair );
                }
            }
        }
    }
    catch ( const std::exception& e )
    {
        RiaLogging::errorInMessageBox( nullptr, "ResInsight", "Flow Diagnostics Exception: " + QString( e.what() ) );
        return result;
    }

    return result; // Relying on implicit move constructor
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFlowDiagSolverInterface::ensureStaticDataObjectInstanceCreated()
{
    if ( m_opmFlowDiagStaticData == nullptr )
    {
        // Get set of files
        std::wstring initFileName = getInitFileName();
        if ( initFileName.empty() ) return false;

        const RigEclipseCaseData* eclipseCaseData = m_eclipseCase->eclipseCaseData();
        if ( eclipseCaseData )
        {
            auto           fileReader = eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->readerInterface();
            ecl_grid_type* mainGrid   = nullptr;

            if ( auto eclOutput = dynamic_cast<const RifReaderEclipseOutput*>( fileReader ) )
            {
                mainGrid = eclOutput->loadAllGrids();
                if ( !mainGrid )
                {
                    return false;
                }
            }

            RiaDefines::EclipseUnitSystem caseUnitSystem = eclipseCaseData->unitsType();
            m_opmFlowDiagStaticData = std::make_unique<RigOpmFlowDiagStaticData>( mainGrid, initFileName, caseUnitSystem );

            if ( mainGrid )
            {
                ecl_grid_free( mainGrid );
            }
        }
    }

    return m_opmFlowDiagStaticData != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFlowDiagSolverInterface::assignPhaseCorrecedPORV( RigFlowDiagResultAddress::PhaseSelection phaseSelection, size_t timeStepIdx )
{
    RigEclipseCaseData* eclipseCaseData = m_eclipseCase->eclipseCaseData();

    const std::vector<double>* phaseSaturation = nullptr;

    switch ( phaseSelection )
    {
        case RigFlowDiagResultAddress::PHASE_OIL:
            phaseSaturation = eclipseCaseData->resultValues( RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                             RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                             RiaResultNames::soil(),
                                                             timeStepIdx );
            break;
        case RigFlowDiagResultAddress::PHASE_GAS:
            phaseSaturation = eclipseCaseData->resultValues( RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                             RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                             RiaResultNames::sgas(),
                                                             timeStepIdx );
            break;
        case RigFlowDiagResultAddress::PHASE_WAT:
            phaseSaturation = eclipseCaseData->resultValues( RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                             RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                             RiaResultNames::swat(),
                                                             timeStepIdx );
            break;
        default:
            m_opmFlowDiagStaticData->m_fldToolbox->assignPoreVolume( m_opmFlowDiagStaticData->m_poreVolume );
            break;
    }

    if ( phaseSaturation )
    {
        std::vector<double> porvAdjusted = m_opmFlowDiagStaticData->m_poreVolume;
        CAF_ASSERT( porvAdjusted.size() == phaseSaturation->size() );
        for ( size_t idx = 0; idx < porvAdjusted.size(); ++idx )
        {
            porvAdjusted[idx] *= phaseSaturation->at( idx );
        }

        m_opmFlowDiagStaticData->m_fldToolbox->assignPoreVolume( porvAdjusted );
    }
    else
    {
        m_opmFlowDiagStaticData->m_fldToolbox->assignPoreVolume( m_opmFlowDiagStaticData->m_poreVolume );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFlowDiagSolverInterface::reportRelPermCurveError( const QString& message )
{
    if ( m_relpermCurveErrorCount == 0 )
    {
        RiaLogging::warning( "RelPerm curve problems: \n" + message );
    }
    m_relpermCurveErrorCount++;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFlowDiagSolverInterface::reportPvtCurveError( const QString& message )
{
    if ( m_pvtCurveErrorCount == 0 )
    {
        RiaLogging::warning( "PVT curve problems: \n" + message );
    }
    m_pvtCurveErrorCount++;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFlowDiagDefines::FlowCharacteristicsResultFrame
    RigFlowDiagSolverInterface::calculateFlowCharacteristics( const std::vector<double>* injector_tof,
                                                              const std::vector<double>* producer_tof,
                                                              const std::vector<size_t>& selected_cell_indices,
                                                              double                     max_pv_fraction )
{
    using namespace Opm::FlowDiagnostics;
    RigFlowDiagDefines::FlowCharacteristicsResultFrame result;

    if ( injector_tof == nullptr || producer_tof == nullptr )
    {
        return result;
    }

    if ( m_opmFlowDiagStaticData == nullptr )
    {
        return result;
    }

    std::vector<double> poreVolume;
    for ( size_t cellIndex : selected_cell_indices )
    {
        poreVolume.push_back( m_opmFlowDiagStaticData->m_poreVolume[cellIndex] );
    }

    try
    {
        Graph flowCapStorCapCurve = flowCapacityStorageCapacityCurve( *injector_tof, *producer_tof, poreVolume, max_pv_fraction );

        result.m_storageCapFlowCapCurve                = flowCapStorCapCurve;
        result.m_lorenzCoefficient                     = lorenzCoefficient( flowCapStorCapCurve );
        result.m_dimensionlessTimeSweepEfficiencyCurve = sweepEfficiency( flowCapStorCapCurve );
    }
    catch ( const std::exception& e )
    {
        RiaLogging::errorInMessageBox( nullptr, "ResInsight", "Flow Diagnostics: " + QString( e.what() ) );
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigFlowDiagDefines::RelPermCurve> RigFlowDiagSolverInterface::calculateRelPermCurves( const std::string& gridName,
                                                                                                  size_t gridLocalActiveCellIndex )
{
    using RawCurve = Opm::ECLSaturationFunc::RawCurve;

    std::vector<RigFlowDiagDefines::RelPermCurve> retCurveArr;

    if ( !ensureStaticDataObjectInstanceCreated() )
    {
        return retCurveArr;
    }

    CVF_ASSERT( m_opmFlowDiagStaticData != nullptr );
    if ( !m_opmFlowDiagStaticData->m_eclSaturationFunc )
    {
        return retCurveArr;
    }

    // Define curve sets to request (Drainage and Imbibition)
    const std::array<RawCurve::CurveSet, 2> curveSets = { RawCurve::CurveSet::Drainage, RawCurve::CurveSet::Imbibition };

    // Base curve definitions - will be created for each curve set
    struct CurveDefinition
    {
        RawCurve::Function                      function;
        RawCurve::SubSystem                     subsystem;
        Opm::ECLPhaseIndex                      phase;
        RigFlowDiagDefines::RelPermCurve::Ident ident;
        std::string                             baseName;
    };

    const std::vector<CurveDefinition> baseCurves =
        { { RawCurve::Function::RelPerm, RawCurve::SubSystem::OilWater, Opm::ECLPhaseIndex::Aqua, RigFlowDiagDefines::RelPermCurve::KRW, "KRW" },
          { RawCurve::Function::RelPerm, RawCurve::SubSystem::OilGas, Opm::ECLPhaseIndex::Vapour, RigFlowDiagDefines::RelPermCurve::KRG, "KRG" },
          { RawCurve::Function::RelPerm, RawCurve::SubSystem::OilWater, Opm::ECLPhaseIndex::Liquid, RigFlowDiagDefines::RelPermCurve::KROW, "KROW" },
          { RawCurve::Function::RelPerm, RawCurve::SubSystem::OilGas, Opm::ECLPhaseIndex::Liquid, RigFlowDiagDefines::RelPermCurve::KROG, "KROG" },
          { RawCurve::Function::CapPress, RawCurve::SubSystem::OilGas, Opm::ECLPhaseIndex::Vapour, RigFlowDiagDefines::RelPermCurve::PCOG, "PCOG" },
          { RawCurve::Function::CapPress, RawCurve::SubSystem::OilWater, Opm::ECLPhaseIndex::Aqua, RigFlowDiagDefines::RelPermCurve::PCOW, "PCOW" } };

    struct CurveRequest
    {
        RigFlowDiagDefines::RelPermCurve::Ident    ident;
        std::string                                name;
        RigFlowDiagDefines::RelPermCurve::CurveSet curveSet;
    };

    std::vector<CurveRequest> curveRequests;
    std::vector<RawCurve>     satFuncRequests;

    // Build requests for both drainage and imbibition curves
    for ( const auto& curveSet : curveSets )
    {
        for ( const auto& baseCurve : baseCurves )
        {
            const RawCurve curve{ baseCurve.function, baseCurve.subsystem, baseCurve.phase, curveSet };

            // Create appropriate name for the curve (prefix "I" for imbibition curves)
            std::string curveName = baseCurve.baseName;
            if ( curveSet == RawCurve::CurveSet::Imbibition )
            {
                curveName = "I" + curveName;
            }

            // Map OPM CurveSet to RigFlowDiagDefines CurveSet
            RigFlowDiagDefines::RelPermCurve::CurveSet rigCurveSet = ( curveSet == RawCurve::CurveSet::Drainage )
                                                                         ? RigFlowDiagDefines::RelPermCurve::DRAINAGE
                                                                         : RigFlowDiagDefines::RelPermCurve::IMBIBITION;

            curveRequests.push_back( { baseCurve.ident, curveName, rigCurveSet } );
            satFuncRequests.push_back( curve );
        }
    }

    try
    {
        // Calculate and return curves both with and without endpoint scaling and tag them accordingly
        // Must use two calls to achieve this
        const std::array<RigFlowDiagDefines::RelPermCurve::EpsMode, 2> epsModeArr = {
            { RigFlowDiagDefines::RelPermCurve::EPS_ON, RigFlowDiagDefines::RelPermCurve::EPS_OFF } };
        for ( RigFlowDiagDefines::RelPermCurve::EpsMode epsMode : epsModeArr )
        {
            const bool useEps = epsMode == RigFlowDiagDefines::RelPermCurve::EPS_ON;

            Opm::ECLSaturationFunc::SatFuncScaling scaling;
            if ( !useEps )
            {
                scaling.enable = static_cast<unsigned char>( 0 );
            }

            std::vector<Opm::FlowDiagnostics::Graph> graphArr =
                m_opmFlowDiagStaticData->m_eclSaturationFunc->getSatFuncCurve( satFuncRequests,
                                                                               m_opmFlowDiagStaticData->m_initData,
                                                                               gridName,
                                                                               static_cast<int>( gridLocalActiveCellIndex ),
                                                                               scaling );

            // Process results - now includes both drainage and imbibition curves
            if ( graphArr.size() != satFuncRequests.size() )
            {
                reportRelPermCurveError( "Mismatch between number of requested and received rel-perm curves." );
                continue;
            }

            for ( size_t i = 0; i < graphArr.size(); i++ )
            {
                const RigFlowDiagDefines::RelPermCurve::Ident    curveIdent = curveRequests[i].ident;
                const std::string&                               curveName  = curveRequests[i].name;
                const RigFlowDiagDefines::RelPermCurve::CurveSet curveSet   = curveRequests[i].curveSet;
                const Opm::FlowDiagnostics::Graph&               srcGraph   = graphArr[i];
                if ( !srcGraph.first.empty() )
                {
                    const std::vector<double>& xVals = srcGraph.first;
                    const std::vector<double>& yVals = srcGraph.second;
                    retCurveArr.push_back( { curveIdent, curveName, epsMode, curveSet, xVals, yVals } );
                }
            }
        }
    }
    catch ( const std::exception& e )
    {
        reportRelPermCurveError( QString( e.what() ) );
        return retCurveArr;
    }

    // For unknown reasons, the ECLSaturationFunc returns a KROG curve when there is no gas in the system.
    // Remove invalid KROG curve if no KRG curve is present
    // https://github.com/OPM/ResInsight/issues/11396

    bool hasKRG = false;
    for ( const RigFlowDiagDefines::RelPermCurve& curve : retCurveArr )
    {
        if ( curve.ident == RigFlowDiagDefines::RelPermCurve::KRG )
        {
            hasKRG = true;
            break;
        }
    }

    if ( !hasKRG )
    {
        retCurveArr.erase( std::remove_if( retCurveArr.begin(),
                                           retCurveArr.end(),
                                           []( const RigFlowDiagDefines::RelPermCurve& curve )
                                           { return curve.ident == RigFlowDiagDefines::RelPermCurve::KROG; } ),
                           retCurveArr.end() );
    }

    return retCurveArr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigFlowDiagDefines::PvtCurve> RigFlowDiagSolverInterface::calculatePvtCurves( RigFlowDiagDefines::PvtCurveType pvtCurveType,
                                                                                          int                              pvtNum )
{
    std::vector<RigFlowDiagDefines::PvtCurve> retCurveArr;

    try
    {
        if ( !ensureStaticDataObjectInstanceCreated() )
        {
            return retCurveArr;
        }

        CVF_ASSERT( m_opmFlowDiagStaticData != nullptr );
        if ( !m_opmFlowDiagStaticData->m_eclPvtCurveCollection )
        {
            return retCurveArr;
        }

        // Requesting FVF or Viscosity
        if ( pvtCurveType == RigFlowDiagDefines::PvtCurveType::PVT_CT_FVF )
        {
            // Bo
            {
                std::vector<Opm::ECLPVT::PVTGraph> graphArr =
                    m_opmFlowDiagStaticData->m_eclPvtCurveCollection->getPvtCurve( Opm::ECLPVT::RawCurve::FVF,
                                                                                   Opm::ECLPhaseIndex::Liquid,
                                                                                   pvtNum );
                for ( Opm::ECLPVT::PVTGraph srcGraph : graphArr )
                {
                    if ( !srcGraph.press.empty() )
                    {
                        retCurveArr.push_back(
                            { RigFlowDiagDefines::PvtCurve::Bo, RigFlowDiagDefines::PvtCurve::OIL, srcGraph.press, srcGraph.value, srcGraph.mixRat } );
                    }
                }
            }

            // Bg
            {
                std::vector<Opm::ECLPVT::PVTGraph> graphArr =
                    m_opmFlowDiagStaticData->m_eclPvtCurveCollection->getPvtCurve( Opm::ECLPVT::RawCurve::FVF,
                                                                                   Opm::ECLPhaseIndex::Vapour,
                                                                                   pvtNum );
                for ( Opm::ECLPVT::PVTGraph srcGraph : graphArr )
                {
                    if ( !srcGraph.press.empty() )
                    {
                        retCurveArr.push_back(
                            { RigFlowDiagDefines::PvtCurve::Bg, RigFlowDiagDefines::PvtCurve::GAS, srcGraph.press, srcGraph.value, srcGraph.mixRat } );
                    }
                }
            }
        }

        else if ( pvtCurveType == RigFlowDiagDefines::PvtCurveType::PVT_CT_VISCOSITY )
        {
            // Visc_o / mu_o
            {
                std::vector<Opm::ECLPVT::PVTGraph> graphArr =
                    m_opmFlowDiagStaticData->m_eclPvtCurveCollection->getPvtCurve( Opm::ECLPVT::RawCurve::Viscosity,
                                                                                   Opm::ECLPhaseIndex::Liquid,
                                                                                   pvtNum );
                for ( Opm::ECLPVT::PVTGraph srcGraph : graphArr )
                {
                    if ( !srcGraph.press.empty() )
                    {
                        retCurveArr.push_back( { RigFlowDiagDefines::PvtCurve::Visc_o,
                                                 RigFlowDiagDefines::PvtCurve::OIL,
                                                 srcGraph.press,
                                                 srcGraph.value,
                                                 srcGraph.mixRat } );
                    }
                }
            }

            // Visc_g / mu_g
            {
                std::vector<Opm::ECLPVT::PVTGraph> graphArr =
                    m_opmFlowDiagStaticData->m_eclPvtCurveCollection->getPvtCurve( Opm::ECLPVT::RawCurve::Viscosity,
                                                                                   Opm::ECLPhaseIndex::Vapour,
                                                                                   pvtNum );
                for ( Opm::ECLPVT::PVTGraph srcGraph : graphArr )
                {
                    if ( !srcGraph.press.empty() )
                    {
                        retCurveArr.push_back( { RigFlowDiagDefines::PvtCurve::Visc_g,
                                                 RigFlowDiagDefines::PvtCurve::GAS,
                                                 srcGraph.press,
                                                 srcGraph.value,
                                                 srcGraph.mixRat } );
                    }
                }
            }
        }
    }
    catch ( const std::exception& e )
    {
        reportPvtCurveError( QString( e.what() ) );
        return retCurveArr;
    }

    return retCurveArr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFlowDiagSolverInterface::calculatePvtDynamicPropertiesFvf( int pvtNum, double pressure, double rs, double rv, double* bo, double* bg )
{
    if ( bo ) *bo = HUGE_VAL;
    if ( bg ) *bg = HUGE_VAL;

    if ( !ensureStaticDataObjectInstanceCreated() )
    {
        return false;
    }

    CVF_ASSERT( m_opmFlowDiagStaticData != nullptr );
    if ( !m_opmFlowDiagStaticData->m_eclPvtCurveCollection )
    {
        return false;
    }

    try
    {
        // Bo
        {
            std::vector<double> phasePress = { pressure };
            std::vector<double> mixRatio   = { rs };
            std::vector<double> valArr =
                m_opmFlowDiagStaticData->m_eclPvtCurveCollection->getDynamicPropertyNative( Opm::ECLPVT::RawCurve::FVF,
                                                                                            Opm::ECLPhaseIndex::Liquid,
                                                                                            pvtNum,
                                                                                            phasePress,
                                                                                            mixRatio );
            if ( !valArr.empty() )
            {
                *bo = valArr[0];
            }
        }

        // Bg
        {
            std::vector<double> phasePress = { pressure };
            std::vector<double> mixRatio   = { rv };
            std::vector<double> valArr =
                m_opmFlowDiagStaticData->m_eclPvtCurveCollection->getDynamicPropertyNative( Opm::ECLPVT::RawCurve::FVF,
                                                                                            Opm::ECLPhaseIndex::Vapour,
                                                                                            pvtNum,
                                                                                            phasePress,
                                                                                            mixRatio );
            if ( !valArr.empty() )
            {
                *bg = valArr[0];
            }
        }
    }
    catch ( const std::exception& e )
    {
        reportPvtCurveError( QString( e.what() ) );
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFlowDiagSolverInterface::calculatePvtDynamicPropertiesViscosity( int pvtNum, double pressure, double rs, double rv, double* mu_o, double* mu_g )
{
    if ( mu_o ) *mu_o = HUGE_VAL;
    if ( mu_g ) *mu_g = HUGE_VAL;

    if ( !ensureStaticDataObjectInstanceCreated() )
    {
        return false;
    }

    CVF_ASSERT( m_opmFlowDiagStaticData != nullptr );
    if ( !m_opmFlowDiagStaticData->m_eclPvtCurveCollection )
    {
        return false;
    }

    try
    {
        // mu_o
        {
            std::vector<double> phasePress = { pressure };
            std::vector<double> mixRatio   = { rs };
            std::vector<double> valArr =
                m_opmFlowDiagStaticData->m_eclPvtCurveCollection->getDynamicPropertyNative( Opm::ECLPVT::RawCurve::Viscosity,
                                                                                            Opm::ECLPhaseIndex::Liquid,
                                                                                            pvtNum,
                                                                                            phasePress,
                                                                                            mixRatio );
            if ( !valArr.empty() )
            {
                *mu_o = valArr[0];
            }
        }

        // mu_o
        {
            std::vector<double> phasePress = { pressure };
            std::vector<double> mixRatio   = { rv };
            std::vector<double> valArr =
                m_opmFlowDiagStaticData->m_eclPvtCurveCollection->getDynamicPropertyNative( Opm::ECLPVT::RawCurve::Viscosity,
                                                                                            Opm::ECLPhaseIndex::Vapour,
                                                                                            pvtNum,
                                                                                            phasePress,
                                                                                            mixRatio );
            if ( !valArr.empty() )
            {
                *mu_g = valArr[0];
            }
        }
    }
    catch ( const std::exception& e )
    {
        reportPvtCurveError( QString( e.what() ) );
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::wstring RigFlowDiagSolverInterface::getInitFileName() const
{
    QString gridFileName = m_eclipseCase->gridFileName();

    QStringList m_filesWithSameBaseName;

    if ( !RifEclipseOutputFileTools::findSiblingFilesWithSameBaseName( gridFileName, &m_filesWithSameBaseName ) ) return std::wstring();

    QString initFileName = RifEclipseOutputFileTools::firstFileNameOfType( m_filesWithSameBaseName, ECL_INIT_FILE );

    return initFileName.toStdWString();
}
