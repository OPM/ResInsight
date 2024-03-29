/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RimFlowDiagSolution.h"
#include "cafPdmUiItem.h"

#include <QList>
#include <QString>

//==================================================================================================
///
///
//==================================================================================================
namespace RimFlowDiagnosticsTools
{
struct TracerComp
{
    bool operator()( const QString& lhs, const QString& rhs ) const;
};

using FDS = RimFlowDiagSolution;

QList<caf::PdmOptionItemInfo> calcOptionsForSelectedTracerField( FDS* flowSol, bool isInjector );
std::set<QString, TracerComp> setOfTracersOfType( const FDS* flowSol, bool isInjector );

std::vector<QString> producerTracersInTimeStep( const FDS* flowSol, int timeStepIndex );
std::vector<QString> injectorTracersInTimeStep( const FDS* flowSol, int timeStepIndex );
std::vector<QString> tracersOfStatusInTimeStep( const FDS* flowSol, FDS::TracerStatusType status, int timeStepIndex );

std::set<QString, RimFlowDiagnosticsTools::TracerComp>
    setOfInjectorTracersFromProducers( FDS* flowSol, const std::vector<QString>& producerTracers, std::vector<int> timeStepIndices );

std::set<QString, RimFlowDiagnosticsTools::TracerComp>
    setOfInjectorTracersFromProducers( FDS* flowSol, const std::vector<QString>& producerTracers, int timeStepIndex );

std::set<QString, RimFlowDiagnosticsTools::TracerComp>
    setOfProducerTracersFromInjectors( FDS* flowSol, const std::vector<QString>& injectorTracers, std::vector<int> timeStepIndices );

std::set<QString, RimFlowDiagnosticsTools::TracerComp>
    setOfProducerTracersFromInjectors( FDS* flowSol, const std::vector<QString>& injectorTracers, int timeStepIndex );

}; // namespace RimFlowDiagnosticsTools
