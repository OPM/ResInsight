/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimPlotWindow.h"

#include "RiaPlotWindowRedrawScheduler.h"
#include "RiaPreferences.h"

#include "RicfCommandObject.h"

#include "RimProject.h"
#include "RiuMultiPlotPage.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmUiComboBoxEditor.h"

#include <QPainter>

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimPlotWindow, "RimPlotWindow" ); // Do not use. Abstract class

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotWindow::RimPlotWindow()
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "PlotWindow",
                                                    "",
                                                    "",
                                                    "",
                                                    "PlotWindow",
                                                    "The Abstract base class for all MDI Windows in the Plot Window" );

    CAF_PDM_InitScriptableField( &m_id, "Id", -1, "View ID", "", "", "" );
    m_id.registerKeywordAlias( "ViewId" );
    m_id.uiCapability()->setUiReadOnly( true );
    m_id.uiCapability()->setUiHidden( true );
    m_id.capability<caf::PdmAbstractFieldScriptingCapability>()->setIOWriteable( false );
    m_id.xmlCapability()->setCopyable( false );

    CAF_PDM_InitField( &m_showPlotTitle, "ShowPlotTitle", true, "Show Plot Title", "", "", "" );
    CAF_PDM_InitField( &m_showPlotLegends, "ShowTrackLegends", true, "Show Legends", "", "", "" );
    CAF_PDM_InitField( &m_plotLegendsHorizontal, "TrackLegendsHorizontal", true, "Legend Orientation", "", "", "" );
    m_plotLegendsHorizontal.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_titleFontSize, "TitleFontSize", "Title Font Size", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_legendFontSize, "LegendDeltaFontSize", "Legend Font Size", "", "", "" );

    m_titleFontSize  = caf::FontTools::RelativeSize::XXLarge;
    m_legendFontSize = caf::FontTools::RelativeSize::Small;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotWindow::~RimPlotWindow()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimPlotWindow::id() const
{
    return m_id;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotWindow& RimPlotWindow::operator=( RimPlotWindow&& rhs )
{
    m_showPlotTitle         = rhs.m_showPlotTitle();
    m_showPlotLegends       = rhs.m_showPlotLegends();
    m_plotLegendsHorizontal = rhs.m_plotLegendsHorizontal();
    m_titleFontSize         = rhs.m_titleFontSize();
    m_legendFontSize        = rhs.m_legendFontSize();
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotWindow::plotTitleVisible() const
{
    return m_showPlotTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotWindow::setPlotTitleVisible( bool showPlotTitle )
{
    m_showPlotTitle = showPlotTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotWindow::legendsVisible() const
{
    return m_showPlotLegends();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotWindow::setLegendsVisible( bool doShow )
{
    m_showPlotLegends = doShow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotWindow::legendsHorizontal() const
{
    return m_plotLegendsHorizontal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotWindow::setLegendsHorizontal( bool horizontal )
{
    m_plotLegendsHorizontal = horizontal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotWindow::updateFonts()
{
    updateLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimPlotWindow::fontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimPlotWindow::titleFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_titleFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimPlotWindow::legendFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_legendFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotWindow::setLegendFontSize( caf::FontTools::RelativeSize fontSize )
{
    m_legendFontSize = fontSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotWindow::updateLayout()
{
    doUpdateLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotWindow::updateParentLayout()
{
    caf::PdmFieldHandle* parentField = this->parentField();
    if ( parentField )
    {
        caf::PdmObjectHandle* parentObject = parentField->ownerObject();
        RimPlotWindow*        plotWindow   = nullptr;
        parentObject->firstAncestorOrThisOfType( plotWindow );
        if ( plotWindow )
        {
            plotWindow->updateLayout();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimPlotWindow::columnCount() const
{
    return static_cast<int>( RiuMultiPlotPage::ColumnCount::COLUMNS_UNLIMITED );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotWindow::renderWindowContent( QPaintDevice* paintDevice )
{
    doRenderWindowContent( paintDevice );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QPageLayout RimPlotWindow::pageLayout() const
{
    QPageLayout defaultPageLayout = RiaPreferences::current()->defaultPageLayout();
    QPageLayout customPageLayout;
    if ( hasCustomPageLayout( &customPageLayout ) )
    {
        return customPageLayout;
    }
    return defaultPageLayout;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotWindow::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                      const QVariant&            oldValue,
                                      const QVariant&            newValue )
{
    RimViewWindow::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_showWindow )
    {
        updateWindowVisibility();
    }

    if ( changedField == &m_showPlotLegends || changedField == &m_plotLegendsHorizontal )
    {
        updateLayout();
    }
    else if ( changedField == &m_legendFontSize || changedField == &m_titleFontSize )
    {
        updateLayout();
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimPlotWindow::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                    bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_plotLegendsHorizontal )
    {
        options.push_back( caf::PdmOptionItemInfo( "Vertical", QVariant::fromValue( false ) ) );
        options.push_back( caf::PdmOptionItemInfo( "Horizontal", QVariant::fromValue( true ) ) );
    }
    else if ( fieldNeedingOptions == &m_titleFontSize || fieldNeedingOptions == &m_legendFontSize )
    {
        options = caf::FontTools::relativeSizeValueOptions( RiaPreferences::current()->defaultPlotFontSize() );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotWindow::uiOrderingForPlotLayout( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_showPlotLegends );
    uiOrdering.add( &m_plotLegendsHorizontal );
    uiOrdering.add( &m_titleFontSize );
    uiOrdering.add( &m_legendFontSize );
}

//--------------------------------------------------------------------------------------------------
/// Re-implement this in sub classes to provide a custom page layout for printing/PDF
//--------------------------------------------------------------------------------------------------
bool RimPlotWindow::hasCustomPageLayout( QPageLayout* customPageLayout ) const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotWindow::updateWindowVisibility()
{
    if ( isMdiWindow() )
    {
        updateMdiWindowVisibility();
    }
    else
    {
        updateParentLayout();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotWindow::setId( int id )
{
    m_id                  = id;
    QString viewIdTooltip = QString( "Plot id: %1" ).arg( m_id );
    this->setUiToolTip( viewIdTooltip );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotWindow::assignIdIfNecessary()
{
    if ( m_id == -1 && isMdiWindow() )
    {
        RimProject::current()->assignPlotIdToPlotWindow( this );
    }
}
