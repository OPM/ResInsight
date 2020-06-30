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

#include "RimPlotCurve.h"

#include "RiaColorTools.h"
#include "RiaCurveDataTools.h"
#include "RiaGuiApplication.h"
#include "RiaPreferences.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimNameConfig.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveCollection.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindowTools.h"
#include "RiuRimQwtPlotCurve.h"

#include "cafPdmUiComboBoxEditor.h"

#include "cvfAssert.h"

#include "qwt_date.h"
#include "qwt_interval_symbol.h"
#include "qwt_plot.h"
#include "qwt_symbol.h"

// NB! Special macro for pure virtual class
CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimPlotCurve, "PlotCurve" );

#define DOUBLE_INF std::numeric_limits<double>::infinity()

namespace caf
{
template <>
void RimPlotCurve::CurveInterpolation::setUp()
{
    addItem( RiuQwtPlotCurve::INTERPOLATION_POINT_TO_POINT, "INTERPOLATION_POINT_TO_POINT", "Point to Point" );
    addItem( RiuQwtPlotCurve::INTERPOLATION_STEP_LEFT, "INTERPOLATION_STEP_LEFT", "Step Left" );

    setDefault( RiuQwtPlotCurve::INTERPOLATION_POINT_TO_POINT );
}

template <>
void RimPlotCurve::LineStyle::setUp()
{
    addItem( RiuQwtPlotCurve::STYLE_NONE, "STYLE_NONE", "None" );
    addItem( RiuQwtPlotCurve::STYLE_SOLID, "STYLE_SOLID", "Solid" );
    addItem( RiuQwtPlotCurve::STYLE_DASH, "STYLE_DASH", "Dashes" );
    addItem( RiuQwtPlotCurve::STYLE_DOT, "STYLE_DOT", "Dots" );
    addItem( RiuQwtPlotCurve::STYLE_DASH_DOT, "STYLE_DASH_DOT", "Dashes and Dots" );

    setDefault( RiuQwtPlotCurve::STYLE_SOLID );
}

template <>
void RimPlotCurve::PointSymbol::setUp()
{
    addItem( RiuQwtSymbol::SYMBOL_NONE, "SYMBOL_NONE", "None" );
    addItem( RiuQwtSymbol::SYMBOL_ELLIPSE, "SYMBOL_ELLIPSE", "Ellipse" );
    addItem( RiuQwtSymbol::SYMBOL_RECT, "SYMBOL_RECT", "Rect" );
    addItem( RiuQwtSymbol::SYMBOL_DIAMOND, "SYMBOL_DIAMOND", "Diamond" );
    addItem( RiuQwtSymbol::SYMBOL_TRIANGLE, "SYMBOL_TRIANGLE", "Triangle" );
    addItem( RiuQwtSymbol::SYMBOL_DOWN_TRIANGLE, "SYMBOL_DOWN_TRIANGLE", "Down Triangle" );
    addItem( RiuQwtSymbol::SYMBOL_CROSS, "SYMBOL_CROSS", "Cross" );
    addItem( RiuQwtSymbol::SYMBOL_XCROSS, "SYMBOL_XCROSS", "X Cross" );
    addItem( RiuQwtSymbol::SYMBOL_STAR1, "SYMBOL_STAR1", "Star 1" );
    addItem( RiuQwtSymbol::SYMBOL_STAR2, "SYMBOL_STAR2", "Star 2" );
    addItem( RiuQwtSymbol::SYMBOL_HEXAGON, "SYMBOL_HEXAGON", "Hexagon" );
    addItem( RiuQwtSymbol::SYMBOL_LEFT_TRIANGLE, "SYMBOL_LEFT_TRIANGLE", "Left Triangle" );
    addItem( RiuQwtSymbol::SYMBOL_RIGHT_TRIANGLE, "SYMBOL_RIGHT_TRIANGLE", "Right Triangle" );
    setDefault( RiuQwtSymbol::SYMBOL_NONE );
}

template <>
void RimPlotCurve::LabelPosition::setUp()
{
    addItem( RiuQwtSymbol::LabelAboveSymbol, "LABEL_ABOVE_SYMBOL", "Label above Symbol" );
    addItem( RiuQwtSymbol::LabelBelowSymbol, "LABEL_BELOW_SYMBOL", "Label below Symbol" );
    addItem( RiuQwtSymbol::LabelLeftOfSymbol, "LABEL_LEFT_OF_SYMBOL", "Label left of Symbol" );
    addItem( RiuQwtSymbol::LabelRightOfSymbol, "LABEL_RIGHT_OF_SYMBOL", "Label right of Symbol" );
    setDefault( RiuQwtSymbol::LabelAboveSymbol );
}

template <>
void RimPlotCurve::FillStyle::setUp()
{
    addItem( Qt::NoBrush, "NO_FILL", "No Fill" );
    addItem( Qt::SolidPattern, "SOLID_FILL", "Solid Fill" );
    addItem( Qt::Dense1Pattern, "DENSE_FILL", "Dense Pattern" );
    addItem( Qt::Dense7Pattern, "SPARSE_FILL", "Sparse Pattern" );
    addItem( Qt::HorPattern, "HOR_FILL", "Horizontal Lines" );
    addItem( Qt::VerPattern, "VER_FILL", "Vertical Lines" );
    addItem( Qt::BDiagPattern, "DIAG_FILL", "Diagonal Lines" );
    addItem( Qt::CrossPattern, "CROSS_FILL", "Mesh" );
    addItem( Qt::DiagCrossPattern, "DIAG_CROSS_FILL", "Diagonal Mesh" );
}

} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotCurve::RimPlotCurve()
    : appearanceChanged( this )
    , visibilityChanged( this )
    , dataChanged( this )
    , nameChanged( this )
{
    CAF_PDM_InitObject( "Curve", ":/WellLogCurve16x16.png", "", "" );

    CAF_PDM_InitField( &m_showCurve, "Show", true, "Show curve", "", "", "" );
    m_showCurve.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_curveName, "CurveName", "Curve Name", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_customCurveName, "CurveDescription", "Custom Name", "", "", "" );
    m_customCurveName.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_legendEntryText, "LegendDescription", "Legend Name", "", "", "" );
    m_legendEntryText.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_isUsingAutoName, "AutoName", true, "Auto Name", "", "", "" );

    CAF_PDM_InitField( &m_curveColor, "Color", cvf::Color3f( cvf::Color3::BLACK ), "Color", "", "", "" );
    CAF_PDM_InitField( &m_fillColor, "FillColor", cvf::Color3f( -1.0, -1.0, -1.0 ), "Fill Color", "", "", "" );

    CAF_PDM_InitField( &m_curveThickness, "Thickness", 1, "Line Thickness", "", "", "" );
    m_curveThickness.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_curveInterpolation, "CurveInterpolation", "Interpolation", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_lineStyle, "LineStyle", "Line Style", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_fillStyle, "FillStyle", "Area Fill Style", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_pointSymbol, "PointSymbol", "Symbol", "", "", "" );
    CAF_PDM_InitField( &m_symbolEdgeColor, "SymbolEdgeColor", cvf::Color3f( cvf::Color3::BLACK ), "Symbol Edge Color", "", "", "" );

    CAF_PDM_InitField( &m_symbolSkipPixelDistance,
                       "SymbolSkipPxDist",
                       0.0f,
                       "Symbol Skip Distance",
                       "",
                       "Minimum pixel distance between symbols",
                       "" );

    CAF_PDM_InitField( &m_showLegend, "ShowLegend", true, "Contribute To Legend", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_symbolLabel, "SymbolLabel", "Symbol Label", "", "", "" );
    CAF_PDM_InitField( &m_symbolSize, "SymbolSize", 6, "Symbol Size", "", "", "" );

    CAF_PDM_InitField( &m_showErrorBars, "ShowErrorBars", true, "Show Error Bars", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_symbolLabelPosition, "SymbolLabelPosition", "Symbol Label Position", "", "", "" );

    m_qwtPlotCurve      = new RiuRimQwtPlotCurve( this );
    m_qwtCurveErrorBars = new QwtPlotIntervalCurve();
    m_qwtCurveErrorBars->setStyle( QwtPlotIntervalCurve::CurveStyle::NoCurve );
    m_qwtCurveErrorBars->setSymbol( new QwtIntervalSymbol( QwtIntervalSymbol::Bar ) );
    m_qwtCurveErrorBars->setItemAttribute( QwtPlotItem::Legend, false );
    m_qwtCurveErrorBars->setZ( RiuQwtPlotCurve::Z_ERROR_BARS );

    m_parentQwtPlot = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotCurve::~RimPlotCurve()
{
    if ( m_qwtPlotCurve )
    {
        m_qwtPlotCurve->detach();
        delete m_qwtPlotCurve;
        m_qwtPlotCurve = nullptr;
    }

    if ( m_qwtCurveErrorBars )
    {
        m_qwtCurveErrorBars->detach();
        delete m_qwtCurveErrorBars;
        m_qwtCurveErrorBars = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_showCurve )
    {
        this->updateCurveVisibility();
        if ( m_showCurve() ) loadDataAndUpdate( false );
        visibilityChanged.send( m_showCurve() );
    }
    else if ( changedField == &m_curveName )
    {
        m_customCurveName = m_curveName;
        updateCurveNameAndUpdatePlotLegendAndTitle();
    }
    else if ( &m_curveColor == changedField || &m_curveThickness == changedField || &m_pointSymbol == changedField ||
              &m_lineStyle == changedField || &m_symbolSkipPixelDistance == changedField ||
              &m_curveInterpolation == changedField || &m_symbolSize == changedField ||
              &m_symbolEdgeColor == changedField || &m_fillStyle == changedField || &m_fillColor == changedField )
    {
        if ( &m_fillStyle == changedField )
        {
            checkAndApplyDefaultFillColor();
        }

        updateCurveAppearance();

        if ( &m_pointSymbol == changedField )
        {
            m_symbolSize.uiCapability()->setUiReadOnly( m_pointSymbol() == RiuQwtSymbol::SYMBOL_NONE );
            m_symbolSkipPixelDistance.uiCapability()->setUiReadOnly( m_pointSymbol() == RiuQwtSymbol::SYMBOL_NONE );
        }
        else if ( &m_lineStyle == changedField )
        {
            m_curveThickness.uiCapability()->setUiReadOnly( m_lineStyle() == RiuQwtPlotCurve::STYLE_NONE );
            m_curveInterpolation.uiCapability()->setUiReadOnly( m_lineStyle() == RiuQwtPlotCurve::STYLE_NONE );
        }

        appearanceChanged.send();
    }
    else if ( changedField == &m_isUsingAutoName )
    {
        if ( !m_isUsingAutoName )
        {
            m_customCurveName = createCurveAutoName();
        }

        updateCurveNameAndUpdatePlotLegendAndTitle();
        nameChanged.send( curveName() );
    }
    else if ( changedField == &m_showLegend )
    {
        updateLegendEntryVisibilityAndPlotLegend();
    }
    else if ( changedField == &m_showErrorBars )
    {
        updateCurveAppearance();
    }
    RiuPlotMainWindowTools::refreshToolbars();
    if ( m_parentQwtPlot ) m_parentQwtPlot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimPlotCurve::objectToggleField()
{
    return &m_showCurve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setCustomName( const QString& customName )
{
    m_isUsingAutoName = false;
    m_customCurveName = customName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPlotCurve::legendEntryText() const
{
    if ( !m_legendEntryText().isEmpty() )
    {
        return m_legendEntryText;
    }
    return m_customCurveName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setLegendEntryText( const QString& legendEntryText )
{
    m_legendEntryText = legendEntryText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateCurveVisibility()
{
    if ( canCurveBeAttached() )
    {
        attachCurveAndErrorBars();
    }
    else
    {
        m_qwtPlotCurve->detach();
        m_qwtCurveErrorBars->detach();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::initAfterRead()
{
    m_symbolSize.uiCapability()->setUiReadOnly( m_pointSymbol() == RiuQwtSymbol::SYMBOL_NONE );
    m_symbolSkipPixelDistance.uiCapability()->setUiReadOnly( m_pointSymbol() == RiuQwtSymbol::SYMBOL_NONE );
    m_curveThickness.uiCapability()->setUiReadOnly( m_lineStyle() == RiuQwtPlotCurve::STYLE_NONE );
    m_curveInterpolation.uiCapability()->setUiReadOnly( m_lineStyle() == RiuQwtPlotCurve::STYLE_NONE );

    checkAndApplyDefaultFillColor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateCurvePresentation( bool updatePlotLegendAndTitle )
{
    this->updateCurveVisibility();

    if ( updatePlotLegendAndTitle )
    {
        this->updateCurveNameAndUpdatePlotLegendAndTitle();
        this->updatePlotTitle();
    }
    else
    {
        this->updateCurveNameNoLegendUpdate();
    }

    updateCurveAppearance();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setParentQwtPlotAndReplot( QwtPlot* plot )
{
    m_parentQwtPlot = plot;
    if ( canCurveBeAttached() )
    {
        attachCurveAndErrorBars();

        m_parentQwtPlot->replot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setParentQwtPlotNoReplot( QwtPlot* plot )
{
    m_parentQwtPlot = plot;
    if ( canCurveBeAttached() )
    {
        attachCurveAndErrorBars();
    }
    else
    {
        m_qwtPlotCurve->detach();
        m_qwtCurveErrorBars->detach();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimPlotCurve::userDescriptionField()
{
    return &m_curveName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setColor( const cvf::Color3f& color )
{
    m_curveColor = color;
    m_fillColor  = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::detachQwtCurve()
{
    m_qwtPlotCurve->detach();
    m_qwtCurveErrorBars->detach();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::reattachQwtCurve()
{
    detachQwtCurve();
    if ( canCurveBeAttached() )
    {
        attachCurveAndErrorBars();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtPlotCurve* RimPlotCurve::qwtPlotCurve() const
{
    return m_qwtPlotCurve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotCurve::isCurveVisible() const
{
    return m_showCurve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setCurveVisibility( bool visible )
{
    m_showCurve = visible;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateCurveName()
{
    if ( m_isUsingAutoName )
    {
        m_curveName = this->createCurveAutoName();
    }
    else
    {
        m_curveName = m_customCurveName;
    }

    if ( m_qwtPlotCurve )
    {
        if ( !m_legendEntryText().isEmpty() )
        {
            m_qwtPlotCurve->setTitle( m_legendEntryText );
        }
        else
        {
            m_qwtPlotCurve->setTitle( m_curveName );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateCurveNameAndUpdatePlotLegendAndTitle()
{
    updateCurveName();
    updateLegendEntryVisibilityAndPlotLegend();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateCurveNameNoLegendUpdate()
{
    updateCurveName();
    updateLegendEntryVisibilityNoPlotUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateOptionSensitivity()
{
    m_curveName.uiCapability()->setUiReadOnly( m_isUsingAutoName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updatePlotTitle()
{
    RimNameConfigHolderInterface* nameConfigHolder = nullptr;
    this->firstAncestorOrThisOfType( nameConfigHolder );
    if ( nameConfigHolder )
    {
        nameConfigHolder->updateAutoName();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateLegendsInPlot()
{
    nameChanged.send( curveName() );
    if ( m_parentQwtPlot != nullptr )
    {
        m_parentQwtPlot->updateLegend();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSamplesFromXYErrorValues(
    const std::vector<double>&   xValues,
    const std::vector<double>&   yValues,
    const std::vector<double>&   errorValues,
    bool                         keepOnlyPositiveValues,
    RiaCurveDataTools::ErrorAxis errorAxis /*= RiuQwtPlotCurve::ERROR_ALONG_Y_AXIS */ )
{
    CVF_ASSERT( xValues.size() == yValues.size() );
    CVF_ASSERT( xValues.size() == errorValues.size() );

    auto intervalsOfValidValues = RiaCurveDataTools::calculateIntervalsOfValidValues( yValues, keepOnlyPositiveValues );
    std::vector<double> filteredYValues;
    std::vector<double> filteredXValues;

    RiaCurveDataTools::getValuesByIntervals( yValues, intervalsOfValidValues, &filteredYValues );
    RiaCurveDataTools::getValuesByIntervals( xValues, intervalsOfValidValues, &filteredXValues );

    std::vector<double> filteredErrorValues;
    RiaCurveDataTools::getValuesByIntervals( errorValues, intervalsOfValidValues, &filteredErrorValues );

    QVector<QwtIntervalSample> errorIntervals;

    errorIntervals.reserve( static_cast<int>( filteredXValues.size() ) );

    for ( size_t i = 0; i < filteredXValues.size(); i++ )
    {
        if ( filteredYValues[i] != DOUBLE_INF && filteredErrorValues[i] != DOUBLE_INF )
        {
            if ( errorAxis == RiaCurveDataTools::ErrorAxis::ERROR_ALONG_Y_AXIS )
            {
                errorIntervals << QwtIntervalSample( filteredXValues[i],
                                                     filteredYValues[i] - filteredErrorValues[i],
                                                     filteredYValues[i] + filteredErrorValues[i] );
            }
            else
            {
                errorIntervals << QwtIntervalSample( filteredYValues[i],
                                                     filteredXValues[i] - filteredErrorValues[i],
                                                     filteredXValues[i] + filteredErrorValues[i] );
            }
        }
    }

    if ( m_qwtPlotCurve )
    {
        m_qwtPlotCurve->setSamples( filteredXValues.data(),
                                    filteredYValues.data(),
                                    static_cast<int>( filteredXValues.size() ) );

        m_qwtPlotCurve->setLineSegmentStartStopIndices( intervalsOfValidValues );
    }

    if ( m_qwtCurveErrorBars )
    {
        m_qwtCurveErrorBars->setSamples( errorIntervals );
        if ( errorAxis == RiaCurveDataTools::ErrorAxis::ERROR_ALONG_Y_AXIS )
        {
            m_qwtCurveErrorBars->setOrientation( Qt::Vertical );
        }
        else
        {
            m_qwtCurveErrorBars->setOrientation( Qt::Horizontal );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSamplesFromXYValues( const std::vector<double>& xValues,
                                           const std::vector<double>& yValues,
                                           bool                       keepOnlyPositiveValues )
{
    if ( m_qwtPlotCurve )
    {
        m_qwtPlotCurve->setSamplesFromXValuesAndYValues( xValues, yValues, keepOnlyPositiveValues );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSamplesFromDatesAndYValues( const std::vector<QDateTime>& dateTimes,
                                                  const std::vector<double>&    yValues,
                                                  bool                          keepOnlyPositiveValues )
{
    if ( m_qwtPlotCurve )
    {
        m_qwtPlotCurve->setSamplesFromDatesAndYValues( dateTimes, yValues, keepOnlyPositiveValues );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSamplesFromTimeTAndYValues( const std::vector<time_t>& dateTimes,
                                                  const std::vector<double>& yValues,
                                                  bool                       keepOnlyPositiveValues )
{
    if ( m_qwtPlotCurve )
    {
        m_qwtPlotCurve->setSamplesFromTimeTAndYValues( dateTimes, yValues, keepOnlyPositiveValues );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::appearanceUiOrdering( caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_curveColor );
    uiOrdering.add( &m_pointSymbol );
    if ( RiuQwtSymbol::isFilledSymbol( m_pointSymbol() ) )
    {
        uiOrdering.add( &m_symbolEdgeColor );
    }
    uiOrdering.add( &m_symbolSize );
    uiOrdering.add( &m_symbolSkipPixelDistance );
    uiOrdering.add( &m_lineStyle );
    uiOrdering.add( &m_fillStyle );
    if ( m_fillStyle != Qt::BrushStyle::NoBrush )
    {
        uiOrdering.add( &m_fillColor );
    }
    uiOrdering.add( &m_curveThickness );
    uiOrdering.add( &m_curveInterpolation );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::curveNameUiOrdering( caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_isUsingAutoName );
    uiOrdering.add( &m_curveName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateUiIconFromPlotSymbol()
{
    if ( m_pointSymbol() != RiuQwtSymbol::NoSymbol && m_qwtPlotCurve )
    {
        CVF_ASSERT( RiaGuiApplication::isRunning() );
        QColor curveColor( m_curveColor.value().rByte(), m_curveColor.value().gByte(), m_curveColor.value().bByte() );

        QSizeF     iconSize( 24, 24 );
        QwtGraphic graphic = m_qwtPlotCurve->legendIcon( 0, iconSize );
        QPixmap    pixmap  = graphic.toPixmap();
        setUiIcon( caf::IconProvider( pixmap ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotCurve::canCurveBeAttached() const
{
    if ( !m_parentQwtPlot )
    {
        return false;
    }

    if ( !m_showCurve() )
    {
        return false;
    }

    bool isVisibleInPossibleParent = true;

    {
        RimSummaryCurveCollection* summaryCurveCollection = nullptr;
        this->firstAncestorOrThisOfType( summaryCurveCollection );
        if ( summaryCurveCollection ) isVisibleInPossibleParent = summaryCurveCollection->isCurvesVisible();

        RimEnsembleCurveSet* ensembleCurveSet = nullptr;
        firstAncestorOrThisOfType( ensembleCurveSet );
        if ( ensembleCurveSet ) isVisibleInPossibleParent = ensembleCurveSet->isCurvesVisible();
    }

    return isVisibleInPossibleParent;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::attachCurveAndErrorBars()
{
    m_qwtPlotCurve->attach( m_parentQwtPlot );

    if ( m_showErrorBars )
    {
        m_qwtCurveErrorBars->attach( m_parentQwtPlot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::checkAndApplyDefaultFillColor()
{
    if ( !m_fillColor().isValid() )
    {
        m_fillColor = m_curveColor;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateCurveAppearance()
{
    QColor     curveColor( m_curveColor.value().rByte(), m_curveColor.value().gByte(), m_curveColor.value().bByte() );
    QwtSymbol* symbol = nullptr;

    if ( m_pointSymbol() != RiuQwtSymbol::SYMBOL_NONE )
    {
        int legendFontSize        = caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(),
                                                                caf::FontTools::RelativeSize::Small );
        RimPlotWindow* plotWindow = nullptr;
        this->firstAncestorOrThisOfType( plotWindow );
        if ( plotWindow )
        {
            legendFontSize = plotWindow->legendFontSize();
        }

        // QwtPlotCurve will take ownership of the symbol
        symbol = new RiuQwtSymbol( m_pointSymbol(), m_symbolLabel(), m_symbolLabelPosition(), legendFontSize );
        symbol->setSize( m_symbolSize, m_symbolSize );
        symbol->setColor( curveColor );

        // If the symbol is a "filled" symbol, we can have a different edge color
        // Otherwise we'll have to use the curve color.
        if ( RiuQwtSymbol::isFilledSymbol( m_pointSymbol() ) )
        {
            QColor symbolEdgeColor( m_symbolEdgeColor.value().rByte(),
                                    m_symbolEdgeColor.value().gByte(),
                                    m_symbolEdgeColor.value().bByte() );

            symbol->setPen( symbolEdgeColor );
        }
        else
        {
            symbol->setPen( curveColor );
        }
    }

    if ( m_qwtCurveErrorBars )
    {
        QwtIntervalSymbol* newSymbol = new QwtIntervalSymbol( QwtIntervalSymbol::Bar );
        newSymbol->setPen( QPen( curveColor ) );
        m_qwtCurveErrorBars->setSymbol( newSymbol );
    }

    if ( m_qwtPlotCurve )
    {
        QColor fillColor( m_fillColor.value().rByte(), m_fillColor.value().gByte(), m_fillColor.value().bByte() );
        fillColor = RiaColorTools::blendQColors( fillColor, QColor( Qt::white ), 3, 1 );
        QBrush fillBrush( fillColor, m_fillStyle() );
        m_qwtPlotCurve->setAppearance( m_lineStyle(), m_curveInterpolation(), m_curveThickness(), curveColor, fillBrush );
        m_qwtPlotCurve->setSymbol( symbol );
        m_qwtPlotCurve->setSymbolSkipPixelDistance( m_symbolSkipPixelDistance() );

        // Make sure the legend lines are long enough to distinguish between line types.
        // Standard width in Qwt is 8 which is too short.
        // Use 10 and scale this by curve thickness + add space for displaying symbol.
        if ( m_lineStyle() != RiuQwtPlotCurve::STYLE_NONE )
        {
            QSize legendIconSize = m_qwtPlotCurve->legendIconSize();

            int symbolWidth = 0;
            if ( symbol )
            {
                symbolWidth = symbol->boundingRect().size().width() + 2;
            }

            int width = std::max( 10 * m_curveThickness, ( symbolWidth * 3 ) / 2 );

            legendIconSize.setWidth( width );
            m_qwtPlotCurve->setLegendIconSize( legendIconSize );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotCurve::isCrossPlotCurve() const
{
    RimSummaryCrossPlot* crossPlot = nullptr;
    this->firstAncestorOrThisOfType( crossPlot );
    if ( crossPlot ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimPlotCurve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                   bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_curveThickness )
    {
        for ( size_t i = 0; i < 10; i++ )
        {
            options.push_back( caf::PdmOptionItemInfo( QString::number( i + 1 ), QVariant::fromValue( i + 1 ) ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::loadDataAndUpdate( bool updateParentPlot )
{
    this->onLoadDataAndUpdate( updateParentPlot );
    if ( updateParentPlot )
    {
        dataChanged.send();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotCurve::xValueRangeInQwt( double* minimumValue, double* maximumValue ) const
{
    CVF_ASSERT( minimumValue && maximumValue );
    CVF_ASSERT( m_qwtPlotCurve );

    if ( m_qwtPlotCurve->data()->size() < 1 )
    {
        return false;
    }

    *minimumValue = m_qwtPlotCurve->minXValue();
    *maximumValue = m_qwtPlotCurve->maxXValue();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotCurve::yValueRangeInQwt( double* minimumValue, double* maximumValue ) const
{
    CVF_ASSERT( minimumValue && maximumValue );
    CVF_ASSERT( m_qwtPlotCurve );

    if ( m_qwtPlotCurve->data()->size() < 1 )
    {
        return false;
    }

    *minimumValue = m_qwtPlotCurve->minYValue();
    *maximumValue = m_qwtPlotCurve->maxYValue();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setLineStyle( RiuQwtPlotCurve::LineStyleEnum lineStyle )
{
    m_lineStyle = lineStyle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSymbol( RiuQwtSymbol::PointSymbolEnum symbolStyle )
{
    m_pointSymbol = symbolStyle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setInterpolation( RiuQwtPlotCurve::CurveInterpolationEnum curveInterpolation )
{
    m_curveInterpolation = curveInterpolation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtSymbol::PointSymbolEnum RimPlotCurve::symbol()
{
    return m_pointSymbol();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimPlotCurve::symbolSize() const
{
    return m_symbolSize();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimPlotCurve::symbolEdgeColor() const
{
    return m_symbolEdgeColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSymbolEdgeColor( const cvf::Color3f& edgeColor )
{
    m_symbolEdgeColor = edgeColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSymbolSkipDistance( float distance )
{
    m_symbolSkipPixelDistance = distance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSymbolLabel( const QString& label )
{
    m_symbolLabel = label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSymbolLabelPosition( RiuQwtSymbol::LabelPosition labelPosition )
{
    m_symbolLabelPosition = labelPosition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSymbolSize( int sizeInPixels )
{
    m_symbolSize = sizeInPixels;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setLineThickness( int thickness )
{
    m_curveThickness = thickness;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::resetAppearance()
{
    setColor( cvf::Color3f( cvf::Color3::BLACK ) );
    setSymbolEdgeColor( cvf::Color3f( cvf::Color3::BLACK ) );
    setLineThickness( 2 );
    setLineStyle( RiuQwtPlotCurve::STYLE_SOLID );
    setSymbol( RiuQwtSymbol::SYMBOL_NONE );
    setSymbolSkipDistance( 10 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Qt::BrushStyle RimPlotCurve::fillStyle() const
{
    return m_fillStyle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setFillStyle( Qt::BrushStyle brushStyle )
{
    m_fillStyle = brushStyle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::showLegend( bool show )
{
    m_showLegend = show;
    updateLegendEntryVisibilityNoPlotUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setZOrder( double z )
{
    if ( m_qwtPlotCurve != nullptr )
    {
        m_qwtPlotCurve->setZ( z );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateLegendEntryVisibilityAndPlotLegend()
{
    updateLegendEntryVisibilityNoPlotUpdate();
    updateLegendsInPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateLegendEntryVisibilityNoPlotUpdate()
{
    if ( !m_qwtPlotCurve ) return;

    RimEnsembleCurveSet* ensembleCurveSet = nullptr;
    this->firstAncestorOrThisOfType( ensembleCurveSet );
    if ( ensembleCurveSet )
    {
        return;
    }

    bool showLegendInQwt = m_showLegend();

    RimSummaryPlot* summaryPlot = nullptr;
    this->firstAncestorOrThisOfType( summaryPlot );
    if ( summaryPlot )
    {
        bool anyCalculated = false;
        for ( const auto c : summaryPlot->summaryCurves() )
        {
            if ( c->summaryAddressY().category() == RifEclipseSummaryAddress::SUMMARY_CALCULATED )
            {
                // Never hide the legend for calculated curves, as the curve legend is used to
                // show some essential auto generated data
                anyCalculated = true;
            }
        }

        if ( !anyCalculated && summaryPlot->ensembleCurveSetCollection()->curveSets().empty() &&
             summaryPlot->curveCount() == 1 )
        {
            // Disable display of legend if the summary plot has only one single curve
            showLegendInQwt = false;
        }
    }
    m_qwtPlotCurve->setItemAttribute( QwtPlotItem::Legend, showLegendInQwt );
}
