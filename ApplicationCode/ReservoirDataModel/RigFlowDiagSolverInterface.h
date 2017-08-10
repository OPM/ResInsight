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
    explicit RigFlowDiagTimeStepResult(size_t activeCellCount);

    void setTracerTOF     (const std::string& tracerName, const std::map<int, double>& cellValues);
    void setTracerFraction(const std::string& tracerName, const std::map<int, double>& cellValues);
    void setInjProdWellPairFlux(const std::string& injectorTracerName,
                                const std::string& producerTracerName, 
                                const std::pair<double, double>& injProdFluxes) ;

    using Curve = std::pair< std::vector<double>, std::vector<double> >;

    // Used to "steal" the data from this one using swap
    std::map<RigFlowDiagResultAddress, std::vector<double> >&                    nativeResults() { return m_nativeResults; }
    std::map<std::pair<std::string, std::string>, std::pair<double, double> > &  injProdWellPairFluxes() { return m_injProdWellPairFluxes; }

private:

    void addResult(const RigFlowDiagResultAddress& resAddr, const std::map<int, double>& cellValues);

    std::map<RigFlowDiagResultAddress, std::vector<double> >                  m_nativeResults;
    std::map<std::pair<std::string, std::string>, std::pair<double, double> > m_injProdWellPairFluxes;

    size_t m_activeCellCount;
};


class RigEclipseCaseData;
class RigOpmFlowDiagStaticData;

class RigFlowDiagSolverInterface : public cvf::Object
{
public:
    struct FlowCharacteristicsResultFrame
    {
        FlowCharacteristicsResultFrame();

        using Curve = std::pair< std::vector<double>, std::vector<double> >;

        Curve m_flowCapStorageCapCurve;
        Curve m_sweepEfficiencyCurve;
        double m_lorenzCoefficient;
    };

public:
    explicit RigFlowDiagSolverInterface(RimEclipseResultCase * eclipseCase);
    virtual ~RigFlowDiagSolverInterface();

    RigFlowDiagTimeStepResult calculate(size_t timestep,  
                                        std::map<std::string, std::vector<int> > injectorTracers, 
                                        std::map<std::string, std::vector<int> > producerTracers);

    FlowCharacteristicsResultFrame calculateFlowCharacteristics(const std::vector<double>* injector_tof,
                                                                const std::vector<double>* producer_tof,
                                                                double max_pv_fraction);

private:
    std::string getInitFileName() const;

    RimEclipseResultCase * m_eclipseCase;

    cvf::ref<RigOpmFlowDiagStaticData> m_opmFlowDiagStaticData;
   
};



