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
#include "RiaPreferences.h"

#include "cafPdmUiComboBoxEditor.h"

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimPlotWindow, "RimPlotWindow" ); // Do not use. Abstract class

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotWindow::RimPlotWindow()
{
    CAF_PDM_InitObject( "Plot", "", "", "" );

    CAF_PDM_InitField( &m_description, "PlotDescription", QString( "" ), "Name", "", "", "" );

    CAF_PDM_InitField( &m_showTitleInPlot, "ShowTitleInPlot", true, "Show Title", "", "", "" );
    CAF_PDM_InitField( &m_showPlotLegends, "ShowTrackLegends", true, "Show Legends", "", "", "" );
    CAF_PDM_InitField( &m_plotLegendsHorizontal, "TrackLegendsHorizontal", false, "Legend Orientation", "", "", "" );
    m_plotLegendsHorizontal.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    int fontSize = RiaFontCache::pointSizeFromFontSizeEnum(
        RiaApplication::instance()->preferences()->defaultPlotFontSize() );
    CAF_PDM_InitField( &m_legendFontSize, "LegendFontSize", fontSize, "Legend Font Size", "", "", "" );
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
void RimPlotWindow::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                      const QVariant&            oldValue,
                                      const QVariant&            newValue )
{
    RimViewWindow::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_showPlotLegends || changedField == &m_plotLegendsHorizontal )
    {
        updateLayout();
    }
    else if ( changedField == &m_showTitleInPlot || changedField == &m_description )
    {
        updatePlotTitle();
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
caf::PdmFieldHandle* RimPlotWindow::userDescriptionField()
{
    return &m_description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotWindow::uiOrderingForPlotLayout( caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_showPlotLegends );
    uiOrdering.add( &m_plotLegendsHorizontal );
    uiOrdering.add( &m_legendFontSize );
}
