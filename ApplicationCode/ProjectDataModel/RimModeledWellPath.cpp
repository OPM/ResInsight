/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 -    equinor ASA
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

#include "RimModeledWellPath.h"

#include "RimWellPathGeometryDef.h"
#include "RimProject.h"

#include "RigWellPath.h"

#include "cafPdmUiTreeOrdering.h"
#include "RimPlotCurve.h"
#include "RiaCompletionTypeCalculationScheduler.h"
#include "RimIntersection.h"
#include "RimWellPath.h"
#include "RimWellPathFractureCollection.h"
#include "RimWellPathFracture.h"


CAF_PDM_SOURCE_INIT(RimModeledWellPath, "ModeledWellPath");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimModeledWellPath::RimModeledWellPath()
{
    CAF_PDM_InitObject("Modeled WellPath", ":/Well.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_geometryDefinition, "WellPathGeometryDef", "Trajectory", "", "", "");
    m_geometryDefinition = new RimWellPathGeometryDef;

    m_name.uiCapability()->setUiReadOnly(false);
    m_name.xmlCapability()->setIOWritable(true);
    m_name.xmlCapability()->setIOReadable(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimModeledWellPath::~RimModeledWellPath()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimModeledWellPath::createWellPathGeometry()
{
    this->setWellPathGeometry(m_geometryDefinition->createWellPathGeometry().p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimModeledWellPath::updateWellPathVisualization()
{
    this->setWellPathGeometry(m_geometryDefinition->createWellPathGeometry().p());
    
    std::vector<RimPlotCurve*> refferingCurves;
    this->objectsWithReferringPtrFieldsOfType(refferingCurves);

    for (auto curve: refferingCurves)
    {
        curve->loadDataAndUpdate(false);
    }

    for ( auto fracture : this->fractureCollection()->activeFractures() )
    {
        fracture->loadDataAndUpdate();
    }

    std::vector<RimIntersection*> refferingIntersections;
    this->objectsWithReferringPtrFieldsOfType(refferingIntersections);

    for (auto intersection: refferingIntersections)
    {
        intersection->rebuildGeometryAndScheduleCreateDisplayModel();
    }

    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    proj->scheduleCreateDisplayModelAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimModeledWellPath::scheduleUpdateOfDependentVisualization()
{

    RiaCompletionTypeCalculationScheduler::instance()->scheduleRecalculateCompletionTypeAndRedrawAllViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathGeometryDef* RimModeledWellPath::geometryDefinition()
{
    return m_geometryDefinition;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimModeledWellPath::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName)
{
    uiTreeOrdering.add(m_geometryDefinition());
    RimWellPath::defineUiTreeOrdering(uiTreeOrdering, uiConfigName);
}

