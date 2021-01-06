/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-    Equinor ASA
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

#include "RifStimPlanModelExporter.h"

#include "RifStimPlanModelAsymmetricFrkExporter.h"
#include "RifStimPlanModelDeviationFrkExporter.h"
#include "RifStimPlanModelGeologicalFrkExporter.h"
#include "RifStimPlanModelPerfsFrkExporter.h"

#include "RimStimPlanModel.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifStimPlanModelExporter::writeToDirectory( RimStimPlanModel* stimPlanModel,
                                                 bool              useDetailedFluidLoss,
                                                 const QString&    directoryPath )
{
    return RifStimPlanModelGeologicalFrkExporter::writeToFile( stimPlanModel,
                                                               useDetailedFluidLoss,
                                                               directoryPath + "/Geological.frk" ) &&
           RifStimPlanModelDeviationFrkExporter::writeToFile( stimPlanModel, directoryPath + "/Deviation.frk" ) &&
           RifStimPlanModelPerfsFrkExporter::writeToFile( stimPlanModel, directoryPath + "/Perfs.frk" ) &&
           RifStimPlanModelAsymmetricFrkExporter::writeToFile( stimPlanModel, directoryPath + "/Asymmetric.frk" );
}
