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
#include "RimSummaryPlot.h"

#include "RiuPlotCurve.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuPlotWidget.h"

#include "cafPdmUiComboBoxEditor.h"

#include "cvfAssert.h"

// NB! Special macro for pure virtual class
CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimPlotCurve, "PlotCurve" );

#define DOUBLE_INF std::numeric_limits<double>::infinity()

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

    CAF_PDM_InitField( &m_showCurve, "Show", true, "Show curve" );
    m_showCurve.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_curveName, "CurveName", "Curve Name" );
    CAF_PDM_InitFieldNoDefault( &m_customCurveName, "CurveDescription", "Custom Name" );
    m_customCurveName.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_legendEntryText, "LegendDescription", "Legend Name" );
    m_legendEntryText.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_isUsingAutoName, "AutoName", true, "Auto Name" );
    CAF_PDM_InitField( &m_showLegend, "ShowLegend", true, "Contribute To Legend" );
    CAF_PDM_InitField( &m_showErrorBars, "ShowErrorBars", true, "Show Error Bars" );

    // Obsolete field: appearance configuration is moved to RimPlotCurveAppearance
    CAF_PDM_InitField( &m_curveColor_OBSOLETE, "Color", RiaColorTools::textColor3f(), "Color" );
    m_curveColor_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitField( &m_fillColor_OBSOLETE, "FillColor", cvf::Color3f( -1.0, -1.0, -1.0 ), "Fill Color" );
    m_fillColor_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitField( &m_curveThickness_OBSOLETE, "Thickness", 1, "Line Thickness" );
    m_curveThickness_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_curveInterpolation_OBSOLETE, "CurveInterpolation", "Interpolation" );
    m_curveInterpolation_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_lineStyle_OBSOLETE, "LineStyle", "Line Style" );
    m_lineStyle_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_fillStyle_OBSOLETE, "FillStyle", "Area Fill Style" );
    m_fillStyle_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_pointSymbol_OBSOLETE, "PointSymbol", "Symbol" );
    m_pointSymbol_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitField( &m_symbolEdgeColor_OBSOLETE,
                       "SymbolEdgeColor",
                       RiaColorTools::textColor3f(),
                       "Symbol Edge Color",
                       "",
                       "",
                       "" );
    m_symbolEdgeColor_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitField( &m_symbolSkipPixelDistance_OBSOLETE,
                       "SymbolSkipPxDist",
                       0.0f,
                       "Symbol Skip Distance",
                       "",
                       "Minimum pixel distance between symbols",
                       "" );
    m_symbolSkipPixelDistance_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_symbolLabel_OBSOLETE, "SymbolLabel", "Symbol Label" );
    m_symbolLabel_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitField( &m_symbolSize_OBSOLETE, "SymbolSize", 6, "Symbol Size" );
    m_symbolSize_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_symbolLabelPosition_OBSOLETE, "SymbolLabelPosition", "Symbol Label Position" );
    m_symbolLabelPosition_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_curveAppearance, "PlotCurveAppearance", "PlotCurveAppearance" );
    m_curveAppearance = new RimPlotCurveAppearance;
    m_curveAppearance.uiCapability()->setUiTreeHidden( true );
    m_curveAppearance.uiCapability()->setUiTreeChildrenHidden( true );
    m_curveAppearance->appearanceChanged.connect( this, &RimPlotCurve::onCurveAppearanceChanged );
    m_curveAppearance->appearanceChanged.connect( this, &RimPlotCurve::onFillColorChanged );

    m_plotCurve  = nullptr;
    m_parentPlot = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotCurve::~RimPlotCurve()
{
    if ( m_plotCurve )
    {
        detach();
        delete m_plotCurve;
        m_plotCurve = nullptr;
    }

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
    refreshParentPlot();
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
void RimPlotCurve::setErrorBarsVisible( bool isVisible )
{
    m_showErrorBars = isVisible;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::initAfterRead()
{
    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2021.06.0" ) )
    {
        updateCurveAppearanceForFilesOlderThan_2021_06();
    }
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
caf::PdmFieldHandle* RimPlotCurve::userDescriptionField()
{
    return &m_curveName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setColor( const cvf::Color3f& color )
{
    m_curveAppearance->setColor( color );
    m_curveAppearance->setFillColor( color );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimPlotCurve::color() const
{
    return m_curveAppearance->color();
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

    if ( !m_legendEntryText().isEmpty() )
    {
        setTitle( m_legendEntryText );
    }
    else
    {
        setTitle( m_curveName );
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
void RimPlotCurve::appearanceUiOrdering( caf::PdmUiOrdering& uiOrdering )
{
    QString configName = "AppearanceOrdering";
    m_curveAppearance->uiOrdering( configName, uiOrdering );
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
void RimPlotCurve::updateCurveAppearanceForFilesOlderThan_2021_06()
{
    // Forward values of obsolete fields to appearance object
    m_curveAppearance->setSymbolSize( m_symbolSize_OBSOLETE );
    m_curveAppearance->setSymbolLabel( m_symbolLabel_OBSOLETE );
    m_curveAppearance->setColor( m_curveColor_OBSOLETE );
    m_curveAppearance->setLineThickness( m_curveThickness_OBSOLETE );
    m_curveAppearance->setSymbolSkipDistance( m_symbolSkipPixelDistance_OBSOLETE );
    m_curveAppearance->setSymbol( m_pointSymbol_OBSOLETE() );
    m_curveAppearance->setLineStyle( m_lineStyle_OBSOLETE() );
    m_curveAppearance->setFillStyle( m_fillStyle_OBSOLETE() );
    m_curveAppearance->setFillColor( m_fillColor_OBSOLETE );
    m_curveAppearance->setInterpolation( m_curveInterpolation_OBSOLETE() );
    m_curveAppearance->setSymbolLabelPosition( m_symbolLabelPosition_OBSOLETE() );
    m_curveAppearance->setSymbolEdgeColor( m_symbolEdgeColor_OBSOLETE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotCurve::canCurveBeAttached() const
{
    if ( !hasParentPlot() )
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
void RimPlotCurve::checkAndApplyDefaultFillColor()
{
    // if ( m_curveAppearance->fillColor().isValid() )
    // {
    //     m_curveAppearance->fillColor() = m_curveAppearance->color();
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

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
void RimPlotCurve::setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum lineStyle )
{
    m_curveAppearance->setLineStyle( lineStyle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSymbol( RiuQwtSymbol::PointSymbolEnum symbolStyle )
{
    m_curveAppearance->setSymbol( symbolStyle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setInterpolation( RiuQwtPlotCurveDefines::CurveInterpolationEnum curveInterpolation )
{
    m_curveAppearance->setInterpolation( curveInterpolation );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtSymbol::PointSymbolEnum RimPlotCurve::symbol()
{
    return m_curveAppearance->symbol();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimPlotCurve::symbolSize() const
{
    return m_curveAppearance->symbolSize();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimPlotCurve::symbolEdgeColor() const
{
    return m_curveAppearance->symbolEdgeColor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSymbolEdgeColor( const cvf::Color3f& edgeColor )
{
    m_curveAppearance->setSymbolEdgeColor( edgeColor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSymbolSkipDistance( float distance )
{
    m_curveAppearance->setSymbolSkipDistance( distance );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSymbolLabel( const QString& label )
{
    m_curveAppearance->setSymbolLabel( label );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSymbolLabelPosition( RiuQwtSymbol::LabelPosition labelPosition )
{
    m_curveAppearance->setSymbolLabelPosition( labelPosition );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSymbolSize( int sizeInPixels )
{
    m_curveAppearance->setSymbolSize( sizeInPixels );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setLineThickness( int thickness )
{
    m_curveAppearance->setLineThickness( thickness );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::resetAppearance()
{
    setColor( RiaColorTools::textColor3f() );
    setSymbolEdgeColor( RiaColorTools::textColor3f() );
    setLineThickness( 2 );
    setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID );
    setSymbol( RiuQwtSymbol::SYMBOL_NONE );
    setSymbolSkipDistance( 10 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Qt::BrushStyle RimPlotCurve::fillStyle() const
{
    return m_curveAppearance->fillStyle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setFillStyle( Qt::BrushStyle brushStyle )
{
    m_curveAppearance->setFillStyle( brushStyle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setFillColor( const cvf::Color3f& fillColor )
{
    m_curveAppearance->setFillColor( fillColor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotCurve::showInLegend() const
{
    return m_showLegend;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotCurve::errorBarsVisible() const
{
    return m_showErrorBars;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setShowInLegend( bool show )
{
    m_showLegend = show;
    updateLegendEntryVisibilityNoPlotUpdate();
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
void RimPlotCurve::onCurveAppearanceChanged( const caf::SignalEmitter* emitter )
{
    checkAndApplyDefaultFillColor();
    updateCurveAppearance();
    appearanceChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::onFillColorChanged( const caf::SignalEmitter* emitter )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateLegendsInPlot()
{
    nameChanged.send( curveName() );
    if ( m_parentPlot != nullptr )
    {
        m_parentPlot->updateLegend();
    }
}

void RimPlotCurve::setTitle( const QString& title )
{
    if ( m_plotCurve ) m_plotCurve->setTitle( title );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::refreshParentPlot()
{
    if ( m_parentPlot ) m_parentPlot->update();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::replotParentPlot()
{
    if ( m_parentPlot ) m_parentPlot->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotCurve::hasParentPlot() const
{
    return ( m_parentPlot != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSamplesFromXYValues( const std::vector<double>& xValues,
                                           const std::vector<double>& yValues,
                                           bool                       keepOnlyPositiveValues )
{
    if ( m_plotCurve )
    {
        m_plotCurve->setSamplesFromXValuesAndYValues( xValues, yValues, keepOnlyPositiveValues );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSamplesFromDatesAndYValues( const std::vector<QDateTime>& dateTimes,
                                                  const std::vector<double>&    yValues,
                                                  bool                          keepOnlyPositiveValues )
{
    if ( m_plotCurve )
    {
        m_plotCurve->setSamplesFromDatesAndYValues( dateTimes, yValues, keepOnlyPositiveValues );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSamplesFromTimeTAndYValues( const std::vector<time_t>& dateTimes,
                                                  const std::vector<double>& yValues,
                                                  bool                       keepOnlyPositiveValues )
{
    if ( m_plotCurve )
    {
        m_plotCurve->setSamplesFromTimeTAndYValues( dateTimes, yValues, keepOnlyPositiveValues );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSamplesFromXYErrorValues( const std::vector<double>&   xValues,
                                                const std::vector<double>&   yValues,
                                                const std::vector<double>&   errorValues,
                                                bool                         keepOnlyPositiveValues,
                                                RiaCurveDataTools::ErrorAxis errorAxis )
{
    if ( m_plotCurve )
    {
        m_plotCurve->setSamplesFromXYErrorValues( xValues, yValues, errorValues, keepOnlyPositiveValues, errorAxis );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateAxisInPlot( RiaDefines::PlotAxis plotAxis )
{
    if ( m_plotCurve )
    {
        // TODO: strange api?????

        // if ( plotAxis == RiaDefines::PlotAxis::PLOT_AXIS_LEFT )
        // {
        //     m_plotCurve->setYAxis( QwtPlot::yLeft );
        // }
        // else
        // {
        //     m_plotCurve->setYAxis( QwtPlot::yRight );
        // }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateLegendEntryVisibilityNoPlotUpdate()
{
    if ( !m_plotCurve ) return;

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
    // TODO: handle this for qwt
    // m_plotCurve->setItemAttribute( QwtPlotItem::Legend, showLegendInQwt );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotCurve::xValueRangeInQwt( double* minimumValue, double* maximumValue ) const
{
    CVF_ASSERT( minimumValue && maximumValue );
    CVF_ASSERT( m_plotCurve );

    // TODO: fix!!!
    // if ( m_plotCurve->data()->size() < 1 )
    // {
    //     return false;
    // }

    // *minimumValue = m_plotCurve->minXValue();
    // *maximumValue = m_plotCurve->maxXValue();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotCurve::yValueRangeInQwt( double* minimumValue, double* maximumValue ) const
{
    CVF_ASSERT( minimumValue && maximumValue );
    CVF_ASSERT( m_plotCurve );

    // TODO: fix!!!
    // if ( m_plotCurve->data()->size() < 1 )
    // {
    //     return false;
    // }

    // *minimumValue = m_plotCurve->minYValue();
    // *maximumValue = m_plotCurve->maxYValue();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setZOrder( double z )
{
    if ( m_plotCurve != nullptr )
    {
        m_plotCurve->setZ( z );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateCurveAppearance()
{
    QColor     curveColor = RiaColorTools::toQColor( m_curveAppearance->color() );
    QwtSymbol* symbol     = nullptr;

    if ( m_curveAppearance->symbol() != RiuQwtSymbol::SYMBOL_NONE )
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
        symbol = new RiuQwtSymbol( m_curveAppearance->symbol(),
                                   m_curveAppearance->symbolLabel(),
                                   m_curveAppearance->symbolLabelPosition(),
                                   legendFontSize );
        symbol->setSize( m_curveAppearance->symbolSize(), m_curveAppearance->symbolSize() );
        symbol->setColor( curveColor );

        // If the symbol is a "filled" symbol, we can have a different edge color
        // Otherwise we'll have to use the curve color.
        if ( RiuQwtSymbol::isFilledSymbol( m_curveAppearance->symbol() ) )
        {
            QColor symbolEdgeColor = RiaColorTools::toQColor( m_curveAppearance->symbolEdgeColor() );
            symbol->setPen( symbolEdgeColor );
        }
        else
        {
            symbol->setPen( curveColor );
        }
    }

    // TODO:

    // if ( m_qwtCurveErrorBars )
    // {
    //     QwtIntervalSymbol* newSymbol = new QwtIntervalSymbol( QwtIntervalSymbol::Bar );
    //     newSymbol->setPen( QPen( curveColor ) );
    //     m_qwtCurveErrorBars->setSymbol( newSymbol );
    // }

    if ( m_plotCurve )
    {
        QColor fillColor = RiaColorTools::toQColor( m_curveAppearance->fillColor() );

        fillColor = RiaColorTools::blendQColors( fillColor, QColor( Qt::white ), 3, 1 );
        QBrush fillBrush( fillColor, m_curveAppearance->fillStyle() );
        m_plotCurve->setAppearance( m_curveAppearance->lineStyle(),
                                    m_curveAppearance->interpolation(),
                                    m_curveAppearance->lineThickness(),
                                    curveColor,
                                    fillBrush );
        // m_plotCurve->setSymbol( symbol );
        // m_plotCurve->setSymbolSkipPixelDistance( m_curveAppearance->symbolSkipDistance() );

        // // Make sure the legend lines are long enough to distinguish between line types.
        // // Standard width in Qwt is 8 which is too short.
        // // Use 10 and scale this by curve thickness + add space for displaying symbol.
        // if ( m_curveAppearance->lineStyle() != RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE )
        // {
        //     QSize legendIconSize = m_plotCurve->legendIconSize();

        //     int symbolWidth = 0;
        //     if ( symbol )
        //     {
        //         symbolWidth = symbol->boundingRect().size().width() + 2;
        //     }

        //     int width = std::max( 10 * m_curveAppearance->lineThickness(), ( symbolWidth * 3 ) / 2 );

        //     legendIconSize.setWidth( width );
        //     m_plotCurve->setLegendIconSize( legendIconSize );
        // }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::clearErrorBars()
{
    m_plotCurve->clearErrorBars();

    // m_qwtCurveErrorBars->setSamples( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateUiIconFromPlotSymbol()
{
    // if ( m_curveAppearance->symbol() != RiuQwtSymbol::SYMBOL_NONE && m_qwtPlotCurve )
    // {
    //     CVF_ASSERT( RiaGuiApplication::isRunning() );
    //     QSizeF     iconSize( 24, 24 );
    //     QwtGraphic graphic = m_qwtPlotCurve->legendIcon( 0, iconSize );
    //     QPixmap    pixmap  = graphic.toPixmap();
    //     setUiIcon( caf::IconProvider( pixmap ) );
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateCurveVisibility()
{
    // if ( canCurveBeAttached() )
    // {
    //     attachCurveAndErrorBars();
    // }
    // else
    // {
    //     m_qwtPlotCurve->detach();
    //     m_qwtCurveErrorBars->detach();
    // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimPlotCurve::dataSize() const
{
    if ( m_plotCurve )
        return m_plotCurve->numSamples();
    else
        return 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setParentPlotNoReplot( RiuPlotWidget* plotWidget )
{
    CAF_ASSERT( plotWidget );

    m_parentPlot = plotWidget;
    if ( !m_plotCurve )
    {
        m_plotCurve = m_parentPlot->createPlotCurve( "", RiaColorTools::toQColor( m_curveAppearance->color() ) );
    }

    m_plotCurve->attachToPlot( plotWidget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setParentPlotAndReplot( RiuPlotWidget* plotWidget )
{
    CAF_ASSERT( plotWidget );
    setParentPlotNoReplot( plotWidget );
    plotWidget->replot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::attach( RiuPlotWidget* plotWidget )
{
    setParentPlotAndReplot( plotWidget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::detach()
{
    if ( m_plotCurve ) m_plotCurve->detach();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::reattach()
{
    if ( m_parentPlot ) m_plotCurve->attachToPlot( m_parentPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotCurve::isSameCurve( const RiuPlotCurve* plotCurve ) const
{
    return m_plotCurve == plotCurve;
}
