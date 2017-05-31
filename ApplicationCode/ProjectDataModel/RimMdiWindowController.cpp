/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RimMdiWindowController.h"

#include "RiaApplication.h"
#include "RimViewWindow.h"
#include "RiuMainWindowBase.h"

CAF_PDM_XML_SOURCE_INIT(RimMdiWindowController, "MdiWindowController"); 

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimMdiWindowController::RimMdiWindowController()
{
    CAF_PDM_InitField(&m_mainWindowID, "MainWindowID", -1, "", "", "", "" );
    CAF_PDM_InitField(& m_x          , "xPos",         -1, "", "", "", "" );
    CAF_PDM_InitField(& m_y          , "yPos",         -1, "", "", "", "" );
    CAF_PDM_InitField(& m_width      , "Width",        -1, "", "", "", "" );
    CAF_PDM_InitField(& m_height     , "Height",       -1, "", "", "", "" );
    CAF_PDM_InitField(& m_isMaximized, "IsMaximized",  false, "", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimMdiWindowController::~RimMdiWindowController()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimMdiWindowController::setMdiWindowGeometry(const RimMdiWindowGeometry& windowGeometry)
{
    m_mainWindowID = windowGeometry.mainWindowID;
    m_x           = windowGeometry.x;
    m_y           = windowGeometry.y;
    m_width       = windowGeometry.width;
    m_height      = windowGeometry.height;
    m_isMaximized = windowGeometry.isMaximized;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimMdiWindowGeometry RimMdiWindowController::mdiWindowGeometry()
{
    RimMdiWindowGeometry windowGeometry;

    windowGeometry.mainWindowID = m_mainWindowID;
    windowGeometry.x           = m_x           ;
    windowGeometry.y           = m_y           ;
    windowGeometry.width       = m_width       ;
    windowGeometry.height      = m_height      ;
    windowGeometry.isMaximized = m_isMaximized ;

    return windowGeometry;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimMdiWindowController::handleViewerDeletion()
{
    viewPdmObject()->m_showWindow = false;

    uiCapability()->updateUiIconFromToggleField();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimMdiWindowController::removeWindowFromMDI()
{
    RiuMainWindowBase* mainWin = getMainWindow();
    if (mainWin && viewWidget()) mainWin->removeViewer(viewWidget());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewWindow* RimMdiWindowController::viewPdmObject()
{
    RimViewWindow * viewWindowObj;
    this->firstAncestorOrThisOfType(viewWindowObj);
    return viewWindowObj;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RimMdiWindowController::viewWidget()
{
    return viewPdmObject()->viewWidget();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuMainWindowBase* RimMdiWindowController::getMainWindow()
{
    return RiaApplication::instance()->mainWindowByID(m_mainWindowID);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimMdiWindowController::setupBeforeSave()
{
    if ( viewWidget() && getMainWindow() )
    {
        this->setMdiWindowGeometry(getMainWindow()->windowGeometryForViewer(viewWidget()));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimMdiWindowController::updateViewerWidget()
{
    RiuMainWindowBase* mainWindow =  getMainWindow();
    if ( !mainWindow ) return;

    if ( viewPdmObject()->m_showWindow() )
    {
        if ( !viewWidget() )
        {
            QWidget * viewWidget = viewPdmObject()->createViewWidget(mainWindow);

            mainWindow->addViewer(viewWidget, this->mdiWindowGeometry());
            mainWindow->setActiveViewer(viewWidget);

            viewPdmObject()->updateViewWidgetAfterCreation();
        }
        else
        {
            mainWindow->setActiveViewer(viewWidget());
        }

        viewPdmObject()->updateMdiWindowTitle();
    }
    else
    {
        if ( viewWidget() )
        {
            this->setMdiWindowGeometry(mainWindow->windowGeometryForViewer(viewWidget()));

            mainWindow->removeViewer(viewWidget());

            viewPdmObject()->deleteViewWidget();
        }
    }
}

