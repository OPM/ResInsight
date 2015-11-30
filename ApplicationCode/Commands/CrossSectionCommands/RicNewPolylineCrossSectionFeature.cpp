/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicNewPolylineCrossSectionFeature.h"

#include "RiaApplication.h"

#include "RimCase.h"
#include "RimCrossSection.h"
#include "RimCrossSectionCollection.h"
#include "RimView.h"

#include "RiuMainWindow.h"
#include "RiuSelectionManager.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicNewPolylineCrossSectionFeature, "RicNewPolylineCrossSectionFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicNewPolylineCrossSectionFeature::RicNewPolylineCrossSectionFeature()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewPolylineCrossSectionFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewPolylineCrossSectionFeature::onActionTriggered(bool isChecked)
{
    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return;
   
    RicNewPolylineCrossSectionFeatureCmd* cmd = new RicNewPolylineCrossSectionFeatureCmd(activeView->crossSectionCollection);
    caf::CmdExecCommandManager::instance()->processExecuteCommand(cmd);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewPolylineCrossSectionFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/CrossSection16x16.png"));
    actionToSetup->setText("New Polyline Intersection");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewPolylineCrossSectionFeature::handleUiEvent(cvf::Object* uiEventObject)
{
    std::vector<RimCrossSection*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    if (selection.size() == 1)
    {
        RicLocalIntersectionUiEvent* polylineUiEvent = dynamic_cast<RicLocalIntersectionUiEvent*>(uiEventObject);
        if (polylineUiEvent)
        {
            RimCrossSection* crossSection = selection[0];

            RimCase* rimCase = NULL;
            crossSection->firstAnchestorOrThisOfType(rimCase);
            CVF_ASSERT(rimCase);

            crossSection->appendPointToPolyLine(rimCase->displayModelOffset() + polylineUiEvent->localIntersectionPoint);

            // Further Ui processing is stopped when true is returned
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicNewPolylineCrossSectionFeatureCmd::RicNewPolylineCrossSectionFeatureCmd(RimCrossSectionCollection* crossSectionCollection)
    : CmdExecuteCommand(NULL),
    m_crossSectionCollection(crossSectionCollection)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicNewPolylineCrossSectionFeatureCmd::~RicNewPolylineCrossSectionFeatureCmd()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicNewPolylineCrossSectionFeatureCmd::name()
{
    return "Start Polyline Intersection";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewPolylineCrossSectionFeatureCmd::redo()
{
    CVF_ASSERT(m_crossSectionCollection);

    RimCrossSection* crossSection = new RimCrossSection();
    crossSection->name = "Polyline";
    crossSection->type = RimCrossSection::CS_POLYLINE;
    m_crossSectionCollection->appendCrossSection(crossSection);

    crossSection->updateActiveUiCommandFeature();

    RiuSelectionManager::instance()->deleteAllItems();

    RiuMainWindow::instance()->setCurrentObjectInTreeView(crossSection);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewPolylineCrossSectionFeatureCmd::undo()
{
}
