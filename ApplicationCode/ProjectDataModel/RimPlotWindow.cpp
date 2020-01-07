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

#include "RiaApplication.h"
#include "RiaPlotWindowRedrawScheduler.h"
#include "RiaPreferences.h"

#include "RicfCommandObject.h"

#include "RimProject.h"

#include "cafPdmUiComboBoxEditor.h"

#include <QPainter>

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimPlotWindow, "RimPlotWindow" ); // Do not use. Abstract class

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotWindow::RimPlotWindow()
{
    CAF_PDM_InitObject( "PlotWindow", "", "", "" );

    RICF_InitField( &m_id, "ViewId", -1, "View ID", "", "", "" );
    m_id.uiCapability()->setUiReadOnly( true );
    m_id.uiCapability()->setUiHidden( true );
    m_id.capability<RicfFieldHandle>()->setIOWriteable( false );
    m_id.xmlCapability()->setCopyable( false );

    CAF_PDM_InitField( &m_showPlotLegends, "ShowTrackLegends", true, "Show Legends", "", "", "" );
    CAF_PDM_InitField( &m_plotLegendsHorizontal, "TrackLegendsHorizontal", true, "Legend Orientation", "", "", "" );
    m_plotLegendsHorizontal.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    int defaultFontSize = RiaApplication::instance()->preferences()->defaultPlotFontSize();
    CAF_PDM_InitField( &m_legendFontSize,
                       "LegendFontSize",
                       std::max( 8, defaultFontSize - 2 ),
                       "Legend Font Size",
                       "",
                       "",
                       "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotWindow::~RimPlotWindow() {}

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
    m_showPlotLegends       = rhs.m_showPlotLegends();
    m_plotLegendsHorizontal = rhs.m_plotLegendsHorizontal();
    m_legendFontSize        = rhs.m_legendFontSize();
    return *this;
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
int RimPlotWindow::legendFontSize() const
{
    return m_legendFontSize;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotWindow::setLegendFontSize( int fontSize )
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
void RimPlotWindow::renderWindowContent( QPaintDevice* paintDevice )
{
    doRenderWindowContent( paintDevice );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QPageLayout RimPlotWindow::pageLayout() const
{
    QPageLayout defaultPageLayout = RiaApplication::instance()->preferences()->defaultPageLayout();
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

    if ( changedField == &m_showPlotLegends || changedField == &m_plotLegendsHorizontal )
    {
        updateLayout();
    }
    else if ( changedField == &m_legendFontSize )
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
    if ( fieldNeedingOptions == &m_legendFontSize )
    {
        std::vector<int> fontSizes;
        fontSizes.push_back( 8 );
        fontSizes.push_back( 9 );
        fontSizes.push_back( 10 );
        fontSizes.push_back( 11 );
        fontSizes.push_back( 12 );
        fontSizes.push_back( 14 );
        fontSizes.push_back( 16 );
        fontSizes.push_back( 18 );
        fontSizes.push_back( 24 );

        for ( int value : fontSizes )
        {
            QString text = QString( "%1" ).arg( value );
            options.push_back( caf::PdmOptionItemInfo( text, value ) );
        }
    }
    else if ( fieldNeedingOptions == &m_plotLegendsHorizontal )
    {
        options.push_back( caf::PdmOptionItemInfo( "Vertical", QVariant::fromValue( false ) ) );
        options.push_back( caf::PdmOptionItemInfo( "Horizontal", QVariant::fromValue( true ) ) );
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotWindow::uiOrderingForLegendSettings( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_showPlotLegends );
    uiOrdering.add( &m_plotLegendsHorizontal );
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
        RiaApplication::instance()->project()->assignPlotIdToPlotWindow( this );
    }
}
