/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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
                                                          RimFlowDiagSolution* const  flowDiagSolution,
                                                          size_t                      selectedTracerCount );

QStringList getResultNamesForResultType( RiaDefines::ResultCatType resultCatType, RigCaseCellResultsData* const results );

QString timeOfFlightString( RD::FlowTracerSelectionState injectorState, RD::FlowTracerSelectionState producerState, bool shorter );
QString maxFractionTracerString( RD::FlowTracerSelectionState injectorState, RD::FlowTracerSelectionState producerState, bool shorter );

QString selectedTracersString( RD::FlowTracerSelectionState injectorState,
                               RD::FlowTracerSelectionState producerState,
                               const std::vector<QString>&  selectedInjectors,
                               const std::vector<QString>&  selectedProducers,
                               int                          maxTracerStringLength );

QString getInputPropertyFileName( RimEclipseCase* const eclipseCase, const QString& resultName );

void updateTernaryLegend( RigCaseCellResultsData* const cellResultsData, RimTernaryLegendConfig* const ternaryLegendConfigToUpdate, int timeStep );

void updateLegendForFlowDiagnostics( RD* const resultDefinition, RimRegularLegendConfig* const legendConfigToUpdate, int timeStep );

void updateCellResultLegend( RD* const resultDefinition, RimRegularLegendConfig* const legendConfigToUpdate, int timeStep );

QList<caf::PdmOptionItemInfo> calcOptionsForVariableUiFieldStandard( RiaDefines::ResultCatType     resultCatType,
                                                                     RigCaseCellResultsData* const results,
                                                                     bool                          showDerivedResultsFirst,
                                                                     bool                          addPerCellFaceOptionItems,
                                                                     bool                          enableTernary );
}; // namespace RimEclipseResultDefinitionTools
