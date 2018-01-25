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

#include "RicNewIntersectionViewFeature.h"

#include "Rim2dIntersectionView.h"
#include "RimCase.h"
#include "RimIntersection.h"

#include "RiuMainWindow.h"
#include "RiuSelectionManager.h"

#include "cafSelectionManagerTools.h"

#include <QAction>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT(RicNewIntersectionViewFeature, "RicNewIntersectionViewFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewIntersectionViewFeature::isCommandEnabled()
{
    std::set<RimIntersection*> objects = selectedIntersections();

    return !objects.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewIntersectionViewFeature::onActionTriggered(bool isChecked)
{
    std::set<RimIntersection*> intersections = selectedIntersections();

    Rim2dIntersectionView* objectToSelect = nullptr;

    for (auto intersection : intersections)
    {
        if (!intersection) continue;

        RimCase* rimCase = nullptr;
        intersection->firstAncestorOrThisOfType(rimCase);
        if (rimCase)
        {
            if (intersection->direction() != RimIntersection::CS_VERTICAL)
            {
                QString text = QString("Intersection '%1' is not a vertical intersection. The intersection view supports"
                                       "only vertical intersections.").arg(intersection->name());

                QMessageBox::warning(RiuMainWindow::instance(), "New Intersection View", text);
            }

            Rim2dIntersectionView* intersectionView = intersection->correspondingIntersectionView();
            intersectionView->setVisible(true);
            intersectionView->loadDataAndUpdate();

            intersectionView->updateConnectedEditors();

            objectToSelect = intersectionView;
        }
    }

    if (objectToSelect)
    {
        RiuMainWindow::instance()->selectAsCurrentItem(objectToSelect);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewIntersectionViewFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Show 2D Intersection View");
    // actionToSetup->setIcon(QIcon(":/chain.png"));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimIntersection*> RicNewIntersectionViewFeature::selectedIntersections()
{
    std::set<RimIntersection*> objects;

    RiuSelectionManager* riuSelManager = RiuSelectionManager::instance();
    RiuSelectionItem*    selItem       = riuSelManager->selectedItem(RiuSelectionManager::RUI_TEMPORARY);

    RiuGeneralSelectionItem* generalSelectionItem = static_cast<RiuGeneralSelectionItem*>(selItem);
    if (generalSelectionItem)
    {
        RimIntersection* intersection = dynamic_cast<RimIntersection*>(generalSelectionItem->m_object);
        if (intersection)
        {
            objects.insert(intersection);

            // Return only the intersection the user clicked on

            return objects;
        }
    }

    {
        std::vector<RimIntersection*> selectedObjects = caf::selectedObjectsByType<RimIntersection*>();
        for (auto obj : selectedObjects)
        {
            objects.insert(obj);
        }
    }

    return objects;
}
