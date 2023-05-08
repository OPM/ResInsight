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

class RimEclipseResultDefinitionTools
{
public:
    static bool isDivideByCellFaceAreaPossible( const QString& resultName );
    static RimEclipseResultDefinition::FlowTracerSelectionState
                       getFlowTracerSelectionState( bool                                                isInjector,
                                                    RimEclipseResultDefinition::FlowTracerSelectionType tracerSelectionType,
                                                    RimFlowDiagSolution*                                flowDiagSolution,
                                                    size_t                                              selectedTracerCount );
    static QStringList getResultNamesForResultType( RiaDefines::ResultCatType resultCatType, const RigCaseCellResultsData* results );

    static QString timeOfFlightString( RimEclipseResultDefinition::FlowTracerSelectionState injectorState,
                                       RimEclipseResultDefinition::FlowTracerSelectionState producerState,
                                       bool                                                 shorter );
    static QString maxFractionTracerString( RimEclipseResultDefinition::FlowTracerSelectionState injectorState,
                                            RimEclipseResultDefinition::FlowTracerSelectionState producerState,
                                            bool                                                 shorter );
};
