/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimQwtPlotCurve.h"

#include "RiaColorTables.h"
#include "RiaColorTools.h"
#include "RiaCurveDataTools.h"
#include "RiaGuiApplication.h"
#include "RiaPreferences.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimNameConfig.h"
#include "RimProject.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotWidget.h"
#include "RiuRimQwtPlotCurve.h"

#include "cafPdmUiComboBoxEditor.h"

#include "cvfAssert.h"

#include "qwt_date.h"
#include "qwt_interval_symbol.h"
#include "qwt_plot.h"
#include "qwt_symbol.h"

// NB! Special macro for pure virtual class
CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimQwtPlotCurve, "QwtPlotCurve" );

#define DOUBLE_INF std::numeric_limits<double>::infinity()

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimQwtPlotCurve::RimQwtPlotCurve()
{
    CAF_PDM_InitObject( "Curve", ":/WellLogCurve16x16.png", "", "" );

    // m_qwtPlotCurve      = new RiuRimQwtPlotCurve( this );
    m_qwtCurveErrorBars = new QwtPlotIntervalCurve();
    m_qwtCurveErrorBars->setStyle( QwtPlotIntervalCurve::CurveStyle::NoCurve );
    m_qwtCurveErrorBars->setSymbol( new QwtIntervalSymbol( QwtIntervalSymbol::Bar ) );
    m_qwtCurveErrorBars->setItemAttribute( QwtPlotItem::Legend, false );
    m_qwtCurveErrorBars->setZ( RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_ERROR_BARS ) );

    // m_parentQwtPlot = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimQwtPlotCurve::~RimQwtPlotCurve()
{
    // if ( m_qwtPlotCurve )
    // {
    //     m_qwtPlotCurve->detach();
    //     delete m_qwtPlotCurve;
    //     m_qwtPlotCurve = nullptr;
    // }

    // if ( m_qwtCurveErrorBars )
    // {
    //     m_qwtCurveErrorBars->detach();
    //     delete m_qwtCurveErrorBars;
    //     m_qwtCurveErrorBars = nullptr;
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimQwtPlotCurve::setParentQwtPlotAndReplot( QwtPlot* plot )
// {
//     // m_parentQwtPlot = plot;
//     // if ( canCurveBeAttached() )
//     // {
//     //     attachCurveAndErrorBars();
//     //     m_parentQwtPlot->replot();
//     // }
// }

// //--------------------------------------------------------------------------------------------------
// ///
// //--------------------------------------------------------------------------------------------------
// void RimQwtPlotCurve::setParentPlotNoReplot( QwtPlot* plot )
// {
//     // m_parentQwtPlot = plot;
//     // if ( canCurveBeAttached() )
//     // {
//     //     attachCurveAndErrorBars();
//     // }
//     // else
//     // {
//     //     m_qwtPlotCurve->detach();
//     //     m_qwtCurveErrorBars->detach();
//     // }
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimQwtPlotCurve::detachQwtCurve()
// {
//     // m_qwtPlotCurve->detach();
//     // m_qwtCurveErrorBars->detach();
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimQwtPlotCurve::reattachQwtCurve()
// {
//     // detachQwtCurve();
//     // if ( canCurveBeAttached() )
//     // {
//     //     attachCurveAndErrorBars();
//     // }
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// QwtPlotCurve* RimQwtPlotCurve::qwtPlotCurve() const
// {
//     return nullptr;
//     // return m_qwtPlotCurve;
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimQwtPlotCurve::setSamplesFromXYErrorValues(
//     const std::vector<double>&   xValues,
//     const std::vector<double>&   yValues,
//     const std::vector<double>&   errorValues,
//     bool                         keepOnlyPositiveValues,
//     RiaCurveDataTools::ErrorAxis errorAxis /*= RiuQwtPlotCurve::ERROR_ALONG_Y_AXIS */ )
// {
//     CVF_ASSERT( xValues.size() == yValues.size() );
//     CVF_ASSERT( xValues.size() == errorValues.size() );

//     auto intervalsOfValidValues = RiaCurveDataTools::calculateIntervalsOfValidValues( yValues, keepOnlyPositiveValues
//     ); std::vector<double> filteredYValues; std::vector<double> filteredXValues;

//     RiaCurveDataTools::getValuesByIntervals( yValues, intervalsOfValidValues, &filteredYValues );
//     RiaCurveDataTools::getValuesByIntervals( xValues, intervalsOfValidValues, &filteredXValues );

//     std::vector<double> filteredErrorValues;
//     RiaCurveDataTools::getValuesByIntervals( errorValues, intervalsOfValidValues, &filteredErrorValues );

//     QVector<QwtIntervalSample> errorIntervals;

//     errorIntervals.reserve( static_cast<int>( filteredXValues.size() ) );

//     for ( size_t i = 0; i < filteredXValues.size(); i++ )
//     {
//         if ( filteredYValues[i] != DOUBLE_INF && filteredErrorValues[i] != DOUBLE_INF )
//         {
//             if ( errorAxis == RiaCurveDataTools::ErrorAxis::ERROR_ALONG_Y_AXIS )
//             {
//                 errorIntervals << QwtIntervalSample( filteredXValues[i],
//                                                      filteredYValues[i] - filteredErrorValues[i],
//                                                      filteredYValues[i] + filteredErrorValues[i] );
//             }
//             else
//             {
//                 errorIntervals << QwtIntervalSample( filteredYValues[i],
//                                                      filteredXValues[i] - filteredErrorValues[i],
//                                                      filteredXValues[i] + filteredErrorValues[i] );
//             }
//         }
//     }

//     if ( m_qwtPlotCurve )
//     {
//         m_qwtPlotCurve->setSamples( filteredXValues.data(),
//                                     filteredYValues.data(),
//                                     static_cast<int>( filteredXValues.size() ) );

//         m_qwtPlotCurve->setLineSegmentStartStopIndices( intervalsOfValidValues );
//     }

//     if ( m_qwtCurveErrorBars )
//     {
//         m_qwtCurveErrorBars->setSamples( errorIntervals );
//         if ( errorAxis == RiaCurveDataTools::ErrorAxis::ERROR_ALONG_Y_AXIS )
//         {
//             m_qwtCurveErrorBars->setOrientation( Qt::Vertical );
//         }
//         else
//         {
//             m_qwtCurveErrorBars->setOrientation( Qt::Horizontal );
//         }
//     }
// }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
// void RimQwtPlotCurve::attachCurveAndErrorBars()
// {
//     m_qwtPlotCurve->attach( m_parentQwtPlot );

//     if ( m_showErrorBars )
//     {
//         m_qwtCurveErrorBars->attach( m_parentQwtPlot );
//     }
// }
