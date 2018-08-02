/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "RicCreateMultipleFracturesFeature.h"
#include "RicFractureNameGenerator.h"

#include "RiaApplication.h"

#include "RicCreateMultipleFracturesUi.h"

#include "RimDialogData.h"
#include "RimFractureTemplate.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManagerTools.h"

#include <QAction>
#include <QPushButton>

CAF_CMD_SOURCE_INIT(RicCreateMultipleFracturesFeature, "RicCreateMultipleFracturesFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleFracturesFeature::slotDeleteAndAppendFractures()
{
    RiuCreateMultipleFractionsUi* multipleFractionsUi = this->multipleFractionsUi();
    if (!multipleFractionsUi) return;

    auto items = multipleFractionsUi->locationsForNewFractures();
    for (auto item : items)
    {
        if (item.wellPath)
        {
            RimWellPathFractureCollection* fractureCollection = item.wellPath->fractureCollection();
            fractureCollection->deleteFractures();
        }
    }

    slotAppendFractures();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleFracturesFeature::slotAppendFractures()
{
    RiuCreateMultipleFractionsUi* multipleFractionsUi = this->multipleFractionsUi();
    if (!multipleFractionsUi) return;

    auto items = multipleFractionsUi->locationsForNewFractures();
    for (auto item : items)
    {
        if (item.wellPath)
        {
            RimWellPathFractureCollection* fractureCollection = item.wellPath->fractureCollection();

            RimWellPathFracture* fracture = new RimWellPathFracture();
            fractureCollection->fractures.push_back(fracture);

            fracture->setFractureUnit(item.wellPath->unitSystem());
            fracture->setMeasuredDepth(item.measuredDepth);
            fracture->setFractureTemplate(item.fractureTemplate);

            QString fractureName = RicFractureNameGenerator::nameForNewFracture();
            if (item.fractureTemplate)
            {
                fractureName = QString("%1_%2").arg(item.fractureTemplate->name()).arg(item.measuredDepth);
            }

            fracture->setName(fractureName);
        }
    }

    RiaApplication* app  = RiaApplication::instance();
    RimProject*     proj = app->project();

    proj->updateConnectedEditors();
    proj->reloadCompletionTypeResultsInAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleFracturesFeature::slotClose()
{
    if (m_dialog)
    {
        m_dialog->close();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleFracturesFeature::slotCancel()
{
    RiuCreateMultipleFractionsUi* multipleFractionsUi = this->multipleFractionsUi();
    if (multipleFractionsUi)
    {
        multipleFractionsUi->readObjectFromXmlString(m_copyOfObject, caf::PdmDefaultObjectFactory::instance());
        multipleFractionsUi->resolveReferencesRecursively();
    }

    slotClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleFracturesFeature::onActionTriggered(bool isChecked)
{
    m_dialog = nullptr;

    RiuCreateMultipleFractionsUi* multipleFractionsUi = this->multipleFractionsUi();
    if (multipleFractionsUi)
    {
        m_copyOfObject = multipleFractionsUi->writeObjectToXmlString();

        caf::PdmUiPropertyViewDialog propertyDialog(
            Riu3DMainWindowTools::mainWindowWidget(), multipleFractionsUi, "Create Multiple Fractions", "");

        m_dialog = &propertyDialog;

        propertyDialog.resize(QSize(600, 400));

        QDialogButtonBox* dialogButtonBox = propertyDialog.dialogButtonBox();

        dialogButtonBox->clear();

        {
            QPushButton* pushButton = dialogButtonBox->addButton("Replace Fractures", QDialogButtonBox::ActionRole);
            connect(pushButton, SIGNAL(clicked()), this, SLOT(slotDeleteAndAppendFractures()));
            pushButton->setDefault(false);
            pushButton->setAutoDefault(false);
        }

        {
            QPushButton* pushButton = dialogButtonBox->addButton("Append Fractures", QDialogButtonBox::ActionRole);
            connect(pushButton, SIGNAL(clicked()), this, SLOT(slotAppendFractures()));
            pushButton->setDefault(false);
            pushButton->setAutoDefault(false);
        }

        {
            QPushButton* pushButton = dialogButtonBox->addButton("Cancel", QDialogButtonBox::ActionRole);
            connect(pushButton, SIGNAL(clicked()), this, SLOT(slotCancel()));
            pushButton->setDefault(false);
            pushButton->setAutoDefault(false);
        }

        {
            QPushButton* pushButton = dialogButtonBox->addButton("Close", QDialogButtonBox::ActionRole);
            connect(pushButton, SIGNAL(clicked()), this, SLOT(slotClose()));
            pushButton->setDefault(false);
            pushButton->setAutoDefault(false);
        }

        propertyDialog.exec();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleFracturesFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/FractureTemplate16x16.png"));
    actionToSetup->setText("Create Multiple Fractures");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateMultipleFracturesFeature::isCommandEnabled()
{
    std::vector<RimWellPath*> selWells = caf::selectedObjectsByTypeStrict<RimWellPath*>();
    return !selWells.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuCreateMultipleFractionsUi* RicCreateMultipleFracturesFeature::multipleFractionsUi() const
{
    RiaApplication* app  = RiaApplication::instance();
    RimProject*     proj = app->project();

    return proj->dialogData()->multipleFractionsData();
}
