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

#include "RicEclipseCaseNewGroupExec.h"

#include "RimProject.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseStatisticsCase.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimOilField.h"

#include "RiaApplication.h"
#include "RiuMainWindow.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicEclipseCaseNewGroupExec::RicEclipseCaseNewGroupExec()
    : CmdExecuteCommand(nullptr)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicEclipseCaseNewGroupExec::~RicEclipseCaseNewGroupExec()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicEclipseCaseNewGroupExec::name()
{
    return "New Grid Case Group";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipseCaseNewGroupExec::redo()
{ 
    RimProject* proj = RiaApplication::instance()->project();
    CVF_ASSERT(proj);

    RimEclipseCaseCollection* analysisModels = proj->activeOilField() ? proj->activeOilField()->analysisModels() : nullptr;

    if (analysisModels)
    {
        RimIdenticalGridCaseGroup* createdObject = new RimIdenticalGridCaseGroup;
        proj->assignIdToCaseGroup(createdObject);

        RimEclipseCase* createdReservoir = createdObject->createAndAppendStatisticsCase();
        proj->assignCaseIdToCase(createdReservoir);
        createdObject->name = QString("Grid Case Group %1").arg(analysisModels->caseGroups().size() + 1);

        analysisModels->caseGroups().push_back(createdObject);
        analysisModels->updateConnectedEditors();
        RiuMainWindow::instance()->selectAsCurrentItem(createdObject);
        RiuMainWindow::instance()->setExpanded(createdObject);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipseCaseNewGroupExec::undo()
{
    // TODO
    CVF_ASSERT(0);
}
