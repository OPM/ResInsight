/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018     Statoil ASA
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

#include "RicHoloLensCreateDummyFileBackedSessionFeature.h"

#include "RicHoloLensSession.h"

#include "RiaQIconTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicHoloLensCreateDummyFiledBackedSessionFeature, "RicHoloLensCreateDummyFiledBackedSessionFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHoloLensCreateDummyFiledBackedSessionFeature::isCommandEnabled()
{
    return !RicHoloLensSession::instance()->isSessionValid();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensCreateDummyFiledBackedSessionFeature::onActionTriggered(bool isChecked)
{
    RicHoloLensSession::instance()->createDummyFileBackedSession();
    
    RicHoloLensSession::refreshToolbarState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensCreateDummyFiledBackedSessionFeature::setupActionLook(QAction* actionToSetup)
{
    QPixmap pixmap(":/hololens.png");
    QPixmap overlayPixmap(":/plus-sign-green.png");

    QPixmap combinedPixmap = RiaQIconTools::appendPixmapUpperLeft(pixmap, overlayPixmap);
    actionToSetup->setIcon(QIcon(combinedPixmap));

    actionToSetup->setText("Create Dummy File Backed Session");
}
