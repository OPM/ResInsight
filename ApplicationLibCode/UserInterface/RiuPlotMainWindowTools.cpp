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

#include "RiuPlotMainWindowTools.h"

#include "RiaGuiApplication.h"

#include "RimViewWindow.h"

#include "RiuPlotMainWindow.h"

#include "cafPdmObject.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindowTools::showPlotMainWindow()
{
    if ( RiaGuiApplication::isRunning() )
    {
        RiaGuiApplication::instance()->getOrCreateAndShowMainPlotWindow();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindowTools::setActiveViewer( QString viewerName )
{
    if ( RiaGuiApplication::isRunning() )
    {
        if ( auto mpw = RiaGuiApplication::instance()->mainPlotWindow() )
        {
            mpw->setActiveViewer( viewerName );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindowTools::setExpanded( const caf::PdmUiItem* uiItem )
{
    if ( RiaGuiApplication::isRunning() )
    {
        if ( auto mpw = RiaGuiApplication::instance()->mainPlotWindow() )
        {
            mpw->setExpanded( uiItem, true );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindowTools::selectAsCurrentItem( const caf::PdmObject* object )
{
    if ( RiaGuiApplication::isRunning() )
    {
        if ( auto mpw = RiaGuiApplication::instance()->mainPlotWindow() )
        {
            mpw->selectAsCurrentItem( object, true /* allowActiveViewChange */ );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const caf::PdmObject* RiuPlotMainWindowTools::firstVisibleAncestorOrThis( const caf::PdmObject* object )
{
    if ( RiaGuiApplication::isRunning() )
    {
        if ( auto mpw = RiaGuiApplication::instance()->mainPlotWindow() )
        {
            auto current = const_cast<caf::PdmObject*>( object );
            while ( current )
            {
                if ( mpw->getTreeViewWithItem( current ) )
                {
                    return current;
                }

                current = current->firstAncestorOfType<caf::PdmObject>();
            }
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindowTools::toggleItemInSelection( const caf::PdmObject* object )
{
    if ( RiaGuiApplication::isRunning() )
    {
        if ( auto mpw = RiaGuiApplication::instance()->mainPlotWindow() )
        {
            mpw->toggleItemInSelection( object, true /* allowActiveViewChange */ );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindowTools::selectOrToggleObject( const caf::PdmObject* object, bool toggle )
{
    if ( toggle )
    {
        RiuPlotMainWindowTools::toggleItemInSelection( object );
    }
    else
    {
        RiuPlotMainWindowTools::selectAsCurrentItem( object );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindowTools::refreshToolbars()
{
    if ( RiaGuiApplication::isRunning() )
    {
        if ( auto mpw = RiaGuiApplication::instance()->mainPlotWindow() )
        {
            mpw->updateWellLogPlotToolBar();
            mpw->updateMultiPlotToolBar();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindowTools::onObjectAppended( const caf::PdmObject* objectToSelect, const caf::PdmObject* objectToExpand )
{
    if ( objectToExpand == nullptr ) objectToExpand = objectToSelect;

    if ( objectToExpand ) RiuPlotMainWindowTools::setExpanded( objectToExpand );
    if ( objectToSelect ) RiuPlotMainWindowTools::selectAsCurrentItem( objectToSelect );

    RiuPlotMainWindowTools::refreshToolbars();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotMainWindowTools::remove3dViewsFromDocking()
{
    if ( !RiaGuiApplication::isRunning() ) return;

    if ( auto plotWnd = RiaGuiApplication::instance()->mainPlotWindow() )
    {
        if ( auto mainWnd = RiaGuiApplication::instance()->mainWindow() )
        {
            for ( auto view : mainWnd->viewWindows() )
            {
                if ( view->isDockedInPlotView() )
                {
                    view->setShowWindow( false );
                    view->removeWindowFromDock();
                    view->dockAs3DViewWindow();
                    view->updateConnectedEditors();
                }
            }
        }
    }
}
