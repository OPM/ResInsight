/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiuPvtPlotWidget.h"

#include "RiuDockedQwtPlot.h"
#include "RiuGuiTheme.h"
#include "RiuPvtPlotPanel.h"
#include "RiuPvtPlotUpdater.h"

#include "RiaEclipseUnitTools.h"

#include "RigFlowDiagSolverInterface.h"

#include "cvfAssert.h"
#include "cvfMath.h"

#include "qwt_legend.h"
#include "qwt_picker_machine.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_picker.h"
#include "qwt_symbol.h"
#include "qwt_text.h"

#include <QHBoxLayout>

#include <functional>

//==================================================================================================
//
//
//
//==================================================================================================
class PvtQwtPlot : public RiuDockedQwtPlot
{
public:
    PvtQwtPlot( QWidget* parent )
        : RiuDockedQwtPlot( parent )
    {
    }
    QSize sizeHint() const override { return QSize( 100, 100 ); }
    QSize minimumSizeHint() const override { return QSize( 0, 0 ); }
};

//==================================================================================================
//
//
//
//==================================================================================================
class RiuPvtQwtPicker : public QwtPicker
{
public:
    RiuPvtQwtPicker( QwtPlot* plot, std::function<QString()> textProvider )
        : QwtPicker( QwtPicker::NoRubberBand, QwtPicker::AlwaysOn, plot->canvas() )
        , m_textProvider( textProvider )
    {
        setStateMachine( new QwtPickerTrackerMachine );
    }

    QwtText trackerText( const QPoint& ) const override
    {
        QwtText text( m_textProvider() );
        text.setRenderFlags( Qt::AlignLeft );
        return text;
    }

private:
    std::function<QString()> m_textProvider;
};

//==================================================================================================
///
/// \class RiuPvtPlotWidget
///
///
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPvtPlotWidget::RiuPvtPlotWidget( RiuPvtPlotPanel* parent )
    : QWidget( parent )
    , m_trackerPlotMarker( nullptr )
{
    m_qwtPlot = new PvtQwtPlot( this );
    m_qwtPlot->setProperty( "qss-class", "PvtPlot" );
    setPlotDefaults( m_qwtPlot );
    applyFontSizes( false );

    QHBoxLayout* layout = new QHBoxLayout();
    layout->addWidget( m_qwtPlot );
    layout->setSpacing( 0 );
    layout->setContentsMargins( 0, 0, 0, 0 );

    setLayout( layout );

    m_qwtPicker = new RiuPvtQwtPicker( m_qwtPlot, [this]() { return m_trackerLabel; } );
    RiuGuiTheme::styleQwtItem( m_qwtPicker );
    connect( m_qwtPicker, SIGNAL( activated( bool ) ), this, SLOT( slotPickerActivated( bool ) ) );
    connect( m_qwtPicker, SIGNAL( moved( const QPoint& ) ), this, SLOT( slotPickerPointChanged( const QPoint& ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotWidget::setPlotDefaults( QwtPlot* plot )
{
    plot->setAutoFillBackground( true );

    QFrame* canvasFrame = dynamic_cast<QFrame*>( plot->canvas() );
    if ( canvasFrame )
    {
        canvasFrame->setFrameShape( QFrame::NoFrame );
    }

    // Grid
    {
        QwtPlotGrid* grid = new QwtPlotGrid;
        grid->attach( plot );
        RiuGuiTheme::styleQwtItem( grid );
    }

    // Axis number font
    {
        QFont axisFont = plot->axisFont( QwtAxis::XBottom );
        axisFont.setPointSize( 8 );
        plot->setAxisFont( QwtAxis::XBottom, axisFont );
        plot->setAxisFont( QwtAxis::YLeft, axisFont );
    }

    // Axis title font
    {
        QwtText axisTitle     = plot->axisTitle( QwtAxis::XBottom );
        QFont   axisTitleFont = axisTitle.font();
        axisTitleFont.setPointSize( 8 );
        axisTitleFont.setBold( false );
        axisTitle.setFont( axisTitleFont );
        axisTitle.setRenderFlags( Qt::AlignRight );
        plot->setAxisTitle( QwtAxis::XBottom, axisTitle );
        plot->setAxisTitle( QwtAxis::YLeft, axisTitle );
    }

    // Title font
    {
        QwtText plotTitle = plot->title();
        QFont   titleFont = plotTitle.font();
        titleFont.setPointSize( 12 );
        plotTitle.setFont( titleFont );
        plot->setTitle( plotTitle );
    }

    plot->setAxisMaxMinor( QwtAxis::XBottom, 2 );
    plot->setAxisMaxMinor( QwtAxis::YLeft, 3 );

    plot->plotLayout()->setAlignCanvasToScales( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotWidget::plotCurves( RiaDefines::EclipseUnitSystem                    unitSystem,
                                   const std::vector<RigFlowDiagDefines::PvtCurve>& curveArr,
                                   double                                           pressure,
                                   double                                           pointMarkerYValue,
                                   const QString&                                   pointMarkerLabel,
                                   const QString&                                   plotTitle,
                                   const QString&                                   yAxisTitle )
{
    m_qwtPlot->detachItems( QwtPlotItem::Rtti_PlotCurve );
    m_qwtPlot->detachItems( QwtPlotItem::Rtti_PlotMarker );
    m_qwtCurveArr.clear();
    m_pvtCurveArr.clear();
    m_trackerPlotMarker = nullptr;

    // Construct an auxiliary curve that connects the first point in all the input curves as a visual aid
    // This should only be shown when the phase being plotted is oil
    // Will not be added to our array of qwt curves since we do not expect the user to interact with it
    {
        std::vector<double> xVals;
        std::vector<double> yVals;
        for ( size_t i = 0; i < curveArr.size(); i++ )
        {
            const RigFlowDiagDefines::PvtCurve& curve = curveArr[i];
            if ( curve.phase == RigFlowDiagDefines::PvtCurve::OIL && !curve.pressureVals.empty() && !curve.yVals.empty() )
            {
                xVals.push_back( curve.pressureVals[0] );
                yVals.push_back( curve.yVals[0] );
            }
        }

        if ( xVals.size() > 1 )
        {
            QwtPlotCurve* qwtCurve = new QwtPlotCurve( "Auxiliary" );
            qwtCurve->setSamples( xVals.data(), yVals.data(), static_cast<int>( xVals.size() ) );

            qwtCurve->setStyle( QwtPlotCurve::Lines );
            qwtCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );

            qwtCurve->attach( m_qwtPlot );
            RiuGuiTheme::styleQwtItem( qwtCurve );
        }
    }

    // Add the primary curves
    for ( size_t i = 0; i < curveArr.size(); i++ )
    {
        const RigFlowDiagDefines::PvtCurve& curve       = curveArr[i];
        QwtPlotCurve*                       qwtCurve    = new QwtPlotCurve();
        QwtSymbol*                          curveSymbol = new QwtSymbol( QwtSymbol::Ellipse );

        CVF_ASSERT( curve.pressureVals.size() == curve.yVals.size() );
        qwtCurve->setSamples( curve.pressureVals.data(), curve.yVals.data(), static_cast<int>( curve.pressureVals.size() ) );

        qwtCurve->setStyle( QwtPlotCurve::Lines );

        if ( curve.phase == RigFlowDiagDefines::PvtCurve::GAS )
        {
            qwtCurve->setTitle( "Gas" );
        }
        else if ( curve.phase == RigFlowDiagDefines::PvtCurve::OIL )
        {
            qwtCurve->setTitle( "Oil" );
        }
        else
        {
            qwtCurve->setTitle( "Undefined" );
        }

        qwtCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );

        curveSymbol->setSize( 6, 6 );
        curveSymbol->setBrush( Qt::NoBrush );
        qwtCurve->setSymbol( curveSymbol );

        qwtCurve->attach( m_qwtPlot );
        RiuGuiTheme::styleQwtItem( qwtCurve );

        m_qwtCurveArr.push_back( qwtCurve );
    }

    m_pvtCurveArr = curveArr;
    CVF_ASSERT( m_pvtCurveArr.size() == m_qwtCurveArr.size() );

    // Add vertical marker line to indicate cell pressure
    if ( pressure != HUGE_VAL )
    {
        QwtPlotMarker* lineMarker = new QwtPlotMarker;

        QPen pen;
        pen.setStyle( Qt::DashLine );
        lineMarker->setLinePen( pen );

        lineMarker->setXValue( pressure );
        lineMarker->setLineStyle( QwtPlotMarker::VLine );
        lineMarker->setLabel( QString( "PRESSURE" ) );
        lineMarker->setLabelAlignment( Qt::AlignTop | Qt::AlignRight );
        lineMarker->setLabelOrientation( Qt::Vertical );
        lineMarker->attach( m_qwtPlot );
        RiuGuiTheme::styleQwtItem( lineMarker );
    }

    // Then point marker
    if ( pressure != HUGE_VAL && pointMarkerYValue != HUGE_VAL )
    {
        QwtPlotMarker* pointMarker = new QwtPlotMarker;
        pointMarker->setValue( pressure, pointMarkerYValue );

        QwtSymbol* symbol = new QwtSymbol( QwtSymbol::Ellipse );
        symbol->setSize( 13, 13 );
        QPen pen;
        pen.setWidth( 2 );
        symbol->setPen( pen );
        symbol->setBrush( Qt::NoBrush );
        pointMarker->setSymbol( symbol );

        if ( !pointMarkerLabel.isEmpty() )
        {
            QwtText text( pointMarkerLabel );
            text.setRenderFlags( Qt::AlignLeft );
            text.setColor( RiuGuiTheme::getColorByVariableName( "textColor" ) );
            pointMarker->setLabel( text );
            pointMarker->setLabelAlignment( Qt::AlignTop | Qt::AlignRight );
        }

        pointMarker->attach( m_qwtPlot );
        RiuGuiTheme::styleQwtItem( pointMarker );
    }

    m_qwtPlot->setTitle( plotTitle );

    m_qwtPlot->setAxisTitle( QwtAxis::XBottom, QString( "Pressure [%1]" ).arg( RiaEclipseUnitTools::unitStringPressure( unitSystem ) ) );
    m_qwtPlot->setAxisTitle( QwtAxis::YLeft, yAxisTitle );

    updateTrackerPlotMarkerAndLabelFromPicker();

    m_qwtPlot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotWidget::applyFontSizes( bool replot )
{
    m_qwtPlot->applyFontSizes( replot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotWidget::updateTrackerPlotMarkerAndLabelFromPicker()
{
    bool    hasValidSamplePoint = false;
    QPointF samplePoint;
    QString mixRatioText = "";
    double  mixRat       = HUGE_VAL;

    if ( m_qwtPicker && m_qwtPicker->isActive() )
    {
        const QPoint trackerPos = m_qwtPicker->trackerPosition();

        int                 pointSampleIdx  = -1;
        const QwtPlotCurve* closestQwtCurve = closestCurveSample( trackerPos, &pointSampleIdx );
        if ( closestQwtCurve && pointSampleIdx >= 0 )
        {
            samplePoint         = closestQwtCurve->sample( pointSampleIdx );
            hasValidSamplePoint = true;

            size_t curveIdx = indexOfQwtCurve( closestQwtCurve );
            if ( curveIdx < m_pvtCurveArr.size() )
            {
                const RigFlowDiagDefines::PvtCurve& pvtCurve = m_pvtCurveArr[curveIdx];
                if ( static_cast<size_t>( pointSampleIdx ) < pvtCurve.mixRatVals.size() )
                {
                    mixRat = pvtCurve.mixRatVals[pointSampleIdx];

                    // The text is Rs or Rv depending on phase
                    mixRatioText = ( pvtCurve.phase == RigFlowDiagDefines::PvtCurve::GAS ) ? "Rv" : "Rs";
                }
            }
        }
    }

    m_trackerLabel = "";

    bool needsReplot = false;

    if ( hasValidSamplePoint )
    {
        if ( !m_trackerPlotMarker )
        {
            m_trackerPlotMarker = new QwtPlotMarker;
            m_trackerPlotMarker->setTitle( QString( "TrackedPoint" ) );

            QwtSymbol* symbol = new QwtSymbol( QwtSymbol::Ellipse );
            symbol->setSize( 13, 13 );
            symbol->setBrush( Qt::NoBrush );
            m_trackerPlotMarker->setSymbol( symbol );
            m_trackerPlotMarker->attach( m_qwtPlot );
            RiuGuiTheme::styleQwtItem( m_trackerPlotMarker );

            needsReplot = true;
        }

        if ( m_trackerPlotMarker->value() != samplePoint )
        {
            m_trackerPlotMarker->setValue( samplePoint );
            needsReplot = true;
        }

        m_trackerLabel = QString( "%1 (%2)" ).arg( samplePoint.y() ).arg( samplePoint.x() );
        if ( mixRat != HUGE_VAL )
        {
            m_trackerLabel += QString( "\n%1 = %2" ).arg( mixRatioText ).arg( mixRat );
        }
    }
    else
    {
        if ( m_trackerPlotMarker )
        {
            m_trackerPlotMarker->detach();
            delete m_trackerPlotMarker;
            m_trackerPlotMarker = nullptr;

            needsReplot = true;
        }
    }

    if ( needsReplot )
    {
        m_qwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QwtPlotCurve* RiuPvtPlotWidget::closestCurveSample( const QPoint& cursorPosition, int* closestSampleIndex ) const
{
    // Construct a set containing the relevant qwt curves to consider
    // These are the curves that have a corresponding Pvt source curve
    std::set<const QwtPlotCurve*> relevantQwtCurvesSet( m_qwtCurveArr.begin(), m_qwtCurveArr.end() );

    if ( closestSampleIndex ) *closestSampleIndex = -1;

    const QwtPlotCurve* closestCurve            = nullptr;
    double              distMin                 = HUGE_VAL;
    int                 closestPointSampleIndex = -1;

    // Ok to access reference to list, no add/remove of item during iteration
    const QwtPlotItemList& itemList = m_qwtPlot->itemList();
    for ( QwtPlotItemIterator it = itemList.begin(); it != itemList.end(); it++ )
    {
        if ( ( *it )->rtti() == QwtPlotItem::Rtti_PlotCurve )
        {
            const QwtPlotCurve* curve = static_cast<const QwtPlotCurve*>( *it );
            if ( relevantQwtCurvesSet.find( curve ) != relevantQwtCurvesSet.end() )
            {
                double dist                 = HUGE_VAL;
                int    candidateSampleIndex = curve->closestPoint( cursorPosition, &dist );
                if ( dist < distMin )
                {
                    closestCurve            = curve;
                    closestPointSampleIndex = candidateSampleIndex;
                    distMin                 = dist;
                }
            }
        }
    }

    if ( closestCurve && closestPointSampleIndex >= 0 && distMin < 50 )
    {
        if ( closestSampleIndex ) *closestSampleIndex = closestPointSampleIndex;
        return closestCurve;
    }
    else
    {
        return nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RiuPvtPlotWidget::indexOfQwtCurve( const QwtPlotCurve* qwtCurve ) const
{
    for ( size_t i = 0; i < m_qwtCurveArr.size(); i++ )
    {
        if ( m_qwtCurveArr[i] == qwtCurve )
        {
            return i;
        }
    }

    return cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotWidget::slotPickerPointChanged( const QPoint& pt )
{
    updateTrackerPlotMarkerAndLabelFromPicker();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPvtPlotWidget::slotPickerActivated( bool on )
{
    updateTrackerPlotMarkerAndLabelFromPicker();
}
