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

#include "RicShowMemoryCleanupDialogFeature.h"

#include "RiaApplication.h"
#include "RiaMemoryCleanup.h"
#include "RiuMainWindow.h"

#include "cafPdmUiPropertyViewDialog.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicShowMemoryCleanupDialogFeature, "RicShowMemoryCleanupDialogFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicShowMemoryCleanupDialogFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowMemoryCleanupDialogFeature::onActionTriggered(bool isChecked)
{
    RiaMemoryCleanup memoryCleanup;
    Rim3dView* view = RiaApplication::instance()->activeReservoirView();
    if (view)
    {
        memoryCleanup.setPropertiesFromView(view);
    }

    caf::PdmUiPropertyViewDialog dialog(RiuMainWindow::instance(), &memoryCleanup, "Clear Results From Memory", "", QDialogButtonBox::Close);
    dialog.resize(QSize(400, 400));
    if (dialog.exec() == QDialog::Accepted)
    {
        memoryCleanup.clearSelectedResultsFromMemory();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowMemoryCleanupDialogFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("&Memory Cleanup...");
}
