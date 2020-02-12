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

#include "RiuRelativePermeabilityPlotPanel.h"
#include "RiuDockedQwtPlot.h"
#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotTools.h"
#include "RiuRelativePermeabilityPlotUpdater.h"
#include "RiuTextDialog.h"

#include "RiaCurveDataTools.h"
#include "RigFlowDiagSolverInterface.h"

#include "cvfAssert.h"
#include "cvfTrace.h"

#include "qwt_legend.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_marker.h"
#include "qwt_scale_engine.h"
#include "qwt_symbol.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QContextMenuEvent>
#include <QDockWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMenu>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

//==================================================================================================
//
//
//
//==================================================================================================
class RelPermQwtPlot : public RiuDockedQwtPlot
{
public:
    RelPermQwtPlot( QWidget* parent )
        : RiuDockedQwtPlot( parent )
    {
    }
    QSize sizeHint() const override { return QSize( 100, 100 ); }
    QSize minimumSizeHint() const override { return QSize( 0, 0 ); }
};

//==================================================================================================
///
/// \class RiuRelativePermeabilityPlotPanel
///
///
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuRelativePermeabilityPlotPanel::RiuRelativePermeabilityPlotPanel( QDockWidget* parent )
    : QWidget( parent )
    , m_unitSystem( RiaEclipseUnitTools::UNITS_UNKNOWN )
    , m_swat( HUGE_VAL )
    , m_sgas( HUGE_VAL )
    , m_plotUpdater( new RiuRelativePermeabilityPlotUpdater( this ) )
{
    m_qwtPlot = new RelPermQwtPlot( this );
    setPlotDefaults( m_qwtPlot );
    applyFontSizes( false );

    m_selectedCurvesButtonGroup = new QButtonGroup( this );
    m_selectedCurvesButtonGroup->setExclusive( false );

    m_selectedCurvesButtonGroup->addButton( new QCheckBox( "KRW" ), RigFlowDiagSolverInterface::RelPermCurve::KRW );
    m_selectedCurvesButtonGroup->addButton( new QCheckBox( "KRG" ), RigFlowDiagSolverInterface::RelPermCurve::KRG );
    m_selectedCurvesButtonGroup->addButton( new QCheckBox( "KROW" ), RigFlowDiagSolverInterface::RelPermCurve::KROW );
    m_selectedCurvesButtonGroup->addButton( new QCheckBox( "KROG" ), RigFlowDiagSolverInterface::RelPermCurve::KROG );
    m_selectedCurvesButtonGroup->addButton( new QCheckBox( "PCOW" ), RigFlowDiagSolverInterface::RelPermCurve::PCOW );
    m_selectedCurvesButtonGroup->addButton( new QCheckBox( "PCOG" ), RigFlowDiagSolverInterface::RelPermCurve::PCOG );

    QGroupBox*   groupBox       = new QGroupBox( "Curves" );
    QGridLayout* groupBoxLayout = new QGridLayout;
    groupBox->setLayout( groupBoxLayout );

    QList<QAbstractButton*> checkButtonList = m_selectedCurvesButtonGroup->buttons();
    for ( int i = 0; i < checkButtonList.size(); i++ )
    {
        checkButtonList[i]->setChecked( true );
        groupBoxLayout->addWidget( checkButtonList[i], i / 2, i % 2 );
    }

    m_logarithmicScaleKrAxisCheckBox = new QCheckBox( "Log Scale Kr Axis" );
    m_showUnscaledCheckBox           = new QCheckBox( "Show Unscaled" );
    m_fixedXAxisCheckBox             = new QCheckBox( "Fixed [0, 1] X-axis" );
    m_fixedLeftYAxisCheckBox         = new QCheckBox( "Fixed [0, 1] Kr-axis" );
    m_fixedXAxisCheckBox->setChecked( true );
    m_fixedLeftYAxisCheckBox->setChecked( true );

    QVBoxLayout* leftLayout = new QVBoxLayout;
    leftLayout->addWidget( groupBox );
    leftLayout->addWidget( m_logarithmicScaleKrAxisCheckBox );
    leftLayout->addWidget( m_showUnscaledCheckBox );
    leftLayout->addWidget( m_fixedXAxisCheckBox );
    leftLayout->addWidget( m_fixedLeftYAxisCheckBox );
    leftLayout->addStretch( 1 );

    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->addLayout( leftLayout );
    mainLayout->addWidget( m_qwtPlot );
    mainLayout->setContentsMargins( 5, 0, 0, 0 );

    setLayout( mainLayout );

    connect( m_selectedCurvesButtonGroup, SIGNAL( buttonClicked( int ) ), SLOT( slotButtonInButtonGroupClicked( int ) ) );
    connect( m_logarithmicScaleKrAxisCheckBox, SIGNAL( stateChanged( int ) ), SLOT( slotSomeCheckBoxStateChanged( int ) ) );
    connect( m_showUnscaledCheckBox, SIGNAL( stateChanged( int ) ), SLOT( slotSomeCheckBoxStateChanged( int ) ) );
    connect( m_fixedXAxisCheckBox, SIGNAL( stateChanged( int ) ), SLOT( slotSomeCheckBoxStateChanged( int ) ) );
    connect( m_fixedLeftYAxisCheckBox, SIGNAL( stateChanged( int ) ), SLOT( slotSomeCheckBoxStateChanged( int ) ) );

    plotUiSelectedCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuRelativePermeabilityPlotPanel::~RiuRelativePermeabilityPlotPanel()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::setPlotDefaults( QwtPlot* plot )
{
    RiuQwtPlotTools::setCommonPlotBehaviour( plot );

    {
        QwtText plotTitle = plot->title();
        QFont   titleFont = plotTitle.font();
        titleFont.setPointSize( 10 );
        plotTitle.setFont( titleFont );
        plot->setTitle( plotTitle );
    }

    plot->enableAxis( QwtPlot::xBottom, true );
    plot->enableAxis( QwtPlot::yLeft, true );
    plot->enableAxis( QwtPlot::xTop, false );
    plot->enableAxis( QwtPlot::yRight, false );

    plot->setAxisMaxMinor( QwtPlot::xBottom, 2 );
    plot->setAxisMaxMinor( QwtPlot::yLeft, 3 );

    QwtLegend* legend = new QwtLegend( plot );
    plot->insertLegend( legend, QwtPlot::BottomLegend );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::setPlotData( RiaEclipseUnitTools::UnitSystem unitSystem,
                                                    const std::vector<RigFlowDiagSolverInterface::RelPermCurve>& relPermCurves,
                                                    double                                                       swat,
                                                    double                                                       sgas,
                                                    const QString& caseName,
                                                    const QString& cellReferenceText )
{
    // cvf::Trace::show("Set RelPerm plot data");

    m_unitSystem        = unitSystem;
    m_allCurvesArr      = relPermCurves;
    m_swat              = swat;
    m_sgas              = sgas;
    m_caseName          = caseName;
    m_cellReferenceText = cellReferenceText;

    plotUiSelectedCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::clearPlot()
{
    // cvf::Trace::show("Clear RelPerm plot data");

    if ( m_allCurvesArr.empty() && m_cellReferenceText.isEmpty() )
    {
        return;
    }

    m_unitSystem = RiaEclipseUnitTools::UNITS_UNKNOWN;
    m_allCurvesArr.clear();
    m_swat = HUGE_VAL;
    m_sgas = HUGE_VAL;
    m_caseName.clear();
    m_cellReferenceText.clear();

    plotCurvesInQwt( m_unitSystem, m_allCurvesArr, m_swat, m_sgas, m_cellReferenceText, false, true, true, m_qwtPlot, &m_myPlotMarkers );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuRelativePermeabilityPlotUpdater* RiuRelativePermeabilityPlotPanel::plotUpdater()
{
    return m_plotUpdater.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::applyFontSizes( bool replot /*= true*/ )
{
    m_qwtPlot->applyFontSizes( replot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::plotUiSelectedCurves()
{
    std::vector<RigFlowDiagSolverInterface::RelPermCurve> selectedCurves = gatherUiSelectedCurves();

    const bool useLogScale = m_logarithmicScaleKrAxisCheckBox->isChecked();
    const bool fixedXAxis  = m_fixedXAxisCheckBox->isChecked();
    const bool fixedYAxis  = m_fixedLeftYAxisCheckBox->isChecked();
    plotCurvesInQwt( m_unitSystem,
                     selectedCurves,
                     m_swat,
                     m_sgas,
                     m_cellReferenceText,
                     useLogScale,
                     fixedXAxis,
                     fixedYAxis,
                     m_qwtPlot,
                     &m_myPlotMarkers );
}

//--------------------------------------------------------------------------------------------------
/// Add a transparent curve to make tooltip available on given points.
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::addTransparentCurve( QwtPlot*                       plot,
                                                            const std::vector<QPointF>&    points,
                                                            const std::vector<WhichYAxis>& axes,
                                                            bool                           logScaleLeftAxis )
{
    QwtPlotCurve* curveLeftAxis  = new QwtPlotCurve();
    QwtPlotCurve* curveRightAxis = new QwtPlotCurve();

    QVector<QPointF> pointsOnLeftAxis;
    QVector<QPointF> pointsOnRightAxis;

    // Each point is defined by either left or right axis
    CVF_ASSERT( points.size() == axes.size() );

    for ( size_t i = 0; i < points.size(); i++ )
    {
        if ( !RiaCurveDataTools::isValidValue( points[i].y(), logScaleLeftAxis ) ) continue;

        if ( axes[i] == LEFT_YAXIS )
        {
            pointsOnLeftAxis.push_back( points[i] );
        }
        else
        {
            pointsOnRightAxis.push_back( points[i] );
        }
    }

    curveLeftAxis->setSamples( pointsOnLeftAxis );
    curveRightAxis->setSamples( pointsOnRightAxis );

    curveLeftAxis->setYAxis( QwtPlot::yLeft );
    curveRightAxis->setYAxis( QwtPlot::yRight );

    curveLeftAxis->setStyle( QwtPlotCurve::NoCurve );
    curveRightAxis->setStyle( QwtPlotCurve::NoCurve );

    curveLeftAxis->setLegendAttribute( QwtPlotCurve::LegendNoAttribute );
    curveRightAxis->setLegendAttribute( QwtPlotCurve::LegendNoAttribute );

    curveLeftAxis->attach( plot );
    curveRightAxis->attach( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::plotCurvesInQwt( RiaEclipseUnitTools::UnitSystem unitSystem,
                                                        const std::vector<RigFlowDiagSolverInterface::RelPermCurve>& curveArr,
                                                        double                       swat,
                                                        double                       sgas,
                                                        QString                      cellReferenceText,
                                                        bool                         logScaleLeftAxis,
                                                        bool                         fixedXAxis,
                                                        bool                         fixedLeftYAxis,
                                                        QwtPlot*                     plot,
                                                        std::vector<QwtPlotMarker*>* myPlotMarkers )
{
    plot->detachItems( QwtPlotItem::Rtti_PlotCurve );

    // Workaround for detaching only plot markers that we have added
    // Needed as long as the curve point tracker is also using plot markers for its marking
    // plot->detachItems(QwtPlotItem::Rtti_PlotMarker);
    for ( QwtPlotMarker* marker : *myPlotMarkers )
    {
        marker->detach();
        delete marker;
    }
    myPlotMarkers->clear();

    std::vector<QPointF>    points;
    std::vector<WhichYAxis> axes;

    bool shouldEnableRightYAxis = false;

    for ( size_t i = 0; i < curveArr.size(); i++ )
    {
        const RigFlowDiagSolverInterface::RelPermCurve& curve = curveArr[i];

        // Which axis should this curve be plotted on
        WhichYAxis plotOnWhichYAxis = LEFT_YAXIS;
        if ( curve.ident == RigFlowDiagSolverInterface::RelPermCurve::PCOW ||
             curve.ident == RigFlowDiagSolverInterface::RelPermCurve::PCOG )
        {
            plotOnWhichYAxis = RIGHT_YAXIS;
        }

        // QwtPlotCurve* qwtCurve = new QwtPlotCurve(curve.name.c_str());
        RiuQwtPlotCurve* qwtCurve = new RiuQwtPlotCurve( curve.name.c_str() );

        CVF_ASSERT( curve.saturationVals.size() == curve.yVals.size() );
        // qwtCurve->setSamples(curve.xVals.data(), curve.yVals.data(), static_cast<int>(curve.xVals.size()));
        const bool includePositiveValuesOnly = ( logScaleLeftAxis && plotOnWhichYAxis == LEFT_YAXIS );
        qwtCurve->setSamplesFromXValuesAndYValues( curve.saturationVals, curve.yVals, includePositiveValuesOnly );

        qwtCurve->setTitle( curve.name.c_str() );

        qwtCurve->setStyle( QwtPlotCurve::Lines );

        Qt::PenStyle penStyle = Qt::SolidLine;
        QColor       clr      = Qt::magenta;
        switch ( curve.ident )
        {
            case RigFlowDiagSolverInterface::RelPermCurve::KRW:
                clr = QColor( 0, 0, 200 );
                break;
            case RigFlowDiagSolverInterface::RelPermCurve::KROW:
                clr = QColor( 0, 0, 200 );
                break;
            case RigFlowDiagSolverInterface::RelPermCurve::PCOW:
                clr      = QColor( 0, 130, 175 );
                penStyle = Qt::DashLine;
                break;
            case RigFlowDiagSolverInterface::RelPermCurve::KRG:
                clr = QColor( 200, 0, 0 );
                break;
            case RigFlowDiagSolverInterface::RelPermCurve::KROG:
                clr = QColor( 200, 0, 0 );
                break;
            case RigFlowDiagSolverInterface::RelPermCurve::PCOG:
                clr      = QColor( 225, 110, 0 );
                penStyle = Qt::DashLine;
                break;
        }

        const QPen curvePen( clr, 1, penStyle );
        qwtCurve->setPen( curvePen );

        QwtSymbol* curveSymbol = new QwtSymbol( QwtSymbol::Ellipse );
        curveSymbol->setSize( 6, 6 );
        curveSymbol->setPen( clr );
        curveSymbol->setBrush( Qt::NoBrush );
        qwtCurve->setSymbol( curveSymbol );

        qwtCurve->setLegendAttribute( QwtPlotCurve::LegendShowLine, true );
        qwtCurve->setLegendAttribute( QwtPlotCurve::LegendShowSymbol, true );
        qwtCurve->setLegendAttribute( QwtPlotCurve::LegendShowBrush, true );

        qwtCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );

        if ( plotOnWhichYAxis == RIGHT_YAXIS )
        {
            qwtCurve->setYAxis( QwtPlot::yRight );
            shouldEnableRightYAxis = true;
        }

        qwtCurve->attach( plot );

        // Add markers to indicate where SWAT and/or SGAS saturation intersects the respective curves
        // Note that if we're using log scale we must guard against non-positive values
        if ( swat != HUGE_VAL )
        {
            if ( curve.ident == RigFlowDiagSolverInterface::RelPermCurve::KRW ||
                 curve.ident == RigFlowDiagSolverInterface::RelPermCurve::KROW ||
                 curve.ident == RigFlowDiagSolverInterface::RelPermCurve::PCOW )
            {
                addCurveConstSaturationIntersectionMarker( curve,
                                                           swat,
                                                           Qt::blue,
                                                           plotOnWhichYAxis,
                                                           plot,
                                                           myPlotMarkers,
                                                           &points,
                                                           &axes );
            }
        }
        if ( sgas != HUGE_VAL )
        {
            if ( curve.ident == RigFlowDiagSolverInterface::RelPermCurve::KRG ||
                 curve.ident == RigFlowDiagSolverInterface::RelPermCurve::KROG ||
                 curve.ident == RigFlowDiagSolverInterface::RelPermCurve::PCOG )
            {
                addCurveConstSaturationIntersectionMarker( curve,
                                                           sgas,
                                                           Qt::red,
                                                           plotOnWhichYAxis,
                                                           plot,
                                                           myPlotMarkers,
                                                           &points,
                                                           &axes );
            }
        }
    }

    plot->enableAxis( QwtPlot::yRight, shouldEnableRightYAxis );

    addTransparentCurve( plot, points, axes, logScaleLeftAxis );

    // Add vertical marker lines to indicate cell SWAT and/or SGAS saturations
    if ( swat != HUGE_VAL )
    {
        addVerticalSaturationMarkerLine( swat, "SWAT", Qt::blue, plot, myPlotMarkers );
    }
    if ( sgas != HUGE_VAL )
    {
        addVerticalSaturationMarkerLine( sgas, "SGAS", Qt::red, plot, myPlotMarkers );
    }

    if ( logScaleLeftAxis )
    {
        if ( !dynamic_cast<QwtLogScaleEngine*>( plot->axisScaleEngine( QwtPlot::yLeft ) ) )
        {
            plot->setAxisScaleEngine( QwtPlot::yLeft, new QwtLogScaleEngine );
        }
    }
    else
    {
        if ( !dynamic_cast<QwtLinearScaleEngine*>( plot->axisScaleEngine( QwtPlot::yLeft ) ) )
        {
            plot->setAxisScaleEngine( QwtPlot::yLeft, new QwtLinearScaleEngine );
        }
    }

    if ( fixedXAxis )
    {
        plot->setAxisScale( QwtPlot::xBottom, 0.0, 1.0 );
        plot->setAxisAutoScale( QwtPlot::xBottom, false );
    }
    else
    {
        plot->setAxisAutoScale( QwtPlot::xBottom, true );
    }

    if ( fixedLeftYAxis )
    {
        if ( logScaleLeftAxis )
        {
            plot->setAxisScale( QwtPlot::yLeft, 1.0e-6, 1.0 );
        }
        else
        {
            plot->setAxisScale( QwtPlot::yLeft, 0.0, 1.0 );
        }
        plot->setAxisAutoScale( QwtPlot::yLeft, false );
    }
    else
    {
        plot->setAxisAutoScale( QwtPlot::yLeft, true );
    }

    QString titleStr = "Relative Permeability";
    if ( !cellReferenceText.isEmpty() )
    {
        titleStr += ", " + cellReferenceText;
    }
    plot->setTitle( titleStr );

    plot->setAxisTitle( QwtPlot::xBottom, determineXAxisTitleFromCurveCollection( curveArr ) );
    plot->setAxisTitle( QwtPlot::yLeft, "Kr" );
    plot->setAxisTitle( QwtPlot::yRight, QString( "Pc [%1]" ).arg( RiaEclipseUnitTools::unitStringPressure( unitSystem ) ) );
    plot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuRelativePermeabilityPlotPanel::determineXAxisTitleFromCurveCollection(
    const std::vector<RigFlowDiagSolverInterface::RelPermCurve>& curveArr )
{
    bool sawWater = false;
    bool sawGas   = false;

    for ( RigFlowDiagSolverInterface::RelPermCurve curve : curveArr )
    {
        switch ( curve.ident )
        {
            case RigFlowDiagSolverInterface::RelPermCurve::KRW:
                sawWater = true;
                break;
            case RigFlowDiagSolverInterface::RelPermCurve::KROW:
                sawWater = true;
                break;
            case RigFlowDiagSolverInterface::RelPermCurve::PCOW:
                sawWater = true;
                break;

            case RigFlowDiagSolverInterface::RelPermCurve::KRG:
                sawGas = true;
                break;
            case RigFlowDiagSolverInterface::RelPermCurve::KROG:
                sawGas = true;
                break;
            case RigFlowDiagSolverInterface::RelPermCurve::PCOG:
                sawGas = true;
                break;
        }
    }

    QString title = "";
    if ( sawWater && sawGas )
        title = "Water/Gas ";
    else if ( sawWater )
        title = "Water ";
    else if ( sawGas )
        title = "Gas ";

    title += "Saturation";

    return title;
}

//--------------------------------------------------------------------------------------------------
/// Add a vertical labeled marker line at the specified saturation value
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::addVerticalSaturationMarkerLine( double                       saturationValue,
                                                                        QString                      label,
                                                                        QColor                       color,
                                                                        QwtPlot*                     plot,
                                                                        std::vector<QwtPlotMarker*>* myPlotMarkers )
{
    QwtPlotMarker* lineMarker = new QwtPlotMarker;
    lineMarker->setXValue( saturationValue );
    lineMarker->setLineStyle( QwtPlotMarker::VLine );
    lineMarker->setLinePen( QPen( color, 1, Qt::DotLine ) );
    lineMarker->setLabel( label );
    lineMarker->setLabelAlignment( Qt::AlignTop | Qt::AlignRight );
    lineMarker->setLabelOrientation( Qt::Vertical );

    lineMarker->attach( plot );
    myPlotMarkers->push_back( lineMarker );
}

//--------------------------------------------------------------------------------------------------
/// Add a marker at the intersection of the passed curve and the constant saturation value
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::addCurveConstSaturationIntersectionMarker(
    const RigFlowDiagSolverInterface::RelPermCurve& curve,
    double                                          saturationValue,
    QColor                                          markerColor,
    WhichYAxis                                      whichYAxis,
    QwtPlot*                                        plot,
    std::vector<QwtPlotMarker*>*                    myPlotMarkers,
    std::vector<QPointF>*                           points,
    std::vector<WhichYAxis>*                        axes )
{
    const double yVal = interpolatedCurveYValue( curve.saturationVals, curve.yVals, saturationValue );
    if ( yVal != HUGE_VAL )
    {
        QwtPlotMarker* pointMarker = new QwtPlotMarker;
        pointMarker->setValue( saturationValue, yVal );

        QwtSymbol* symbol = new QwtSymbol( QwtSymbol::Ellipse );
        symbol->setSize( 13, 13 );
        symbol->setPen( QPen( markerColor, 2 ) );
        symbol->setBrush( Qt::NoBrush );
        pointMarker->setSymbol( symbol );
        pointMarker->attach( plot );

        if ( whichYAxis == RIGHT_YAXIS )
        {
            pointMarker->setYAxis( QwtPlot::yRight );
        }

        myPlotMarkers->push_back( pointMarker );
        axes->push_back( whichYAxis );
        points->push_back( QPointF( saturationValue, yVal ) );
    }
}

//--------------------------------------------------------------------------------------------------
/// Assumes that all the x-values are ordered in increasing order
//--------------------------------------------------------------------------------------------------
double RiuRelativePermeabilityPlotPanel::interpolatedCurveYValue( const std::vector<double>& xVals,
                                                                  const std::vector<double>& yVals,
                                                                  double                     x )
{
    if ( xVals.size() == 0 ) return HUGE_VAL;
    if ( x < xVals.front() ) return HUGE_VAL;
    if ( x > xVals.back() ) return HUGE_VAL;

    // Find first element greater or equal to the passed x-value
    std::vector<double>::const_iterator it = std::upper_bound( xVals.begin(), xVals.end(), x );

    // Due to checks above, we should never come up empty, but to safeguard against NaNs etc
    if ( it == xVals.end() )
    {
        return HUGE_VAL;
    }

    // Corner case - exact match on first element
    if ( it == xVals.begin() )
    {
        return yVals.front();
    }

    const size_t idx1 = it - xVals.begin();
    CVF_ASSERT( idx1 > 0 );
    const size_t idx0 = idx1 - 1;

    const double x0 = xVals[idx0];
    const double y0 = yVals[idx0];
    const double x1 = xVals[idx1];
    const double y1 = yVals[idx1];
    CVF_ASSERT( x1 > x0 );

    const double t = ( x1 - x0 ) > 0 ? ( x - x0 ) / ( x1 - x0 ) : 0;
    const double y = y0 * ( 1.0 - t ) + y1 * t;

    return y;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigFlowDiagSolverInterface::RelPermCurve> RiuRelativePermeabilityPlotPanel::gatherUiSelectedCurves() const
{
    std::vector<RigFlowDiagSolverInterface::RelPermCurve> selectedCurves;

    // Determine which curves to actually plot based on selection in GUI
    const RigFlowDiagSolverInterface::RelPermCurve::EpsMode epsModeToShow =
        m_showUnscaledCheckBox->isChecked() ? RigFlowDiagSolverInterface::RelPermCurve::EPS_OFF
                                            : RigFlowDiagSolverInterface::RelPermCurve::EPS_ON;

    for ( size_t i = 0; i < m_allCurvesArr.size(); i++ )
    {
        const RigFlowDiagSolverInterface::RelPermCurve::Ident   curveIdent   = m_allCurvesArr[i].ident;
        const RigFlowDiagSolverInterface::RelPermCurve::EpsMode curveEpsMode = m_allCurvesArr[i].epsMode;

        if ( curveEpsMode == epsModeToShow )
        {
            if ( m_selectedCurvesButtonGroup->button( curveIdent ) &&
                 m_selectedCurvesButtonGroup->button( curveIdent )->isChecked() )
            {
                selectedCurves.push_back( m_allCurvesArr[i] );
            }
        }
    }

    return selectedCurves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuRelativePermeabilityPlotPanel::asciiDataForUiSelectedCurves() const
{
    std::vector<RigFlowDiagSolverInterface::RelPermCurve> selectedCurves = gatherUiSelectedCurves();

    QString outTxt;

    // Info header
    outTxt += m_caseName + ", " + m_cellReferenceText + "\n";

    // Column headers
    for ( size_t icurve = 0; icurve < selectedCurves.size(); icurve++ )
    {
        if ( icurve > 0 ) outTxt += "\t";

        const RigFlowDiagSolverInterface::RelPermCurve& curve = selectedCurves[icurve];
        outTxt += "Saturation";
        outTxt += "\t";
        outTxt += curve.name.c_str();
    }

    // Table data
    size_t sampleIndex              = 0;
    bool   iterationContributedData = true;
    while ( iterationContributedData )
    {
        iterationContributedData = false;

        QString lineStr = "\n";

        for ( size_t icurve = 0; icurve < selectedCurves.size(); icurve++ )
        {
            if ( icurve > 0 ) lineStr += "\t";

            const RigFlowDiagSolverInterface::RelPermCurve& curve = selectedCurves[icurve];
            if ( sampleIndex < curve.saturationVals.size() && sampleIndex < curve.yVals.size() )
            {
                lineStr += QString::number( curve.saturationVals[sampleIndex], 'g', 6 );
                lineStr += "\t";
                lineStr += QString::number( curve.yVals[sampleIndex], 'g', 6 );

                iterationContributedData = true;
            }
            else
            {
                lineStr += "\t";
            }
        }

        if ( iterationContributedData )
        {
            outTxt += lineStr;
        }

        sampleIndex++;
    }

    return outTxt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::contextMenuEvent( QContextMenuEvent* event )
{
    QMenu menu;

    const int curveCount = m_qwtPlot->itemList( QwtPlotItem::Rtti_PlotCurve ).count();

    QAction* act = menu.addAction( "Show Plot Data", this, SLOT( slotCurrentPlotDataInTextDialog() ) );
    act->setEnabled( curveCount > 0 );

    menu.exec( event->globalPos() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::slotCurrentPlotDataInTextDialog()
{
    QString outTxt = asciiDataForUiSelectedCurves();

    RiuTextDialog* textDialog = new RiuTextDialog( this );
    textDialog->setMinimumSize( 400, 600 );
    textDialog->setWindowTitle( "Relative Permeability Data" );
    textDialog->setText( outTxt );
    textDialog->show();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::slotButtonInButtonGroupClicked( int )
{
    plotUiSelectedCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::slotSomeCheckBoxStateChanged( int )
{
    plotUiSelectedCurves();
}

//==================================================================================================
//
//
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuRelativePermeabilityPlotPanel::ValueRange::ValueRange()
    : min( HUGE_VAL )
    , max( -HUGE_VAL )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::ValueRange::add( const ValueRange& range )
{
    if ( range.max >= range.min )
    {
        if ( range.max > max )
        {
            max = range.max;
        }
        if ( range.min < min )
        {
            min = range.min;
        }
    }
}
