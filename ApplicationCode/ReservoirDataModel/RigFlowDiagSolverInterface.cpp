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

#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimFlowDiagSolution.h"

#include <QMessageBox>
#include "cafProgressInfo.h"

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
    }

    std::unique_ptr<Opm::ECLGraph>                  m_eclGraph;
    std::vector<double>                             m_poreVolume;
    std::unique_ptr<Opm::FlowDiagnostics::Toolbox>  m_fldToolbox;
    bool                                            m_hasUnifiedRestartFile;
    std::vector<Opm::ECLRestartData>                m_singleRestartDataTimeSteps;
    std::unique_ptr<Opm::ECLRestartData>            m_unifiedRestartData;
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


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFlowDiagTimeStepResult RigFlowDiagSolverInterface::calculate(size_t timeStepIndex,
                                                                RigFlowDiagResultAddress::PhaseSelection phaseSelection,
                                                                std::map<std::string, std::vector<int> > injectorTracers,
                                                                std::map<std::string, std::vector<int> > producerTracers)
{
    using namespace Opm::FlowDiagnostics;

    RigFlowDiagTimeStepResult result(m_eclipseCase->eclipseCaseData()->activeCellInfo(RiaPorosityModel::MATRIX_MODEL)->reservoirActiveCellCount());

    caf::ProgressInfo progressInfo(8, "Calculating Flow Diagnostics");

    if ( m_opmFlowDiagStaticData.isNull() )
    {
        progressInfo.setProgressDescription("Grid access");

        // Get set of files
        QString gridFileName = m_eclipseCase->gridFileName();

        std::string initFileName = getInitFileName();

        if (initFileName.empty()) return result;

        m_opmFlowDiagStaticData = new RigOpmFlowDiagStaticData(gridFileName.toStdString(), initFileName);

        progressInfo.incrementProgress();
        progressInfo.setProgressDescription("Calculating Connectivities");

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
            size_t maxTimeStepCount = m_eclipseCase->eclipseCaseData()->results(RiaPorosityModel::MATRIX_MODEL)->maxTimeStepCount();

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
    m_eclipseCase->eclipseCaseData()->results(RiaPorosityModel::MATRIX_MODEL)->maxTimeStepCount(&resultIndexWithMaxTimeSteps);

    int reportStepNumber =  m_eclipseCase->eclipseCaseData()->results(RiaPorosityModel::MATRIX_MODEL)->reportStepNumber(resultIndexWithMaxTimeSteps, timeStepIndex);

    if ( !currentRestartData->selectReportStep(reportStepNumber) )
    {
        QMessageBox::critical(nullptr, "ResInsight", "Flow Diagnostics: Could not find the requested timestep in the result file. Results will not be loaded.");
        return result;
    }


    // Set up flow Toolbox with timestep data
    Opm::FlowDiagnostics::CellSetValues sumWellFluxPrCell;

    {
        if (m_eclipseCase->eclipseCaseData()->results(RiaPorosityModel::MATRIX_MODEL)->hasFlowDiagUsableFluxes())
        {
            Opm::FlowDiagnostics::ConnectionValues connectionsVals = RigFlowDiagInterfaceTools::extractFluxFieldFromRestartFile(*(m_opmFlowDiagStaticData->m_eclGraph),
                                                                                                                                *currentRestartData,
                                                                                                                                phaseSelection);
            m_opmFlowDiagStaticData->m_fldToolbox->assignConnectionFlux(connectionsVals);
        }
        else
        {
            Opm::ECLInitFileData init(getInitFileName());
            Opm::FlowDiagnostics::ConnectionValues connectionVals = RigFlowDiagInterfaceTools::calculateFluxField((*m_opmFlowDiagStaticData->m_eclGraph), init, *currentRestartData, phaseSelection);
            m_opmFlowDiagStaticData->m_fldToolbox->assignConnectionFlux(connectionVals);
        }


        progressInfo.incrementProgress();

        Opm::ECLWellSolution wsol = Opm::ECLWellSolution{-1.0 , false};

        std::vector<std::string> gridNames = m_opmFlowDiagStaticData->m_eclGraph->activeGrids();

        const std::vector<Opm::ECLWellSolution::WellData> well_fluxes = wsol.solution(*currentRestartData, gridNames);

        sumWellFluxPrCell =  RigFlowDiagInterfaceTools::extractWellFlows(*(m_opmFlowDiagStaticData->m_eclGraph), well_fluxes);

        m_opmFlowDiagStaticData->m_fldToolbox->assignInflowFlux(sumWellFluxPrCell);

        // Start Hack: Filter connection cells with inconsistent well in flow direction (Hack, we should do something better)

        for ( auto& tracerCellIdxsPair: injectorTracers )
        {
            std::vector<int> filteredCellIndices;
                        
            for (int activeCellIdx : tracerCellIdxsPair.second)
            {
                auto activeCellIdxFluxPair = sumWellFluxPrCell.find(activeCellIdx);
                CVF_TIGHT_ASSERT(activeCellIdxFluxPair != sumWellFluxPrCell.end());

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
                auto activeCellIdxFluxPair = sumWellFluxPrCell.find(activeCellIdx);
                CVF_TIGHT_ASSERT(activeCellIdxFluxPair != sumWellFluxPrCell.end());

                if (activeCellIdxFluxPair->second < 0 )
                {
                    filteredCellIndices.push_back(activeCellIdx);
                }
            }
            if (tracerCellIdxsPair.second.size() != filteredCellIndices.size()) tracerCellIdxsPair.second = filteredCellIndices;
        }

        // End Hack
    }

    progressInfo.incrementProgress();
    progressInfo.setProgressDescription("Injector Solution");

    {
        // Injection Solution

        std::vector<CellSet> injectorCellSets;
        for ( const auto& tIt: injectorTracers )
        {
            injectorCellSets.push_back(CellSet(CellSetID(tIt.first), tIt.second));
        }

        std::unique_ptr<Toolbox::Forward> injectorSolution;
        try 
        {
            injectorSolution.reset(new Toolbox::Forward( m_opmFlowDiagStaticData->m_fldToolbox->computeInjectionDiagnostics(injectorCellSets)));
        }
        catch (const std::exception& e)
        {
            QMessageBox::critical(nullptr, "ResInsight", "Flow Diagnostics: " + QString(e.what()));
            return result;
        }

        for ( const CellSetID& tracerId: injectorSolution->fd.startPoints() )
        {
            CellSetValues tofVals = injectorSolution->fd.timeOfFlight(tracerId);
            result.setTracerTOF(tracerId.to_string(), phaseSelection, tofVals);
            CellSetValues fracVals = injectorSolution->fd.concentration(tracerId);
            result.setTracerFraction(tracerId.to_string(), phaseSelection, fracVals);
        }

        progressInfo.incrementProgress();
        progressInfo.setProgressDescription("Producer Solution");

        // Producer Solution

        std::vector<CellSet> prodjCellSets;
        for ( const auto& tIt: producerTracers )
        {
            prodjCellSets.push_back(CellSet(CellSetID(tIt.first), tIt.second));
        }

        std::unique_ptr<Toolbox::Reverse> producerSolution;
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
            CellSetValues tofVals = producerSolution->fd.timeOfFlight(tracerId);
            result.setTracerTOF(tracerId.to_string(), phaseSelection, tofVals);
            CellSetValues fracVals = producerSolution->fd.concentration(tracerId);
            result.setTracerFraction(tracerId.to_string(), phaseSelection, fracVals);
        }

        progressInfo.incrementProgress();
        progressInfo.setProgressDescription("Well pair fluxes");
        
        int producerTracerCount = static_cast<int>( prodjCellSets.size());

        #pragma omp parallel for
        for ( int pIdx = 0; pIdx < producerTracerCount; ++pIdx )
        {
            const auto& prodCellSet = prodjCellSets[pIdx];

            for ( const auto& injCellSet : injectorCellSets )
            {
                std::pair<double, double> fluxPair = injectorProducerPairFlux(*(injectorSolution.get()), 
                                                                              *(producerSolution.get()),
                                                                              injCellSet, 
                                                                              prodCellSet,
                                                                              sumWellFluxPrCell);
                #pragma omp critical
                {
                    result.setInjProdWellPairFlux(injCellSet.id().to_string(),
                                                  prodCellSet.id().to_string(),
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
RigFlowDiagSolverInterface::FlowCharacteristicsResultFrame RigFlowDiagSolverInterface::calculateFlowCharacteristics(const std::vector<double>* injector_tof,
                                                                                                                    const std::vector<double>* producer_tof,
                                                                                                                    double max_pv_fraction)
{
    using namespace Opm::FlowDiagnostics;
    RigFlowDiagSolverInterface::FlowCharacteristicsResultFrame result;

    if (injector_tof == nullptr || producer_tof == nullptr)
    {
        return result;
    }

    try
    {
        Graph flowCapStorCapCurve = flowCapacityStorageCapacityCurve(*injector_tof,
                                                                     *producer_tof,
                                                                     m_opmFlowDiagStaticData->m_poreVolume,
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
