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
#include "RimFlowDiagSolution.h"
#include "RimEclipseResultCase.h"
#include "RigCaseCellResultsData.h"

#include "RigFlowDiagInterfaceTools.h"
#include "RifEclipseOutputFileTools.h"
#include "RigCaseData.h"
#include "RimEclipseCase.h"
#include "RifReaderInterface.h"
#include <QMessageBox>

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
void RigFlowDiagTimeStepResult::setInjectorTracerTOF(const std::string& tracerName, const std::map<int, double>& cellValues)
{
    std::set<std::string> tracers; 
    tracers.insert(tracerName);

    this->addResult(RigFlowDiagResultAddress(RIG_FLD_TOF_RESNAME, tracers), cellValues);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagTimeStepResult::setInjectorTracerFraction(const std::string& tracerName, const std::map<int, double>& cellValues)
{
    std::set<std::string> tracers;
    tracers.insert(tracerName);

    this->addResult(RigFlowDiagResultAddress(RIG_FLD_CELL_FRACTION_RESNAME, tracers), cellValues);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagTimeStepResult::setProducerTracerTOF(const std::string& tracerName, const std::map<int, double>& cellValues)
{
    std::set<std::string> tracers;
    tracers.insert(tracerName);

    this->addResult(RigFlowDiagResultAddress(RIG_FLD_TOF_RESNAME, tracers), cellValues);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagTimeStepResult::setProducerTracerFraction(const std::string& tracerName, const std::map<int, double>& cellValues)
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

    // Get set of files
    QString gridFileName = m_eclipseCase->gridFileName();

    QStringList m_filesWithSameBaseName;

    if ( !RifEclipseOutputFileTools::findSiblingFilesWithSameBaseName(gridFileName, &m_filesWithSameBaseName) ) return result;

    QString initFileName = RifEclipseOutputFileTools::firstFileNameOfType(m_filesWithSameBaseName, ECL_INIT_FILE);

    Opm::ECLGraph graph = Opm::ECLGraph::load(gridFileName.toStdString(),
                                              initFileName.toStdString());

    // Look for unified restart file
    QString restartFileName = RifEclipseOutputFileTools::firstFileNameOfType(m_filesWithSameBaseName, ECL_UNIFIED_RESTART_FILE);

    if ( restartFileName.isEmpty() )
    {
        // Look for set of restart files (one file per time step)

        QStringList restartFileNames;
        restartFileNames = RifEclipseOutputFileTools::filterFileNamesOfType(m_filesWithSameBaseName, ECL_RESTART_FILE);
        if (restartFileNames.size() <= timeStepIndex &&  restartFileNames.size() != m_eclipseCase->reservoirData()->results(RifReaderInterface::MATRIX_RESULTS)->maxTimeStepCount() )
        {
            QMessageBox::critical(nullptr, "ResInsight", "Flow Diagnostics: Could not find all the restart files. Results will not be loaded.");
            return result;
        }

        restartFileNames.sort(); // To make sure they are sorted in increasing *.X000N order. Hack. Should probably be actual time stored on file.
        restartFileName = restartFileNames[static_cast<int>(timeStepIndex)];
    }

    std::string casePath;
    graph.assignFluxDataSource(restartFileName.toStdString());

    if ( ! graph.selectReportStep(static_cast<int>(timeStepIndex)) )
    {
        QMessageBox::critical(nullptr, "ResInsight", "Flow Diagnostics: Could not find the requested timestep in the result file. Results will not be loaded.");
        return result;
    }

    {
        Toolbox fdTool = RigFlowDiagInterfaceTools::initialiseFlowDiagnostics(graph);
        {
            std::vector<CellSet> injectorCellSet;
            for ( const auto& tIt: injectorTracers )
            {
                injectorCellSet.push_back(CellSet(CellSetID(tIt.first), tIt.second));
            }

            Solution injSol = fdTool.computeInjectionDiagnostics(injectorCellSet).fd;

            for ( const CellSetID& tracerId: injSol.startPoints() )
            {
                CellSetValues tofVals = injSol.timeOfFlight(tracerId);
                result.setInjectorTracerTOF(tracerId.to_string(), tofVals);
                CellSetValues fracVals = injSol.concentration(tracerId);
                result.setInjectorTracerFraction(tracerId.to_string(), fracVals);
            }
        }

        {
            std::vector<CellSet> prodjCellSet;
            for ( const auto& tIt: producerTracers )
            {
                prodjCellSet.push_back(CellSet(CellSetID(tIt.first), tIt.second));
            }
            Solution prodSol = fdTool.computeProductionDiagnostics(prodjCellSet).fd;
            for ( const CellSetID& tracerId: prodSol.startPoints() )
            {
                CellSetValues tofVals = prodSol.timeOfFlight(tracerId);
                result.setProducerTracerTOF(tracerId.to_string(), tofVals);
                CellSetValues fracVals = prodSol.concentration(tracerId);
                result.setInjectorTracerFraction(tracerId.to_string(), fracVals);
            }
        }
    }


    return result; // Relying on implicit move constructor
}

