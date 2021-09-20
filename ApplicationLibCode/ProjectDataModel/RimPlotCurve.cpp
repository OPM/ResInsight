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
    CAF_PDM_InitField( &m_showLegend, "ShowLegend", true, "Contribute To Legend", "", "", "" );
    CAF_PDM_InitField( &m_showErrorBars, "ShowErrorBars", true, "Show Error Bars", "", "", "" );

    // Obsolete field: appearance configuration is moved to RimPlotCurveAppearance
    CAF_PDM_InitField( &m_curveColor_OBSOLETE, "Color", RiaColorTools::textColor3f(), "Color", "", "", "" );
    m_curveColor_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitField( &m_fillColor_OBSOLETE, "FillColor", cvf::Color3f( -1.0, -1.0, -1.0 ), "Fill Color", "", "", "" );
    m_fillColor_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitField( &m_curveThickness_OBSOLETE, "Thickness", 1, "Line Thickness", "", "", "" );
    m_curveThickness_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_curveInterpolation_OBSOLETE, "CurveInterpolation", "Interpolation", "", "", "" );
    m_curveInterpolation_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_lineStyle_OBSOLETE, "LineStyle", "Line Style", "", "", "" );
    m_lineStyle_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_fillStyle_OBSOLETE, "FillStyle", "Area Fill Style", "", "", "" );
    m_fillStyle_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_pointSymbol_OBSOLETE, "PointSymbol", "Symbol", "", "", "" );
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

    CAF_PDM_InitFieldNoDefault( &m_symbolLabel_OBSOLETE, "SymbolLabel", "Symbol Label", "", "", "" );
    m_symbolLabel_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitField( &m_symbolSize_OBSOLETE, "SymbolSize", 6, "Symbol Size", "", "", "" );
    m_symbolSize_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_symbolLabelPosition_OBSOLETE, "SymbolLabelPosition", "Symbol Label Position", "", "", "" );
    m_symbolLabelPosition_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_curveAppearance, "PlotCurveAppearance", "PlotCurveAppearance", "", "", "" );
    m_curveAppearance = new RimPlotCurveAppearance;
    m_curveAppearance.uiCapability()->setUiTreeHidden( true );
    m_curveAppearance.uiCapability()->setUiTreeChildrenHidden( true );
    m_curveAppearance->appearanceChanged.connect( this, &RimPlotCurve::onCurveAppearanceChanged );
    m_curveAppearance->appearanceChanged.connect( this, &RimPlotCurve::onFillColorChanged );

    m_qwtPlotCurve      = new RiuRimQwtPlotCurve( this );
    m_qwtCurveErrorBars = new QwtPlotIntervalCurve();
    m_qwtCurveErrorBars->setStyle( QwtPlotIntervalCurve::CurveStyle::NoCurve );
    m_qwtCurveErrorBars->setSymbol( new QwtIntervalSymbol( QwtIntervalSymbol::Bar ) );
    m_qwtCurveErrorBars->setItemAttribute( QwtPlotItem::Legend, false );
    m_qwtCurveErrorBars->setZ( RiuQwtPlotCurveDefines::zDepthForIndex( RiuQwtPlotCurveDefines::ZIndex::Z_ERROR_BARS ) );

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
void RimPlotCurve::setErrorBarsVisible( bool isVisible )
{
    m_showErrorBars = isVisible;
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
    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2024.10.1" ) )
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
void RimPlotCurve::updateUiIconFromPlotSymbol()
{
    if ( m_curveAppearance->symbol() != RiuQwtSymbol::SYMBOL_NONE && m_qwtPlotCurve )
    {
        CVF_ASSERT( RiaGuiApplication::isRunning() );
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
    // if ( m_curveAppearance->fillColor().isValid() )
    // {
    //     m_curveAppearance->fillColor() = m_curveAppearance->color();
    // }
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

    if ( m_qwtCurveErrorBars )
    {
        QwtIntervalSymbol* newSymbol = new QwtIntervalSymbol( QwtIntervalSymbol::Bar );
        newSymbol->setPen( QPen( curveColor ) );
        m_qwtCurveErrorBars->setSymbol( newSymbol );
    }

    if ( m_qwtPlotCurve )
    {
        QColor fillColor = RiaColorTools::toQColor( m_curveAppearance->fillColor() );

        fillColor = RiaColorTools::blendQColors( fillColor, QColor( Qt::white ), 3, 1 );
        QBrush fillBrush( fillColor, m_curveAppearance->fillStyle() );
        m_qwtPlotCurve->setAppearance( m_curveAppearance->lineStyle(),
                                       m_curveAppearance->interpolation(),
                                       m_curveAppearance->lineThickness(),
                                       curveColor,
                                       fillBrush );
        m_qwtPlotCurve->setSymbol( symbol );
        m_qwtPlotCurve->setSymbolSkipPixelDistance( m_curveAppearance->symbolSkipDistance() );

        // Make sure the legend lines are long enough to distinguish between line types.
        // Standard width in Qwt is 8 which is too short.
        // Use 10 and scale this by curve thickness + add space for displaying symbol.
        if ( m_curveAppearance->lineStyle() != RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE )
        {
            QSize legendIconSize = m_qwtPlotCurve->legendIconSize();

            int symbolWidth = 0;
            if ( symbol )
            {
                symbolWidth = symbol->boundingRect().size().width() + 2;
            }

            int width = std::max( 10 * m_curveAppearance->lineThickness(), ( symbolWidth * 3 ) / 2 );

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
