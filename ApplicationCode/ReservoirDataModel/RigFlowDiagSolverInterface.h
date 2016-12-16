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
#pragma once

#include "RigFlowDiagResultAddress.h"
#include "cvfBase.h"
#include "cvfObject.h"
#include "cafPdmPointer.h"

#include <string>
#include <vector>
#include <map>

class RimEclipseResultCase;

struct RigTracerCells
{
    std::string tracerName;
    std::vector<int> tracerCellActiveIndices;
};



class RigFlowDiagTimeStepResult: public cvf::Object
{
public:
    RigFlowDiagTimeStepResult(size_t activeCellCount);

    void setInjectorTracerTOF     (const std::string& tracerName, const std::map<int, double>& cellValues);
    void setInjectorTracerFraction(const std::string& tracerName, const std::map<int, double>& cellValues);
    void setProducerTracerTOF     (const std::string& tracerName, const std::map<int, double>& cellValues);
    void setProducerTracerFraction(const std::string& tracerName, const std::map<int, double>& cellValues);

    std::vector<double>& nativeResult(const RigFlowDiagResultAddress& resAddr); // Use to "steal" the data from this one using swap

private:

    void addResult(const RigFlowDiagResultAddress& resAddr, const std::map<int, double>& cellValues);

    std::map<RigFlowDiagResultAddress, std::vector<double> > m_nativeResults;
};




class RigFlowDiagSolverInterface : public cvf::Object
{
public:
    RigFlowDiagSolverInterface(RimEclipseResultCase* eclCase);
    virtual ~RigFlowDiagSolverInterface();

    cvf::ref<RigFlowDiagTimeStepResult> calculate(size_t timestep, const std::vector<RigTracerCells > & allTracerData);

private:
    caf::PdmPointer<RimEclipseResultCase> m_eclipseCase;    
};



