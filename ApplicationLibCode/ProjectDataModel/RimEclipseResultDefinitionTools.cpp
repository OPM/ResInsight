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

#include "RimEclipseResultDefinitionTools.h"
#include "RimFlowDiagnosticsTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultDefinitionTools::isDivideByCellFaceAreaPossible( const QString& resultName )
{
    if ( resultName == "FLRWATI+" ) return true;
    if ( resultName == "FLRWATJ+" ) return true;
    if ( resultName == "FLRWATK+" ) return true;

    if ( resultName == "FLROILI+" ) return true;
    if ( resultName == "FLROILJ+" ) return true;
    if ( resultName == "FLROILK+" ) return true;

    if ( resultName == "FLRGASI+" ) return true;
    if ( resultName == "FLRGASJ+" ) return true;
    if ( resultName == "FLRGASK+" ) return true;

    if ( resultName == "TRANX" ) return true;
    if ( resultName == "TRANY" ) return true;
    if ( resultName == "TRANZ" ) return true;

    if ( resultName == "riTRANX" ) return true;
    if ( resultName == "riTRANY" ) return true;
    if ( resultName == "riTRANZ" ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultDefinition::FlowTracerSelectionState
    RimEclipseResultDefinitionTools::getFlowTracerSelectionState( bool                                                isInjector,
                                                                  RimEclipseResultDefinition::FlowTracerSelectionType tracerSelectionType,
                                                                  RimFlowDiagSolution*                                flowDiagSolution,
                                                                  size_t                                              selectedTracerCount )
{
    if ( tracerSelectionType == RimEclipseResultDefinition::FLOW_TR_INJECTORS ||
         tracerSelectionType == RimEclipseResultDefinition::FLOW_TR_INJ_AND_PROD )
    {
        return RimEclipseResultDefinition::ALL_SELECTED;
    }
    else if ( tracerSelectionType == RimEclipseResultDefinition::FLOW_TR_BY_SELECTION )
    {
        if ( selectedTracerCount == RimFlowDiagnosticsTools::setOfTracersOfType( flowDiagSolution, isInjector ).size() )
        {
            return RimEclipseResultDefinition::ALL_SELECTED;
        }
        else if ( selectedTracerCount == (size_t)1 )
        {
            return RimEclipseResultDefinition::ONE_SELECTED;
        }
        else if ( selectedTracerCount > (size_t)1 )
        {
            return RimEclipseResultDefinition::MULTIPLE_SELECTED;
        }
    }

    return RimEclipseResultDefinition::NONE_SELECTED;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimEclipseResultDefinitionTools::getResultNamesForResultType( RiaDefines::ResultCatType     resultCatType,
                                                                          const RigCaseCellResultsData* results )
{
    if ( resultCatType != RiaDefines::ResultCatType::FLOW_DIAGNOSTICS )
    {
        if ( !results ) return QStringList();

        return results->resultNames( resultCatType );
    }

    QStringList flowVars;
    flowVars.push_back( RIG_FLD_TOF_RESNAME );
    flowVars.push_back( RIG_FLD_CELL_FRACTION_RESNAME );
    flowVars.push_back( RIG_FLD_MAX_FRACTION_TRACER_RESNAME );
    flowVars.push_back( RIG_FLD_COMMUNICATION_RESNAME );
    return flowVars;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseResultDefinitionTools::timeOfFlightString( RimEclipseResultDefinition::FlowTracerSelectionState injectorState,
                                                             RimEclipseResultDefinition::FlowTracerSelectionState producerState,
                                                             bool                                                 shorter )
{
    QString tofString;
    bool    multipleSelected = false;
    if ( injectorState != RimEclipseResultDefinition::NONE_SELECTED && producerState != RimEclipseResultDefinition::NONE_SELECTED )
    {
        tofString        = shorter ? "Res.Time" : "Residence Time";
        multipleSelected = true;
    }
    else if ( injectorState != RimEclipseResultDefinition::NONE_SELECTED )
    {
        tofString = shorter ? "Fwd.TOF" : "Forward Time of Flight";
    }
    else if ( producerState != RimEclipseResultDefinition::NONE_SELECTED )
    {
        tofString = shorter ? "Rev.TOF" : "Reverse Time of Flight";
    }
    else
    {
        tofString = shorter ? "TOF" : "Time of Flight";
    }

    multipleSelected = multipleSelected || injectorState >= RimEclipseResultDefinition::MULTIPLE_SELECTED ||
                       producerState >= RimEclipseResultDefinition::MULTIPLE_SELECTED;

    if ( multipleSelected && !shorter )
    {
        tofString += " (Average)";
    }

    tofString += " [days]";
    // Conversion from seconds in flow module to days is done in RigFlowDiagTimeStepResult::setTracerTOF()

    return tofString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEclipseResultDefinitionTools::maxFractionTracerString( RimEclipseResultDefinition::FlowTracerSelectionState injectorState,
                                                                  RimEclipseResultDefinition::FlowTracerSelectionState producerState,
                                                                  bool                                                 shorter )
{
    QString mfString;
    if ( injectorState >= RimEclipseResultDefinition::ONE_SELECTED && producerState == RimEclipseResultDefinition::NONE_SELECTED )
    {
        mfString = shorter ? "FloodReg" : "Flooding Region";
        if ( injectorState >= RimEclipseResultDefinition::MULTIPLE_SELECTED ) mfString += "s";
    }
    else if ( injectorState == RimEclipseResultDefinition::NONE_SELECTED && producerState >= RimEclipseResultDefinition::ONE_SELECTED )
    {
        mfString = shorter ? "DrainReg" : "Drainage Region";
        if ( producerState >= RimEclipseResultDefinition::MULTIPLE_SELECTED ) mfString += "s";
    }
    else
    {
        mfString = shorter ? "Drain&FloodReg" : "Drainage/Flooding Regions";
    }
    return mfString;
}
