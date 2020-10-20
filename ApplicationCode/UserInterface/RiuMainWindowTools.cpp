/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RiuMainWindowTools.h"

#include "RiaGuiApplication.h"

#include "RimViewWindow.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimProject.h"

#include "RiuInterfaceToViewWindow.h"
#include "RiuMainWindow.h"
#include "RiuMainWindowBase.h"
#include "RiuPlotMainWindow.h"
#include "RiuViewer.h"

#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeView.h"

#include <QMainWindow>
#include <QMdiSubWindow>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowTools::collapseSiblings( const caf::PdmUiItem* sourceUiItem )
{
    if ( !sourceUiItem ) return;

    if ( !RiaGuiApplication::isRunning() ) return;

    {
        caf::PdmUiTreeView*     sourceTreeView         = nullptr;
        caf::PdmUiTreeOrdering* sourceTreeOrderingItem = nullptr;

        {
            QModelIndex modIndex;

            if ( RiuMainWindow::instance() )
            {
                modIndex = RiuMainWindow::instance()->projectTreeView()->findModelIndex( sourceUiItem );
            }

            if ( modIndex.isValid() )
            {
                sourceTreeView = RiuMainWindow::instance()->projectTreeView();
            }
            else
            {
                RiuPlotMainWindow* mpw = RiaGuiApplication::instance()->mainPlotWindow();
                if ( mpw )
                {
                    modIndex = mpw->projectTreeView()->findModelIndex( sourceUiItem );
                    if ( modIndex.isValid() )
                    {
                        sourceTreeView = mpw->projectTreeView();
                    }
                }
            }

            if ( !modIndex.isValid() ) return;

            sourceTreeOrderingItem = static_cast<caf::PdmUiTreeOrdering*>( modIndex.internalPointer() );
        }

        if ( sourceTreeView && sourceTreeOrderingItem && sourceTreeOrderingItem->parent() )
        {
            for ( int i = 0; i < sourceTreeOrderingItem->parent()->childCount(); i++ )
            {
                auto siblingTreeOrderingItem = sourceTreeOrderingItem->parent()->child( i );
                if ( siblingTreeOrderingItem != sourceTreeOrderingItem )
                {
                    sourceTreeView->setExpanded( siblingTreeOrderingItem->activeItem(), false );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowTools::setWindowSizeOnWidgetsInMdiWindows( RiuMainWindowBase* mainWindow, int width, int height )
{
    if ( !mainWindow ) return;

    auto widgets = mainWindow->findChildren<QMdiSubWindow*>();
    for ( auto w : widgets )
    {
        if ( !w ) continue;

        w->showNormal();
    }

    // Process events before resize to make sure the widget is ready for resize
    // If not, a maximized window with not get the prescribed window size
    QApplication::processEvents();

    for ( auto w : widgets )
    {
        if ( !w ) continue;
        auto viewWindow = RiuInterfaceToViewWindow::viewWindowFromWidget( w->widget() );

        if ( viewWindow && viewWindow->viewWidget() )
        {
            QWidget* viewWidget = viewWindow->viewWidget();

            viewWidget->resize( width, height );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowTools::setFixedWindowSizeFor3dViews( RiuMainWindowBase* mainWindow, int width, int height )
{
    if ( !mainWindow ) return;

    RimProject* proj = RimProject::current();
    if ( !proj ) return;

    std::vector<RimCase*> projectCases;
    proj->allCases( projectCases );

    for ( RimCase* cas : projectCases )
    {
        if ( !cas ) continue;

        std::vector<Rim3dView*> views = cas->views();

        for ( Rim3dView* riv : views )
        {
            if ( riv && riv->viewer() )
            {
                // Make sure all views are maximized for snapshotting
                QMdiSubWindow* subWnd = mainWindow->findMdiSubWindow( riv->viewer()->layoutWidget() );
                if ( subWnd )
                {
                    subWnd->showMaximized();
                }

                // This size is set to match the regression test reference images
                QSize windowSize( width, height );

                riv->viewer()->setFixedSize( windowSize );
            }
        }
    }
}
