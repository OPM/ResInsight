/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RimEclipseResultDefinition.h"

#include <QString>

using RD = RimEclipseResultDefinition;

namespace RimEclipseResultDefinitionTools
{
bool isDivideByCellFaceAreaPossible( const QString& resultName );

RD::FlowTracerSelectionState getFlowTracerSelectionState( bool                        isInjector,
                                                          RD::FlowTracerSelectionType tracerSelectionType,
                                                          const RimFlowDiagSolution*  flowDiagSolution,
                                                          size_t                      selectedTracerCount );

QStringList getResultNamesForResultType( RiaDefines::ResultCatType resultCatType, const RigCaseCellResultsData* results );

QString timeOfFlightString( RD::FlowTracerSelectionState injectorState, RD::FlowTracerSelectionState producerState, bool shorter );
QString maxFractionTracerString( RD::FlowTracerSelectionState injectorState, RD::FlowTracerSelectionState producerState, bool shorter );

QString selectedTracersString( RD::FlowTracerSelectionState injectorState,
                               RD::FlowTracerSelectionState producerState,
                               const std::vector<QString>&  selectedInjectors,
                               const std::vector<QString>&  selectedProducers,
                               int                          maxTracerStringLength );

QString getInputPropertyFileName( const RimEclipseCase* eclipseCase, const QString& resultName );

void updateTernaryLegend( RigCaseCellResultsData* cellResultsData, RimTernaryLegendConfig* ternaryLegendConfigToUpdate, int timeStep );

void updateLegendForFlowDiagnostics( const RimEclipseResultDefinition* resultDefinition,
                                     RimRegularLegendConfig*           legendConfigToUpdate,
                                     int                               timeStep );

void updateCellResultLegend( const RimEclipseResultDefinition* resultDefinition,
                             RimRegularLegendConfig*           legendConfigToUpdate,
                             int                               timeStep,
                             RimEclipseView*                   sourceCellVisibilityView );

QList<caf::PdmOptionItemInfo> calcOptionsForVariableUiFieldStandard( RiaDefines::ResultCatType     resultCatType,
                                                                     const RigCaseCellResultsData* results,
                                                                     bool                          showDerivedResultsFirst,
                                                                     bool                          addPerCellFaceOptionItems,
                                                                     bool                          enableTernary );
}; // namespace RimEclipseResultDefinitionTools
