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

#include "RimViewWindow.h"

#include "RiaColorTables.h"
#include "RiaColorTools.h"
#include "RiaFieldHandleTools.h"
#include "RiaGuiApplication.h"
#include "RiaPreferencesSystem.h"

#include "RicfCommandObject.h"

#include "RimMdiWindowController.h"
#include "RimProject.h"

#include "cafPdmUiTreeViewEditor.h"

#include <QDebug>
#include <QWidget>

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimViewWindow, "ViewWindow" ); // Do not use. Abstract class

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow::RimViewWindow( void )
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "View window",
                                                    "",
                                                    "",
                                                    "",
                                                    "ViewWindow",
                                                    "The Base Class for all Views and Plots in ResInsight" );

    CAF_PDM_InitFieldNoDefault( &m_windowController, "WindowController", "", "", "", "" );
    m_windowController.uiCapability()->setUiTreeHidden( true );
    m_windowController.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitField( &m_showWindow, "ShowWindow", true, "Show Window", "", "", "" );
    m_showWindow.uiCapability()->setUiHidden( true );

    // Obsolete field
    CAF_PDM_InitFieldNoDefault( &obsoleteField_windowGeometry, "WindowGeometry", "", "", "", "" );
    RiaFieldhandleTools::disableWriteAndSetFieldHidden( &obsoleteField_windowGeometry );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow::~RimViewWindow( void )
{
    if ( m_windowController() ) delete m_windowController();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewWindow::showWindow() const
{
    return m_showWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow::setShowWindow( bool showWindow )
{
    m_showWindow = showWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow::loadDataAndUpdate()
{
    assignIdIfNecessary();
    onLoadDataAndUpdate();
    updateUiIconFromToggleField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow::removeMdiWindowFromMdiArea()
{
    if ( m_windowController() ) m_windowController->removeWindowFromMDI();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimViewWindow::windowTitle()
{
    if ( this->userDescriptionField() )
    {
        caf::PdmUiFieldHandle* uiFieldHandle = this->userDescriptionField()->uiCapability();
        if ( uiFieldHandle )
        {
            QVariant v = uiFieldHandle->uiValue();
            return v.toString();
        }
    }
    return QString( "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow::handleMdiWindowClosed()
{
    if ( m_windowController() ) m_windowController->handleViewerDeletion();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow::updateMdiWindowVisibility()
{
    if ( !RiaGuiApplication::isRunning() ) return;

    if ( m_windowController() )
    {
        m_windowController->updateViewerWidget();
    }
    else
    {
        if ( viewWidget() )
        {
            if ( isWindowVisible() )
            {
                viewWidget()->show();
            }
            else
            {
                viewWidget()->hide();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow::setAs3DViewMdiWindow()
{
    setAsMdiWindow( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow::setAsPlotMdiWindow()
{
    setAsMdiWindow( 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow::revokeMdiWindowStatus()
{
    if ( m_windowController() )
    {
        handleMdiWindowClosed();
        deleteViewWidget();
        delete m_windowController();
        m_windowController = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewWindow::isMdiWindow() const
{
    if ( m_windowController() )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow::setMdiWindowGeometry( const RimMdiWindowGeometry& windowGeometry )
{
    if ( m_windowController() ) m_windowController()->setMdiWindowGeometry( windowGeometry );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMdiWindowGeometry RimViewWindow::mdiWindowGeometry()
{
    if ( m_windowController() )
        return m_windowController()->mdiWindowGeometry();
    else
        return RimMdiWindowGeometry();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow::viewNavigationChanged()
{
    onViewNavigationChanged();
}

//--------------------------------------------------------------------------------------------------
/// Default implementation of virtual method to trigger updates on view navigation (zoom, camera move, etc)
//--------------------------------------------------------------------------------------------------
void RimViewWindow::onViewNavigationChanged()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewWindow::isWindowVisible() const
{
    return m_showWindow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimViewWindow::objectToggleField()
{
    return &m_showWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                      const QVariant&            oldValue,
                                      const QVariant&            newValue )
{
    if ( changedField == &m_showWindow )
    {
        if ( isWindowVisible() )
        {
            onLoadDataAndUpdate();
        }
        else
        {
            updateMdiWindowVisibility();
        }
        uiCapability()->updateUiIconFromToggleField();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow::updateMdiWindowTitle()
{
    if ( viewWidget() )
    {
        viewWidget()->setWindowTitle( windowTitle() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow::setAsMdiWindow( int mainWindowID )
{
    if ( !m_windowController() )
    {
        m_windowController = new RimMdiWindowController;
        RimMdiWindowGeometry mwg;
        mwg.mainWindowID = mainWindowID;
        setMdiWindowGeometry( mwg );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
#include "Rim3dView.h"

void RimViewWindow::initAfterRead()
{
    if ( obsoleteField_windowGeometry.value().size() == 5 )
    {
        RimMdiWindowGeometry wg;
        int                  mainWindowID = -1;

        if ( dynamic_cast<Rim3dView*>( this ) )
            mainWindowID = 0;
        else
            mainWindowID = 1;

        wg.mainWindowID = mainWindowID;
        wg.x            = obsoleteField_windowGeometry.value()[0];
        wg.y            = obsoleteField_windowGeometry.value()[1];
        wg.width        = obsoleteField_windowGeometry.value()[2];
        wg.height       = obsoleteField_windowGeometry.value()[3];
        wg.isMaximized  = obsoleteField_windowGeometry.value()[4];

        setAsMdiWindow( mainWindowID );
        setMdiWindowGeometry( wg );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    caf::PdmUiTreeViewItemAttribute* treeItemAttribute = dynamic_cast<caf::PdmUiTreeViewItemAttribute*>( attribute );
    if ( treeItemAttribute && RiaPreferencesSystem::current()->showViewIdInProjectTree() && id() >= 0 )
    {
        treeItemAttribute->tags.clear();
        auto tag                   = caf::PdmUiTreeViewItemAttribute::Tag::create();
        tag->text                  = QString( "%1" ).arg( id() );
        cvf::Color3f viewColor     = RiaColorTables::contrastCategoryPaletteColors().cycledColor3f( (size_t)id() );
        cvf::Color3f viewTextColor = RiaColorTools::contrastColor( viewColor );
        tag->bgColor               = QColor( RiaColorTools::toQColor( viewColor ) );
        tag->fgColor               = QColor( RiaColorTools::toQColor( viewTextColor ) );
        treeItemAttribute->tags.push_back( std::move( tag ) );
    }
}
