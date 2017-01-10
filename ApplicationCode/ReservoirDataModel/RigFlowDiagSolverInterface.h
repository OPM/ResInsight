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
class RimFlowDiagSolution;


class RigFlowDiagTimeStepResult
{
public:
    RigFlowDiagTimeStepResult(size_t activeCellCount);

    void setTracerTOF     (const std::string& tracerName, const std::map<int, double>& cellValues);
    void setTracerFraction(const std::string& tracerName, const std::map<int, double>& cellValues);

    // Use to "steal" the data from this one using swap
    std::map<RigFlowDiagResultAddress, std::vector<double> >&  nativeResults() { return m_nativeResults; }
private:

    void addResult(const RigFlowDiagResultAddress& resAddr, const std::map<int, double>& cellValues);

    std::map<RigFlowDiagResultAddress, std::vector<double> > m_nativeResults;
    size_t m_activeCellCount;
};


class RigEclipseCaseData;
class RigOpmFldStaticData;

class RigFlowDiagSolverInterface : public cvf::Object
{
public:
    RigFlowDiagSolverInterface(RimEclipseResultCase * eclipseCase);
    virtual ~RigFlowDiagSolverInterface();

    RigFlowDiagTimeStepResult calculate(size_t timestep,  
                                        std::map<std::string, std::vector<int> > injectorTracers, 
                                        std::map<std::string, std::vector<int> > producerTracers);

private:
    RimEclipseResultCase * m_eclipseCase;

    cvf::ref<RigOpmFldStaticData> m_opmFldData;
   
};



