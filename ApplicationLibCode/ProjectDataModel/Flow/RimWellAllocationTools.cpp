/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022- Equinor ASA
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

#include "RimWellAllocationTools.h"

#include "RiaDefines.h"

#include "RigFlowDiagResultAddress.h"
#include "RigFlowDiagResults.h"
#include "RigSimWellData.h"

#include "RimFlowDiagSolution.h"

std::map<QString, const std::vector<double>*>
    RimWellAllocationTools::findOrCreateRelevantTracerCellFractions( const RigSimWellData* simWellData,
                                                                     RimFlowDiagSolution*  flowDiagSolution,
                                                                     int                   timeStepIndex )
{
    std::map<QString, const std::vector<double>*> tracerCellFractionValues = {};
    if ( flowDiagSolution && simWellData->hasWellResult( timeStepIndex ) )
    {
        RimFlowDiagSolution::TracerStatusType requestedTracerType = RimFlowDiagSolution::TracerStatusType::UNDEFINED;

        const RiaDefines::WellProductionType prodType = simWellData->wellProductionType( timeStepIndex );
        if ( prodType == RiaDefines::WellProductionType::PRODUCER || prodType == RiaDefines::WellProductionType::UNDEFINED_PRODUCTION_TYPE )
        {
            requestedTracerType = RimFlowDiagSolution::TracerStatusType::INJECTOR;
        }
        else
        {
            requestedTracerType = RimFlowDiagSolution::TracerStatusType::PRODUCER;
        }

        std::vector<QString> tracerNames = flowDiagSolution->tracerNames();
        for ( const QString& tracerName : tracerNames )
        {
            if ( flowDiagSolution->tracerStatusInTimeStep( tracerName, timeStepIndex ) == requestedTracerType )
            {
                RigFlowDiagResultAddress resAddr( RIG_FLD_CELL_FRACTION_RESNAME, RigFlowDiagResultAddress::PHASE_ALL, tracerName.toStdString() );
                const std::vector<double>* tracerCellFractions = flowDiagSolution->flowDiagResults()->resultValues( resAddr, timeStepIndex );
                if ( tracerCellFractions )
                {
                    tracerCellFractionValues[tracerName] = tracerCellFractions;
                }
            }
        }
    }

    return tracerCellFractionValues;
}
