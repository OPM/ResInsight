/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RimCurveSetAppearance.h"

#include "RiaColorTools.h"

#include "RimSummaryEnsemble.h"

#include "RiuGuiTheme.h"

#include "cafPdmFieldCvfColor.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_SOURCE_INIT( RimCurveSetAppearance, "RimCurveSetAppearance" );

namespace internal
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> parametersWithVariation( RimSummaryEnsemble* ensemble )
{
    if ( !ensemble ) return {};

    auto parameters = ensemble->variationSortedEnsembleParameters( true );

    std::vector<QString> names;
    for ( const auto& param : parameters )
    {
        names.emplace_back( param.name );
    }
    return names;
}

} // namespace internal

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCurveSetAppearance::RimCurveSetAppearance()
    : appearanceChanged( this )
{
    CAF_PDM_InitObject( "Curve Set Apperance" );

    CAF_PDM_InitField( &m_colorMode, "ColorMode", caf::AppEnum<ColorMode>( ColorMode::SINGLE_COLOR_WITH_ALPHA ), "Coloring Mode" );

    CAF_PDM_InitField( &m_colorForRealizations, "Color", RiaColorTools::textColor3f(), "Color" );
    CAF_PDM_InitField( &m_mainEnsembleColor, "MainEnsembleColor", RiaColorTools::textColor3f(), "Color" );
    CAF_PDM_InitField( &m_colorTransparency, "ColorTransparency", 0.3, "Transparency" );
    m_colorTransparency.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_ensembleParameter, "EnsembleParameter", QString( "" ), "Parameter" );
    m_ensembleParameter.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_ensembleParameterSorting, "EnsembleParameterSorting", "Parameter Sorting" );

    auto defaultLineStyle   = LineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE );
    auto defaultPointSymbol = PointSymbol( RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_CROSS );

    CAF_PDM_InitFieldNoDefault( &m_useCustomAppearance, "UseCustomAppearance", "Appearance" );
    CAF_PDM_InitField( &m_lineStyle, "LineStyle", defaultLineStyle, "Line Style" );
    CAF_PDM_InitField( &m_pointSymbol, "PointSymbol", defaultPointSymbol, "Symbol" );
    CAF_PDM_InitField( &m_symbolSize, "SymbolSize", 6, "Symbol Size" );

    auto defaultStatisticsPointSymbol = PointSymbol( RiuPlotCurveSymbol::PointSymbolEnum::SYMBOL_ELLIPSE );
    CAF_PDM_InitFieldNoDefault( &m_statisticsUseCustomAppearance, "StatisticsUseCustomAppearance", "Appearance" );
    CAF_PDM_InitField( &m_statisticsLineStyle, "StatisticsLineStyle", defaultLineStyle, "Line Style" );
    CAF_PDM_InitField( &m_statisticsPointSymbol, "StatisticsPointSymbol", defaultStatisticsPointSymbol, "Symbol" );
    CAF_PDM_InitField( &m_statisticsSymbolSize, "StatisticsSymbolSize", 6, "Symbol Size" );

    CAF_PDM_InitFieldNoDefault( &m_ensemble, "SummaryEnsemble", "Ensemble" );

    CAF_PDM_InitFieldNoDefault( &m_ensembleLegendConfig, "LegendConfig", "" );
    m_ensembleLegendConfig = new RimRegularLegendConfig();
    m_ensembleLegendConfig->setColorLegend(
        RimRegularLegendConfig::mapToColorLegend( RimEnsembleCurveSetColorManager::DEFAULT_ENSEMBLE_COLOR_RANGE ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimCurveSetAppearance::curveColor( const RimSummaryEnsemble* ensemble, const RimSummaryCase* summaryCase ) const
{
    if ( m_colorMode() == RimEnsembleCurveSetColorManager::ColorMode::BY_ENSEMBLE_PARAM )
    {
        if ( !ensemble || !summaryCase ) return cvf::Color3f( 0.0, 0.0, 0.0 );
        auto ensembleParam = ensemble->ensembleParameter( m_ensembleParameter() );
        return RimEnsembleCurveSetColorManager::caseColor( m_ensembleLegendConfig, summaryCase, ensembleParam );
    }

    return m_colorForRealizations();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimCurveSetAppearance::statisticsCurveColor() const
{
    return m_mainEnsembleColor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimCurveSetAppearance::legendConfig() const
{
    if ( m_colorMode() == RimEnsembleCurveSetColorManager::ColorMode::SINGLE_COLOR ||
         m_colorMode() == RimEnsembleCurveSetColorManager::ColorMode::SINGLE_COLOR_WITH_ALPHA )
    {
        return nullptr;
    }

    if ( !m_ensembleLegendConfig->showLegend() ) return nullptr;

    updateEnsembleParameterLegend( m_ensembleLegendConfig );

    return m_ensembleLegendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveSetAppearance::setMainEnsembleColor( const cvf::Color3f& color )
{
    m_mainEnsembleColor = color;

    updateRealizationColor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveSetAppearance::setEnsemble( RimSummaryEnsemble* ensemble )
{
    m_ensemble = ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveSetAppearance::setShowLineStyleAndSymbols( bool showLineStyleAndSymbols )
{
    m_showLineStyleAndSymbols = showLineStyleAndSymbols;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveSetAppearance::updateEnsembleParameterLegend( RimRegularLegendConfig* legendConfig ) const
{
    if ( !m_ensemble || !legendConfig ) return;

    auto ensembleParam = m_ensemble->ensembleParameter( m_ensembleParameter );
    legendConfig->setTitle( m_ensemble->name() + "\n" + m_ensembleParameter );
    RimEnsembleCurveSetColorManager::initializeLegendConfig( legendConfig, ensembleParam );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveSetAppearance::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_colorMode || changedField == &m_colorTransparency || changedField == &m_mainEnsembleColor )
    {
        updateRealizationColor();
    }

    if ( changedField == &m_colorMode || changedField == &m_ensembleParameter || changedField == &m_ensembleParameterSorting )
    {
        updateEnsembleParameterLegend( m_ensembleLegendConfig );
    }

    appearanceChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveSetAppearance::updateRealizationColor()
{
    if ( m_colorMode() == RimEnsembleCurveSetColorManager::ColorMode::SINGLE_COLOR_WITH_ALPHA )
    {
        auto backgroundColor = RiuGuiTheme::getColorByVariableName( "backgroundColor1" );

        auto sourceColor      = RiaColorTools::toQColor( m_mainEnsembleColor );
        auto sourceWeight     = 100;
        int  backgroundWeight = std::max( 1, static_cast<int>( sourceWeight * 10 * m_colorTransparency ) );
        auto blendedColor     = RiaColorTools::blendQColors( backgroundColor, sourceColor, backgroundWeight, sourceWeight );

        m_colorForRealizations = RiaColorTools::fromQColorTo3f( blendedColor );
    }
    else
    {
        m_colorForRealizations = m_mainEnsembleColor();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveSetAppearance::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_colorMode );

    if ( m_colorMode() == RimEnsembleCurveSetColorManager::ColorMode::SINGLE_COLOR )
    {
        uiOrdering.add( &m_mainEnsembleColor );
    }
    else if ( m_colorMode() == RimEnsembleCurveSetColorManager::ColorMode::SINGLE_COLOR_WITH_ALPHA )
    {
        uiOrdering.add( &m_mainEnsembleColor );
        uiOrdering.add( &m_colorTransparency );
    }
    else if ( m_colorMode == ColorMode::BY_ENSEMBLE_PARAM )
    {
        uiOrdering.add( &m_ensembleParameterSorting );
        uiOrdering.add( &m_ensembleParameter );
    }

    if ( m_showLineStyleAndSymbols )
    {
        uiOrdering.add( &m_useCustomAppearance );
        if ( m_useCustomAppearance() == RimCurveAppearanceDefines::AppearanceMode::CUSTOM )
        {
            uiOrdering.add( &m_lineStyle );
            uiOrdering.add( &m_pointSymbol );
            uiOrdering.add( &m_symbolSize );
        }

        uiOrdering.add( &m_statisticsUseCustomAppearance );
        if ( m_statisticsUseCustomAppearance() == RimCurveAppearanceDefines::AppearanceMode::CUSTOM )
        {
            uiOrdering.add( &m_statisticsLineStyle );
            uiOrdering.add( &m_statisticsPointSymbol );
            uiOrdering.add( &m_statisticsSymbolSize );
        }
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveSetAppearance::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= "" */ )
{
    if ( m_colorMode == ColorMode::BY_ENSEMBLE_PARAM )
    {
        uiTreeOrdering.add( m_ensembleLegendConfig() );
    }
    uiTreeOrdering.skipRemainingChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimCurveSetAppearance::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_ensembleParameter )
    {
        auto params = internal::parametersWithVariation( m_ensemble );

        if ( m_ensembleParameterSorting() == RimCurveAppearanceDefines::ParameterSorting::ALPHABETICALLY )
        {
            std::sort( params.begin(), params.end() );
        }

        for ( const auto& param : params )
        {
            options.push_back( caf::PdmOptionItemInfo( param, param ) );
        }
    }
    else if ( fieldNeedingOptions == &m_colorMode )
    {
        auto singleColorOption          = ColorModeEnum( ColorMode::SINGLE_COLOR );
        auto singleColorWithAlphaOption = ColorModeEnum( ColorMode::SINGLE_COLOR_WITH_ALPHA );
        auto byEnsParamOption           = ColorModeEnum( ColorMode::BY_ENSEMBLE_PARAM );

        options.push_back( caf::PdmOptionItemInfo( singleColorOption.uiText(), ColorMode::SINGLE_COLOR ) );
        options.push_back( caf::PdmOptionItemInfo( singleColorWithAlphaOption.uiText(), ColorMode::SINGLE_COLOR_WITH_ALPHA ) );
        options.push_back( caf::PdmOptionItemInfo( byEnsParamOption.uiText(), ColorMode::BY_ENSEMBLE_PARAM ) );
    }

    return options;
}
