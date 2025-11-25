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

#include "RigFlowDiagTimeStepResult.h"

#include "RigFlowDiagDefines.h"

#include "cvfTrace.h"

#include <set>

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