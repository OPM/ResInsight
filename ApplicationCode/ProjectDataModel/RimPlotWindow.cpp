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

#include "cafPdmUiComboBoxEditor.h"

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimPlotWindow, "RimPlotWindow" ); // Do not use. Abstract class

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotWindow::RimPlotWindow()
{
    CAF_PDM_InitObject( "Plot", "", "", "" );

    CAF_PDM_InitField( &m_description, "PlotDescription", QString( "" ), "Name", "", "", "" );

    CAF_PDM_InitField( &m_showTitleInPlot, "ShowTitleInPlot", false, "Show Title", "", "", "" );
    CAF_PDM_InitField( &m_showPlotLegends, "ShowTrackLegends", true, "Show Legends", "", "", "" );
    CAF_PDM_InitField( &m_plotLegendsHorizontal, "TrackLegendsHorizontal", false, "Legend Orientation", "", "", "" );
    m_plotLegendsHorizontal.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotWindow::~RimPlotWindow() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotWindow& RimPlotWindow::operator=( RimPlotWindow&& rhs )
{
    m_showTitleInPlot       = rhs.m_showTitleInPlot();
    m_showPlotLegends       = rhs.m_showPlotLegends();
    m_plotLegendsHorizontal = rhs.m_plotLegendsHorizontal();
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotWindow::setDescription( const QString& description )
{
    m_description = description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPlotWindow::description() const
{
    return m_description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPlotWindow::fullPlotTitle() const
{
    return m_description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotWindow::isPlotTitleVisible() const
{
    return m_showTitleInPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotWindow::setPlotTitleVisible( bool visible )
{
    m_showTitleInPlot = visible;
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
void RimPlotWindow::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                      const QVariant&            oldValue,
                                      const QVariant&            newValue )
{
    RimViewWindow::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_showPlotLegends || changedField == &m_plotLegendsHorizontal )
    {
        updateLayout();
    }
    else if ( changedField == &m_showTitleInPlot )
    {
        updatePlotTitle();
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
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmUiGroup* RimPlotWindow::createPlotSettingsUiGroup( caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* titleAndLegendsGroup = uiOrdering.addNewGroup( "Title and Legends" );
    titleAndLegendsGroup->add( &m_showPlotLegends );
    titleAndLegendsGroup->add( &m_plotLegendsHorizontal );
    titleAndLegendsGroup->add( &m_showTitleInPlot );
    return titleAndLegendsGroup;
}
