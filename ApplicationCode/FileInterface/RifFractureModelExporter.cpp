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

#include "RifFractureModelExporter.h"

#include "RifFractureModelAsymmetricFrkExporter.h"
#include "RifFractureModelDeviationFrkExporter.h"
#include "RifFractureModelGeologicalFrkExporter.h"
#include "RifFractureModelPerfsFrkExporter.h"

#include "RimFractureModel.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifFractureModelExporter::writeToDirectory( RimFractureModel* fractureModel,
                                                 bool              useDetailedFluidLoss,
                                                 const QString&    directoryPath )
{
    return RifFractureModelGeologicalFrkExporter::writeToFile( fractureModel,
                                                               useDetailedFluidLoss,
                                                               directoryPath + "/Geological.frk" ) &&
           RifFractureModelDeviationFrkExporter::writeToFile( fractureModel, directoryPath + "/Deviation.frk" ) &&
           RifFractureModelPerfsFrkExporter::writeToFile( fractureModel, directoryPath + "/Perfs.frk" ) &&
           RifFractureModelAsymmetricFrkExporter::writeToFile( fractureModel, directoryPath + "/Asymmetric.frk" );
}
