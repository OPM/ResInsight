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
void RigFlowDiagTimeStepResult::setTracerTOF(const std::string& tracerName, const std::map<int, double>& cellValues)
{
    std::set<std::string> tracers;
    tracers.insert(tracerName);
    
    RigFlowDiagResultAddress resAddr(RIG_FLD_TOF_RESNAME, tracers);

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
void RigFlowDiagTimeStepResult::setTracerFraction(const std::string& tracerName, const std::map<int, double>& cellValues)
{
    std::set<std::string> tracers;
    tracers.insert(tracerName);

    this->addResult(RigFlowDiagResultAddress(RIG_FLD_CELL_FRACTION_RESNAME, tracers), cellValues);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagTimeStepResult::addResult(const RigFlowDiagResultAddress& resAddr, const std::map<int, double>& cellValues)
{
    std::vector<double>& activeCellValues =  m_nativeResults[resAddr];
    activeCellValues.resize(m_activeCellCount, HUGE_VAL);

    for (const auto& pairIt : cellValues)
    {
        activeCellValues[pairIt.first]  = pairIt.second;
    }
}


class RigOpmFldStaticData : public cvf::Object
{
public:
    RigOpmFldStaticData(const std::string& grid, const std::string& init) : eclGraph(Opm::ECLGraph::load(grid, init)), m_hasUnifiedRestartFile(false) {}

    Opm::ECLGraph eclGraph;
    std::unique_ptr<Opm::FlowDiagnostics::Toolbox> fldToolbox;
    bool m_hasUnifiedRestartFile;
    QStringList restartFileNames;
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
                                                                std::map<std::string, std::vector<int> > injectorTracers,
                                                                std::map<std::string, std::vector<int> > producerTracers)
{
    using namespace Opm::FlowDiagnostics;

    RigFlowDiagTimeStepResult result(m_eclipseCase->reservoirData()->activeCellInfo(RifReaderInterface::MATRIX_RESULTS)->reservoirActiveCellCount());

    caf::ProgressInfo progressInfo(7, "Calculating Flow Diagnostics");
   

    if ( m_opmFldData.isNull() )
    {
        progressInfo.setProgressDescription("Grid access");

        // Get set of files
        QString gridFileName = m_eclipseCase->gridFileName();

        QStringList m_filesWithSameBaseName;

        if ( !RifEclipseOutputFileTools::findSiblingFilesWithSameBaseName(gridFileName, &m_filesWithSameBaseName) ) return result;

        QString initFileName = RifEclipseOutputFileTools::firstFileNameOfType(m_filesWithSameBaseName, ECL_INIT_FILE);

        m_opmFldData = new RigOpmFldStaticData(gridFileName.toStdString(),
                                               initFileName.toStdString());

        progressInfo.incrementProgress();
        progressInfo.setProgressDescription("Calculating Connectivities");

        const  Opm::FlowDiagnostics::ConnectivityGraph connGraph =
            Opm::FlowDiagnostics::ConnectivityGraph{ static_cast<int>(m_opmFldData->eclGraph.numCells()),
            m_opmFldData->eclGraph.neighbours() };

        progressInfo.incrementProgress();
        progressInfo.setProgressDescription("Initialize Solver");

        // Create the Toolbox.

        m_opmFldData->fldToolbox.reset(new Opm::FlowDiagnostics::Toolbox{ connGraph });
        m_opmFldData->fldToolbox->assignPoreVolume( m_opmFldData->eclGraph.poreVolume());

        // Look for unified restart file

        QString restartFileName = RifEclipseOutputFileTools::firstFileNameOfType(m_filesWithSameBaseName, ECL_UNIFIED_RESTART_FILE);
        if ( !restartFileName.isEmpty() )
        {
            m_opmFldData->eclGraph.assignFluxDataSource(restartFileName.toStdString());
            m_opmFldData->m_hasUnifiedRestartFile = true;
        }
        else
        {

            m_opmFldData->restartFileNames = RifEclipseOutputFileTools::filterFileNamesOfType(m_filesWithSameBaseName, ECL_RESTART_FILE);

            size_t restartFileCount = static_cast<size_t>(m_opmFldData->restartFileNames.size());
            size_t maxTimeStepCount = m_eclipseCase->reservoirData()->results(RifReaderInterface::MATRIX_RESULTS)->maxTimeStepCount();

            if (restartFileCount <= timeStepIndex &&  restartFileCount !=  maxTimeStepCount )
            {
                QMessageBox::critical(nullptr, "ResInsight", "Flow Diagnostics: Could not find all the restart files. Results will not be loaded.");
                return result;
            }

            m_opmFldData->restartFileNames.sort(); // To make sure they are sorted in increasing *.X000N order. Hack. Should probably be actual time stored on file.
            m_opmFldData->m_hasUnifiedRestartFile = false;
        }
    }

    progressInfo.setProgress(3);
    progressInfo.setProgressDescription("Assigning Flux Field");

    if ( ! m_opmFldData->m_hasUnifiedRestartFile  )
    {
        QString restartFileName = m_opmFldData->restartFileNames[static_cast<int>(timeStepIndex)];
        m_opmFldData->eclGraph.assignFluxDataSource(restartFileName.toStdString());
    }

    size_t resultIndexWithMaxTimeSteps = cvf::UNDEFINED_SIZE_T;
    m_eclipseCase->reservoirData()->results(RifReaderInterface::MATRIX_RESULTS)->maxTimeStepCount(&resultIndexWithMaxTimeSteps);

    int reportStepNumber =  m_eclipseCase->reservoirData()->results(RifReaderInterface::MATRIX_RESULTS)->reportStepNumber(resultIndexWithMaxTimeSteps, timeStepIndex);

    if ( !  m_opmFldData->eclGraph.selectReportStep(reportStepNumber) )
    {
        QMessageBox::critical(nullptr, "ResInsight", "Flow Diagnostics: Could not find the requested timestep in the result file. Results will not be loaded.");
        return result;
    }


    // Set up flow Toolbox with timestep data
    {
        Opm::FlowDiagnostics::ConnectionValues connectionsVals = RigFlowDiagInterfaceTools::extractFluxField(m_opmFldData->eclGraph, false);

        m_opmFldData->fldToolbox->assignConnectionFlux(connectionsVals);

        progressInfo.incrementProgress();

        Opm::ECLWellSolution wsol = Opm::ECLWellSolution{-1.0 , false};

        const std::vector<Opm::ECLWellSolution::WellData> well_fluxes =
            wsol.solution(m_opmFldData->eclGraph.rawResultData(), m_opmFldData->eclGraph.numGrids());

        m_opmFldData->fldToolbox->assignInflowFlux(RigFlowDiagInterfaceTools::extractWellFlows(m_opmFldData->eclGraph, well_fluxes));
    }

    progressInfo.incrementProgress();
    progressInfo.setProgressDescription("Injector Solution");

    // Injection Solution
    {
        std::vector<CellSet> injectorCellSet;
        for ( const auto& tIt: injectorTracers )
        {
            injectorCellSet.push_back(CellSet(CellSetID(tIt.first), tIt.second));
        }

        Solution injSol = m_opmFldData->fldToolbox->computeInjectionDiagnostics(injectorCellSet).fd;

        for ( const CellSetID& tracerId: injSol.startPoints() )
        {
            CellSetValues tofVals = injSol.timeOfFlight(tracerId);
            result.setTracerTOF(tracerId.to_string(), tofVals);
            CellSetValues fracVals = injSol.concentration(tracerId);
            result.setTracerFraction(tracerId.to_string(), fracVals);
        }
    }

    progressInfo.incrementProgress();
    progressInfo.setProgressDescription("Producer Solution");

    // Producer Solution
    {
        std::vector<CellSet> prodjCellSet;
        for ( const auto& tIt: producerTracers )
        {
            prodjCellSet.push_back(CellSet(CellSetID(tIt.first), tIt.second));
        }

        Solution prodSol = m_opmFldData->fldToolbox->computeProductionDiagnostics(prodjCellSet).fd;
        
        for ( const CellSetID& tracerId: prodSol.startPoints() )
        {
            CellSetValues tofVals = prodSol.timeOfFlight(tracerId);
            result.setTracerTOF(tracerId.to_string(), tofVals);
            CellSetValues fracVals = prodSol.concentration(tracerId);
            result.setTracerFraction(tracerId.to_string(), fracVals);
        }
    }

    return result; // Relying on implicit move constructor
}

