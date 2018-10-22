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

#include "RicHoloLensCreateSessionFeature.h"

#include "RiaQIconTools.h"

#include "RicHoloLensCreateSessionUi.h"
#include "RicHoloLensServerSettings.h"
#include "RicHoloLensSession.h"

#include "cafPdmSettings.h"
#include "cafPdmUiPropertyViewDialog.h"

#include <QAction>
#include <QPushButton>

CAF_CMD_SOURCE_INIT(RicHoloLensCreateSessionFeature, "RicHoloLensCreateSessionFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHoloLensCreateSessionFeature::isCommandEnabled()
{
    return !RicHoloLensSession::instance()->isSessionValid();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensCreateSessionFeature::onActionTriggered(bool isChecked)
{
    RicHoloLensCreateSessionUi createSessionUi;

    caf::PdmUiPropertyViewDialog propertyDialog(nullptr, &createSessionUi, "HoloLens - Create Session", "");
    propertyDialog.resize(QSize(400, 330));

    {
        QDialogButtonBox* dialogButtonBox = propertyDialog.dialogButtonBox();
        dialogButtonBox->clear();

        QPushButton* pushButton = dialogButtonBox->addButton("Create Session", QDialogButtonBox::ActionRole);
        connect(pushButton, SIGNAL(clicked()), &propertyDialog, SLOT(close()));
    }

    propertyDialog.exec();

    RicHoloLensSession::instance()->createSession(
        createSessionUi.serverUrl(), createSessionUi.sessionName(), createSessionUi.sessionPinCode());

    RicHoloLensSession::refreshToolbarState();

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensCreateSessionFeature::setupActionLook(QAction* actionToSetup)
{
    QPixmap pixmap(":/hololens.png");
    QPixmap overlayPixmap(":/plus-sign-green.png");

    QPixmap combinedPixmap = RiaQIconTools::appendPixmapUpperLeft(pixmap, overlayPixmap);
    actionToSetup->setIcon(QIcon(combinedPixmap));

    actionToSetup->setText("Create Session");
}
