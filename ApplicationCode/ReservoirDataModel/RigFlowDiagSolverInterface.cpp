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

#include "RifEclipseOutputFileTools.h"
#include "RifReaderInterface.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"

#include "RigFlowDiagInterfaceTools.h"
#include "opm/flowdiagnostics/DerivedQuantities.hpp"

#include "opm/utility/ECLSaturationFunc.hpp"
#include "opm/utility/ECLPvtCurveCollection.hpp"

#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimFlowDiagSolution.h"

#include <QMessageBox>
#include "cafProgressInfo.h"

#include "cvfTrace.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFlowDiagTimeStepResult::RigFlowDiagTimeStepResult(size_t activeCellCount)
    : m_activeCellCount(activeCellCount)
{

}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagTimeStepResult::setTracerTOF(const std::string& tracerName,
                                             RigFlowDiagResultAddress::PhaseSelection phaseSelection,
                                             const std::map<int, double>& cellValues)
{
    std::set<std::string> tracers;
    tracers.insert(tracerName);
    
    RigFlowDiagResultAddress resAddr(RIG_FLD_TOF_RESNAME, phaseSelection, tracers);

    this->addResult(resAddr, cellValues);

    std::vector<double>& activeCellValues =  m_nativeResults[resAddr];
    for (double & val: activeCellValues)
    {
        val = val * 1.15741e-5; // days pr second. Converting to days
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagTimeStepResult::setTracerFraction(const std::string& tracerName,
                                                  RigFlowDiagResultAddress::PhaseSelection phaseSelection,
                                                  const std::map<int, double>& cellValues)
{
    std::set<std::string> tracers;
    tracers.insert(tracerName);

    this->addResult(RigFlowDiagResultAddress(RIG_FLD_CELL_FRACTION_RESNAME, phaseSelection, tracers), cellValues);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagTimeStepResult::setInjProdWellPairFlux(const std::string& injectorTracerName, 
                                                       const std::string& producerTracerName, 
                                                       const std::pair<double, double>& injProdFluxes)
{
    m_injProdWellPairFluxes[std::make_pair(injectorTracerName, producerTracerName)] = injProdFluxes;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagTimeStepResult::addResult(const RigFlowDiagResultAddress& resAddr, const std::map<int, double>& cellValues)
{
    std::vector<double>& activeCellValues =  m_nativeResults[resAddr];

    CVF_ASSERT(activeCellValues.empty());

    activeCellValues.resize(m_activeCellCount, HUGE_VAL);

    for (const auto& pairIt : cellValues)
    {
        activeCellValues[pairIt.first]  = pairIt.second;
    }
}


class RigOpmFlowDiagStaticData : public cvf::Object
{
public:
    RigOpmFlowDiagStaticData(const std::string& grid, const std::string& init)
    {
        Opm::ECLInitFileData initData(init);

        m_eclGraph.reset(new Opm::ECLGraph(Opm::ECLGraph::load(grid, initData)));

        m_hasUnifiedRestartFile = false;
        m_poreVolume = m_eclGraph->poreVolume();

        m_eclSaturationFunc.reset(new Opm::ECLSaturationFunc(*m_eclGraph, initData));

        try
        {
            m_eclPvtCurveCollection.reset(new Opm::ECLPVT::ECLPvtCurveCollection(*m_eclGraph, initData));
        }
        catch (...)
        {
            cvf::Trace::show("Exception trying to construct ECLPvtCurveCollection instance");
        }
    }

    std::unique_ptr<Opm::ECLGraph>                  m_eclGraph;
    std::vector<double>                             m_poreVolume;
    std::unique_ptr<Opm::FlowDiagnostics::Toolbox>  m_fldToolbox;
    bool                                            m_hasUnifiedRestartFile;
    std::vector<Opm::ECLRestartData>                m_singleRestartDataTimeSteps;
    std::unique_ptr<Opm::ECLRestartData>            m_unifiedRestartData;

    std::unique_ptr<Opm::ECLSaturationFunc>             m_eclSaturationFunc;
    std::unique_ptr<Opm::ECLPVT::ECLPvtCurveCollection> m_eclPvtCurveCollection;
};



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFlowDiagSolverInterface::RigFlowDiagSolverInterface(RimEclipseResultCase * eclipseCase)
: m_eclipseCase(eclipseCase)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFlowDiagSolverInterface::~RigFlowDiagSolverInterface()
{

}

#if 0
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void removeCrossFlowCells(std::pair<const std::string, std::vector<int>> & tracerCellIdxsPair,
                          std::map<Opm::FlowDiagnostics::CellSetID, Opm::FlowDiagnostics::CellSetValues> & WellInFluxPrCell,
                          std::function<bool(double)> isFlowOkFunction)
{
    std::string tracerName = tracerCellIdxsPair.first;
    tracerName = RimFlowDiagSolution::removeCrossFlowEnding(QString::fromStdString(tracerName)).toStdString();
    auto cellSetIdInFlowsPair =  WellInFluxPrCell.find(Opm::FlowDiagnostics::CellSetID(tracerName));

    CVF_TIGHT_ASSERT(cellSetIdInFlowsPair != WellInFluxPrCell.end());

    std::vector<int> filteredCellIndices;

    for ( int activeCellIdx : tracerCellIdxsPair.second )
    {
        auto activeCellIdxFluxPair = cellSetIdInFlowsPair->second.find(activeCellIdx);
        CVF_TIGHT_ASSERT(activeCellIdxFluxPair != cellSetIdInFlowsPair->second.end());

        if ( isFlowOkFunction(activeCellIdxFluxPair->second) )
        {
            filteredCellIndices.push_back(activeCellIdx);
        }
    }

    if ( tracerCellIdxsPair.second.size() != filteredCellIndices.size() ) 
    { 
        tracerCellIdxsPair.second = filteredCellIndices;
    }
}
#endif

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string removeCrossFlowEnding(std::string tracerName)
{
    return RimFlowDiagSolution::removeCrossFlowEnding(QString::fromStdString(tracerName)).toStdString();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool hasCrossFlowEnding(std::string tracerName)
{
    return RimFlowDiagSolution::hasCrossFlowEnding(QString::fromStdString(tracerName));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string addCrossFlowEnding(std::string tracerName)
{
    return RimFlowDiagSolution::addCrossFlowEnding(QString::fromStdString(tracerName)).toStdString();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFlowDiagTimeStepResult RigFlowDiagSolverInterface::calculate(size_t timeStepIndex,
                                                                RigFlowDiagResultAddress::PhaseSelection phaseSelection,
                                                                std::map<std::string, std::vector<int> > injectorTracers,
                                                                std::map<std::string, std::vector<int> > producerTracers)
{
    using namespace Opm::FlowDiagnostics;

    RigFlowDiagTimeStepResult result(m_eclipseCase->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL)->reservoirActiveCellCount());

    caf::ProgressInfo progressInfo(8, "Calculating Flow Diagnostics");

    {
        progressInfo.setProgressDescription("Grid access");

        if (!ensureStaticDataObjectInstanceCreated())
        {
            return result;
        }

        progressInfo.incrementProgress();
        progressInfo.setProgressDescription("Calculating Connectivities");

        CVF_ASSERT(m_opmFlowDiagStaticData.notNull());
        const  Opm::FlowDiagnostics::ConnectivityGraph connGraph =
            Opm::FlowDiagnostics::ConnectivityGraph{ static_cast<int>(m_opmFlowDiagStaticData->m_eclGraph->numCells()),
            m_opmFlowDiagStaticData->m_eclGraph->neighbours() };

        progressInfo.incrementProgress();
        progressInfo.setProgressDescription("Initialize Solver");

        // Create the Toolbox.

        m_opmFlowDiagStaticData->m_fldToolbox.reset(new Opm::FlowDiagnostics::Toolbox{ connGraph });
        m_opmFlowDiagStaticData->m_fldToolbox->assignPoreVolume( m_opmFlowDiagStaticData->m_poreVolume);

        // Look for unified restart file
        QStringList m_filesWithSameBaseName;

        QString gridFileName = m_eclipseCase->gridFileName();
        if ( !RifEclipseOutputFileTools::findSiblingFilesWithSameBaseName(gridFileName, &m_filesWithSameBaseName) ) return result;

        QString restartFileName = RifEclipseOutputFileTools::firstFileNameOfType(m_filesWithSameBaseName, ECL_UNIFIED_RESTART_FILE);
        if ( !restartFileName.isEmpty() )
        {
            m_opmFlowDiagStaticData->m_unifiedRestartData.reset(new Opm::ECLRestartData(Opm::ECLRestartData(restartFileName.toStdString())));
            m_opmFlowDiagStaticData->m_hasUnifiedRestartFile = true;
        }
        else
        {
            QStringList restartFileNames = RifEclipseOutputFileTools::filterFileNamesOfType(m_filesWithSameBaseName, ECL_RESTART_FILE);

            size_t restartFileCount = static_cast<size_t>(restartFileNames.size());
            size_t maxTimeStepCount = m_eclipseCase->eclipseCaseData()->results(RiaDefines::MATRIX_MODEL)->maxTimeStepCount();

            if (restartFileCount <= timeStepIndex &&  restartFileCount != maxTimeStepCount )
            {
                QMessageBox::critical(nullptr, "ResInsight", "Flow Diagnostics: Could not find all the restart files. Results will not be loaded.");
                return result;
            }

            restartFileNames.sort(); // To make sure they are sorted in increasing *.X000N order. Hack. Should probably be actual time stored on file.
            m_opmFlowDiagStaticData->m_hasUnifiedRestartFile = false;

            for (auto restartFileName : restartFileNames)
            {
                m_opmFlowDiagStaticData->m_singleRestartDataTimeSteps.push_back(Opm::ECLRestartData(restartFileName.toStdString()));
            }
        }
    }

    progressInfo.setProgress(3);
    progressInfo.setProgressDescription("Assigning Flux Field");

    Opm::ECLRestartData* currentRestartData = nullptr;

    if ( ! m_opmFlowDiagStaticData->m_hasUnifiedRestartFile  )
    {
        currentRestartData = &(m_opmFlowDiagStaticData->m_singleRestartDataTimeSteps[timeStepIndex]);
    }
    else
    {
        currentRestartData = m_opmFlowDiagStaticData->m_unifiedRestartData.get();
    }

    CVF_ASSERT(currentRestartData);

    size_t resultIndexWithMaxTimeSteps = cvf::UNDEFINED_SIZE_T;
    m_eclipseCase->eclipseCaseData()->results(RiaDefines::MATRIX_MODEL)->maxTimeStepCount(&resultIndexWithMaxTimeSteps);

    int reportStepNumber =  m_eclipseCase->eclipseCaseData()->results(RiaDefines::MATRIX_MODEL)->reportStepNumber(resultIndexWithMaxTimeSteps, timeStepIndex);

    if ( !currentRestartData->selectReportStep(reportStepNumber) )
    {
        QMessageBox::critical(nullptr, "ResInsight", "Flow Diagnostics: Could not find the requested timestep in the result file. Results will not be loaded.");
        return result;
    }


    // Set up flow Toolbox with timestep data
    std::map<Opm::FlowDiagnostics::CellSetID, Opm::FlowDiagnostics::CellSetValues> WellInFluxPrCell;

    {
        if (m_eclipseCase->eclipseCaseData()->results(RiaDefines::MATRIX_MODEL)->hasFlowDiagUsableFluxes())
        {
            Opm::FlowDiagnostics::ConnectionValues connectionsVals = RigFlowDiagInterfaceTools::extractFluxFieldFromRestartFile(*(m_opmFlowDiagStaticData->m_eclGraph),
                                                                                                                                *currentRestartData,
                                                                                                                                phaseSelection);
            m_opmFlowDiagStaticData->m_fldToolbox->assignConnectionFlux(connectionsVals);
        }
        else
        {
            Opm::ECLInitFileData init(getInitFileName());
            Opm::FlowDiagnostics::ConnectionValues connectionVals = RigFlowDiagInterfaceTools::calculateFluxField((*m_opmFlowDiagStaticData->m_eclGraph), 
                                                                                                                  init, 
                                                                                                                  *currentRestartData, 
                                                                                                                  phaseSelection);
            m_opmFlowDiagStaticData->m_fldToolbox->assignConnectionFlux(connectionVals);
        }


        progressInfo.incrementProgress();

        Opm::ECLWellSolution wsol = Opm::ECLWellSolution{-1.0 , false};

        std::vector<std::string> gridNames = m_opmFlowDiagStaticData->m_eclGraph->activeGrids();

        const std::vector<Opm::ECLWellSolution::WellData> well_fluxes = wsol.solution(*currentRestartData, gridNames);

        WellInFluxPrCell =  RigFlowDiagInterfaceTools::extractWellFlows(*(m_opmFlowDiagStaticData->m_eclGraph), well_fluxes);

        m_opmFlowDiagStaticData->m_fldToolbox->assignInflowFlux(WellInFluxPrCell);

        #if 0
        // Start Hack: Filter connection cells with inconsistent well in flow direction (Hack, we should do something better)
        for ( auto& tracerCellIdxsPair: injectorTracers )
        {
            std::vector<int> filteredCellIndices;
                        
            for (int activeCellIdx : tracerCellIdxsPair.second)
            {
                auto activeCellIdxFluxPair = WellInFluxPrCell.find(activeCellIdx);
                CVF_TIGHT_ASSERT(activeCellIdxFluxPair != WellInFluxPrCell.end());

                if (activeCellIdxFluxPair->second > 0 )
                {
                    filteredCellIndices.push_back(activeCellIdx);
                }
            }

            if (tracerCellIdxsPair.second.size() != filteredCellIndices.size()) tracerCellIdxsPair.second = filteredCellIndices;
        }

        for ( auto& tracerCellIdxsPair: producerTracers )
        {
            std::vector<int> filteredCellIndices;

            for (int activeCellIdx : tracerCellIdxsPair.second)
            {
                auto activeCellIdxFluxPair = WellInFluxPrCell.find(activeCellIdx);
                CVF_TIGHT_ASSERT(activeCellIdxFluxPair != WellInFluxPrCell.end());

                if (activeCellIdxFluxPair->second < 0 )
                {
                    filteredCellIndices.push_back(activeCellIdx);
                }
            }
            if (tracerCellIdxsPair.second.size() != filteredCellIndices.size()) tracerCellIdxsPair.second = filteredCellIndices;
        }
      

        // End Hack
        // New Filtering Probably not neccesary
        for ( auto& tracerCellIdxsPair: injectorTracers )
        {
            removeCrossFlowCells(tracerCellIdxsPair, WellInFluxPrCell, [](double inFlow){ return inFlow > 0;});
        }

        for ( auto& tracerCellIdxsPair: producerTracers )
        {
            removeCrossFlowCells(tracerCellIdxsPair, WellInFluxPrCell, [](double inFlow){ return inFlow < 0;});
        }
        #endif
    }

    progressInfo.incrementProgress();
    progressInfo.setProgressDescription("Injector Solution");

    {
        // Injection Solution
        std::set<std::string> injectorCrossFlowTracers;
        std::vector<CellSet> injectorCellSets;
        std::unique_ptr<Toolbox::Forward> injectorSolution;
        {
            for ( const auto& tIt: injectorTracers )
            {
                std::string tracerName = tIt.first;
                if (hasCrossFlowEnding(tracerName)) 
                {
                    tracerName = removeCrossFlowEnding(tracerName);
                    injectorCrossFlowTracers.insert(tracerName);
                }
                injectorCellSets.push_back(CellSet(CellSetID(tracerName), tIt.second));
            }

            try
            {
                injectorSolution.reset(new Toolbox::Forward(m_opmFlowDiagStaticData->m_fldToolbox->computeInjectionDiagnostics(injectorCellSets)));
            }
            catch ( const std::exception& e )
            {
                QMessageBox::critical(nullptr, "ResInsight", "Flow Diagnostics: " + QString(e.what()));
                return result;
            }

            for ( const CellSetID& tracerId: injectorSolution->fd.startPoints() )
            {
                std::string tracername = tracerId.to_string();
                if (injectorCrossFlowTracers.count(tracername)) tracername = addCrossFlowEnding(tracername);

                CellSetValues tofVals = injectorSolution->fd.timeOfFlight(tracerId);
                result.setTracerTOF(tracername, phaseSelection, tofVals);
                CellSetValues fracVals = injectorSolution->fd.concentration(tracerId);
                result.setTracerFraction(tracername, phaseSelection, fracVals);
            }
        }

        progressInfo.incrementProgress();
        progressInfo.setProgressDescription("Producer Solution");

        // Producer Solution
        std::set<std::string> producerCrossFlowTracers;
        std::vector<CellSet> prodjCellSets;
        std::unique_ptr<Toolbox::Reverse> producerSolution;
        {
            for ( const auto& tIt: producerTracers )
            {
                std::string tracerName = tIt.first;
                if (hasCrossFlowEnding(tracerName)) 
                {
                    tracerName = removeCrossFlowEnding(tracerName);
                    producerCrossFlowTracers.insert(tracerName);
                }
                prodjCellSets.push_back(CellSet(CellSetID(tracerName), tIt.second));
            }

            try
            {
                producerSolution.reset(new Toolbox::Reverse(m_opmFlowDiagStaticData->m_fldToolbox->computeProductionDiagnostics(prodjCellSets)));
            }
            catch ( const std::exception& e )
            {
                QMessageBox::critical(nullptr, "ResInsight", "Flow Diagnostics: " + QString(e.what()));
                return result;
            }

            for ( const CellSetID& tracerId: producerSolution->fd.startPoints() )
            {
                std::string tracername = tracerId.to_string();
                if (producerCrossFlowTracers.count(tracername)) tracername = addCrossFlowEnding(tracername);

                CellSetValues tofVals = producerSolution->fd.timeOfFlight(tracerId);
                result.setTracerTOF(tracername, phaseSelection, tofVals);
                CellSetValues fracVals = producerSolution->fd.concentration(tracerId);
                result.setTracerFraction(tracername, phaseSelection, fracVals);
            }
        }

        progressInfo.incrementProgress();
        progressInfo.setProgressDescription("Well pair fluxes");
        
        int producerTracerCount = static_cast<int>( prodjCellSets.size());

        #pragma omp parallel for
        for ( int pIdx = 0; pIdx < producerTracerCount; ++pIdx )
        {
            const auto& prodCellSet = prodjCellSets[pIdx];

            std::string prodTracerName = prodCellSet.id().to_string();
            CellSetID prodID(prodTracerName);

            std::string uiProducerTracerName = prodTracerName;
            if (producerCrossFlowTracers.count(prodTracerName)) 
            {
                uiProducerTracerName = addCrossFlowEnding(prodTracerName);
            }

            for ( const auto& injCellSet : injectorCellSets )
            {
                std::string injTracerName = injCellSet.id().to_string();
                CellSetID injID(injTracerName);

                std::pair<double, double> fluxPair = injectorProducerPairFlux(*(injectorSolution.get()), 
                                                                              *(producerSolution.get()),
                                                                              injID, 
                                                                              prodID,
                                                                              WellInFluxPrCell);
                std::string uiInjectorTracerName = injTracerName;

                if (injectorCrossFlowTracers.count(injTracerName)) 
                {
                    uiInjectorTracerName = addCrossFlowEnding(injTracerName);
                }

               
                #pragma omp critical
                {
                    result.setInjProdWellPairFlux(uiInjectorTracerName,
                                                  uiProducerTracerName,
                                                  fluxPair);
                }
            }
        }
    }

    return result; // Relying on implicit move constructor
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigFlowDiagSolverInterface::ensureStaticDataObjectInstanceCreated()
{
    if (m_opmFlowDiagStaticData.isNull())
    {
        // Get set of files
        QString gridFileName = m_eclipseCase->gridFileName();
        std::string initFileName = getInitFileName();
        if (initFileName.empty()) return false;

        m_opmFlowDiagStaticData = new RigOpmFlowDiagStaticData(gridFileName.toStdString(), initFileName);
    }

    return m_opmFlowDiagStaticData.notNull() ? true : false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFlowDiagSolverInterface::FlowCharacteristicsResultFrame RigFlowDiagSolverInterface::calculateFlowCharacteristics(const std::vector<double>* injector_tof,
                                                                                                                    const std::vector<double>* producer_tof,
                                                                                                                    const std::vector<size_t>& selected_cell_indices,
                                                                                                                    double max_pv_fraction)
{
    using namespace Opm::FlowDiagnostics;
    RigFlowDiagSolverInterface::FlowCharacteristicsResultFrame result;

    if (injector_tof == nullptr || producer_tof == nullptr)
    {
        return result;
    }
    
    std::vector<double> poreVolume;
    for (size_t cellIndex : selected_cell_indices)
    {
        poreVolume.push_back(m_opmFlowDiagStaticData->m_poreVolume[cellIndex]);
    }

    try
    {
        Graph flowCapStorCapCurve = flowCapacityStorageCapacityCurve(*injector_tof,
                                                                     *producer_tof,
                                                                     poreVolume,
                                                                     max_pv_fraction);

        result.m_flowCapStorageCapCurve = flowCapStorCapCurve;
        result.m_lorenzCoefficient = lorenzCoefficient(flowCapStorCapCurve);
        result.m_sweepEfficiencyCurve = sweepEfficiency(flowCapStorCapCurve);
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(nullptr, "ResInsight", "Flow Diagnostics: " + QString(e.what()));
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigFlowDiagSolverInterface::RelPermCurve> RigFlowDiagSolverInterface::calculateRelPermCurvesForActiveCell(size_t activeCellIndex)
{
    std::vector<RelPermCurve> retCurveArr;

    if (!ensureStaticDataObjectInstanceCreated())
    {
        return retCurveArr;
    }

    CVF_ASSERT(m_opmFlowDiagStaticData.notNull());
    CVF_ASSERT(m_opmFlowDiagStaticData->m_eclSaturationFunc);

    const Opm::ECLSaturationFunc::RawCurve krw  { Opm::ECLSaturationFunc::RawCurve::Function::RelPerm,  Opm::ECLSaturationFunc::RawCurve::SubSystem::OilWater, Opm::ECLPhaseIndex::Aqua };    // water rel-perm in oil-water system
    const Opm::ECLSaturationFunc::RawCurve krg  { Opm::ECLSaturationFunc::RawCurve::Function::RelPerm,  Opm::ECLSaturationFunc::RawCurve::SubSystem::OilGas,   Opm::ECLPhaseIndex::Vapour };  // gas rel-perm in oil-gas system
    const Opm::ECLSaturationFunc::RawCurve krow { Opm::ECLSaturationFunc::RawCurve::Function::RelPerm,  Opm::ECLSaturationFunc::RawCurve::SubSystem::OilWater, Opm::ECLPhaseIndex::Liquid };  // oil rel-perm in oil-water system
    const Opm::ECLSaturationFunc::RawCurve krog { Opm::ECLSaturationFunc::RawCurve::Function::RelPerm,  Opm::ECLSaturationFunc::RawCurve::SubSystem::OilGas,   Opm::ECLPhaseIndex::Liquid };  // oil rel-perm in oil-gas system
    const Opm::ECLSaturationFunc::RawCurve pcgo { Opm::ECLSaturationFunc::RawCurve::Function::CapPress, Opm::ECLSaturationFunc::RawCurve::SubSystem::OilGas,   Opm::ECLPhaseIndex::Vapour };  // gas/oil capillary pressure (Pg-Po) in G/O system
    const Opm::ECLSaturationFunc::RawCurve pcow { Opm::ECLSaturationFunc::RawCurve::Function::CapPress, Opm::ECLSaturationFunc::RawCurve::SubSystem::OilWater, Opm::ECLPhaseIndex::Aqua };    // oil/water capillary pressure (Po-Pw) in O/W system

    std::vector<std::pair<RelPermCurve::Ident, std::string>> curveIdentNameArr;
    std::vector<Opm::ECLSaturationFunc::RawCurve> satFuncRequests;
    curveIdentNameArr.push_back(std::make_pair(RelPermCurve::KRW,  "KRW"));    satFuncRequests.push_back(krw);
    curveIdentNameArr.push_back(std::make_pair(RelPermCurve::KRG,  "KRG"));    satFuncRequests.push_back(krg);
    curveIdentNameArr.push_back(std::make_pair(RelPermCurve::KROW, "KROW"));   satFuncRequests.push_back(krow);
    curveIdentNameArr.push_back(std::make_pair(RelPermCurve::KROG, "KROG"));   satFuncRequests.push_back(krog);
    curveIdentNameArr.push_back(std::make_pair(RelPermCurve::PCOG, "PCOG"));   satFuncRequests.push_back(pcgo);
    curveIdentNameArr.push_back(std::make_pair(RelPermCurve::PCOW, "PCOW"));   satFuncRequests.push_back(pcow);

    // Calculate and return curves both with and without endpoint scaling and tag them accordingly
    // Must use two calls to achieve this
    const std::array<RelPermCurve::EpsMode, 2> epsModeArr = { RelPermCurve::EPS_ON , RelPermCurve::EPS_OFF };
    for (RelPermCurve::EpsMode epsMode : epsModeArr)
    {
        const bool useEps = epsMode == RelPermCurve::EPS_ON ? true : false;
        std::vector<Opm::FlowDiagnostics::Graph> graphArr = m_opmFlowDiagStaticData->m_eclSaturationFunc->getSatFuncCurve(satFuncRequests, static_cast<int>(activeCellIndex), useEps);
        for (size_t i = 0; i < graphArr.size(); i++)
        {
            const RelPermCurve::Ident curveIdent = curveIdentNameArr[i].first;
            const std::string curveName = curveIdentNameArr[i].second;
            const Opm::FlowDiagnostics::Graph& srcGraph = graphArr[i];
            if (srcGraph.first.size() > 0)
            {
                const std::vector<double>& xVals = srcGraph.first;
                const std::vector<double>& yVals = srcGraph.second;
                retCurveArr.push_back({ curveIdent, curveName, epsMode, xVals, yVals });
            }
        }
    }

    return retCurveArr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigFlowDiagSolverInterface::PvtCurve> RigFlowDiagSolverInterface::calculatePvtCurvesForActiveCell(PvtCurveType pvtCurveType, size_t activeCellIndex)
{
    std::vector<PvtCurve> retCurveArr;

    if (!ensureStaticDataObjectInstanceCreated())
    {
        return retCurveArr;
    }

    CVF_ASSERT(m_opmFlowDiagStaticData.notNull());
    if (!m_opmFlowDiagStaticData->m_eclPvtCurveCollection)
    {
        return retCurveArr;
    }


    // Requesting FVF or Viscosity
    const Opm::ECLPVT::RawCurve rawCurveType = (pvtCurveType == PvtCurveType::PVT_CT_FVF) ? Opm::ECLPVT::RawCurve::FVF : Opm::ECLPVT::RawCurve::Viscosity;

    const std::array<Opm::ECLPhaseIndex, 3> queryPhaseArr = { Opm::ECLPhaseIndex::Vapour, Opm::ECLPhaseIndex::Liquid, Opm::ECLPhaseIndex::Aqua };
    const std::array<PvtCurve::Phase, 3>    mapToPhaseArr = { PvtCurve::GAS,              PvtCurve::OIL,              PvtCurve::WATER };

    for (size_t i = 0; i < queryPhaseArr.size(); i++)
    {
        const Opm::ECLPhaseIndex queryPhaseIndex = queryPhaseArr[i];
        const PvtCurve::Phase mapToPhase = mapToPhaseArr[i];

        std::vector<Opm::FlowDiagnostics::Graph> graphArr = m_opmFlowDiagStaticData->m_eclPvtCurveCollection->getPvtCurve(rawCurveType, queryPhaseIndex, static_cast<int>(activeCellIndex));
        for (Opm::FlowDiagnostics::Graph srcGraph : graphArr)
        {
            if (srcGraph.first.size() > 0)
            {
                retCurveArr.push_back({ mapToPhase, srcGraph.first, srcGraph.second });
            }
        }
    }

    return retCurveArr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RigFlowDiagSolverInterface::getInitFileName() const
{
    QString gridFileName = m_eclipseCase->gridFileName();

    QStringList m_filesWithSameBaseName;

    if (!RifEclipseOutputFileTools::findSiblingFilesWithSameBaseName(gridFileName, &m_filesWithSameBaseName)) return std::string();

    QString initFileName = RifEclipseOutputFileTools::firstFileNameOfType(m_filesWithSameBaseName, ECL_INIT_FILE);

    return initFileName.toStdString();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFlowDiagSolverInterface::FlowCharacteristicsResultFrame::FlowCharacteristicsResultFrame()
    : m_lorenzCoefficient(HUGE_VAL)
{

}
