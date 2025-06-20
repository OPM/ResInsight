/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RimHistogramMultiPlot.h"

#include "RiaNumericalTools.h"
#include "RiaPlotDefines.h"

#include "RimEnsembleCurveSet.h"
#include "RimHistogramPlot.h"
#include "RimMainPlotCollection.h"
#include "RimMultiPlotCollection.h"
#include "RimPlotAxisProperties.h"

#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotWidget.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "qwt_scale_engine.h"

#include <QKeyEvent>

#include <cmath>

CAF_PDM_SOURCE_INIT( RimHistogramMultiPlot, "MultiHistogramPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramMultiPlot::RimHistogramMultiPlot()
    : duplicatePlot( this )
{
    CAF_PDM_InitObject( "Multi Histogram Plot", ":/HistogramPlotLight16x16.png" );
    setDeletable( true );

    CAF_PDM_InitField( &m_autoPlotTitle, "AutoPlotTitle", true, "Auto Plot Title" );
    CAF_PDM_InitField( &m_autoSubPlotTitle, "AutoSubPlotTitle", true, "Auto Sub Plot Title" );

    CAF_PDM_InitField( &m_createPlotDuplicate, "DuplicatePlot", false, "", "", "Duplicate Plot" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_createPlotDuplicate );
    m_createPlotDuplicate.uiCapability()->setUiIconFromResourceString( ":/Copy.svg" );

    CAF_PDM_InitField( &m_disableWheelZoom, "DisableWheelZoom", true, "", "", "Disable Mouse Wheel Zooming in Multi Histogram Plot" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_disableWheelZoom );
    m_disableWheelZoom.uiCapability()->setUiIconFromResourceString( ":/DisableZoom.png" );

    CAF_PDM_InitField( &m_autoAdjustAppearance, "AutoAdjustAppearance", true, "Auto Plot Settings" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_autoAdjustAppearance );
    CAF_PDM_InitField( &m_allow3DSelectionLink, "Allow3DSelectionLink", true, "Allow Well Selection from 3D View" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_allow3DSelectionLink );

    CAF_PDM_InitField( &m_hidePlotsWithValuesBelow, "HidePlotsWithValuesBelow", false, "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_hidePlotsWithValuesBelow );

    setBottomMargin( 40 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramMultiPlot::~RimHistogramMultiPlot()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::addPlot( RimPlot* plot )
{
    auto* sumPlot = dynamic_cast<RimHistogramPlot*>( plot );
    CVF_ASSERT( sumPlot != nullptr );
    if ( sumPlot )
    {
        RimMultiPlot::addPlot( plot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::insertPlot( RimPlot* plot, size_t index )
{
    auto* sumPlot = dynamic_cast<RimHistogramPlot*>( plot );
    CVF_ASSERT( sumPlot != nullptr );
    if ( sumPlot )
    {
        sumPlot->curvesChanged.connect( this, &RimHistogramMultiPlot::onSubPlotChanged );
        sumPlot->titleChanged.connect( this, &RimHistogramMultiPlot::onSubPlotChanged );
        sumPlot->autoTitleChanged.connect( this, &RimHistogramMultiPlot::onSubPlotAutoTitleChanged );

        RimMultiPlot::insertPlot( plot, index );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::removePlot( RimPlot* plot )
{
    auto* sumPlot = dynamic_cast<RimHistogramPlot*>( plot );
    CVF_ASSERT( sumPlot != nullptr );
    if ( sumPlot )
    {
        RimMultiPlot::removePlot( plot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::removePlotNoUpdate( RimPlot* plot )
{
    auto* sumPlot = dynamic_cast<RimHistogramPlot*>( plot );
    CVF_ASSERT( sumPlot != nullptr );
    if ( sumPlot )
    {
        RimMultiPlot::removePlotNoUpdate( plot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::updateAfterPlotRemove()
{
    onPlotAdditionOrRemoval();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto titlesGroup = uiOrdering.addNewGroup( "Main Plot Settings" );
    titlesGroup->setCollapsedByDefault();

    // If a checkbox is first in the group, it is not responding to mouse clicks. Set title as first element.
    // https://github.com/OPM/ResInsight/issues/10321
    titlesGroup->add( &m_plotWindowTitle );
    titlesGroup->add( &m_autoPlotTitle );
    titlesGroup->add( &m_showPlotWindowTitle );
    titlesGroup->add( &m_titleFontSize );

    auto subPlotSettingsGroup = uiOrdering.addNewGroup( "Sub Plot Settings" );
    subPlotSettingsGroup->setCollapsedByDefault();
    subPlotSettingsGroup->add( &m_autoSubPlotTitle );
    subPlotSettingsGroup->add( &m_showIndividualPlotTitles );
    subPlotSettingsGroup->add( &m_subTitleFontSize );

    auto legendsGroup = uiOrdering.addNewGroup( "Legends" );
    legendsGroup->setCollapsedByDefault();
    legendsGroup->add( &m_showPlotLegends );
    legendsGroup->add( &m_plotLegendsHorizontal );
    legendsGroup->add( &m_legendPosition );
    legendsGroup->add( &m_legendFontSize );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_autoPlotTitle || changedField == &m_autoSubPlotTitle )
    {
        onLoadDataAndUpdate();
        updateLayout();
    }
    else if ( changedField == &m_hidePlotsWithValuesBelow )
    {
        m_hidePlotsWithValuesBelow = false;
        updatePlotVisibility();
    }
    else if ( changedField == &m_createPlotDuplicate )
    {
        m_createPlotDuplicate = false;
        duplicate();
    }
    else if ( changedField == &m_autoAdjustAppearance )
    {
        analyzePlotsAndAdjustAppearanceSettings();
    }
    else if ( changedField == &m_plotWindowTitle )
    {
        // If the user has changed the plot title, disable the auto plot title
        // Workaround for https://github.com/OPM/ResInsight/issues/9681

        m_autoPlotTitle = false;
    }

    RimMultiPlot::fieldChangedByUi( changedField, oldValue, newValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::updatePlotTitles()
{
    if ( m_autoPlotTitle )
    {
        auto collection = RimMainPlotCollection::current()->histogramMultiPlotCollection();

        size_t index = 0;
        for ( auto p : collection->histogramMultiPlots() )
        {
            index++;
            if ( p == this ) break;
        }

        QString title = QString( "Plot %1" ).arg( index );

        setMultiPlotTitle( title );
    }

    if ( !m_viewer.isNull() ) m_viewer->scheduleTitleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::setAutoPlotTitle( bool enable )
{
    m_autoPlotTitle = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::setAutoSubPlotTitle( bool enable )
{
    m_autoSubPlotTitle = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimHistogramPlot*> RimHistogramMultiPlot::histogramPlots() const
{
    std::vector<RimHistogramPlot*> typedPlots;

    for ( auto plot : plots() )
    {
        auto histogramPlot = dynamic_cast<RimHistogramPlot*>( plot );
        if ( histogramPlot ) typedPlots.push_back( histogramPlot );
    }

    return typedPlots;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimHistogramPlot*> RimHistogramMultiPlot::visibleHistogramPlots() const
{
    std::vector<RimHistogramPlot*> visiblePlots;

    for ( auto plot : histogramPlots() )
    {
        if ( plot->showWindow() ) visiblePlots.push_back( plot );
    }

    return visiblePlots;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimHistogramMultiPlot::handleGlobalKeyEvent( QKeyEvent* keyEvent )
{
    if ( isMouseCursorInsidePlot() )
    {
        if ( keyEvent->key() == Qt::Key_PageUp )
        {
            m_viewer->goToPrevPage();
            return true;
        }

        if ( keyEvent->key() == Qt::Key_PageDown )
        {
            m_viewer->goToNextPage();
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimHistogramMultiPlot::handleGlobalWheelEvent( QWheelEvent* wheelEvent )
{
    if ( m_disableWheelZoom )
    {
        if ( isMouseCursorInsidePlot() )
        {
            if ( wheelEvent->angleDelta().y() > 0 )
            {
                m_viewer->goToPrevPage();
            }
            else if ( wheelEvent->angleDelta().y() < 0 )
            {
                m_viewer->goToNextPage();
            }

            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::initAfterRead()
{
    RimMultiPlot::initAfterRead();

    for ( auto plot : histogramPlots() )
    {
        plot->curvesChanged.connect( this, &RimHistogramMultiPlot::onSubPlotChanged );
        plot->titleChanged.connect( this, &RimHistogramMultiPlot::onSubPlotChanged );
        plot->autoTitleChanged.connect( this, &RimHistogramMultiPlot::onSubPlotAutoTitleChanged );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::onLoadDataAndUpdate()
{
    RimMultiPlot::onLoadDataAndUpdate();
    updatePlotTitles();

    for ( auto p : histogramPlots() )
    {
        p->updateAxes();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::zoomAll()
{
    // Reset zoom to make sure the complete range for min/max is available
    RimMultiPlot::zoomAll();

    syncAxisRanges();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::syncAxisRanges()
{
    // Reset zoom for axes with no custom range set to make sure the complete range for min/max is available

    for ( auto p : histogramPlots() )
    {
        for ( auto ax : p->plotAxes( RimPlotAxisProperties::Orientation::ANY ) )
        {
            ax->setAutoZoomIfNoCustomRangeIsSet();
        }
    }
    updateZoom();

    std::map<QString, std::pair<double, double>> axisRanges;

    // gather current min/max values for each category (axis label)
    for ( auto plot : histogramPlots() )
    {
        for ( auto axis : plot->plotAxes( RimPlotAxisProperties::Orientation::ANY ) )
        {
            double minVal = axis->visibleRangeMin();
            double maxVal = axis->visibleRangeMax();
            if ( axis->isAxisInverted() ) std::swap( minVal, maxVal );

            auto key = axis->objectName();
            if ( axisRanges.count( key ) == 0 )
            {
                axisRanges[key] = std::make_pair( minVal, maxVal );
            }
            else
            {
                auto& [currentMin, currentMax] = axisRanges[key];
                axisRanges[key]                = std::make_pair( std::min( currentMin, minVal ), std::max( currentMax, maxVal ) );
            }
        }
    }

    // set all plots to use the global min/max values for each category
    for ( auto plot : histogramPlots() )
    {
        for ( auto axis : plot->plotAxes( RimPlotAxisProperties::Orientation::ANY ) )
        {
            auto [minVal, maxVal] = axisRanges[axis->objectName()];
            if ( axis->isAxisInverted() ) std::swap( minVal, maxVal );
            axis->setAutoZoom( false );

            axis->setAutoValueVisibleRangeMin( minVal );
            axis->setAutoValueVisibleRangeMax( maxVal );
        }

        plot->updateAxes();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::updatePlotVisibility()
{
    for ( auto plot : histogramPlots() )
    {
        plot->setShowWindow( true );
    }

    updateLayout();

    if ( !m_viewer.isNull() ) m_viewer->scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::duplicate()
{
    duplicatePlot.send( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::analyzePlotsAndAdjustAppearanceSettings()
{
    if ( m_autoAdjustAppearance )
    {
        // Required to sync axis ranges before computing the auto scale
        syncAxisRanges();

        const bool notifyFieldChanged = false;

        for ( auto p : histogramPlots() )
        {
            for ( auto* axisProp : p->plotAxes( RimPlotAxisProperties::Orientation::ANY ) )
            {
                auto tickMarkCount = ( rowsPerPage() == 1 ) ? RimPlotAxisProperties::LegendTickmarkCount::TICKMARK_DEFAULT
                                                            : RimPlotAxisProperties::LegendTickmarkCount::TICKMARK_FEW;

                axisProp->setAutoValueForMajorTickmarkCount( tickMarkCount, notifyFieldChanged );

                axisProp->computeAndSetAutoValueForScaleFactor();

                auto [row, col] = gridLayoutInfoForSubPlot( p );

                bool isFirstColumn = ( col == 0 );
                axisProp->setShowUnitText( isFirstColumn );
                axisProp->setShowDescription( isFirstColumn );
            }

            p->updateAxes();
        }
    }
    else
    {
        for ( auto p : histogramPlots() )
        {
            for ( auto* axisProp : p->plotAxes( RimPlotAxisProperties::Orientation::ANY ) )
            {
                axisProp->computeAndSetAutoValueForScaleFactor();
                axisProp->setShowUnitText( true );
                axisProp->setShowDescription( true );
            }

            p->updateAxes();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::makeSureIsVisible( RimHistogramPlot* histogramPlot )
{
    if ( histogramPlot->plotWidget() && !m_viewer.isNull() ) m_viewer->scrollToPlot( histogramPlot->plotWidget() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, int> RimHistogramMultiPlot::gridLayoutInfoForSubPlot( RimHistogramPlot* histogramPlot ) const
{
    auto it = m_gridLayoutInfo.find( histogramPlot );
    if ( it != m_gridLayoutInfo.end() ) return it->second;

    return { -1, -1 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::setLayoutInfo( RimHistogramPlot* histogramPlot, int row, int col )
{
    m_gridLayoutInfo[histogramPlot] = std::make_pair( row, col );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::clearLayoutInfo()
{
    m_gridLayoutInfo.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::onSubPlotChanged( const caf::SignalEmitter* emitter )
{
    updatePlotTitles();
    applyPlotWindowTitleToWidgets();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::onSubPlotAutoTitleChanged( const caf::SignalEmitter* emitter, bool isEnabled )
{
    m_autoSubPlotTitle = isEnabled;

    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimHistogramMultiPlot::adjustedMinMax( const RimPlotAxisProperties* axis, double min, double max ) const
{
    if ( !axis->isLogarithmicScaleEnabled() )
    {
        int                  maxMajorTickIntervalCount = RimPlotAxisProperties::tickmarkCountFromEnum( axis->majorTickmarkCount() );
        double               stepSize                  = 0.0;
        QwtLinearScaleEngine scaleEngine;

        // Do not adjust minimum value, as we usually want to keep zero unchanged
        double adjustedMin = min;

        // Adjust the max value to get some space between the top of the plot and the top of the curve
        double adjustedMax = max * 1.05;

        scaleEngine.autoScale( maxMajorTickIntervalCount, adjustedMin, adjustedMax, stepSize );

        return { adjustedMin, adjustedMax };
    }

    auto adjustedMin = RiaNumericalTools::roundToClosestPowerOfTenFloor( min );
    auto adjustedMax = RiaNumericalTools::roundToClosestPowerOfTenCeil( max );

    return { adjustedMin, adjustedMax };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimHistogramMultiPlot::createViewWidget( QWidget* mainWindowParent )
{
    if ( m_viewer.isNull() )
    {
        m_viewer = new RiuMultiPlotBook( this, mainWindowParent );
    }
    recreatePlotWidgets();

    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::onPlotAdditionOrRemoval()
{
    m_disableWheelZoom = ( histogramPlots().size() > 1 );

    RimMultiPlot::onPlotAdditionOrRemoval();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistogramMultiPlot::keepVisiblePageAfterUpdate( bool keepPage )
{
    if ( !m_viewer ) return;

    if ( keepPage ) m_viewer->keepCurrentPageAfterUpdate();
}
