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

#include <map>
#include <string>
#include <vector>

class RigFlowDiagTimeStepResult
{
public:
    explicit RigFlowDiagTimeStepResult( size_t activeCellCount );

    void setTracerTOF( const std::string&                       tracerName,
                       RigFlowDiagResultAddress::PhaseSelection phaseSelection,
                       const std::map<int, double>&             cellValues );
    void setTracerFraction( const std::string&                       tracerName,
                            RigFlowDiagResultAddress::PhaseSelection phaseSelection,
                            const std::map<int, double>&             cellValues );
    void setInjProdWellPairFlux( const std::string&               injectorTracerName,
                                 const std::string&               producerTracerName,
                                 const std::pair<double, double>& injProdFluxes );

    using Curve = std::pair<std::vector<double>, std::vector<double>>;

    // Used to "steal" the data from this one using swap
    std::map<RigFlowDiagResultAddress, std::vector<double>>&                  nativeResults() { return m_nativeResults; }
    std::map<std::pair<std::string, std::string>, std::pair<double, double>>& injProdWellPairFluxes() { return m_injProdWellPairFluxes; }

private:
    void addResult( const RigFlowDiagResultAddress& resAddr, const std::map<int, double>& cellValues );

    std::map<RigFlowDiagResultAddress, std::vector<double>>                  m_nativeResults;
    std::map<std::pair<std::string, std::string>, std::pair<double, double>> m_injProdWellPairFluxes;

    size_t m_activeCellCount;
};