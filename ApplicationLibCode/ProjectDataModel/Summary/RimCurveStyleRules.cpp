/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimCurveStyleRules.h"

#include "RiaGuiApplication.h"

#include "RimColorLegend.h"
#include "RimColorLegendCollection.h"
#include "RimColorLegendItem.h"
#include "RimCurveStyleRule.h"
#include "RimCurveStyleSettings.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimProject.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveAppearanceCalculator.h"
#include "RimSummaryPlot.h"

#include "RiuPlotCurveSymbolImageCreator.h"
#include "RiuPlotMainWindow.h"

#include "cafPdmUiLabel.h"
#include "cafPdmUiListEditor.h"

#include <algorithm>
#include <map>
#include <set>

CAF_PDM_SOURCE_INIT( RimCurveStyleRules, "RimCurveStyleRules" );

namespace caf
{
template <>
void caf::AppEnum<RimCurveStyleRules::AppearanceType>::setUp()
{
    using T = RimCurveStyleRules::AppearanceType;
    addItem( T::COLOR, "COLOR", "Color" );
    addItem( T::SYMBOL, "SYMBOL", "Symbol" );
    setDefault( T::COLOR );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCurveStyleRules::RimCurveStyleRules()
{
    CAF_PDM_InitObject( "Curve Style Rules" );

    CAF_PDM_InitField( &m_useGlobalAppearance,
                       "UseGlobalAppearance",
                       false,
                       "Use Global Curve Appearance",
                       "",
                       "Assign curve appearance in all summary plots from the global curve appearance definition" );

    CAF_PDM_InitFieldNoDefault( &m_settings, "Settings", "Settings" );
    m_settings.uiCapability()->setUiTreeChildrenHidden( true );
    m_settings.uiCapability()->setUiHidden( true );
    m_settings = new RimCurveStyleSettings;

    CAF_PDM_InitFieldNoDefault( &m_savedAppearances, "SavedAppearances", "Saved Appearances" );
    m_savedAppearances.uiCapability()->setUiTreeChildrenHidden( true );
    m_savedAppearances.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_rules, "Rules", "Rules" );
    m_rules.uiCapability()->setUiTreeChildrenHidden( true );
    m_rules.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_caseOrder, "CaseOrder", "Case Order" );
    m_caseOrder.uiCapability()->setUiHidden( true );
    CAF_PDM_InitFieldNoDefault( &m_wellOrder, "WellOrder", "Well Order" );
    m_wellOrder.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_selectedItem, "SelectedItem", "Selected Item" );
    m_selectedItem.uiCapability()->setUiTreeChildrenHidden( true );
    m_selectedItem.uiCapability()->setUiHidden( true );
    m_selectedItem.xmlCapability()->disableIO();
    m_selectedItem = new RimCurveStyleRule;

    CAF_PDM_InitFieldNoDefault( &m_appearanceType, "AppearanceType", "Appearance Property" );
    m_appearanceType.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_autoColorLegend, "AutoColorLegend", "Color Palette" );

    CAF_PDM_InitField( &m_symbolLegend, "SymbolLegend", 0, "Symbol Sequence" );
    m_symbolLegend.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );
    m_symbolLegend.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::TOP );
    m_symbolLegend.uiCapability()->setUiReadOnly( true );
    m_symbolLegend.uiCapability()->setAttribute( caf::PdmUiListEditor::Keys::HEIGHT_HINT, 120 );
    m_symbolLegend.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_currentType, "CurrentType", "Differentiate By" );
    m_currentType.registerGetMethod( this, &RimCurveStyleRules::currentDimension );
    m_currentType.registerSetMethod( this, &RimCurveStyleRules::setCurrentDimension );
    m_currentType.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_currentItemIndex, "CurrentItemIndex", 0, "Items" );
    m_currentItemIndex.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );
    m_currentItemIndex.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::TOP );
    m_currentItemIndex.uiCapability()->setAttribute( caf::PdmUiListEditor::Keys::HEIGHT_HINT, 200 );
    m_currentItemIndex.uiCapability()->setAttribute( caf::PdmUiListEditor::Keys::ICON_SIZE, QSize( 52, 16 ) );
    m_currentItemIndex.xmlCapability()->disableIO();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCurveStyleSettings* RimCurveStyleRules::settings() const
{
    return m_settings();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimCurveStyleRule*> RimCurveStyleRules::rules() const
{
    return m_rules.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCurveStyleRule* RimCurveStyleRules::findOrCreateRule( RimCurveStyleRule::Dimension dimension, const QString& name )
{
    if ( auto rule = findRule( dimension, name ) ) return rule;

    auto rule = new RimCurveStyleRule;
    rule->setDimension( dimension );
    rule->setItemName( name );
    m_rules.push_back( rule );

    return rule;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RimCurveStyleRule*> RimCurveStyleRules::rulesFor( const RifEclipseSummaryAddress& address, const std::string& caseName ) const
{
    const auto colorDimension  = m_settings->colorDimension();
    const auto symbolDimension = m_settings->symbolDimension();

    std::vector<const RimCurveStyleRule*> result;
    for ( auto rule : m_rules )
    {
        if ( !rule->isCustom() ) continue;

        const auto dimension = rule->dimension();
        if ( dimension != colorDimension && dimension != symbolDimension ) continue;

        const auto value = RimCurveStyleSettings::dimensionValue( dimension, address, caseName );
        if ( !value.empty() && QString::fromStdString( value ) == rule->itemName() ) result.push_back( rule );
    }

    // Fixed precedence by dimension, so the result does not depend on the order the rules were created in
    std::stable_sort( result.begin(),
                      result.end(),
                      []( const RimCurveStyleRule* a, const RimCurveStyleRule* b ) { return a->dimension() < b->dimension(); } );

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::rulesChanged()
{
    auto project = RimProject::current();
    if ( !project ) return;

    RimCurveStyleRule::AvailableNamesCache namesCache;
    for ( auto plot : project->descendantsIncludingThisOfType<RimSummaryPlot>() )
    {
        plot->updateCurveAppearanceFromGlobalSettings();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::ruleChanged( RimCurveStyleRule* rule )
{
    if ( rule == m_selectedItem() )
    {
        // Rules are only stored for items that have been made custom
        auto stored = rule->isCustom() ? findOrCreateRule( rule->dimension(), rule->itemName() )
                                       : findRule( rule->dimension(), rule->itemName() );
        if ( stored ) stored->copySettingsFrom( *rule );
    }

    rulesChanged();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCurveStyleRules::controlsColor( const RifEclipseSummaryAddress& address, const std::string& caseName ) const
{
    const auto dimension = m_settings->colorDimension();
    if ( !m_useGlobalAppearance() || dimension == RimCurveStyleRule::Dimension::NONE ) return false;

    return !RimCurveStyleSettings::dimensionValue( dimension, address, caseName ).empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCurveStyleRules::controlsSymbol( const RifEclipseSummaryAddress& address, const std::string& caseName ) const
{
    const auto dimension = m_settings->symbolDimension();
    if ( !m_useGlobalAppearance() || dimension == RimCurveStyleRule::Dimension::NONE ) return false;

    return !RimCurveStyleSettings::dimensionValue( dimension, address, caseName ).empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::registerItemNames( RimCurveStyleRule::Dimension dimension, const QStringList& names )
{
    auto order = itemOrder( dimension );
    if ( !order ) return;

    auto                    values = ( *order )();
    const std::set<QString> registered( values.begin(), values.end() );
    for ( const auto& name : names )
    {
        if ( !registered.contains( name ) ) values.push_back( name );
    }

    if ( values.size() != ( *order )().size() ) order->setValue( values );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, int> RimCurveStyleRules::itemIndices( RimCurveStyleRule::Dimension dimension ) const
{
    auto order = itemOrder( dimension );
    if ( !order ) return {};

    std::map<QString, int> indices;

    int index = 0;
    for ( const auto& name : ( *order )() )
    {
        if ( indices.emplace( name, index ).second ) index++;
    }

    for ( const auto& name : RimCurveStyleRule::availableNames( dimension ) )
    {
        if ( indices.emplace( name, index ).second ) index++;
    }

    return indices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmField<std::vector<QString>>* RimCurveStyleRules::itemOrder( RimCurveStyleRule::Dimension dimension )
{
    if ( dimension == RimCurveStyleRule::Dimension::CASE ) return &m_caseOrder;
    if ( dimension == RimCurveStyleRule::Dimension::WELL ) return &m_wellOrder;
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const caf::PdmField<std::vector<QString>>* RimCurveStyleRules::itemOrder( RimCurveStyleRule::Dimension dimension ) const
{
    return const_cast<RimCurveStyleRules*>( this )->itemOrder( dimension );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::addGlobalAppearanceLink( caf::PdmUiOrdering& uiOrdering, const QString& text )
{
    auto label = uiOrdering.addNewLabel( text );
    label->setLinkActivatedCallback(
        []( const QString& )
        {
            if ( auto window = RiaGuiApplication::instance()->mainPlotWindow() ) window->showCurveAppearance();
        } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCurveStyleRules::useGlobalAppearance() const
{
    return m_useGlobalAppearance();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::setUseGlobalAppearance( bool enable )
{
    if ( m_useGlobalAppearance() == enable ) return;

    m_useGlobalAppearance = enable;
    applyGlobalAppearanceMode();
}

//--------------------------------------------------------------------------------------------------
/// Applies the global appearance to all plots when enabled, and restores the original appearance when disabled
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::applyGlobalAppearanceMode()
{
    auto project = RimProject::current();
    if ( !project ) return;

    if ( m_useGlobalAppearance() )
    {
        RimCurveStyleRule::AvailableNamesCache namesCache;

        m_savedAppearances.deleteChildren();
        for ( auto plot : project->descendantsIncludingThisOfType<RimSummaryPlot>() )
        {
            saveAppearance( plot );
            plot->updateCurveAppearanceFromGlobalSettings();
        }
    }
    else
    {
        restoreAppearances();
        m_savedAppearances.deleteChildren();
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::saveAppearance( RimSummaryPlot* plot )
{
    for ( auto curve : plot->summaryCurves() )
    {
        auto saved = new RimSavedCurveAppearance;
        saved->setCurve( curve );
        m_savedAppearances.push_back( saved );
    }

    for ( auto curveSet : plot->ensembleCurveSetCollection()->curveSets() )
    {
        auto saved = new RimSavedCurveAppearance;
        saved->setEnsembleCurveSet( curveSet );
        m_savedAppearances.push_back( saved );
    }
}

//--------------------------------------------------------------------------------------------------
/// Curves without a saved appearance, typically created while the global appearance was enabled, get the
/// calculated default appearance
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::restoreAppearances()
{
    auto project = RimProject::current();
    if ( !project ) return;

    std::set<caf::PdmObject*> restored;
    for ( auto saved : m_savedAppearances )
    {
        if ( auto object = saved->restore() ) restored.insert( object );
    }

    for ( auto plot : project->descendantsIncludingThisOfType<RimSummaryPlot>() )
    {
        std::vector<RimSummaryCurve*> curves;
        for ( auto curve : plot->summaryCurves() )
        {
            if ( !restored.contains( curve ) ) curves.push_back( curve );
        }
        if ( !curves.empty() ) plot->applyDefaultCurveAppearances( curves );

        std::vector<RimEnsembleCurveSet*> curveSets;
        for ( auto curveSet : plot->ensembleCurveSetCollection()->curveSets() )
        {
            if ( !restored.contains( curveSet ) ) curveSets.push_back( curveSet );
        }
        if ( !curveSets.empty() ) plot->applyDefaultCurveAppearances( curveSets );

        plot->updateConnectedEditors();
        plot->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<cvf::Color3f> RimCurveStyleRules::autoLegendColor( int index ) const
{
    if ( !m_autoColorLegend() || index < 0 ) return std::nullopt;

    const auto items = m_autoColorLegend()->colorLegendItems();
    if ( items.empty() ) return std::nullopt;

    return items[index % items.size()]->color();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimCurveStyleRules::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_autoColorLegend )
    {
        options.push_back( caf::PdmOptionItemInfo( "Default Palette", nullptr ) );
        if ( auto project = RimProject::current() )
        {
            for ( auto legend : project->colorLegendCollection()->allColorLegends() )
            {
                options.push_back( caf::PdmOptionItemInfo( legend->colorLegendName(), legend, false, legend->paletteIconProvider() ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_symbolLegend )
    {
        const int count = static_cast<int>( caf::AppEnum<RiuPlotCurveSymbol::PointSymbolEnum>::size() ) - 1;
        for ( int i = 0; i < count; i++ )
        {
            const auto symbol = RimSummaryCurveAppearanceCalculator::cycledSymbol( i );
            const auto image  = RiuPlotCurveSymbolImageCreator::createSymbolImage( symbol, QSize( 14, 14 ), QColor( Qt::black ) );
            options.push_back( caf::PdmOptionItemInfo( caf::AppEnum<RiuPlotCurveSymbol::PointSymbolEnum>::uiText( symbol ),
                                                       i,
                                                       false,
                                                       caf::IconProvider( QPixmap::fromImage( image ) ) ) );
        }
    }
    else if ( fieldNeedingOptions == &m_currentType )
    {
        using D = RimCurveStyleRule::Dimension;
        for ( size_t i = 0; i < caf::AppEnum<D>::size(); i++ )
        {
            const auto dimension = caf::AppEnum<D>::fromIndex( i );
            options.push_back( caf::PdmOptionItemInfo( caf::AppEnum<D>::uiText( dimension ), static_cast<int>( dimension ) ) );
        }
    }
    else if ( fieldNeedingOptions == &m_currentItemIndex )
    {
        RimCurveStyleRule::AvailableNamesCache namesCache;

        const bool isColorDim      = m_appearanceType() == AppearanceType::COLOR;
        const bool isSymbolDim     = m_appearanceType() == AppearanceType::SYMBOL;
        const auto plotAppearances = RimCurveStyleRule::appearancesFromPlots( m_currentType() );
        const auto indices         = itemIndices( m_currentType() );
        for ( const auto& name : RimCurveStyleRule::availableNames( m_currentType() ) )
        {
            const auto customRule = findRule( m_currentType(), name );
            const bool isCustom   = customRule && customRule->isCustom();
            const auto indexIt    = indices.find( name );
            const int  index      = indexIt != indices.end() ? indexIt->second : static_cast<int>( options.size() );

            std::optional<cvf::Color3f>                        autoColor;
            std::optional<RiuPlotCurveSymbol::PointSymbolEnum> autoSymbol;
            assignedAppearance( name, index, plotAppearances, autoColor, autoSymbol );

            std::optional<cvf::Color3f> color;
            if ( isColorDim && autoColor ) color = isCustom ? customRule->overriddenColor().value_or( *autoColor ) : autoColor;

            std::optional<RiuPlotCurveSymbol::PointSymbolEnum> symbol;
            if ( isSymbolDim && autoSymbol ) symbol = isCustom ? customRule->overriddenSymbol().value_or( *autoSymbol ) : autoSymbol;
            const auto icon = RimCurveStyleRule::dimensionColorIcon( m_currentType(), color, symbol );
            options.push_back(
                caf::PdmOptionItemInfo( isCustom ? name + "  [Custom]" : name + "  [Auto]", static_cast<int>( options.size() ), false, icon ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// The color and symbol the global appearance assigns to the item at the given position
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::assignedAppearance( const QString&                                              name,
                                             int                                                         index,
                                             const std::map<QString, RimCurveStyleRule::PlotAppearance>& plotAppearances,
                                             std::optional<cvf::Color3f>&                                color,
                                             std::optional<RiuPlotCurveSymbol::PointSymbolEnum>&         symbol ) const
{
    const bool projectWide = m_currentType() == RimCurveStyleRule::Dimension::CASE || m_currentType() == RimCurveStyleRule::Dimension::WELL;

    const auto fromPlots = plotAppearances.find( name );
    const bool inPlots   = fromPlots != plotAppearances.end();

    if ( projectWide ) color = RimSummaryCurveAppearanceCalculator::cycledPaletteColor( index );
    if ( !color && inPlots ) color = fromPlots->second.color;
    if ( !color ) color = RimSummaryCurveAppearanceCalculator::cycledPaletteColor( index );

    if ( projectWide ) symbol = RimSummaryCurveAppearanceCalculator::cycledSymbol( index );
    if ( !symbol && inPlots ) symbol = fromPlots->second.symbol;
    if ( !symbol ) symbol = RimSummaryCurveAppearanceCalculator::cycledSymbol( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCurveStyleRule* RimCurveStyleRules::findRule( RimCurveStyleRule::Dimension dimension, const QString& name ) const
{
    for ( auto item : m_rules )
    {
        if ( item->dimension() == dimension && item->itemName() == name ) return item;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimCurveStyleRule::AvailableNamesCache namesCache;

    if ( changedField == &m_useGlobalAppearance )
    {
        applyGlobalAppearanceMode();
        return;
    }

    if ( changedField == &m_autoColorLegend )
    {
        rulesChanged();
    }
    else if ( changedField == &m_appearanceType )
    {
        m_currentItemIndex = 0;
    }
    else if ( changedField == &m_currentType )
    {
        m_currentItemIndex = 0;
        rulesChanged();
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCurveStyleRules::currentItemName() const
{
    const auto names = RimCurveStyleRule::availableNames( m_currentType() );
    const int  index = m_currentItemIndex();
    return index >= 0 && index < names.size() ? names[index] : QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::AppEnum<RimCurveStyleRule::Dimension> RimCurveStyleRules::currentDimension() const
{
    auto settings = m_settings();
    if ( !settings ) return RimCurveStyleRule::Dimension::NONE;

    return m_appearanceType() == AppearanceType::COLOR ? settings->colorDimension() : settings->symbolDimension();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::setCurrentDimension( const caf::AppEnum<RimCurveStyleRule::Dimension>& dimension )
{
    auto settings = m_settings();
    if ( !settings ) return;

    if ( m_appearanceType() == AppearanceType::COLOR )
        settings->setColorDimension( dimension );
    else
        settings->setSymbolDimension( dimension );
}

//--------------------------------------------------------------------------------------------------
/// Shows the stored custom settings of the selected item, or the appearance assigned automatically. Only the
/// selected item object is changed, it is not stored in the project.
//--------------------------------------------------------------------------------------------------
bool RimCurveStyleRules::updateSelectedItem()
{
    const QString itemName = currentItemName();
    if ( itemName.isEmpty() ) return false;

    auto selected = m_selectedItem();
    selected->setDimension( m_currentType() );
    selected->setItemName( itemName );

    auto stored = findRule( m_currentType(), itemName );
    if ( stored && stored->isCustom() )
    {
        selected->copySettingsFrom( *stored );
        return true;
    }

    const auto indices = itemIndices( m_currentType() );
    const auto indexIt = indices.find( itemName );
    const int  index   = indexIt != indices.end() ? indexIt->second : m_currentItemIndex();

    std::optional<cvf::Color3f>                        autoColor;
    std::optional<RiuPlotCurveSymbol::PointSymbolEnum> autoSymbol;
    assignedAppearance( itemName, index, RimCurveStyleRule::appearancesFromPlots( m_currentType() ), autoColor, autoSymbol );

    selected->setCustom( false );
    selected->setAutoAssigned( m_appearanceType() == AppearanceType::COLOR ? autoColor : std::nullopt,
                               m_appearanceType() == AppearanceType::SYMBOL ? autoSymbol : std::nullopt );
    selected->updateAppearanceFromCurves();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimCurveStyleRule::AvailableNamesCache namesCache;

    uiOrdering.add( &m_useGlobalAppearance );
    uiOrdering.add( &m_appearanceType );
    uiOrdering.add( &m_currentType );

    auto rulesGroup = uiOrdering.addNewGroup( "Assigned Appearance" );
    if ( m_appearanceType() == AppearanceType::COLOR )
        rulesGroup->add( &m_autoColorLegend );
    else
        rulesGroup->add( &m_symbolLegend );
    rulesGroup->add( &m_currentItemIndex );

    if ( updateSelectedItem() )
    {
        auto appearanceGroup = uiOrdering.addNewGroup( "Selected Item" );
        m_selectedItem->appendFieldsToUiOrdering( uiConfigName, *appearanceGroup );
    }

    uiOrdering.skipRemainingFields();
}
