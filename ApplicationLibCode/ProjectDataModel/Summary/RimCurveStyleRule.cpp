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

#include "RiaColorTools.h"
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

    CAF_PDM_InitFieldNoDefault( &m_dimension, "Dimension", "Dimension" );
    CAF_PDM_InitFieldNoDefault( &m_itemName, "ItemName", "Item Name" );
    CAF_PDM_InitFieldNoDefault( &m_mode, "Mode", "Mode" );
    CAF_PDM_InitField( &m_highlighted, "Highlighted", false, "Highlighted" );

    CAF_PDM_InitFieldNoDefault( &m_curveAppearance, "CurveAppearance", "Curve Appearance" );
    m_curveAppearance = new RimPlotCurveAppearance;
    m_curveAppearance.uiCapability()->setUiTreeChildrenHidden( true );
    setUpAppearance();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRule::setUpAppearance()
{
    if ( !m_curveAppearance() ) return;

    m_curveAppearance->appearanceChanged.connect( this, &RimCurveStyleRule::onAppearanceChanged );

    // Only show the properties applied by applyToCurve()
    m_curveAppearance->setInterpolationVisible( false );
    m_curveAppearance->setFillOptionsVisible( false );
    m_curveAppearance->setCurveFittingToleranceVisible( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRule::initAfterRead()
{
    setUpAppearance();
}

//--------------------------------------------------------------------------------------------------
/// Editing any appearance value turns the item into a custom item
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRule::onAppearanceChanged( const caf::SignalEmitter* emitter )
{
    m_mode = Mode::CUSTOM;

    if ( auto collection = firstAncestorOrThisOfType<RimCurveStyleRules>() ) collection->ruleChanged( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRule::applyToCurve( RimPlotCurve* curve, bool applyColor, bool applySymbol ) const
{
    auto*       curveAppearance = curve->curveAppearance();
    const auto& rule            = *m_curveAppearance;

    // The base styles define the line style and thickness, or the symbol size, shared by all items
    if ( m_baseKind != BaseKind::NONE )
    {
        if ( m_mode() != Mode::CUSTOM ) return;

        if ( m_baseKind == BaseKind::COLOR && applyColor )
        {
            curveAppearance->setLineStyle( rule.lineStyle() );
            curveAppearance->setLineThickness( rule.lineThickness() );
        }
        if ( m_baseKind == BaseKind::SYMBOL && applySymbol )
        {
            curveAppearance->setSymbolSize( rule.symbolSize() );
            curveAppearance->setSymbolSkipDistance( rule.symbolSkipDistance() );
        }
        return;
    }

    if ( m_mode() != Mode::CUSTOM ) return;

    // Only the explicit overrides are applied. Line stepping, fill, labels and curve fitting are never controlled by the rule.
    // Color settings are only edited for color rules, and symbol settings for symbol rules.
    if ( applyColor )
    {
        curveAppearance->setColor( rule.color() );
        curveAppearance->setCurveColorOpacity( rule.curveColorOpacity() );
        curveAppearance->setSymbolEdgeColor( rule.symbolEdgeColor() );
        curveAppearance->setLineStyle( rule.lineStyle() );
        curveAppearance->setLineThickness( rule.lineThickness() );
    }

    if ( applySymbol )
    {
        curveAppearance->setSymbol( rule.symbol() );
        curveAppearance->setSymbolSize( rule.symbolSize() );
        curveAppearance->setSymbolSkipDistance( rule.symbolSkipDistance() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<cvf::Color3f> RimCurveStyleRule::overriddenColor() const
{
    if ( m_mode() != Mode::CUSTOM || m_baseKind != BaseKind::NONE ) return std::nullopt;
    return m_curveAppearance->color();
}

namespace
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string ensembleCaseName( RimEnsembleCurveSet* curveSet )
{
    return curveSet->summaryEnsemble() ? curveSet->summaryEnsemble()->name().toStdString() : std::string();
}
} // namespace

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
void RimCurveStyleRule::setCustom( bool isCustom )
{
    m_mode = isCustom ? Mode::CUSTOM : Mode::AUTO;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRule::copySettingsFrom( const RimCurveStyleRule& other )
{
    m_mode = other.m_mode();
    m_curveAppearance->copyFrom( *other.m_curveAppearance() );
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

    if ( dimension == Dimension::NONE || !RimProject::current() ) return {};

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

    if ( auto collection = firstAncestorOrThisOfType<RimCurveStyleRules>() ) collection->ruleChanged( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRule::appendBaseStyleToUiOrdering( caf::PdmUiOrdering& uiOrdering )
{
    m_curveAppearance->setColorVisible( false );
    m_curveAppearance->setSymbolVisible( false );
    m_curveAppearance->setLineVisible( true );

    auto appearanceGroup = uiOrdering.addNewGroup( "Curve Style" );
    m_curveAppearance->uiOrdering( "AppearanceOrdering", *appearanceGroup );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRule::setBaseKind( BaseKind kind )
{
    m_baseKind = kind;

    // Unedited base styles start with the default line thickness and symbol size
    if ( kind == BaseKind::NONE || m_mode() == Mode::CUSTOM ) return;

    if ( kind == BaseKind::COLOR )
    {
        m_curveAppearance->setLineThickness( 2 );
        m_mode = Mode::CUSTOM;
    }
    if ( kind == BaseKind::SYMBOL ) setSymbolSize( 10 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCurveStyleRule::isHighlighted() const
{
    return m_highlighted();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRule::setHighlighted( bool highlighted )
{
    m_highlighted = highlighted;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimCurveStyleRule::symbolSize() const
{
    return m_curveAppearance->symbolSize();
}

//--------------------------------------------------------------------------------------------------
/// The skip distance is derived from the size, so symbols do not overlap
//--------------------------------------------------------------------------------------------------
void RimCurveStyleRule::setSymbolSize( int size )
{
    m_curveAppearance->setSymbolSize( size );
    m_curveAppearance->setSymbolSkipDistance( 10.0f * size );
    m_mode = Mode::CUSTOM;
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
    painter.setBrush( RiaColorTools::toQColor( color ) );
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
    painter.setPen( Qt::gray );
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
            return ":/summary/components/images/well.svg";
        case D::LGR:
            return ":/LGR16x16.png";
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
    const bool showColor  = color.has_value();
    const bool showSymbol = symbol && *symbol != RiuPlotCurveSymbol::SYMBOL_NONE;
    const int  width      = 19 + ( showColor ? 14 : 0 ) + ( showColor && showSymbol ? 3 : 0 ) + ( showSymbol ? 14 : 0 );

    QPixmap pixmap( width, 16 );
    pixmap.fill( Qt::transparent );

    QPainter painter( &pixmap );
    painter.drawPixmap( 0, 0, QIcon( dimensionIconPath( dimension ) ).pixmap( 16, 16 ) );

    const QColor qColor = color ? RiaColorTools::toQColor( *color ) : QColor( Qt::gray );

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
    if ( m_mode() != Mode::CUSTOM || m_baseKind != BaseKind::NONE ) return std::nullopt;
    return m_curveAppearance->symbol();
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