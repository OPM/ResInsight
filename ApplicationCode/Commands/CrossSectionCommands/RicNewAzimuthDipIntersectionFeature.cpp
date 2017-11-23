/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicNewAzimuthDipIntersectionFeature.h"

#include "RiaApplication.h"

#include "RimCase.h"
#include "RimIntersection.h"
#include "RimIntersectionCollection.h"
#include "RimView.h"

#include "RiuMainWindow.h"
#include "RiuSelectionManager.h"
#include "RiuViewer.h"

#include "cafCmdExecCommandManager.h"
#include "cafDisplayCoordTransform.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"
#include "cvfBoundingBox.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicNewAzimuthDipIntersectionFeature, "RicNewAzimuthDipIntersectionFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicNewAzimuthDipIntersectionFeature::RicNewAzimuthDipIntersectionFeature()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewAzimuthDipIntersectionFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewAzimuthDipIntersectionFeature::onActionTriggered(bool isChecked)
{
    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return;
   
    RicNewAzimuthDipIntersectionFeatureCmd* cmd = new RicNewAzimuthDipIntersectionFeatureCmd(activeView->crossSectionCollection);
    caf::CmdExecCommandManager::instance()->processExecuteCommand(cmd);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewAzimuthDipIntersectionFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/CrossSection16x16.png"));
    actionToSetup->setText("Azimuth and Dip Intersection");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicNewAzimuthDipIntersectionFeatureCmd::RicNewAzimuthDipIntersectionFeatureCmd(RimIntersectionCollection* intersectionCollection)
    : CmdExecuteCommand(NULL),
    m_intersectionCollection(intersectionCollection)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicNewAzimuthDipIntersectionFeatureCmd::~RicNewAzimuthDipIntersectionFeatureCmd()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicNewAzimuthDipIntersectionFeatureCmd::name()
{
    return "Start Azimuth and Dip Intersection";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewAzimuthDipIntersectionFeatureCmd::redo()
{
    CVF_ASSERT(m_intersectionCollection);

    RimIntersection* intersection = new RimIntersection();
    intersection->name = "Azimuth and Dip";
    intersection->type = RimIntersection::CS_AZIMUTHLINE;
    intersection->inputTwoAzimuthPointsFromViewerEnabled = true;
    
    RimCase* rimCase;
    m_intersectionCollection->firstAncestorOrThisOfTypeAsserted(rimCase);
    cvf::BoundingBox bBox = rimCase->allCellsBoundingBox();
    if (bBox.isValid())
    {
        intersection->setHeight(cvf::Math::floor(bBox.extent()[2]));
    }
    
    m_intersectionCollection->appendIntersection(intersection);

    RiuSelectionManager::instance()->deleteAllItems();
    RiuMainWindow::instance()->selectAsCurrentItem(intersection);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewAzimuthDipIntersectionFeatureCmd::undo()
{
}
