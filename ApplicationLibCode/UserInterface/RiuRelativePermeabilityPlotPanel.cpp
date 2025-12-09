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

#include "RiaCurveDataTools.h"
#include "RiaDefines.h"
#include "RiaEclipseUnitTools.h"
#include "RiaInterpolationTools.h"
#include "RiaPhaseTools.h"
#include "RiaPlotDefines.h"
#include "RiaResultNames.h"

#include "RigFlowDiagSolverInterface.h"

#include "RiuDockedQwtPlot.h"
#include "RiuGuiTheme.h"
#include "RiuPlotCurveSymbol.h"
#include "RiuQwtPlotTools.h"
#include "RiuQwtSymbol.h"
#include "RiuRelativePermeabilityPlotUpdater.h"
#include "RiuTextDialog.h"

#include "cvfAssert.h"

#include "qwt_legend.h"
#include "qwt_picker_machine.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_picker.h"
#include "qwt_scale_engine.h"
#include "qwt_symbol.h"
#include "qwt_text.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QContextMenuEvent>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <functional>

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
//
//
//
//==================================================================================================
class RiuRelPermQwtPicker : public QwtPicker
{
public:
    RiuRelPermQwtPicker( QwtPlot* plot, std::function<QString()> textProvider )
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
/// \class RiuRelativePermeabilityPlotPanel
///
///
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuRelativePermeabilityPlotPanel::RiuRelativePermeabilityPlotPanel( QWidget* parent )
    : QWidget( parent )
    , m_unitSystem( RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN )
    , m_swat( HUGE_VAL )
    , m_sgas( HUGE_VAL )
    , m_plotUpdater( new RiuRelativePermeabilityPlotUpdater( this ) )
    , m_qwtPlot( nullptr )
    , m_curveSetGroupBox( nullptr )
    , m_showDrainageCheckBox( nullptr )
    , m_showImbibitionCheckBox( nullptr )
    , m_selectedCurvesButtonGroup( nullptr )
    , m_groupBox( nullptr )
    , m_logarithmicScaleKrAxisCheckBox( nullptr )
    , m_showUnscaledCheckBox( nullptr )
    , m_showScaledCheckBox( nullptr )
    , m_fixedXAxisCheckBox( nullptr )
    , m_fixedLeftYAxisCheckBox( nullptr )
    , m_trackerPlotMarker( nullptr )

{
    m_qwtPlot = new RelPermQwtPlot( this );
    m_qwtPlot->setProperty( "qss-class", "RelPermPlot" );

    setPlotDefaults( m_qwtPlot );
    applyFontSizes( false );

    // Initialize picker for curve value tracking
    m_qwtPicker = new RiuRelPermQwtPicker( m_qwtPlot, [this]() { return m_trackerLabel; } );
    RiuGuiTheme::styleQwtItem( m_qwtPicker );
    connect( m_qwtPicker, SIGNAL( activated( bool ) ), this, SLOT( slotPickerActivated( bool ) ) );
    connect( m_qwtPicker, SIGNAL( moved( const QPoint& ) ), this, SLOT( slotPickerPointChanged( const QPoint& ) ) );

    m_selectedCurvesButtonGroup = new QButtonGroup( this );
    m_selectedCurvesButtonGroup->setExclusive( false );

    m_selectedCurvesButtonGroup->addButton( new QCheckBox( "KRW" ), RigFlowDiagDefines::RelPermCurve::KRW );
    m_selectedCurvesButtonGroup->addButton( new QCheckBox( "KRG" ), RigFlowDiagDefines::RelPermCurve::KRG );
    m_selectedCurvesButtonGroup->addButton( new QCheckBox( "KROW" ), RigFlowDiagDefines::RelPermCurve::KROW );
    m_selectedCurvesButtonGroup->addButton( new QCheckBox( "KROG" ), RigFlowDiagDefines::RelPermCurve::KROG );
    m_selectedCurvesButtonGroup->addButton( new QCheckBox( "PCOW" ), RigFlowDiagDefines::RelPermCurve::PCOW );
    m_selectedCurvesButtonGroup->addButton( new QCheckBox( "PCOG" ), RigFlowDiagDefines::RelPermCurve::PCOG );

    // Create Curve Set groupbox
    m_curveSetGroupBox          = new QGroupBox( "Curve Set" );
    QHBoxLayout* curveSetLayout = new QHBoxLayout;
    m_curveSetGroupBox->setLayout( curveSetLayout );

    m_showDrainageCheckBox   = new QCheckBox( "Drainage" );
    m_showImbibitionCheckBox = new QCheckBox( "Imbibition" );
    m_showDrainageCheckBox->setChecked( true );
    m_showImbibitionCheckBox->setChecked( false );

    curveSetLayout->addWidget( m_showDrainageCheckBox );
    curveSetLayout->addWidget( m_showImbibitionCheckBox );

    m_groupBox                  = new QGroupBox( "Curves" );
    QGridLayout* groupBoxLayout = new QGridLayout;
    m_groupBox->setLayout( groupBoxLayout );

    QList<QAbstractButton*> checkButtonList = m_selectedCurvesButtonGroup->buttons();
    for ( int i = 0; i < checkButtonList.size(); i++ )
    {
        checkButtonList[i]->setChecked( true );
        groupBoxLayout->addWidget( checkButtonList[i], i / 2, i % 2 );
    }

    m_logarithmicScaleKrAxisCheckBox = new QCheckBox( "Log Scale Kr Axis" );
    m_showUnscaledCheckBox           = new QCheckBox( "Show Unscaled" );
    m_showScaledCheckBox             = new QCheckBox( "Show Scaled" );
    m_fixedXAxisCheckBox             = new QCheckBox( "Fixed [0, 1] X-axis" );
    m_fixedLeftYAxisCheckBox         = new QCheckBox( "Fixed [0, 1] Kr-axis" );

    m_fixedXAxisCheckBox->setChecked( true );
    m_fixedLeftYAxisCheckBox->setChecked( true );
    m_showScaledCheckBox->setChecked( true );

    QCheckBox* showCurveSelection = new QCheckBox( "Show Curve Selection" );
    showCurveSelection->setCheckState( Qt::Unchecked );
    connect( showCurveSelection, SIGNAL( stateChanged( int ) ), SLOT( slotShowCurveSelectionWidgets( int ) ) );

    QVBoxLayout* leftLayout = new QVBoxLayout;
    leftLayout->addWidget( m_curveSetGroupBox );
    leftLayout->addWidget( showCurveSelection );
    leftLayout->addWidget( m_groupBox );
    leftLayout->addWidget( m_logarithmicScaleKrAxisCheckBox );
    leftLayout->addWidget( m_showScaledCheckBox );
    leftLayout->addWidget( m_showUnscaledCheckBox );
    leftLayout->addWidget( m_fixedXAxisCheckBox );
    leftLayout->addWidget( m_fixedLeftYAxisCheckBox );
    leftLayout->addStretch( 1 );

    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->addLayout( leftLayout );
    mainLayout->addWidget( m_qwtPlot );
    mainLayout->setContentsMargins( 5, 0, 0, 0 );

    setLayout( mainLayout );

    connect( m_selectedCurvesButtonGroup, SIGNAL( idClicked( int ) ), SLOT( slotButtonInButtonGroupClicked( int ) ) );
    connect( m_showDrainageCheckBox, SIGNAL( stateChanged( int ) ), SLOT( slotSomeCheckBoxStateChanged( int ) ) );
    connect( m_showImbibitionCheckBox, SIGNAL( stateChanged( int ) ), SLOT( slotSomeCheckBoxStateChanged( int ) ) );
    connect( m_logarithmicScaleKrAxisCheckBox, SIGNAL( stateChanged( int ) ), SLOT( slotSomeCheckBoxStateChanged( int ) ) );
    connect( m_showUnscaledCheckBox, SIGNAL( stateChanged( int ) ), SLOT( slotSomeCheckBoxStateChanged( int ) ) );
    connect( m_showScaledCheckBox, SIGNAL( stateChanged( int ) ), SLOT( slotSomeCheckBoxStateChanged( int ) ) );
    connect( m_fixedXAxisCheckBox, SIGNAL( stateChanged( int ) ), SLOT( slotSomeCheckBoxStateChanged( int ) ) );
    connect( m_fixedLeftYAxisCheckBox, SIGNAL( stateChanged( int ) ), SLOT( slotSomeCheckBoxStateChanged( int ) ) );

    slotShowCurveSelectionWidgets( showCurveSelection->checkState() );

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

    plot->setAxesCount( QwtAxis::XBottom, 1 );
    plot->setAxesCount( QwtAxis::YLeft, 1 );

    plot->setAxisMaxMinor( QwtAxis::XBottom, 2 );
    plot->setAxisMaxMinor( QwtAxis::YLeft, 3 );

    QwtLegend* legend = new QwtLegend( plot );
    plot->insertLegend( legend, QwtPlot::BottomLegend );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::setPlotData( RiaDefines::EclipseUnitSystem                        unitSystem,
                                                    const std::vector<RigFlowDiagDefines::RelPermCurve>& relPermCurves,
                                                    double                                               swat,
                                                    double                                               sgas,
                                                    const QString&                                       caseName,
                                                    const QString&                                       cellReferenceText,
                                                    const std::set<RiaDefines::PhaseType>&               availablePhases )
{
    // cvf::Trace::show("Set RelPerm plot data");

    m_unitSystem        = unitSystem;
    m_allCurvesArr      = relPermCurves;
    m_swat              = swat;
    m_sgas              = sgas;
    m_caseName          = caseName;
    m_cellReferenceText = cellReferenceText;
    m_availablePhases   = availablePhases;

    updateCheckboxTexts();
    plotUiSelectedCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::enableImbibitionCurveSelection( bool enable )
{
    if ( !enable )
    {
        m_showImbibitionCheckBox->setChecked( false );
        m_showImbibitionCheckBox->setDisabled( true );
    }
    else
    {
        m_showImbibitionCheckBox->setDisabled( false );
    }
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

    m_unitSystem = RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
    m_allCurvesArr.clear();
    m_swat = HUGE_VAL;
    m_sgas = HUGE_VAL;
    m_caseName.clear();
    m_cellReferenceText.clear();

    plotCurvesInQwt( m_unitSystem,
                     m_allCurvesArr,
                     m_swat,
                     m_sgas,
                     m_cellReferenceText,
                     false,
                     true,
                     true,
                     m_qwtPlot,
                     &m_myPlotMarkers,
                     false,
                     false,
                     m_availablePhases );
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
    std::vector<RigFlowDiagDefines::RelPermCurve> selectedCurves = gatherUiSelectedCurves();

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
                     &m_myPlotMarkers,
                     m_showScaledCheckBox->isChecked(),
                     m_showUnscaledCheckBox->isChecked(),
                     m_availablePhases );
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

    curveLeftAxis->setYAxis( QwtAxis::YLeft );
    curveRightAxis->setYAxis( QwtAxis::YRight );

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
void RiuRelativePermeabilityPlotPanel::plotCurvesInQwt( RiaDefines::EclipseUnitSystem                        unitSystem,
                                                        const std::vector<RigFlowDiagDefines::RelPermCurve>& curveArr,
                                                        double                                               swat,
                                                        double                                               sgas,
                                                        QString                                              cellReferenceText,
                                                        bool                                                 logScaleLeftAxis,
                                                        bool                                                 fixedXAxis,
                                                        bool                                                 fixedLeftYAxis,
                                                        QwtPlot*                                             plot,
                                                        std::vector<QwtPlotMarker*>*                         myPlotMarkers,
                                                        bool                                                 showScaled,
                                                        bool                                                 showUnscaled,
                                                        const std::set<RiaDefines::PhaseType>&               availablePhases )
{
    bool skipUnscaledLegends = showScaled && showUnscaled;

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

    const QColor waterColor = RiuGuiTheme::getColorByVariableName( "curveColorWater" );
    const QColor gasColor   = RiuGuiTheme::getColorByVariableName( "curveColorGas" );

    bool shouldEnableRightYAxis = false;

    for ( size_t i = 0; i < curveArr.size(); i++ )
    {
        const RigFlowDiagDefines::RelPermCurve& curve = curveArr[i];

        const auto unscaledCurve = ( curve.epsMode == RigFlowDiagDefines::RelPermCurve::EPS_OFF );

        // Which axis should this curve be plotted on
        WhichYAxis plotOnWhichYAxis = LEFT_YAXIS;
        if ( curve.ident == RigFlowDiagDefines::RelPermCurve::PCOW || curve.ident == RigFlowDiagDefines::RelPermCurve::PCOG )
        {
            plotOnWhichYAxis = RIGHT_YAXIS;
        }

        // Determine display name for the curve
        std::string displayName = curve.name;
        if ( curve.ident == RigFlowDiagDefines::RelPermCurve::PCOG )
        {
            std::string preferredName;
            if ( curve.curveSet == RigFlowDiagDefines::RelPermCurve::IMBIBITION )
            {
                preferredName += "I";
            }
            preferredName += RiaPhaseTools::getPreferredPcogName( availablePhases ).toStdString();

            displayName = preferredName;
        }

        QwtPlotCurve* qwtCurve = new QwtPlotCurve( displayName.c_str() );

        CVF_ASSERT( curve.saturationVals.size() == curve.yVals.size() );
        qwtCurve->setSamples( curve.saturationVals.data(), curve.yVals.data(), static_cast<int>( curve.saturationVals.size() ) );

        // Use the display name which may be modified for GAS/WATER cases
        qwtCurve->setTitle( displayName.c_str() );

        qwtCurve->setStyle( QwtPlotCurve::Lines );

        auto penStyle = Qt::SolidLine;
        if ( curve.ident == RigFlowDiagDefines::RelPermCurve::PCOW || curve.ident == RigFlowDiagDefines::RelPermCurve::PCOG )
        {
            penStyle = Qt::DashLine;
        }

        const QPen curvePen( QBrush(), 1, penStyle );
        qwtCurve->setPen( curvePen );

        auto* curveSymbol = unscaledCurve ? new RiuQwtSymbol( RiuPlotCurveSymbol::SYMBOL_CROSS )
                                          : new RiuQwtSymbol( RiuPlotCurveSymbol::SYMBOL_ELLIPSE );
        curveSymbol->setSize( 6, 6 );
        curveSymbol->setBrush( Qt::NoBrush );
        qwtCurve->setSymbol( curveSymbol );

        qwtCurve->setLegendAttribute( QwtPlotCurve::LegendShowLine, true );
        qwtCurve->setLegendAttribute( QwtPlotCurve::LegendShowSymbol, false );
        qwtCurve->setLegendAttribute( QwtPlotCurve::LegendShowBrush, true );

        const bool showLegend = !( unscaledCurve && skipUnscaledLegends );
        qwtCurve->setItemAttribute( QwtPlotItem::Legend, showLegend );

        qwtCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );

        if ( plotOnWhichYAxis == RIGHT_YAXIS )
        {
            qwtCurve->setYAxis( { QwtAxis::YRight, 0 } );
            shouldEnableRightYAxis = true;
        }

        qwtCurve->attach( plot );

        RiuGuiTheme::styleQwtItem( qwtCurve );

        // Add markers to indicate where SWAT and/or SGAS saturation intersects the respective curves
        // Note that if we're using log scale we must guard against non-positive values
        if ( swat != HUGE_VAL )
        {
            if ( curve.ident == RigFlowDiagDefines::RelPermCurve::KRW || curve.ident == RigFlowDiagDefines::RelPermCurve::KROW ||
                 curve.ident == RigFlowDiagDefines::RelPermCurve::PCOW )
            {
                addCurveConstSaturationIntersectionMarker( curve, swat, waterColor, plotOnWhichYAxis, plot, myPlotMarkers, &points, &axes );
            }
        }
        if ( sgas != HUGE_VAL )
        {
            if ( curve.ident == RigFlowDiagDefines::RelPermCurve::KRG || curve.ident == RigFlowDiagDefines::RelPermCurve::KROG ||
                 curve.ident == RigFlowDiagDefines::RelPermCurve::PCOG )
            {
                addCurveConstSaturationIntersectionMarker( curve, sgas, gasColor, plotOnWhichYAxis, plot, myPlotMarkers, &points, &axes );
            }
        }
    }

    if ( shouldEnableRightYAxis )
    {
        plot->setAxesCount( QwtAxis::YRight, 1 );
        plot->setAxisVisible( QwtAxis::YRight, true );
    }
    else
    {
        plot->setAxesCount( QwtAxis::YRight, 0 );
        plot->setAxisVisible( QwtAxis::YRight, false );
    }

    addTransparentCurve( plot, points, axes, logScaleLeftAxis );

    // Add vertical marker lines to indicate cell SWAT and/or SGAS saturations
    if ( swat != HUGE_VAL )
    {
        addVerticalSaturationMarkerLine( swat, RiaResultNames::swat(), waterColor, plot, myPlotMarkers );
    }
    if ( sgas != HUGE_VAL )
    {
        addVerticalSaturationMarkerLine( sgas, RiaResultNames::sgas(), gasColor, plot, myPlotMarkers );
    }

    if ( logScaleLeftAxis )
    {
        if ( !dynamic_cast<QwtLogScaleEngine*>( plot->axisScaleEngine( QwtAxis::YLeft ) ) )
        {
            plot->setAxisScaleEngine( QwtAxis::YLeft, new QwtLogScaleEngine );
        }
    }
    else
    {
        if ( !dynamic_cast<QwtLinearScaleEngine*>( plot->axisScaleEngine( QwtAxis::YLeft ) ) )
        {
            plot->setAxisScaleEngine( QwtAxis::YLeft, new QwtLinearScaleEngine );
        }
    }

    if ( fixedXAxis )
    {
        plot->setAxisScale( QwtAxis::XBottom, 0.0, 1.0 );
        plot->setAxisAutoScale( QwtAxis::XBottom, false );
    }
    else
    {
        plot->setAxisAutoScale( QwtAxis::XBottom, true );
    }

    if ( fixedLeftYAxis )
    {
        if ( logScaleLeftAxis )
        {
            plot->setAxisScale( QwtAxis::YLeft, 1.0e-6, 1.0 );
        }
        else
        {
            plot->setAxisScale( QwtAxis::YLeft, 0.0, 1.0 );
        }
        plot->setAxisAutoScale( QwtAxis::YLeft, false );
    }
    else
    {
        plot->setAxisAutoScale( QwtAxis::YLeft, true );
    }

    if ( showScaled )
    {
        QwtPlotCurve* curve = getLegendCurve( "Scaled", true /*scaled*/ );
        curve->attach( plot );
    }

    if ( showUnscaled )
    {
        QwtPlotCurve* curve = getLegendCurve( "Unscaled", false /*scaled*/ );
        curve->attach( plot );
    }

    QString titleStr = "Relative Permeability";
    if ( !cellReferenceText.isEmpty() )
    {
        titleStr += ", " + cellReferenceText;
    }
    plot->setTitle( titleStr );

    plot->setAxisTitle( QwtAxis::XBottom, determineXAxisTitleFromCurveCollection( curveArr ) );
    plot->setAxisTitle( QwtAxis::YLeft, "Kr" );
    plot->setAxisTitle( QwtAxis::YRight, QString( "Pc [%1]" ).arg( RiaEclipseUnitTools::unitStringPressure( unitSystem ) ) );
    plot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtPlotCurve* RiuRelativePermeabilityPlotPanel::getLegendCurve( QString title, bool scaled )
{
    QwtPlotCurve* curve = new QwtPlotCurve( title );

    curve->setTitle( title );

    curve->setStyle( QwtPlotCurve::Lines );

    const QPen curvePen( QBrush(), 1, Qt::SolidLine );
    curve->setPen( curvePen );

    auto* curveSymbol = scaled ? new RiuQwtSymbol( RiuPlotCurveSymbol::SYMBOL_ELLIPSE ) : new RiuQwtSymbol( RiuPlotCurveSymbol::SYMBOL_CROSS );
    curveSymbol->setSize( 6, 6 );
    curveSymbol->setBrush( QBrush() );
    curve->setSymbol( curveSymbol );

    curve->setLegendAttribute( QwtPlotCurve::LegendShowLine, true );
    curve->setLegendAttribute( QwtPlotCurve::LegendShowSymbol, true );
    curve->setLegendAttribute( QwtPlotCurve::LegendShowBrush, false );
    curve->setItemAttribute( QwtPlotItem::Legend, true );

    return curve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuRelativePermeabilityPlotPanel::determineXAxisTitleFromCurveCollection( const std::vector<RigFlowDiagDefines::RelPermCurve>& curveArr )
{
    bool sawWater = false;
    bool sawGas   = false;

    for ( const RigFlowDiagDefines::RelPermCurve& curve : curveArr )
    {
        if ( curve.isWaterCurve() ) sawWater = true;
        if ( curve.isGasCurve() ) sawGas = true;
    }

    QString title = "";
    if ( sawWater && sawGas )
        title = "SWAT/SGAS";
    else if ( sawWater )
        title = "SWAT";
    else if ( sawGas )
        title = "SGAS";

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
void RiuRelativePermeabilityPlotPanel::addCurveConstSaturationIntersectionMarker( const RigFlowDiagDefines::RelPermCurve& curve,
                                                                                  double                                  saturationValue,
                                                                                  QColor                                  markerColor,
                                                                                  WhichYAxis                              whichYAxis,
                                                                                  QwtPlot*                                plot,
                                                                                  std::vector<QwtPlotMarker*>*            myPlotMarkers,
                                                                                  std::vector<QPointF>*                   points,
                                                                                  std::vector<WhichYAxis>*                axes )
{
    const double yVal = RiaInterpolationTools::linear( curve.saturationVals, curve.yVals, saturationValue );
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
            pointMarker->setYAxis( QwtAxis::YRight );
        }

        myPlotMarkers->push_back( pointMarker );
        axes->push_back( whichYAxis );
        points->push_back( QPointF( saturationValue, yVal ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigFlowDiagDefines::RelPermCurve> RiuRelativePermeabilityPlotPanel::gatherUiSelectedCurves() const
{
    std::vector<RigFlowDiagDefines::RelPermCurve> selectedCurves;

    // Determine which curves to actually plot based on selection in GUI
    const bool showScaled     = m_showScaledCheckBox->isChecked();
    const bool showUnscaled   = m_showUnscaledCheckBox->isChecked();
    const bool showDrainage   = m_showDrainageCheckBox->isChecked();
    const bool showImbibition = m_showImbibitionCheckBox->isChecked();

    for ( size_t i = 0; i < m_allCurvesArr.size(); i++ )
    {
        const RigFlowDiagDefines::RelPermCurve::Ident    curveIdent   = m_allCurvesArr[i].ident;
        const RigFlowDiagDefines::RelPermCurve::EpsMode  curveEpsMode = m_allCurvesArr[i].epsMode;
        const RigFlowDiagDefines::RelPermCurve::CurveSet curveSet     = m_allCurvesArr[i].curveSet;

        // Check if curve type is selected
        const bool curveSetSelected = ( curveSet == RigFlowDiagDefines::RelPermCurve::DRAINAGE && showDrainage ) ||
                                      ( curveSet == RigFlowDiagDefines::RelPermCurve::IMBIBITION && showImbibition );

        if ( curveSetSelected && m_selectedCurvesButtonGroup->button( curveIdent ) &&
             m_selectedCurvesButtonGroup->button( curveIdent )->isChecked() )
        {
            if ( ( ( curveEpsMode == RigFlowDiagDefines::RelPermCurve::EPS_ON ) && showScaled ) ||
                 ( ( curveEpsMode == RigFlowDiagDefines::RelPermCurve::EPS_OFF ) && showUnscaled ) )
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
    std::vector<RigFlowDiagDefines::RelPermCurve> selectedCurves = gatherUiSelectedCurves();

    QString outTxt;

    // Info header
    outTxt += m_caseName + ", " + m_cellReferenceText + "\n";

    // Column headers
    for ( size_t icurve = 0; icurve < selectedCurves.size(); icurve++ )
    {
        if ( icurve > 0 ) outTxt += "\t";

        const RigFlowDiagDefines::RelPermCurve& curve = selectedCurves[icurve];

        if ( curve.isWaterCurve() )
            outTxt += "SWAT";
        else if ( curve.isGasCurve() )
            outTxt += "SGAS";
        else
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

            const RigFlowDiagDefines::RelPermCurve& curve = selectedCurves[icurve];
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
void RiuRelativePermeabilityPlotPanel::updateCheckboxTexts()
{
    // Update PCOG checkbox text to PCGW for GAS/WATER cases
    if ( auto button = m_selectedCurvesButtonGroup->button( RigFlowDiagDefines::RelPermCurve::PCOG ) )
    {
        QString preferredName = RiaPhaseTools::getPreferredPcogName( m_availablePhases );
        button->setText( preferredName );
    }
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
void RiuRelativePermeabilityPlotPanel::slotShowCurveSelectionWidgets( int state )
{
    bool setVisible = ( state != Qt::CheckState::Unchecked );

    m_groupBox->setVisible( setVisible );
    m_showUnscaledCheckBox->setVisible( setVisible );
    m_showScaledCheckBox->setVisible( setVisible );
    m_logarithmicScaleKrAxisCheckBox->setVisible( setVisible );
    m_fixedXAxisCheckBox->setVisible( setVisible );
    m_fixedLeftYAxisCheckBox->setVisible( setVisible );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::showEvent( QShowEvent* event )
{
    if ( m_plotUpdater != nullptr ) m_plotUpdater->doDelayedUpdate();
    QWidget::showEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::slotPickerPointChanged( const QPoint& pt )
{
    updateTrackerPlotMarkerAndLabelFromPicker();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::slotPickerActivated( bool on )
{
    updateTrackerPlotMarkerAndLabelFromPicker();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuRelativePermeabilityPlotPanel::updateTrackerPlotMarkerAndLabelFromPicker()
{
    bool    hasValidSamplePoint = false;
    QPointF samplePoint;
    QString curveTitle = "";

    if ( m_qwtPicker && m_qwtPicker->isActive() )
    {
        const QPoint trackerPos = m_qwtPicker->trackerPosition();

        int                 pointSampleIdx  = -1;
        const QwtPlotCurve* closestQwtCurve = closestCurveSample( trackerPos, &pointSampleIdx );
        if ( closestQwtCurve && pointSampleIdx >= 0 )
        {
            samplePoint         = closestQwtCurve->sample( pointSampleIdx );
            hasValidSamplePoint = true;
            curveTitle          = closestQwtCurve->title().text();
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

        m_trackerLabel = QString( "%1: (%2, %3)" ).arg( curveTitle ).arg( samplePoint.x(), 0, 'f', 3 ).arg( samplePoint.y(), 0, 'f', 3 );
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
const QwtPlotCurve* RiuRelativePermeabilityPlotPanel::closestCurveSample( const QPoint& cursorPosition, int* closestSampleIndex ) const
{
    if ( !m_qwtPlot ) return nullptr;

    // Get all curves currently attached to the plot
    const QwtPlotItemList& plotItems = m_qwtPlot->itemList( QwtPlotItem::Rtti_PlotCurve );

    double              minDistSquared = std::numeric_limits<double>::max();
    const QwtPlotCurve* closestCurve   = nullptr;
    int                 closestPtIdx   = -1;

    for ( auto plotItem : plotItems )
    {
        const QwtPlotCurve* candidateCurve = dynamic_cast<const QwtPlotCurve*>( plotItem );
        if ( !candidateCurve || !candidateCurve->isVisible() ) continue;

        for ( size_t ptIdx = 0; ptIdx < candidateCurve->dataSize(); ptIdx++ )
        {
            QPointF samplePoint = candidateCurve->sample( static_cast<int>( ptIdx ) );
            QPoint  screenPoint = QPoint( m_qwtPlot->transform( candidateCurve->xAxis(), samplePoint.x() ),
                                         m_qwtPlot->transform( candidateCurve->yAxis(), samplePoint.y() ) );

            double distSquared = pow( screenPoint.x() - cursorPosition.x(), 2 ) + pow( screenPoint.y() - cursorPosition.y(), 2 );

            if ( distSquared < minDistSquared )
            {
                minDistSquared = distSquared;
                closestCurve   = candidateCurve;
                closestPtIdx   = static_cast<int>( ptIdx );
            }
        }
    }

    const double maxAllowedDistanceSquared = 20.0 * 20.0; // 20 pixels
    if ( minDistSquared <= maxAllowedDistanceSquared )
    {
        if ( closestSampleIndex ) *closestSampleIndex = closestPtIdx;
        return closestCurve;
    }

    return nullptr;
}
