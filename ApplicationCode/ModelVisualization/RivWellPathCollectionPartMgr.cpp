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

#include "RivWellPathCollectionPartMgr.h"
#include "RimProject.h"
#include "RimCase.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimScriptCollection.h"
#include "RimReservoirView.h"
#include "RimReservoirCellResultsCacher.h"
#include "RimCaseCollection.h"
#include "RimResultSlot.h"
#include "RimCellEdgeResultSlot.h"
#include "RimCellPropertyFilterCollection.h"
#include "RimWellCollection.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimWellPathCollection.h"
#include "RivWellPathPartMgr.h"
#include "RimWellPathCollection.h"
#include "RimCellRangeFilterCollection.h"
#include "RimOilField.h"
#include "RimAnalysisModels.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellPathCollectionPartMgr::RivWellPathCollectionPartMgr(RimWellPathCollection* wellPathCollection)
{
    m_wellPathCollection = wellPathCollection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellPathCollectionPartMgr::~RivWellPathCollectionPartMgr()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPathCollectionPartMgr::scheduleGeometryRegen()
{
    for (size_t wIdx = 0; wIdx < m_wellPathCollection->wellPaths.size(); wIdx++)
    {
        m_wellPathCollection->wellPaths[wIdx]->partMgr()->scheduleGeometryRegen();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPathCollectionPartMgr::setScaleTransform(cvf::Transform * scaleTransform)
{
    for (size_t wIdx = 0; wIdx < m_wellPathCollection->wellPaths.size(); wIdx++)
    {
        m_wellPathCollection->wellPaths[wIdx]->partMgr()->setScaleTransform(scaleTransform);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellPathCollectionPartMgr::appendStaticGeometryPartsToModel(cvf::ModelBasicList* model, cvf::Vec3d displayModelOffset, cvf::Transform* scaleTransform, double characteristicCellSize, cvf::BoundingBox boundingBox)
{
    setScaleTransform(scaleTransform);

    if (m_wellPathCollection->wellPathVisibility() == RimWellPathCollection::FORCE_ALL_OFF) return;
    
    for (size_t wIdx = 0; wIdx < m_wellPathCollection->wellPaths.size(); wIdx++)
    {
        RivWellPathPartMgr* partMgr = m_wellPathCollection->wellPaths[wIdx]->partMgr();
        partMgr->setScaleTransform(scaleTransform);
        partMgr->appendStaticGeometryPartsToModel(model, displayModelOffset, characteristicCellSize, boundingBox);
    }
}

