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

#include "RiaGuiApplication.h"

#include "RimCaseCollection.h"
#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimIdenticalGridCaseGroup.h"

#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"

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
RiuTreeViewEventFilter::RiuTreeViewEventFilter( QObject* parent )
    : QObject( parent )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuTreeViewEventFilter::activateFeatureFromKeyEvent( QKeyEvent* keyEvent )
{
    QKeySequence keySeq( keyEvent->modifiers() + keyEvent->key() );

    auto matches = caf::CmdFeatureManager::instance()->commandFeaturesMatchingKeyboardShortcut( keySeq );

    bool wasFeatureActivated = activateFirstEnabledFeature( matches );
    if ( wasFeatureActivated )
    {
        keyEvent->setAccepted( true );

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuTreeViewEventFilter::activateFirstEnabledFeature( const std::vector<caf::CmdFeature*>& features )
{
    for ( caf::CmdFeature* feature : features )
    {
        if ( feature->canFeatureBeExecuted() )
        {
            feature->actionTriggered( false );

            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuTreeViewEventFilter::eventFilter( QObject* obj, QEvent* event )
{
    if ( event->type() == QEvent::KeyPress )
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>( event );

        std::vector<caf::PdmUiItem*> uiItems;
        caf::SelectionManager::instance()->selectedItems( uiItems );

        if ( uiItems.size() > 0 )
        {
            std::vector<caf::CmdFeature*> matches;
            if ( keyEvent->matches( QKeySequence::Copy ) )
            {
                matches.push_back(
                    caf::CmdFeatureManager::instance()->getCommandFeature( "RicCopyReferencesToClipboardFeature" ) );
            }
            else if ( keyEvent->matches( QKeySequence::Cut ) )
            {
                matches.push_back(
                    caf::CmdFeatureManager::instance()->getCommandFeature( "RicCutReferencesToClipboardFeature" ) );
            }
            else if ( keyEvent->matches( QKeySequence::Paste ) )
            {
                if ( uiItems.size() == 1 )
                {
                    matches = caf::CmdFeatureManager::instance()->commandFeaturesMatchingSubString( "Paste" );
                }
            }
            else if ( keyEvent->matches( QKeySequence::Delete ) )
            {
                matches =
                    caf::CmdFeatureManager::instance()->commandFeaturesMatchingKeyboardShortcut( QKeySequence::Delete );
            }
            else
            {
                QKeySequence keySeq( keyEvent->modifiers() + keyEvent->key() );

                matches = caf::CmdFeatureManager::instance()->commandFeaturesMatchingKeyboardShortcut( keySeq );
            }

            bool wasFeatureActivated = RiuTreeViewEventFilter::activateFirstEnabledFeature( matches );
            if ( wasFeatureActivated )
            {
                keyEvent->setAccepted( true );
                return true;
            }
        }

        // Do not toggle state if currently editing a name in the tree view
        bool toggleStateForSelection = true;
        if ( RiuMainWindow::instance()->projectTreeView() &&
             RiuMainWindow::instance()->projectTreeView()->isTreeItemEditWidgetActive() )
        {
            toggleStateForSelection = false;
        }
        else if ( RiaGuiApplication::instance()->mainPlotWindow() &&
                  RiaGuiApplication::instance()->mainPlotWindow()->projectTreeView() &&
                  RiaGuiApplication::instance()->mainPlotWindow()->projectTreeView()->isTreeItemEditWidgetActive() )
        {
            toggleStateForSelection = false;
        }

        if ( toggleStateForSelection )
        {
            switch ( keyEvent->key() )
            {
                case Qt::Key_Space:
                case Qt::Key_Enter:
                case Qt::Key_Return:
                case Qt::Key_Select:
                {
                    RicToggleItemsFeatureImpl::setObjectToggleStateForSelection( RicToggleItemsFeatureImpl::TOGGLE );

                    keyEvent->setAccepted( true );
                    return true;
                }
            }
        }
    }

    // standard event processing
    return QObject::eventFilter( obj, event );
}
