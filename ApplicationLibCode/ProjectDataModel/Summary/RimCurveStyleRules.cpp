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
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveAppearanceCalculator.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"

#include "RiuPlotCurveSymbolImageCreator.h"
#include "RiuPlotMainWindow.h"

#include "cafPdmUiLabel.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

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

namespace
{
std::vector<RimSummaryMultiPlot*> summaryMultiPlots()
{
    auto plotCollection = RimMainPlotCollection::current();
    if ( !plotCollection || !plotCollection->summaryMultiPlotCollection() ) return {};

    return plotCollection->summaryMultiPlotCollection()->multiPlots();
}
} // namespace
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCurveStyleRules::RimCurveStyleRules()
{
    CAF_PDM_InitObject( "Curve Style Rules" );

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

    CAF_PDM_InitFieldNoDefault( &m_colorBaseStyle, "ColorBaseStyle", "Color Base Style" );
    m_colorBaseStyle.uiCapability()->setUiTreeChildrenHidden( true );
    m_colorBaseStyle.uiCapability()->setUiHidden( true );
    m_colorBaseStyle = new RimCurveStyleRule;

    CAF_PDM_InitFieldNoDefault( &m_symbolBaseStyle, "SymbolBaseStyle", "Symbol Base Style" );
    m_symbolBaseStyle.uiCapability()->setUiTreeChildrenHidden( true );
    m_symbolBaseStyle.uiCapability()->setUiHidden( true );
    m_symbolBaseStyle = new RimCurveStyleRule;

    setUpBaseStyles();

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

    // Checked items are highlighted
    CAF_PDM_InitFieldNoDefault( &m_highlightedItems, "HighlightedItems", "Items" );
    m_highlightedItems.registerGetMethod( this, &RimCurveStyleRules::highlightedItems );
    m_highlightedItems.registerSetMethod( this, &RimCurveStyleRules::setHighlightedItems );
    m_highlightedItems.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_highlightedItems.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::TOP );
    m_highlightedItems.uiCapability()->setAttribute( caf::PdmUiTreeSelectionEditor::Keys::SHOW_TEXT_FILTER, false );
    m_highlightedItems.uiCapability()->setAttribute( caf::PdmUiTreeSelectionEditor::Keys::SHOW_TOGGLE_ALL_CHECKBOX, false );
    m_highlightedItems.uiCapability()->setAttribute( caf::PdmUiTreeSelectionEditor::Keys::HEIGHT_HINT, 200 );
    m_highlightedItems.uiCapability()->setAttribute( caf::PdmUiTreeSelectionEditor::Keys::ICON_SIZE, QSize( 33, 16 ) );
    m_highlightedItems.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_symbolSize, "SymbolSize", "Symbol Size" );
    m_symbolSize.registerGetMethod( this, &RimCurveStyleRules::symbolSize );
    m_symbolSize.registerSetMethod( this, &RimCurveStyleRules::setSymbolSize );
    m_symbolSize.xmlCapability()->disableIO();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::initAfterRead()
{
    setUpBaseStyles();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::setUpBaseStyles()
{
    if ( m_colorBaseStyle() ) m_colorBaseStyle()->setBaseKind( RimCurveStyleRule::BaseKind::COLOR );
    if ( m_symbolBaseStyle() ) m_symbolBaseStyle()->setBaseKind( RimCurveStyleRule::BaseKind::SYMBOL );
}

//--------------------------------------------------------------------------------------------------
/// The names of the highlighted items of the current dimension
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimCurveStyleRules::highlightedItems() const
{
    std::vector<QString> names;
    for ( auto rule : m_rules )
    {
        if ( rule->dimension() == m_currentType() && rule->isHighlighted() ) names.push_back( rule->itemName() );
    }

    return names;
}

//--------------------------------------------------------------------------------------------------
/// Rules are only kept for items that are highlighted or custom
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::setHighlightedItems( const std::vector<QString>& names )
{
    const std::set<QString> highlighted( names.begin(), names.end() );

    for ( const auto& name : RimCurveStyleRule::availableNames( m_currentType() ) )
    {
        if ( highlighted.contains( name ) )
        {
            findOrCreateRule( m_currentType(), name )->setHighlighted( true );
        }
        else if ( auto rule = findRule( m_currentType(), name ) )
        {
            rule->setHighlighted( false );
            if ( !rule->isCustom() )
            {
                m_rules.removeChild( rule );
                delete rule;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimCurveStyleRules::symbolSize() const
{
    return m_symbolBaseStyle()->symbolSize();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::setSymbolSize( const int& size )
{
    m_symbolBaseStyle()->setSymbolSize( size );
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

    std::vector<const RimCurveStyleRule*> others;
    for ( auto rule : m_rules )
    {
        if ( !rule->isCustom() && !rule->isHighlighted() ) continue;

        const auto dimension = rule->dimension();
        if ( dimension != colorDimension && dimension != symbolDimension ) continue;

        const auto value = RimCurveStyleSettings::dimensionValue( dimension, address, caseName );
        if ( !value.empty() && QString::fromStdString( value ) == rule->itemName() ) others.push_back( rule );
    }

    // Fixed precedence by dimension, so the result does not depend on the order the rules were created in
    std::stable_sort( others.begin(),
                      others.end(),
                      []( const RimCurveStyleRule* a, const RimCurveStyleRule* b ) { return a->dimension() < b->dimension(); } );

    // The base styles are applied first, as the rules for specific items build on them
    std::vector<const RimCurveStyleRule*> result;
    if ( colorDimension != RimCurveStyleRule::Dimension::NONE &&
         !RimCurveStyleSettings::dimensionValue( colorDimension, address, caseName ).empty() )
    {
        m_colorBaseStyle()->setDimension( colorDimension );
        result.push_back( m_colorBaseStyle() );
    }
    if ( symbolDimension != RimCurveStyleRule::Dimension::NONE &&
         !RimCurveStyleSettings::dimensionValue( symbolDimension, address, caseName ).empty() )
    {
        m_symbolBaseStyle()->setDimension( symbolDimension );
        result.push_back( m_symbolBaseStyle() );
    }

    result.insert( result.end(), others.begin(), others.end() );
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

    // The data source tree shows a tag with the global color of each case
    updateSummaryCaseTags();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::updateSummaryCaseTags()
{
    m_tagCache.clear();

    auto project = RimProject::current();
    if ( !project ) return;

    for ( auto summaryCase : project->allSummaryCases() )
    {
        summaryCase->updateConnectedEditors();
    }

    for ( auto ensemble : project->summaryEnsembles() )
    {
        ensemble->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::ruleChanged( RimCurveStyleRule* rule )
{
    rulesChanged();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCurveStyleRules::controlsColor( const RifEclipseSummaryAddress& address, const std::string& caseName ) const
{
    const auto dimension = m_settings->colorDimension();
    if ( dimension == RimCurveStyleRule::Dimension::NONE ) return false;

    return !RimCurveStyleSettings::dimensionValue( dimension, address, caseName ).empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCurveStyleRules::controlsSymbol( const RifEclipseSummaryAddress& address, const std::string& caseName ) const
{
    const auto dimension = m_settings->symbolDimension();
    if ( dimension == RimCurveStyleRule::Dimension::NONE ) return false;

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
bool RimCurveStyleRules::isGlobalColorUsed() const
{
    for ( auto multiPlot : summaryMultiPlots() )
    {
        if ( multiPlot->useGlobalColor() ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCurveStyleRules::isGlobalSymbolUsed() const
{
    for ( auto multiPlot : summaryMultiPlots() )
    {
        if ( multiPlot->useGlobalSymbol() ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// Tags showing the color and symbol assigned to an item of the dimension, for the data source tree.
/// The assigned appearances are found once per dimension, as it can require scanning all summary addresses and plots.
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmUiTreeViewItemTag> RimCurveStyleRules::treeItemTags( RimCurveStyleRule::Dimension dimension, const QString& name ) const
{
    std::vector<caf::PdmUiTreeViewItemTag> tags;
    if ( name.isEmpty() || !m_settings() ) return tags;

    const bool useColor  = m_settings()->colorDimension() == dimension && isGlobalColorUsed();
    const bool useSymbol = m_settings()->symbolDimension() == dimension && isGlobalSymbolUsed();
    if ( !useColor && !useSymbol ) return tags;

    auto cacheIt = m_tagCache.find( dimension );
    // Items added after the cache was built are not in the cache, rebuild to include them
    if ( cacheIt == m_tagCache.end() || !cacheIt->second.contains( name ) )
    {
        RimCurveStyleRule::AvailableNamesCache namesCache;

        std::map<QString, TagAppearance> appearances;

        const auto plotAppearances = RimCurveStyleRule::appearancesFromPlots( dimension );
        const auto indices         = itemIndices( dimension );
        for ( const auto& itemName : RimCurveStyleRule::availableNames( dimension ) )
        {
            const auto indexIt = indices.find( itemName );
            const int  index   = indexIt != indices.end() ? indexIt->second : static_cast<int>( appearances.size() );

            TagAppearance appearance;
            assignedAppearance( dimension, itemName, index, plotAppearances, appearance.color, appearance.symbol );

            if ( const auto rule = findRule( dimension, itemName ); rule && rule->isCustom() )
            {
                if ( rule->overriddenColor() ) appearance.color = rule->overriddenColor();
                if ( rule->overriddenSymbol() ) appearance.symbol = rule->overriddenSymbol();
            }

            appearances[itemName] = appearance;
        }

        cacheIt = m_tagCache.insert_or_assign( dimension, std::move( appearances ) ).first;
    }

    const auto appearanceIt = cacheIt->second.find( name );
    if ( appearanceIt == cacheIt->second.end() ) return tags;

    if ( useColor && appearanceIt->second.color )
    {
        auto tag    = caf::PdmUiTreeViewItemTag::create();
        tag.icon    = RimCurveStyleRule::swatchIcon( *appearanceIt->second.color );
        tag.toolTip = "Global curve color";
        tags.push_back( tag );
    }

    if ( useSymbol && appearanceIt->second.symbol )
    {
        const auto image =
            RiuPlotCurveSymbolImageCreator::createSymbolImage( *appearanceIt->second.symbol, QSize( 14, 14 ), QColor( Qt::gray ) );

        auto tag    = caf::PdmUiTreeViewItemTag::create();
        tag.icon    = caf::IconProvider( QPixmap::fromImage( image ) );
        tag.toolTip = "Global curve symbol";
        tags.push_back( tag );
    }

    return tags;
}

//--------------------------------------------------------------------------------------------------
/// Applies the global appearance to the plots when enabled for them, and restores the original appearance when disabled
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::applyGlobalAppearance( const std::vector<RimSummaryPlot*>& plots )
{
    RimCurveStyleRule::AvailableNamesCache namesCache;

    // Restore saved appearances first, so properties no longer controlled globally (e.g. color when changing from color
    // and symbol to symbol only) go back to their original values before the global appearance is applied again.
    // Plots still using the global appearance keep curves without a saved appearance as they are.
    std::vector<RimSummaryPlot*> enabledPlots;
    std::vector<RimSummaryPlot*> disabledPlots;
    for ( auto plot : plots )
        ( plot->useGlobalCurveAppearance() ? enabledPlots : disabledPlots ).push_back( plot );

    restoreAppearances( enabledPlots, false );
    restoreAppearances( disabledPlots, true );

    for ( auto plot : enabledPlots )
    {
        saveAppearance( plot );
        plot->updateCurveAppearanceFromGlobalSettings();
    }

    updateSummaryCaseTags();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// Appearances saved earlier are kept, they are the appearance before the global appearance was used
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::saveAppearance( RimSummaryPlot* plot )
{
    std::set<caf::PdmObject*> alreadySaved;
    for ( auto saved : m_savedAppearances )
    {
        if ( auto object = saved->object() ) alreadySaved.insert( object );
    }

    for ( auto curve : plot->summaryCurves() )
    {
        if ( alreadySaved.contains( curve ) ) continue;

        auto saved = new RimSavedCurveAppearance;
        saved->setCurve( curve );
        m_savedAppearances.push_back( saved );
    }

    for ( auto curveSet : plot->ensembleCurveSetCollection()->curveSets() )
    {
        if ( alreadySaved.contains( curveSet ) ) continue;

        auto saved = new RimSavedCurveAppearance;
        saved->setEnsembleCurveSet( curveSet );
        m_savedAppearances.push_back( saved );
    }
}

//--------------------------------------------------------------------------------------------------
/// Curves without a saved appearance, typically created while the global appearance was enabled, get the
/// calculated default appearance
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::restoreAppearances( const std::vector<RimSummaryPlot*>& plots, bool applyDefaultsToUnsaved )
{
    std::set<caf::PdmObject*> plotObjects;
    for ( auto plot : plots )
    {
        for ( auto curve : plot->summaryCurves() )
            plotObjects.insert( curve );
        for ( auto curveSet : plot->ensembleCurveSetCollection()->curveSets() )
            plotObjects.insert( curveSet );
    }

    std::set<caf::PdmObject*>             restored;
    std::vector<RimSavedCurveAppearance*> consumed;
    for ( auto saved : m_savedAppearances )
    {
        const auto object = saved->object();
        if ( !object || !plotObjects.contains( object ) ) continue;

        if ( saved->restore() ) restored.insert( object );
        consumed.push_back( saved );
    }

    for ( auto saved : consumed )
    {
        m_savedAppearances.removeChild( saved );
        delete saved;
    }

    if ( !applyDefaultsToUnsaved ) return;

    for ( auto plot : plots )
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
        if ( auto project = RimProject::current() )
        {
            auto collection    = project->colorLegendCollection();
            auto defaultLegend = collection->findByName( RimColorLegendCollection::summaryCurveColorLegendName() );

            // Not set (null) means the default palette, which is shown with the icon of the standard summary curve legend
            options.push_back( caf::PdmOptionItemInfo( "Default Palette",
                                                       nullptr,
                                                       false,
                                                       defaultLegend ? defaultLegend->paletteIconProvider() : caf::IconProvider() ) );

            for ( auto legend : collection->allColorLegends() )
            {
                if ( legend == defaultLegend ) continue;
                options.push_back( caf::PdmOptionItemInfo( legend->colorLegendName(), legend, false, legend->paletteIconProvider() ) );
            }
        }
        else
        {
            options.push_back( caf::PdmOptionItemInfo( "Default Palette", nullptr ) );
        }
    }
    else if ( fieldNeedingOptions == &m_symbolLegend )
    {
        const int count = static_cast<int>( caf::AppEnum<RiuPlotCurveSymbol::PointSymbolEnum>::size() ) - 1;
        for ( int i = 0; i < count; i++ )
        {
            const auto symbol = RimSummaryCurveAppearanceCalculator::cycledSymbol( i );
            const auto image  = RiuPlotCurveSymbolImageCreator::createSymbolImage( symbol, QSize( 14, 14 ), QColor( Qt::gray ) );
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
    else if ( fieldNeedingOptions == &m_highlightedItems )
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
            assignedAppearance( m_currentType(), name, index, plotAppearances, autoColor, autoSymbol );

            std::optional<cvf::Color3f> color;
            if ( isColorDim && autoColor ) color = isCustom ? customRule->overriddenColor().value_or( *autoColor ) : autoColor;

            std::optional<RiuPlotCurveSymbol::PointSymbolEnum> symbol;
            if ( isSymbolDim && autoSymbol ) symbol = isCustom ? customRule->overriddenSymbol().value_or( *autoSymbol ) : autoSymbol;

            const auto icon = RimCurveStyleRule::dimensionColorIcon( m_currentType(), color, symbol );
            options.push_back( caf::PdmOptionItemInfo( name, name, false, icon ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// The color and symbol the global appearance assigns to the item at the given position
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::assignedAppearance( RimCurveStyleRule::Dimension                                dimension,
                                             const QString&                                              name,
                                             int                                                         index,
                                             const std::map<QString, RimCurveStyleRule::PlotAppearance>& plotAppearances,
                                             std::optional<cvf::Color3f>&                                color,
                                             std::optional<RiuPlotCurveSymbol::PointSymbolEnum>&         symbol ) const
{
    const bool projectWide = dimension == RimCurveStyleRule::Dimension::CASE || dimension == RimCurveStyleRule::Dimension::WELL;

    const auto fromPlots = plotAppearances.find( name );
    const bool inPlots   = fromPlots != plotAppearances.end();

    if ( projectWide ) color = RimSummaryCurveAppearanceCalculator::cycledPaletteColor( index, true );
    if ( !color && inPlots ) color = fromPlots->second.color;
    if ( !color ) color = RimSummaryCurveAppearanceCalculator::cycledPaletteColor( index, true );

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

    if ( changedField == &m_autoColorLegend )
    {
        rulesChanged();
    }
    else if ( changedField == &m_currentType || changedField == &m_highlightedItems || changedField == &m_symbolSize )
    {
        rulesChanged();
    }

    updateConnectedEditors();
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
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimCurveStyleRule::AvailableNamesCache namesCache;

    uiOrdering.add( &m_appearanceType );
    uiOrdering.add( &m_currentType );

    if ( m_appearanceType() == AppearanceType::COLOR )
    {
        // Defines the style shared by all items, the color is assigned by the palette
        m_colorBaseStyle()->appendBaseStyleToUiOrdering( uiOrdering );
    }
    else
    {
        uiOrdering.add( &m_symbolSize );
    }

    auto rulesGroup = uiOrdering.addNewGroup( "Assigned Appearance" );
    if ( m_appearanceType() == AppearanceType::COLOR )
        rulesGroup->add( &m_autoColorLegend );
    else
        rulesGroup->add( &m_symbolLegend );
    rulesGroup->add( &m_highlightedItems );

    auto advancedGroup = uiOrdering.addNewGroup( "Advanced" );
    advancedGroup->setCollapsedByDefault();
    advancedGroup->addNewButton( "Connect All Plots to Global", [this]() { connectAllPlots( true ); } );
    advancedGroup->addNewButton( "Disconnect All Plots", [this]() { connectAllPlots( false ); } );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
/// Sets all summary multi plots to use global color and symbol, or to use local appearance
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRules::connectAllPlots( bool connect )
{
    const auto appearance = connect ? RimSummaryMultiPlot::GlobalAppearance::GLOBAL : RimSummaryMultiPlot::GlobalAppearance::LOCAL;

    std::vector<RimSummaryPlot*> plots;
    for ( auto multiPlot : summaryMultiPlots() )
    {
        multiPlot->setGlobalAppearance( appearance );
        multiPlot->updateConnectedEditors();

        for ( auto plot : multiPlot->summaryPlots() )
            plots.push_back( plot );
    }

    applyGlobalAppearance( plots );
}
