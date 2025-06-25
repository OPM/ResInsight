/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RimHistogramCurve.h"

#include "RiaGuiApplication.h"
#include "RiaPlotDefines.h"

#include "RimHistogramCurveCollection.h"
#include "RimHistogramDataSource.h"
#include "RimHistogramPlot.h"
#include "RimPlotAxisProperties.h"
#include "RimProject.h"
#include "RimTools.h"

#include "RiuPlotAxis.h"
#include "RiuPlotCurve.h"
#include "RiuPlotMainWindow.h"

#include "cafAssert.h"
#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimHistogramCurve, "HistogramCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramCurve::RimHistogramCurve()
{
    CAF_PDM_InitObject( "Histogram Curve", ":/SummaryCurve16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_yPlotAxisProperties, "YAxis", "Y Axis" );
    CAF_PDM_InitFieldNoDefault( &m_xPlotAxisProperties, "XAxis", "X Axis" );
    CAF_PDM_InitFieldNoDefault( &m_dataSource, "DataSource", "Data Source" );
    m_dataSource.uiCapability()->setUiTreeHidden( true );

    setSymbolSkipDistance( 10.0f );
    setLineThickness( 2 );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramCurve::~RimHistogramCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::setTopOrBottomAxisX( RiuPlotAxis plotAxis )
{
    CAF_ASSERT( plotAxis.isHorizontal() );

    RimHistogramPlot* plot = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();
    m_xPlotAxisProperties  = plot->axisPropertiesForPlotAxis( plotAxis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::setDataSource( RimHistogramDataSource* dataSource )
{
    m_dataSource = dataSource;

    if ( m_dataSource )
    {
        m_dataSource->uiCapability()->setUiTreeHidden( true );
        m_dataSource->dataSourceChanged.connect( this, &RimHistogramCurve::onDataSourceChanged );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RimHistogramCurve::axisX() const
{
    if ( m_xPlotAxisProperties )
        return m_xPlotAxisProperties->plotAxis();
    else
        return RiuPlotAxis::defaultBottom();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimHistogramCurve::unitNameY() const
{
    if ( m_dataSource )
        return m_dataSource()->unitNameY();
    else
        return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimHistogramCurve::unitNameX() const
{
    if ( m_dataSource )
        return m_dataSource()->unitNameX();
    else
        return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimHistogramCurve::valuesY() const
{
    if ( m_dataSource )
    {
        RimHistogramPlot* plot = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();
        return m_dataSource()->valuesY( plot->graphType(), plot->frequencyType() );
    }
    else
        return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimHistogramCurve::valuesX() const
{
    if ( m_dataSource )
    {
        RimHistogramPlot* plot = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();
        return m_dataSource()->valuesX( plot->graphType() );
    }
    else
        return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::setLeftOrRightAxisY( RiuPlotAxis plotAxis )
{
    CAF_ASSERT( plotAxis.isVertical() );

    RimHistogramPlot* plot = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();
    m_yPlotAxisProperties  = plot->axisPropertiesForPlotAxis( plotAxis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RimHistogramCurve::axisY() const
{
    if ( m_yPlotAxisProperties )
        return m_yPlotAxisProperties->plotAxis();
    else
        return RiuPlotAxis::defaultLeft();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimHistogramCurve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options = RimPlotCurve::calculateValueOptions( fieldNeedingOptions );
    if ( !options.isEmpty() )
        return options;
    else if ( fieldNeedingOptions == &m_yPlotAxisProperties )
    {
        auto plot = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();

        for ( auto axis : plot->plotAxes( RimPlotAxisProperties::Orientation::VERTICAL ) )
        {
            options.push_back( caf::PdmOptionItemInfo( axis->objectName(), axis ) );
        }
    }
    else if ( fieldNeedingOptions == &m_xPlotAxisProperties )
    {
        auto plot = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();

        for ( auto axis : plot->plotAxes( RimPlotAxisProperties::Orientation::HORIZONTAL ) )
        {
            options.push_back( caf::PdmOptionItemInfo( axis->objectName(), axis ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimHistogramCurve::createCurveAutoName()
{
    QString curveName = "";
    if ( m_dataSource )
    {
        curveName = QString::fromStdString( m_dataSource->name() );
    }

    if ( curveName.isEmpty() )
    {
        curveName = "Curve Name Placeholder";
    }

    return curveName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::updateZoomInParentPlot()
{
    auto plot = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();

    plot->updatePlotWidgetFromAxisRanges();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
    RimPlotCurve::updateCurvePresentation( updateParentPlot );

    updateConnectedEditors();

    if ( isChecked() )
    {
        std::vector<double> curveValuesX = valuesX();
        std::vector<double> curveValuesY = valuesY();

        auto plot                = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();
        bool useLogarithmicScale = plot->isLogarithmicScaleEnabled( axisY() );

        setSamplesFromXYValues( curveValuesX, curveValuesY, useLogarithmicScale );
    }

    if ( updateParentPlot && hasParentPlot() )
    {
        updateZoomInParentPlot();
        replotParentPlot();
    }

    if ( updateParentPlot ) updatePlotAxis();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::updateLegendsInPlot()
{
    auto plot = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();
    plot->updateLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    RimPlotCurve::defineUiTreeOrdering( uiTreeOrdering, uiConfigName );

    caf::IconProvider iconProvider = uiIconProvider();
    if ( !iconProvider.valid() ) return;

    uiTreeOrdering.skipRemainingChildren( true );

    setUiIcon( iconProvider );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimPlotCurve::updateFieldUiState();

    if ( m_dataSource )
    {
        caf::PdmUiGroup* dataSourceGroup = uiOrdering.addNewGroup( "Data Source" );
        m_dataSource->uiOrdering( uiConfigName, *dataSourceGroup );
    }

    caf::PdmUiGroup* stackingGroup = uiOrdering.addNewGroup( "Stacking" );
    RimStackablePlotCurve::stackingUiOrdering( *stackingGroup );

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup( "Appearance" );
    RimPlotCurve::appearanceUiOrdering( *appearanceGroup );

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Curve Name" );
    nameGroup->setCollapsedByDefault();
    nameGroup->add( &m_showLegend );
    RimPlotCurve::curveNameUiOrdering( *nameGroup );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::updatePlotAxis()
{
    updateYAxisInPlot( axisY() );
    updateXAxisInPlot( axisX() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimStackablePlotCurve::fieldChangedByUi( changedField, oldValue, newValue );

    auto plot = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();

    bool loadAndUpdate = false;
    if ( &m_showCurve == changedField )
    {
        plot->updateAxes();
        plot->updatePlotTitle();
    }
    else if ( changedField == &m_yPlotAxisProperties )
    {
        updateYAxisInPlot( axisY() );
        plot->updateAxes();
        dataChanged.send();
    }
    else if ( changedField == &m_xPlotAxisProperties )
    {
        updateXAxisInPlot( axisX() );
        plot->updateAxes();
        dataChanged.send();
    }

    if ( loadAndUpdate )
    {
        loadAndUpdateDataAndPlot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::loadAndUpdateDataAndPlot()
{
    loadDataAndUpdate( true );

    auto plot = firstAncestorOrThisOfTypeAsserted<RimHistogramPlot>();

    plot->updateAxes();
    plot->updatePlotTitle();
    plot->updateConnectedEditors();

    RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
    mainPlotWindow->updateMultiPlotToolBar();

    dataChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::updateLegendEntryVisibilityNoPlotUpdate()
{
    if ( !m_plotCurve ) return;

    bool showLegendInPlot = m_showLegend();

    m_plotCurve->setVisibleInLegend( showLegendInPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimHistogramCurve::canCurveBeAttached() const
{
    if ( !RimPlotCurve::canCurveBeAttached() ) return false;

    bool isVisibleInPossibleParent = true;

    auto histogramCurveCollection = firstAncestorOrThisOfType<RimHistogramCurveCollection>();
    if ( histogramCurveCollection ) isVisibleInPossibleParent = histogramCurveCollection->isCurvesVisible();

    return isVisibleInPossibleParent;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::onDataSourceChanged( const caf::SignalEmitter* emitter )
{
    loadAndUpdateDataAndPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramCurve::setAppearanceFromGraphType( RimHistogramPlot::GraphType graphType )
{
    auto fillType = graphType == RimHistogramPlot::GraphType::BAR_GRAPH ? Qt::SolidPattern : Qt::NoBrush;
    setFillStyle( fillType );
    float transparency = graphType == RimHistogramPlot::GraphType::BAR_GRAPH ? 0.2 : 1.0;
    setFillColorTransparency( transparency );
}
