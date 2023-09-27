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

#include "RimAsciiDataCurve.h"

#include "RiaDefines.h"
#include "RimEclipseResultCase.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryPlot.h"
#include "RimSummaryTimeAxisProperties.h"

#include "RiuQwtPlotCurve.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "qwt_date.h"
#include "qwt_plot.h"

CAF_PDM_SOURCE_INIT( RimAsciiDataCurve, "AsciiDataCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAsciiDataCurve::RimAsciiDataCurve()
{
    CAF_PDM_InitObject( "ASCII Data Curve", ":/SummaryCurve16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_plotAxis, "PlotAxis", "Axis" );
    CAF_PDM_InitFieldNoDefault( &m_timeSteps, "TimeSteps", "Time Steps" );
    CAF_PDM_InitFieldNoDefault( &m_values, "Values", "Values" );
    m_values.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_title, "Title", "Title" );

    setSymbolSkipDistance( 10.0f );
    setLineThickness( 2 );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAsciiDataCurve::~RimAsciiDataCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimAsciiDataCurve::yValues() const
{
    return m_values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RimAsciiDataCurve::timeSteps() const
{
    static std::vector<time_t> timeSteps;
    timeSteps.clear();

    for ( const QDateTime& dateTime : m_timeSteps() )
    {
        timeSteps.push_back( dateTime.toSecsSinceEpoch() );
    }

    return timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAsciiDataCurve::setYAxis( RiaDefines::PlotAxis plotAxis )
{
    m_plotAxis = plotAxis;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAxis RimAsciiDataCurve::yAxis() const
{
    return RiuPlotAxis( m_plotAxis() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimAsciiDataCurve::createCurveAutoName()
{
    return m_title();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAsciiDataCurve::updateZoomInParentPlot()
{
    auto plot = firstAncestorOrThisOfType<RimSummaryPlot>();

    plot->updateZoomInParentPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAsciiDataCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
    RimPlotCurve::updateCurvePresentation( updateParentPlot );

    if ( isChecked() )
    {
        std::vector<time_t> dateTimes = timeSteps();
        std::vector<double> values    = yValues();

        auto plot                = firstAncestorOrThisOfType<RimSummaryPlot>();
        bool useLogarithmicScale = plot->isLogarithmicScaleEnabled( yAxis() );

        if ( !dateTimes.empty() && dateTimes.size() == values.size() )
        {
            if ( plot->timeAxisProperties()->timeMode() == RimSummaryTimeAxisProperties::DATE )
            {
                m_plotCurve->setSamplesFromTimeTAndYValues( dateTimes, values, useLogarithmicScale );
            }
            else
            {
                double timeScale = plot->timeAxisProperties()->fromTimeTToDisplayUnitScale();

                std::vector<double> times;
                if ( !dateTimes.empty() )
                {
                    time_t startDate = dateTimes[0];
                    for ( time_t& date : dateTimes )
                    {
                        times.push_back( timeScale * ( date - startDate ) );
                    }
                }

                m_plotCurve->setSamplesFromXValuesAndYValues( times, values, useLogarithmicScale );
            }
        }
        else
        {
            m_plotCurve->setSamplesFromTimeTAndYValues( std::vector<time_t>(), std::vector<double>(), useLogarithmicScale );
        }

        updateZoomInParentPlot();

        if ( m_parentPlot ) m_parentPlot->replot();
    }

    updateQwtPlotAxis();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAsciiDataCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimPlotCurve::updateFieldUiState();

    uiOrdering.add( &m_plotAxis );

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup( "Appearance" );
    RimPlotCurve::appearanceUiOrdering( *appearanceGroup );

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Curve Name" );
    nameGroup->setCollapsedByDefault();
    nameGroup->add( &m_showLegend );
    RimPlotCurve::curveNameUiOrdering( *nameGroup );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAsciiDataCurve::updateQwtPlotAxis()
{
    if ( m_plotCurve ) updateYAxisInPlot( yAxis() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAsciiDataCurve::setTimeSteps( const std::vector<QDateTime>& timeSteps )
{
    m_timeSteps = timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAsciiDataCurve::setValues( const std::vector<double>& values )
{
    m_values = values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAsciiDataCurve::setTitle( const QString& title )
{
    m_title = title;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimAsciiDataCurve::curveData( std::vector<QDateTime>* timeSteps, std::vector<double>* values ) const
{
    CVF_ASSERT( timeSteps && values );

    *timeSteps = m_timeSteps();
    *values    = m_values();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAsciiDataCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimPlotCurve::fieldChangedByUi( changedField, oldValue, newValue );

    auto plot = firstAncestorOrThisOfTypeAsserted<RimSummaryPlot>();

    if ( changedField == &m_plotAxis )
    {
        updateQwtPlotAxis();
        plot->updateAxes();
    }
    if ( changedField == &m_showCurve )
    {
        plot->updateAxes();
    }
}
