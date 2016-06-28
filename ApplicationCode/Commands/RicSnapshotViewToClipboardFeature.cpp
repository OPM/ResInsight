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

#include "RicSnapshotViewToClipboardFeature.h"

#include "RiaApplication.h"

#include "RimView.h"
#include "RimViewWindow.h"
#include "RimWellLogPlot.h"

#include "RiuMainPlotWindow.h"
#include "RiuMainWindow.h"

#include <QClipboard>
#include <QAction>

CAF_CMD_SOURCE_INIT(RicSnapshotViewToClipboardFeature, "RicSnapshotViewToClipboardFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSnapshotViewToClipboardFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToClipboardFeature::onActionTriggered(bool isChecked)
{
    RimViewWindow* viewWindow = NULL;

    QWidget* topLevelWidget = RiaApplication::activeWindow();

    if (dynamic_cast<RiuMainWindow*>(topLevelWidget))
    {
        viewWindow = RiaApplication::instance()->activeReservoirView();
    }

    if (dynamic_cast<RiuMainPlotWindow*>(topLevelWidget))
    {
        viewWindow = RiaApplication::instance()->activeWellLogPlot();
    }

    if (viewWindow)
    {
        QClipboard* clipboard = QApplication::clipboard();
        if (clipboard)
        {
            QImage image = viewWindow->snapshotWindowContent();
            if (!image.isNull())
            {
                clipboard->setImage(image);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToClipboardFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Snapshot To Clipboard");
    actionToSetup->setIcon(QIcon(":/SnapShot.png"));
}

