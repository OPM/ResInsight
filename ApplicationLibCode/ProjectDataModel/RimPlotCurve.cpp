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

#include "RimNameConfig.h"
#include "RimPlotRectAnnotation.h"
#include "RimPlotWindow.h"
#include "RimProject.h"

#include "RiuGuiTheme.h"
#include "RiuPlotCurve.h"
#include "RiuPlotCurveSymbol.h"
#include "RiuPlotWidget.h"

#include "cafAssert.h"
#include "cafPdmUiColorEditor.h"
#include "cafPdmUiTreeAttributes.h"
#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT( RimPlotCurve, "PlotCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotCurve::RimPlotCurve()
    : appearanceChanged( this )
    , visibilityChanged( this )
    , dataChanged( this )
    , nameChanged( this )
{
    CAF_PDM_InitObject( "Curve", ":/WellLogCurve16x16.png" );

    CAF_PDM_InitField( &m_showCurve, "Show", true, "Show curve" );
    m_showCurve.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_autoCheckStateBasedOnCurveData, "AutoCheckStateBasedOnCurveData", false, "Hide Curve If No Curve Data" );

    CAF_PDM_InitFieldNoDefault( &m_curveName, "CurveName", "" );

    auto templateText = QString( "%1, %2" ).arg( RiaDefines::namingVariableCase() ).arg( RiaDefines::namingVariableResultName() );
    CAF_PDM_InitField( &m_curveNameTemplateText, "TemplateText", templateText, "Template Text" );
    CAF_PDM_InitFieldNoDefault( &m_namingMethod, "CurveNamingMethod", "Curve Name" );

    CAF_PDM_InitFieldNoDefault( &m_legendEntryText, "LegendDescription", "Legend Name" );
    m_legendEntryText.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_isUsingAutoName_OBSOLETE, "AutoName", false, "Auto Name" );
    m_isUsingAutoName_OBSOLETE.xmlCapability()->setIOWritable( false );
    m_isUsingAutoName_OBSOLETE.uiCapability()->setUiHidden( true );

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

    CAF_PDM_InitField( &m_symbolEdgeColor_OBSOLETE, "SymbolEdgeColor", RiaColorTools::textColor3f(), "Symbol Edge Color" );
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
    m_curveAppearance.uiCapability()->setUiTreeChildrenHidden( true );
    m_curveAppearance->appearanceChanged.connect( this, &RimPlotCurve::onCurveAppearanceChanged );
    m_curveAppearance->fillColorChanged.connect( this, &RimPlotCurve::onFillColorChanged );

    CAF_PDM_InitFieldNoDefault( &m_additionalDataSources, "AdditionalDataSources", "Additional Data Sources" );
    m_additionalDataSources.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_additionalDataSources.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitFieldNoDefault( &m_rectAnnotations, "RectAnnotation", "Plot Rect Annotations" );

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
        m_plotCurve->detach();
        delete m_plotCurve;
        m_plotCurve = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_showCurve )
    {
        updateCurveVisibility();
        if ( m_showCurve() ) loadDataAndUpdate( false );
        visibilityChanged.send( m_showCurve() );
    }
    else if ( changedField == &m_autoCheckStateBasedOnCurveData )
    {
        updateCheckStateBasedOnCurveData();
    }
    else if ( changedField == &m_curveName )
    {
        updateCurveNameAndUpdatePlotLegendAndTitle();
    }
    else if ( changedField == &m_namingMethod )
    {
        updateCurveNameAndUpdatePlotLegendAndTitle();
    }
    else if ( changedField == &m_namingMethod || changedField == &m_curveNameTemplateText )
    {
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

    replotParentPlot();
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
    m_namingMethod = RiaDefines::ObjectNamingMethod::CUSTOM;
    m_curveName    = customName;
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
    updateCurveAppearance();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPlotCurve::createCurveNameFromTemplate( const QString& templateText )
{
    return templateText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotCurve::isCurveNameTemplateSupported() const
{
    return !supportedCurveNameVariables().isEmpty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPlotCurve::createCurveAutoName()
{
    return "Default Curve Name";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimPlotCurve::supportedCurveNameVariables() const
{
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateZoomInParentPlot()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
    if ( updateParentPlot && m_parentPlot )
    {
        m_parentPlot->scheduleReplot();
    }
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

    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2022.06.1" ) )
    {
        if ( m_isUsingAutoName_OBSOLETE() )
        {
            m_namingMethod = RiaDefines::ObjectNamingMethod::AUTO;
        }
        else
        {
            m_namingMethod = RiaDefines::ObjectNamingMethod::CUSTOM;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateCurvePresentation( bool updatePlotLegendAndTitle )
{
    updateCurveVisibility( false );

    if ( updatePlotLegendAndTitle )
    {
        updateCurveNameAndUpdatePlotLegendAndTitle();
        updatePlotTitle();
    }
    else
    {
        updateCurveNameNoLegendUpdate();
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
QList<caf::PdmOptionItemInfo> RimPlotCurve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_namingMethod )
    {
        options.push_back( caf::PdmOptionItemInfo( caf::AppEnum<RiaDefines::ObjectNamingMethod>::uiText( RiaDefines::ObjectNamingMethod::AUTO ),
                                                   RiaDefines::ObjectNamingMethod::AUTO ) );

        options.push_back( caf::PdmOptionItemInfo( caf::AppEnum<RiaDefines::ObjectNamingMethod>::uiText( RiaDefines::ObjectNamingMethod::CUSTOM ),
                                                   RiaDefines::ObjectNamingMethod::CUSTOM ) );

        if ( isCurveNameTemplateSupported() )
        {
            options.push_back(
                caf::PdmOptionItemInfo( caf::AppEnum<RiaDefines::ObjectNamingMethod>::uiText( RiaDefines::ObjectNamingMethod::TEMPLATE ),
                                        RiaDefines::ObjectNamingMethod::TEMPLATE ) );
        }
    }
    else if ( fieldNeedingOptions == &m_additionalDataSources )
    {
        // Find all plot windows above this object upwards in the object hierarchy. Use the top most plot window as the
        // root to find all plot curves.
        auto parentPlots = allAncestorsOfType<RimPlotWindow>();

        if ( !parentPlots.empty() )
        {
            auto plotCurves = parentPlots.back()->descendantsOfType<RimPlotCurve>();
            for ( auto p : plotCurves )
            {
                caf::PdmOptionItemInfo optionInfo( p->curveName(), p );

                options.push_back( optionInfo );
            }
        }
    }

    return options;
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
bool RimPlotCurve::isChecked() const
{
    return m_showCurve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setCheckState( bool isChecked )
{
    m_showCurve = isChecked;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setAutoCheckStateBasedOnCurveData( bool enable )
{
    m_autoCheckStateBasedOnCurveData = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateCheckStateBasedOnCurveData()
{
    if ( !m_autoCheckStateBasedOnCurveData ) return;

    setCheckState( isAnyCurveDataPresent() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateCurveName()
{
    if ( m_namingMethod == RiaDefines::ObjectNamingMethod::AUTO )
    {
        m_curveName = createCurveAutoName();
    }
    else if ( m_namingMethod == RiaDefines::ObjectNamingMethod::TEMPLATE )
    {
        m_curveName = createCurveNameFromTemplate( m_curveNameTemplateText );
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
void RimPlotCurve::setCurveNameTemplateText( const QString& templateText )
{
    m_curveNameTemplateText = templateText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setNamingMethod( RiaDefines::ObjectNamingMethod namingMethod )
{
    m_namingMethod = namingMethod;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateFieldUiState()
{
    m_curveName.uiCapability()->setUiReadOnly( m_namingMethod != RiaDefines::ObjectNamingMethod::CUSTOM );

    m_curveNameTemplateText.uiCapability()->setUiHidden( m_namingMethod != RiaDefines::ObjectNamingMethod::TEMPLATE );

    auto templateVariables = supportedCurveNameVariables();
    if ( !templateVariables.isEmpty() )
    {
        auto toolTipText = templateVariables.join( "," );
        m_curveNameTemplateText.uiCapability()->setUiToolTip( toolTipText );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updatePlotTitle()
{
    auto nameConfigHolder = firstAncestorOrThisOfType<RimNameConfigHolderInterface>();
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
    updateFieldUiState();

    uiOrdering.add( &m_namingMethod );
    uiOrdering.add( &m_curveNameTemplateText );
    uiOrdering.add( &m_curveName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::additionalDataSourcesUiOrdering( caf::PdmUiOrdering& uiOrdering )
{
    auto group = uiOrdering.addNewGroup( RiaDefines::additionalDataSourcesGroupName() );
    group->add( &m_additionalDataSources );
    group->setCollapsedByDefault();
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

    return true;
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
void RimPlotCurve::loadDataAndUpdate( bool updateParentPlot )
{
    onLoadDataAndUpdate( updateParentPlot );
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
void RimPlotCurve::setSymbol( RiuPlotCurveSymbol::PointSymbolEnum symbolStyle )
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
RiuPlotCurveSymbol::PointSymbolEnum RimPlotCurve::symbol()
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
void RimPlotCurve::setSymbolLabelPosition( RiuPlotCurveSymbol::LabelPosition labelPosition )
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
    setSymbol( RiuPlotCurveSymbol::SYMBOL_NONE );
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
void RimPlotCurve::setFillColorTransparency( float fillColorTransparency )
{
    m_curveAppearance->setFillColorTransparency( fillColorTransparency );
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
    if ( m_plotCurve ) m_plotCurve->setVisibleInLegend( show );

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setTitle( const QString& title )
{
    if ( m_plotCurve ) m_plotCurve->setTitle( title );
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
void RimPlotCurve::setSamplesFromXYValues( const std::vector<double>& xValues, const std::vector<double>& yValues, bool useLogarithmicScale )
{
    if ( m_plotCurve )
    {
        m_plotCurve->setSamplesFromXValuesAndYValues( xValues, yValues, useLogarithmicScale );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSamplesFromDatesAndYValues( const std::vector<QDateTime>& dateTimes,
                                                  const std::vector<double>&    yValues,
                                                  bool                          useLogarithmicScale )
{
    if ( m_plotCurve )
    {
        m_plotCurve->setSamplesFromDatesAndYValues( dateTimes, yValues, useLogarithmicScale );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSamplesFromTimeTAndYValues( const std::vector<time_t>& dateTimes, const std::vector<double>& yValues, bool useLogarithmicScale )
{
    if ( m_plotCurve )
    {
        m_plotCurve->setSamplesFromTimeTAndYValues( dateTimes, yValues, useLogarithmicScale );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimPlotCurve::computeCurveZValue()
{
    return 1.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setSamplesFromXYErrorValues( const std::vector<double>&   xValues,
                                                const std::vector<double>&   yValues,
                                                const std::vector<double>&   errorValues,
                                                bool                         useLogarithmicScale,
                                                RiaCurveDataTools::ErrorAxis errorAxis )
{
    if ( m_plotCurve )
    {
        m_plotCurve->setSamplesFromXYErrorValues( xValues, yValues, errorValues, useLogarithmicScale, errorAxis );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateYAxisInPlot( RiuPlotAxis plotAxis )
{
    if ( m_plotCurve ) m_plotCurve->setYAxis( plotAxis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateXAxisInPlot( RiuPlotAxis plotAxis )
{
    if ( m_plotCurve ) m_plotCurve->setXAxis( plotAxis );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotCurve::isAnyCurveDataPresent() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateLegendEntryVisibilityNoPlotUpdate()
{
    if ( !m_plotCurve ) return;

    m_plotCurve->setVisibleInLegend( m_showLegend() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotCurve::xValueRange( double* minimumValue, double* maximumValue ) const
{
    CAF_ASSERT( minimumValue && maximumValue );
    CAF_ASSERT( m_plotCurve );

    if ( m_plotCurve->numSamples() < 1 )
    {
        return false;
    }

    auto [min, max] = m_plotCurve->xDataRange();
    *minimumValue   = min;
    *maximumValue   = max;

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotCurve::yValueRange( double* minimumValue, double* maximumValue ) const
{
    CAF_ASSERT( minimumValue && maximumValue );
    CAF_ASSERT( m_plotCurve );

    if ( m_plotCurve->numSamples() < 1 )
    {
        return false;
    }

    auto [min, max] = m_plotCurve->yDataRange();
    *minimumValue   = min;
    *maximumValue   = max;

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
    QColor curveColor = RiaColorTools::toQColor( m_curveAppearance->color() );

    if ( !m_plotCurve ) return;

    RiuPlotCurveSymbol* symbol = nullptr;
    if ( m_curveAppearance->symbol() != RiuPlotCurveSymbol::SYMBOL_NONE )
    {
        int legendFontSize =
            caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), caf::FontTools::RelativeSize::Small );

        auto plotWindow = firstAncestorOrThisOfType<RimPlotWindow>();
        if ( plotWindow )
        {
            legendFontSize = plotWindow->legendFontSize();
        }

        // Plot curve will take ownership of the symbol
        symbol = m_plotCurve->createSymbol( m_curveAppearance->symbol() );

        if ( symbol )
        {
            symbol->setLabelPosition( m_curveAppearance->symbolLabelPosition() );
            symbol->setGlobalLabel( m_curveAppearance->symbolLabel() );
            symbol->setSize( m_curveAppearance->symbolSize(), m_curveAppearance->symbolSize() );
            symbol->setColor( curveColor );
            symbol->setLabelFontSize( legendFontSize );

            // If the symbol is a "filled" symbol, we can have a different edge color
            // Otherwise we'll have to use the curve color.
            if ( RiuPlotCurveSymbol::isFilledSymbol( m_curveAppearance->symbol() ) )
            {
                QColor symbolEdgeColor = RiaColorTools::toQColor( m_curveAppearance->symbolEdgeColor() );
                symbol->setPen( symbolEdgeColor );
            }
            else
            {
                symbol->setPen( curveColor );
            }
        }
    }

    m_plotCurve->updateErrorBarsAppearance( m_showErrorBars, curveColor );

    QColor fillColor = RiaColorTools::toQColor( m_curveAppearance->fillColor() );

    fillColor = RiaColorTools::blendQColors( fillColor, QColor( Qt::white ), 3, 1 );
    fillColor.setAlphaF( m_curveAppearance->fillColorTransparency() );

    QBrush fillBrush( fillColor, fillStyle() );
    m_plotCurve->setAppearance( m_curveAppearance->lineStyle(),
                                m_curveAppearance->interpolation(),
                                m_curveAppearance->lineThickness(),
                                curveColor,
                                fillBrush );

    m_plotCurve->setSymbolSkipPixelDistance( m_curveAppearance->symbolSkipDistance() );
    m_plotCurve->setSymbol( symbol );

    // Make sure the legend lines are long enough to distinguish between line types.
    // Standard width in Qwt is 8 which is too short.
    if ( m_curveAppearance->lineStyle() != RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE )
    {
        QSize legendIconSize = m_plotCurve->legendIconSize();

        int symbolWidth = 0;
        if ( symbol )
        {
            symbolWidth = symbol->boundingRect().size().width() + 2;
        }

        int width = std::max( 20, ( symbolWidth * 3 ) / 2 );

        legendIconSize.setWidth( width );
        m_plotCurve->setLegendIconSize( legendIconSize );
    }

    double tolerance = 0.0;
    if ( m_curveAppearance->lineStyle() != RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID )
    {
        tolerance = m_curveAppearance->curveFittingTolerance();
    }
    m_plotCurve->setCurveFittingTolerance( tolerance );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::clearErrorBars()
{
    if ( m_plotCurve ) m_plotCurve->clearErrorBars();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateUiIconFromPlotSymbol()
{
    if ( m_curveAppearance->symbol() != RiuPlotCurveSymbol::SYMBOL_NONE && m_plotCurve )
    {
        CAF_ASSERT( RiaGuiApplication::isRunning() );
        QSizeF  iconSize( 24, 24 );
        QPixmap pixmap = m_plotCurve->legendIcon( iconSize );
        setUiIcon( caf::IconProvider( pixmap ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::updateCurveVisibility( bool updateParentPlot )
{
    if ( canCurveBeAttached() )
    {
        reattach( updateParentPlot );
    }
    else
    {
        detach( updateParentPlot );
    }
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
std::pair<double, double> RimPlotCurve::sample( int index ) const
{
    CAF_ASSERT( m_plotCurve );
    CAF_ASSERT( index >= 0 && index <= dataSize() );
    return m_plotCurve->sample( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimPlotCurve::closestYValueForX( double xValue ) const
{
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotCurve*> RimPlotCurve::additionalDataSources() const
{
    return m_additionalDataSources.ptrReferencedObjectsByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotRectAnnotation*> RimPlotCurve::rectAnnotations() const
{
    return m_rectAnnotations.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::setParentPlotNoReplot( RiuPlotWidget* plotWidget )
{
    if ( !plotWidget ) return;

    m_parentPlot = plotWidget;
    if ( m_plotCurve )
    {
        m_plotCurve->attachToPlot( plotWidget );
        return;
    }

    auto color  = RiaColorTools::toQColor( m_curveAppearance->color() );
    m_plotCurve = m_parentPlot->createPlotCurve( this, m_curveName );
    m_plotCurve->updateErrorBarsAppearance( m_showErrorBars, color );

    // PERFORMANCE NOTE
    // When the z-value of a curve is changed, several update calls are made to the plot. Make sure that the default
    // z-value is correct to avoid these calls.
    m_plotCurve->setZ( computeCurveZValue() );

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
void RimPlotCurve::detach( bool deletePlotCurve )
{
    if ( m_plotCurve )
    {
        if ( deletePlotCurve )
        {
            delete m_plotCurve;
            m_plotCurve = nullptr;
        }
        else
        {
            m_plotCurve->detach();
        }
    }

    if ( m_parentPlot ) m_parentPlot->scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::reattach( bool updateParentPlot )
{
    if ( m_parentPlot && canCurveBeAttached() )
    {
        setParentPlotNoReplot( m_parentPlot );
        if ( updateParentPlot )
        {
            m_parentPlot->replot();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPlotCurve::isSameCurve( const RiuPlotCurve* plotCurve ) const
{
    return m_plotCurve == plotCurve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::deletePlotCurve()
{
    delete m_plotCurve;
    m_plotCurve = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotCurve* RimPlotCurve::plotCurve() const
{
    return m_plotCurve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPlotCurve::curveName() const
{
    return m_curveName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPlotCurve::curveExportDescription( const RifEclipseSummaryAddress& address ) const
{
    return m_curveName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( auto* treeItemAttribute = dynamic_cast<caf::PdmUiTreeViewItemAttribute*>( attribute ) )
    {
        treeItemAttribute->tags.clear();

        auto tag = caf::PdmUiTreeViewItemAttribute::createTag( RiaColorTools::toQColor( m_curveAppearance->color() ),
                                                               RiuGuiTheme::getColorByVariableName( "backgroundColor1" ),
                                                               "---" );

        tag->clicked.connect( this, &RimPlotCurve::onColorTagClicked );

        treeItemAttribute->tags.push_back( std::move( tag ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::onColorTagClicked( const SignalEmitter* emitter, size_t index )
{
    QColor sourceColor = RiaColorTools::toQColor( m_curveAppearance->color() );
    QColor newColor    = caf::PdmUiColorEditor::getColor( sourceColor );

    if ( newColor.isValid() && newColor != sourceColor )
    {
        m_curveAppearance->setColorWithFieldChanged( newColor );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup( "Appearance" );
    RimPlotCurve::appearanceUiOrdering( *appearanceGroup );
    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Curve Name" );
    nameGroup->add( &m_curveName );
    nameGroup->add( &m_showLegend );
    uiOrdering.skipRemainingFields( true );
}
