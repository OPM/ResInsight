/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RicSaveProjectFeature.h"

#include "RiaApplication.h"

#include "RimTreeViewStateSerializer.h"
#include "RimProject.h"

#include "RiuMainWindow.h"

#include "cafPdmUiTreeView.h"

#include <QAction>
#include <QTreeView>

CAF_CMD_SOURCE_INIT(RicSaveProjectFeature, "RicSaveProjectFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSaveProjectFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSaveProjectFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();

    RicSaveProjectFeature::storeTreeViewState();

    app->saveProject();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSaveProjectFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("&Save Project");
    actionToSetup->setIcon(QIcon(":/Save.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSaveProjectFeature::storeTreeViewState()
{
    caf::PdmUiTreeView* projectTreeView = RiuMainWindow::instance()->projectTreeView();
    if (projectTreeView)
    {
        QString treeViewState;
        RimTreeViewStateSerializer::storeTreeViewStateToString(projectTreeView->treeView(), treeViewState);

        QModelIndex mi = projectTreeView->treeView()->currentIndex();

        QString encodedModelIndexString;
        RimTreeViewStateSerializer::encodeStringFromModelIndex(mi, encodedModelIndexString);

        RiaApplication::instance()->project()->treeViewState = treeViewState;
        RiaApplication::instance()->project()->currentModelIndexPath = encodedModelIndexString;
    }
}

