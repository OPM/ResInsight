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

#include "RimCurveStyleRule.h"

#include "Summary/RiaSummaryTools.h"

#include "RimCurveStyleRules.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimPlotCurve.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryPlot.h"

#include "RiuPlotCurveSymbolImageCreator.h"

#include <QColor>
#include <QLinearGradient>
#include <QPainter>
#include <QPixmap>

#include <set>

namespace caf
{
template <>
void caf::AppEnum<RimCurveStyleRule::Mode>::setUp()
{
    addItem( RimCurveStyleRule::Mode::AUTO, "AUTO", "Auto" );
    addItem( RimCurveStyleRule::Mode::CUSTOM, "CUSTOM", "Custom" );
    setDefault( RimCurveStyleRule::Mode::AUTO );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimCurveStyleRule, "RimCurveStyleRule" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCurveStyleRule::RimCurveStyleRule()
{
    CAF_PDM_InitObject( "Curve Style Rule" );

    CAF_PDM_InitFieldNoDefault( &m_dimension, "Dimension", "Type" );
    CAF_PDM_InitFieldNoDefault( &m_itemName, "ItemName", "Name" );

    CAF_PDM_InitFieldNoDefault( &m_mode, "Mode", "Assignment" );
    // m_mode.uiCapability()->setUiIconFromResourceString( ":/edit.svg" );

    CAF_PDM_InitFieldNoDefault( &m_appearance, "CurveAppearance", "Curve Appearance" );
    m_appearance = new RimPlotCurveAppearance;
    m_appearance.uiCapability()->setUiTreeChildrenHidden( true );
    connectAppearanceSignal();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRule::connectAppearanceSignal()
{
    if ( m_appearance() ) m_appearance->appearanceChanged.connect( this, &RimCurveStyleRule::onAppearanceChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRule::initAfterRead()
{
    connectAppearanceSignal();
}

//--------------------------------------------------------------------------------------------------
/// Editing any appearance value turns the item into a custom item
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRule::onAppearanceChanged( const caf::SignalEmitter* emitter )
{
    m_mode = Mode::CUSTOM;

    if ( auto collection = firstAncestorOrThisOfType<RimCurveStyleRules>() )
    {
        collection->rulesChanged();
        collection->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRule::applyToCurve( RimPlotCurve* curve, bool applyColor, bool applySymbol ) const
{
    if ( m_mode() != Mode::CUSTOM ) return;

    auto       curveAppearance = curve->curveAppearance();
    const auto color           = curveAppearance->color();
    const auto symbol          = curveAppearance->symbol();
    const auto label           = curveAppearance->symbolLabel();
    const auto labelPosition   = curveAppearance->symbolLabelPosition();

    curveAppearance->copyFrom( *m_appearance );

    // Color and symbol are only controlled by the rule when its dimension is the one selected for them
    if ( !applyColor ) curveAppearance->setColor( color );
    if ( !applySymbol ) curveAppearance->setSymbol( symbol );

    // Labels are specific to each curve
    curveAppearance->setSymbolLabel( label );
    curveAppearance->setSymbolLabelPosition( labelPosition );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<cvf::Color3f> RimCurveStyleRule::overriddenColor() const
{
    if ( m_mode() != Mode::CUSTOM ) return std::nullopt;
    return m_appearance->color();
}

namespace
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool matchesItem( RimCurveStyleRule::Dimension dimension, const QString& itemName, const RifEclipseSummaryAddress& address, const std::string& caseName )
{
    return QString::fromStdString( RimCurveStyleSettings::dimensionValue( dimension, address, caseName ) ) == itemName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string ensembleCaseName( RimEnsembleCurveSet* curveSet )
{
    return curveSet->summaryEnsemble() ? curveSet->summaryEnsemble()->name().toStdString() : std::string();
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// Shows what the global appearance gives for the item. The assigned color and symbol are used even if no
/// plot has a matching curve.
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRule::updateAppearanceFromCurves()
{
    copyAppearanceFromCurves();
    if ( m_autoColor ) m_appearance->setColor( *m_autoColor );
    if ( m_autoSymbol ) m_appearance->setSymbol( *m_autoSymbol );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRule::setAutoAssigned( const std::optional<cvf::Color3f>&                        color,
                                         const std::optional<RiuPlotCurveSymbol::PointSymbolEnum>& symbol )
{
    m_autoColor  = color;
    m_autoSymbol = symbol;
}

//--------------------------------------------------------------------------------------------------
/// Copies the appearance of the first matching curve
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRule::copyAppearanceFromCurves()
{
    auto project = RimProject::current();
    if ( !project ) return;

    for ( auto plot : project->descendantsIncludingThisOfType<RimSummaryPlot>() )
    {
        if ( !plot->useGlobalCurveAppearance() ) continue;

        for ( auto curve : plot->summaryCurves() )
        {
            if ( matchesItem( m_dimension(), m_itemName(), curve->summaryAddressY(), RimCurveStyleSettings::caseName( curve->summaryCaseY() ) ) )
            {
                m_appearance->copyFrom( *curve->curveAppearance() );
                return;
            }
        }

        for ( auto curveSet : plot->ensembleCurveSetCollection()->curveSets() )
        {
            if ( matchesItem( m_dimension(), m_itemName(), curveSet->summaryAddressY(), ensembleCaseName( curveSet ) ) )
            {
                m_appearance->setColor( curveSet->color() );
                return;
            }
        }
    }
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCurveStyleRule::Dimension RimCurveStyleRule::dimension() const
{
    return m_dimension();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCurveStyleRule::itemName() const
{
    return m_itemName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRule::setDimension( Dimension dimension )
{
    m_dimension = dimension;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRule::setItemName( const QString& itemName )
{
    m_itemName = itemName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
namespace
{
std::optional<std::map<RimCurveStyleRule::Dimension, QStringList>> availableNamesCache;
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCurveStyleRule::AvailableNamesCache::AvailableNamesCache()
    : m_isOwner( !availableNamesCache )
{
    if ( m_isOwner ) availableNamesCache.emplace();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCurveStyleRule::AvailableNamesCache::~AvailableNamesCache()
{
    if ( m_isOwner ) availableNamesCache.reset();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimCurveStyleRule::availableNames( Dimension dimension )
{
    if ( availableNamesCache )
    {
        auto it = availableNamesCache->find( dimension );
        if ( it != availableNamesCache->end() ) return it->second;
    }

    std::set<QString> names;
    if ( dimension == Dimension::WELL )
    {
        for ( const auto& wellName : RiaSummaryTools::allSummaryWellNames() )
        {
            names.insert( QString::fromStdString( wellName ) );
        }
    }
    else if ( dimension == Dimension::CASE )
    {
        if ( auto project = RimProject::current() )
        {
            for ( auto ensemble : project->summaryEnsembles() )
            {
                names.insert( ensemble->name() );
            }
            for ( auto summaryCase : project->allSummaryCases() )
            {
                if ( !summaryCase->ensemble() ) names.insert( summaryCase->displayCaseName() );
            }
        }
    }
    else if ( auto project = RimProject::current() )
    {
        for ( auto summaryCase : project->allSummaryCases() )
        {
            auto reader = summaryCase->summaryReader();
            if ( !reader ) continue;

            for ( const auto& address : reader->allResultAddresses() )
            {
                const auto value = RimCurveStyleSettings::dimensionValue( dimension, address, {} );
                if ( !value.empty() ) names.insert( QString::fromStdString( value ) );
            }
        }
    }

    QStringList result;
    for ( const auto& name : names )
    {
        result.push_back( name );
    }

    if ( availableNamesCache ) ( *availableNamesCache )[dimension] = result;

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, RimCurveStyleRule::PlotAppearance> RimCurveStyleRule::appearancesFromPlots( Dimension dimension )
{
    std::map<QString, PlotAppearance> appearances;

    auto project = RimProject::current();
    if ( !project ) return appearances;

    for ( auto plot : project->descendantsIncludingThisOfType<RimSummaryPlot>() )
    {
        if ( !plot->useGlobalCurveAppearance() ) continue;

        for ( auto curve : plot->summaryCurves() )
        {
            const auto value = RimCurveStyleSettings::dimensionValue( dimension,
                                                                      curve->summaryAddressY(),
                                                                      RimCurveStyleSettings::caseName( curve->summaryCaseY() ) );

            auto& appearance = appearances[QString::fromStdString( value )];
            if ( !appearance.color ) appearance.color = curve->color();
            if ( !appearance.symbol ) appearance.symbol = curve->symbol();
        }

        for ( auto curveSet : plot->ensembleCurveSetCollection()->curveSets() )
        {
            const auto value = RimCurveStyleSettings::dimensionValue( dimension, curveSet->summaryAddressY(), ensembleCaseName( curveSet ) );

            auto& appearance = appearances[QString::fromStdString( value )];
            if ( !appearance.color ) appearance.color = curveSet->color();
        }
    }

    return appearances;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCurveStyleRule::isCustom() const
{
    return m_mode() == Mode::CUSTOM;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRule::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    // The available names depend on the type
    if ( changedField == &m_dimension ) m_itemName = QString();
    if ( changedField == &m_mode && m_mode() == Mode::AUTO ) updateAppearanceFromCurves();

    if ( auto collection = firstAncestorOrThisOfType<RimCurveStyleRules>() )
    {
        collection->rulesChanged();
        collection->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRule::appendFieldsToUiOrdering( const QString& uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_mode );

    // Auto shows the values given by the global appearance, editing any value makes the item custom
    if ( m_mode() == Mode::AUTO ) updateAppearanceFromCurves();

    auto appearanceGroup = uiOrdering.addNewGroup( "Curve Style" );
    m_appearance->uiOrdering( "AppearanceOrdering", *appearanceGroup );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRule::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_dimension );
    uiOrdering.add( &m_itemName );
    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::IconProvider RimCurveStyleRule::swatchIcon( const cvf::Color3f& color )
{
    QPixmap pixmap( 16, 16 );
    pixmap.fill( Qt::transparent );

    QPainter painter( &pixmap );
    painter.setPen( Qt::darkGray );
    painter.setBrush( QColor::fromRgbF( color.r(), color.g(), color.b() ) );
    painter.drawRect( 1, 1, 13, 13 );

    return caf::IconProvider( pixmap );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::IconProvider RimCurveStyleRule::paletteIcon()
{
    QPixmap pixmap( 16, 16 );
    pixmap.fill( Qt::transparent );

    QLinearGradient gradient( 0, 0, 16, 0 );
    gradient.setColorAt( 0.0, Qt::red );
    gradient.setColorAt( 0.5, Qt::green );
    gradient.setColorAt( 1.0, Qt::blue );

    QPainter painter( &pixmap );
    painter.setPen( Qt::darkGray );
    painter.setBrush( gradient );
    painter.drawRect( 1, 1, 13, 13 );

    return caf::IconProvider( pixmap );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::IconProvider RimCurveStyleRule::symbolIcon()
{
    QPixmap pixmap( 16, 16 );
    pixmap.fill( Qt::transparent );

    QPainter painter( &pixmap );
    painter.setRenderHint( QPainter::Antialiasing );
    painter.setPen( Qt::black );
    painter.setBrush( Qt::NoBrush );
    painter.drawEllipse( 2, 2, 5, 5 );
    painter.drawRect( 9, 2, 5, 5 );
    painter.drawLine( 2, 14, 8, 9 );
    painter.drawLine( 8, 9, 14, 14 );
    painter.drawLine( 2, 14, 14, 14 );

    return caf::IconProvider( pixmap );
}

namespace
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString dimensionIconPath( RimCurveStyleRule::Dimension dimension )
{
    using D = RimCurveStyleRule::Dimension;
    switch ( dimension )
    {
        case D::CASE:
            return ":/SummaryCase.svg";
        case D::VECTOR:
            return ":/summary/components/images/misc.svg";
        case D::WELL:
        case D::LGR:
            return ":/summary/components/images/well.svg";
        case D::GROUP:
            return ":/summary/components/images/group.svg";
        case D::NETWORK:
            return ":/summary/components/images/network.svg";
        case D::REGION:
            return ":/summary/components/images/region.svg";
        case D::REGION_2_REGION:
            return ":/summary/components/images/region-region.svg";
        case D::AQUIFER:
            return ":/summary/components/images/aquifer.svg";
        case D::BLOCK:
        case D::CONNECTION:
            return ":/summary/components/images/block.svg";
        case D::SEGMENT:
            return ":/summary/components/images/segment.svg";
        default:
            return ":/DataVector.svg";
    }
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::IconProvider RimCurveStyleRule::dimensionColorIcon( Dimension                                                 dimension,
                                                         const std::optional<cvf::Color3f>&                        color,
                                                         const std::optional<RiuPlotCurveSymbol::PointSymbolEnum>& symbol )
{
    QPixmap pixmap( 52, 16 );
    pixmap.fill( Qt::transparent );

    QPainter painter( &pixmap );
    painter.drawPixmap( 0, 0, QIcon( dimensionIconPath( dimension ) ).pixmap( 16, 16 ) );

    const QColor qColor = color ? QColor::fromRgbF( color->r(), color->g(), color->b() ) : QColor( Qt::black );

    int x = 19;
    if ( color )
    {
        painter.setPen( Qt::darkGray );
        painter.setBrush( qColor );
        painter.drawRect( x, 1, 13, 13 );
        x += 17;
    }

    if ( symbol && *symbol != RiuPlotCurveSymbol::SYMBOL_NONE )
    {
        painter.drawImage( x, 1, RiuPlotCurveSymbolImageCreator::createSymbolImage( *symbol, QSize( 14, 14 ), qColor ) );
    }

    return caf::IconProvider( pixmap );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<RiuPlotCurveSymbol::PointSymbolEnum> RimCurveStyleRule::overriddenSymbol() const
{
    if ( m_mode() != Mode::CUSTOM ) return std::nullopt;
    return m_appearance->symbol();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::IconProvider RimCurveStyleRule::dimensionIcon( Dimension dimension )
{
    return caf::IconProvider( dimensionIconPath( dimension ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimCurveStyleRule::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_mode )
    {
        options.push_back( caf::PdmOptionItemInfo( "Auto", Mode::AUTO, false, caf::IconProvider( ":/Refresh.svg" ) ) );
        options.push_back( caf::PdmOptionItemInfo( "Custom", Mode::CUSTOM, false, caf::IconProvider( ":/edit.svg" ) ) );
    }

    return options;
}