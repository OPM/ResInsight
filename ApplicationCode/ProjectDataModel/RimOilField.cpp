/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RiaStdInclude.h"

#include "RimOilField.h"
#include "cafAppEnum.h"
#include "RimReservoirView.h"

#include "RimIdenticalGridCaseGroup.h"

#include "RiaApplication.h"
#include "RiaVersionInfo.h"

#include "RigGridManager.h"
#include "RigCaseData.h"
#include "RimResultCase.h"
#include "RimWellPathCollection.h"
#include "RimAnalysisModels.h"

#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"
#include "RimReservoirCellResultsCacher.h"
#include "RimCellEdgeResultSlot.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCellPropertyFilterCollection.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimWellCollection.h"
#include "RimCaseCollection.h"
#include "RimResultSlot.h"
#include "RimStatisticsCase.h"

CAF_PDM_SOURCE_INIT(RimOilField, "ResInsightOilField");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimOilField::RimOilField(void)
{
    CAF_PDM_InitObject("Oil Field", "", "", "");

    CAF_PDM_InitFieldNoDefault(&analysisModels, "AnalysisModels", "Grid Models", ":/GridModels.png", "", "");
    CAF_PDM_InitFieldNoDefault(&wellPathCollection, "WellPathCollection", "Well Paths", ":/WellCollection.png", "", "");
    
    analysisModels = new RimAnalysisModels();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimOilField::~RimOilField(void)
{
    close();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimOilField::close()
{
    if (wellPathCollection()) delete wellPathCollection();
    if (analysisModels()) delete analysisModels();
}
