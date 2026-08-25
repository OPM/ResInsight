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
#include "RiaGuiApplication.h"
#include "RiaPreferencesSystem.h"

#include "RicfCommandObject.h"

#include "RimDockWindowController.h"
#include "RimMdiWindowController.h"

#include "RiuDockWidgetTools.h"

#include "cafPdmUiTreeAttributes.h"

#include "DockManager.h"
#include "DockWidget.h"

#include <QApplication>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QWidget>

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimViewWindow, "ViewWindow" ); // Do not use. Abstract class

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow::RimViewWindow()
    : m_dockWidget( nullptr )
    , m_isActiveViewer( false )
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "View window", "", "", "", "ViewWindow", "The Base Class for all Views and Plots in ResInsight" );

    CAF_PDM_InitFieldNoDefault( &m_windowController, "DockWindow", "" );
    m_windowController.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_legacyWindowController, "WindowController", "" );
    m_legacyWindowController.uiCapability()->setUiTreeChildrenHidden( true );
    m_legacyWindowController.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_showWindow, "ShowWindow", true, "Show Window" );
    m_showWindow.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_dockWindowId, "DockWindowId", QString(), "Dock Window Id" );
    m_dockWindowId.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow::~RimViewWindow()
{
    if ( m_windowController() ) delete m_windowController();
    if ( m_legacyWindowController() ) delete m_legacyWindowController();
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
void RimViewWindow::setAsActiveViewer( bool allowSelectionChange )
{
    if ( m_windowController ) m_windowController->setAsActiveViewer( allowSelectionChange );
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
bool RimViewWindow::isMainDockedWindow() const
{
    return m_windowController != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewWindow::isDockedIn3DView() const
{
    return ( m_windowController != nullptr ) && ( m_windowController->mainWindowId() == RimDockWindowController::MAIN_WINDOW_ID_3D );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewWindow::isDockedInPlotView() const
{
    return ( m_windowController != nullptr ) && ( m_windowController->mainWindowId() == RimDockWindowController::MAIN_WINDOW_ID_PLOTS );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow::removeWindowFromDock()
{
    if ( m_windowController != nullptr ) m_windowController->removeWindowFromDock();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimViewWindow::windowTitle()
{
    if ( userDescriptionField() )
    {
        caf::PdmUiFieldHandle* uiFieldHandle = userDescriptionField()->uiCapability();
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
void RimViewWindow::deleteDockWidget()
{
    if ( m_dockWidget && m_dockWidget->dockManager() )
    {
        m_dockWidget->dockManager()->removeDockWidget( m_dockWidget );
        m_dockWidget->deleteLater();
        m_dockWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow::updateDockWindowVisibility()
{
    if ( !RiaGuiApplication::isRunning() ) return;

    if ( m_windowController != nullptr )
    {
        m_windowController->setViewToControl( this );
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
void RimViewWindow::dockAs3DViewWindow()
{
    dockInWindow( RimDockWindowController::MAIN_WINDOW_ID_3D );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow::dockAsPlotWindow()
{
    dockInWindow( RimDockWindowController::MAIN_WINDOW_ID_PLOTS );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimViewWindow::snapshotWindowContent()
{
    QImage image;

    QWidget* widget = viewWidget();
    if ( widget )
    {
        QPixmap pix = widget->grab();
        image       = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimViewWindow::captureSnapshot( int width, int height )
{
    return internalCaptureSnapshot( m_dockWidget, viewWidget(), width, height );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimViewWindow::internalCaptureSnapshot( ads::CDockWidget* dockWidget, QWidget* widget, int width, int height )
{
    if ( !widget ) return QImage();

    if ( dockWidget && !dockWidget->isCurrentTab() )
    {
        // Move the dock widget that is stacked behind others in the same dock area to the top to make sure the snapshot is rendered
        dockWidget->setAsCurrentTab();
        QApplication::processEvents();
    }

    bool shouldResize = width > 0 && height > 0 && ( widget->width() != width || widget->height() != height );

    QSize orgSize = widget->size();
    if ( shouldResize )
    {
        widget->setFixedSize( width, height );

        // setFixedSize() only posts a deferred resize. If the pending resize is delivered from inside QWidget::render() below, a child
        // QScrollArea may move its viewport mid-render and crash in Qt's backing-store blit (QWidgetRepaintManager::bltRect).
        QApplication::processEvents();
    }
    else
    {
        width  = widget->width();
        height = widget->height();
    }

    QPixmap pix( width, height );
    pix.fill( Qt::transparent );

    QPainter painter( &pix );
    widget->render( &painter );

    if ( shouldResize )
    {
        widget->setMinimumSize( 0, 0 );
        widget->setMaximumSize( QWIDGETSIZE_MAX, QWIDGETSIZE_MAX );
        widget->resize( orgSize );
    }

    return pix.toImage();
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
void RimViewWindow::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_showWindow )
    {
        if ( isWindowVisible() )
        {
            onLoadDataAndUpdate();
            setAsActiveViewer();
        }
        else
        {
            updateDockWindowVisibility();
        }
        uiCapability()->updateUiIconFromToggleField();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow::updateWindowTitle()
{
    updateWindowTitleIcon( windowTitle() );
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow::updateWindowTitleIcon( QString title )
{
    if ( viewWidget() && dockWidget() )
    {
        viewWidget()->setWindowTitle( title );
        dockWidget()->setWindowTitle( title );

        if ( isActiveViewer() )
        {
            dockWidget()->setIcon( QIcon( ":/ActiveWindow.svg" ) );
        }
        else
        {
            dockWidget()->setIcon( QIcon() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimViewWindow::dockWindowName()
{
    if ( m_dockWindowId().isEmpty() )
    {
        m_dockWindowId = RiuDockWidgetTools::uniqueIdForDockWidget();
    }
    return m_dockWindowId();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow::resetDockWindowId()
{
    m_dockWindowId = "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow::dockInWindow( int mainWindowID )
{
    if ( m_windowController == nullptr )
    {
        m_windowController = new RimDockWindowController();
    }
    m_windowController->setViewToControl( this );
    m_windowController->setMainWindowId( mainWindowID );

    // Keep the compatibility section in sync, see RimMdiWindowController_OBSOLETE
    if ( m_legacyWindowController == nullptr )
    {
        m_legacyWindowController = new RimMdiWindowController_OBSOLETE();
    }
    m_legacyWindowController->setMainWindowId( mainWindowID );
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
        auto tag                   = caf::PdmUiTreeViewItemAttribute::createTag();
        tag->text                  = QString( "%1" ).arg( id() );
        cvf::Color3f viewColor     = RiaColorTables::contrastCategoryPaletteColors().cycledColor3f( (size_t)id() );
        cvf::Color3f viewTextColor = RiaColorTools::contrastColor( viewColor );
        tag->bgColor               = QColor( RiaColorTools::toQColor( viewColor ) );
        tag->fgColor               = QColor( RiaColorTools::toQColor( viewTextColor ) );
        treeItemAttribute->tags.push_back( std::move( tag ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ads::CDockWidget* RimViewWindow::dockWidget() const
{
    return m_dockWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ads::CDockWidget* RimViewWindow::createDockWidget()
{
    if ( !m_dockWidget )
    {
        m_dockWidget = new ads::CDockWidget( windowTitle() );
    }
    return m_dockWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewWindow::setActive( bool active )
{
    m_isActiveViewer = active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewWindow::isActiveViewer() const
{
    return m_isActiveViewer;
}
