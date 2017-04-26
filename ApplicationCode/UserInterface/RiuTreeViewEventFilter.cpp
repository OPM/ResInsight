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

#include "RiuTreeViewEventFilter.h"

#include "ToggleCommands/RicToggleItemsFeatureImpl.h"

#include "RiaApplication.h"

#include "RimCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimIdenticalGridCaseGroup.h"

#include "RiuMainWindow.h"
#include "RiuMainPlotWindow.h"

#include "cafCmdFeature.h"
#include "cafCmdFeatureManager.h"
#include "cafPdmUiTreeView.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QKeyEvent>
#include <QTreeView>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuTreeViewEventFilter::RiuTreeViewEventFilter(QObject* parent)
    : QObject(parent)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiuTreeViewEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent *>(event);

        caf::PdmUiItem* uiItem = caf::SelectionManager::instance()->selectedItem();
        if (uiItem)
        {
            if (keyEvent->matches(QKeySequence::Copy))
            {
                QAction* actionToTrigger = caf::CmdFeatureManager::instance()->action("RicCopyReferencesToClipboardFeature");
                assert(actionToTrigger);

                actionToTrigger->trigger();

                keyEvent->setAccepted(true);
                return true;
            }
            else if (keyEvent->matches(QKeySequence::Paste))
            {
                std::vector<caf::CmdFeature*> matches = caf::CmdFeatureManager::instance()->commandFeaturesMatchingSubString("Paste");

                for (caf::CmdFeature* feature : matches)
                {
                    if (feature->canFeatureBeExecuted())
                    {
                        feature->actionTriggered(false);

                        keyEvent->setAccepted(true);
                        return true;
                    }
                }
            }
        }

        // Do not toggle state if currently editing a name in the tree view
        bool toggleStateForSelection = true;
        if (RiuMainWindow::instance()->projectTreeView()->isTreeItemEditWidgetActive())
        {
            toggleStateForSelection = false;
        }
        else if (RiaApplication::instance()->mainPlotWindow() && RiaApplication::instance()->mainPlotWindow()->projectTreeView()->isTreeItemEditWidgetActive())
        {
            toggleStateForSelection = false;
        }

        if (toggleStateForSelection)
        {
            switch (keyEvent->key())
            {
                case Qt::Key_Space:
                case Qt::Key_Enter:
                case Qt::Key_Return:
                case Qt::Key_Select:
                {
                    RicToggleItemsFeatureImpl::setObjectToggleStateForSelection(RicToggleItemsFeatureImpl::TOGGLE);

                    keyEvent->setAccepted(true);
                    return true;
                }
            }
        }
    }

    // standard event processing
    return QObject::eventFilter(obj, event);
}

