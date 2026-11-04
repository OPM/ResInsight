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

#include "RifGroupNetworkReader.h"

#include "RiaLogging.h"
#include "RiaStdStringTools.h"

#include "opm/input/eclipse/Deck/Deck.hpp"
#include "opm/input/eclipse/Deck/DeckItem.hpp"
#include "opm/input/eclipse/Deck/DeckKeyword.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"
#include "opm/input/eclipse/Parser/InputErrorAction.hpp"
#include "opm/input/eclipse/Parser/ParseContext.hpp"
#include "opm/input/eclipse/Parser/Parser.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/A.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/B.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/D.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/E.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/G.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/I.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/N.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/P.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/S.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/T.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/W.hpp"

#include <QDateTime>
#include <QTimeZone>

#include <deque>
#include <filesystem>
#include <optional>
#include <set>

namespace
{
const std::string fieldName             = "FIELD";
const std::string terminalPressureKey   = "TERMINAL_PRESSURE";
const std::string vfpTableKey           = "VFP_TABLE";
const std::string alqKey                = "ALQ";
constexpr int     removeBranchVfpNumber = 9999;

//--------------------------------------------------------------------------------------------------
/// Child to parent edges in the order they were first defined
//--------------------------------------------------------------------------------------------------
struct EdgeMap
{
    struct Value
    {
        std::string                   parent;
        std::map<std::string, double> properties;
    };

    void set( const std::string& child, const Value& value )
    {
        if ( !values.contains( child ) ) order.push_back( child );
        values[child] = value;
    }

    void remove( const std::string& child )
    {
        if ( values.erase( child ) > 0 ) std::erase( order, child );
    }

    // Removes the edge of a node and the edges of all nodes below it
    void removeSubtree( const std::string& child )
    {
        std::vector<std::string> pending = { child };
        while ( !pending.empty() )
        {
            const std::string current = pending.back();
            pending.pop_back();
            remove( current );
            for ( const auto& [name, value] : values )
            {
                if ( value.parent == current ) pending.push_back( name );
            }
        }
    }

    std::vector<std::string>     order;
    std::map<std::string, Value> values;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int monthFromText( const std::string& text )
{
    static const std::map<std::string, int, std::less<>> months = { { "JAN", 1 },
                                                                    { "FEB", 2 },
                                                                    { "MAR", 3 },
                                                                    { "APR", 4 },
                                                                    { "MAY", 5 },
                                                                    { "JUN", 6 },
                                                                    { "JUL", 7 },
                                                                    { "JLY", 7 },
                                                                    { "AUG", 8 },
                                                                    { "SEP", 9 },
                                                                    { "OCT", 10 },
                                                                    { "NOV", 11 },
                                                                    { "DEC", 12 } };

    const std::string key = RiaStdStringTools::toUpper( RiaStdStringTools::trimString( text ).substr( 0, 3 ) );

    auto it = months.find( key );
    return it != months.end() ? it->second : 0;
}

//--------------------------------------------------------------------------------------------------
/// DATES and START records have the same items
//--------------------------------------------------------------------------------------------------
std::optional<std::time_t> dateFromRecord( const Opm::DeckRecord& record )
{
    using DATES = Opm::ParserKeywords::DATES;

    const int day   = record.getItem<DATES::DAY>().get<int>( 0 );
    const int month = monthFromText( record.getItem<DATES::MONTH>().getTrimmedString( 0 ) );
    const int year  = record.getItem<DATES::YEAR>().get<int>( 0 );

    const QDate date( year, month, day );
    if ( !date.isValid() ) return std::nullopt;

    QTime time( 0, 0 );
    if ( record.size() > 3 && record.getItem<DATES::TIME>().hasValue( 0 ) )
    {
        const QString timeText = QString::fromStdString( record.getItem<DATES::TIME>().getTrimmedString( 0 ) );
        const QTime   parsed   = QTime::fromString( timeText, "HH:mm:ss" );
        if ( parsed.isValid() ) time = parsed;
    }

    return QDateTime( date, time, QTimeZone::UTC ).toSecsSinceEpoch();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<double> doubleValue( const Opm::DeckItem& item )
{
    if ( !item.hasValue( 0 ) || item.defaultApplied( 0 ) ) return std::nullopt;
    return item.get<double>( 0 );
}

//--------------------------------------------------------------------------------------------------
/// Collects the network keywords while walking through the deck
//--------------------------------------------------------------------------------------------------
class NetworkAccumulator
{
public:
    void processKeyword( const Opm::DeckKeyword& keyword )
    {
        const std::string& name = keyword.name();

        // Keywords in ACTIONX blocks are only applied when the condition is met during the simulation, so they are
        // not part of the network defined by the schedule
        if ( name == Opm::ParserKeywords::ACTIONX::keywordName )
        {
            if ( !m_actionReported )
            {
                RiaLogging::warning( "Production network: Network keywords in ACTIONX blocks are ignored" );
                m_actionReported = true;
            }
            m_isInAction = true;
            return;
        }
        if ( name == Opm::ParserKeywords::ENDACTIO::keywordName )
        {
            m_isInAction = false;
            return;
        }
        if ( m_isInAction ) return;

        if ( name == Opm::ParserKeywords::DATES::keywordName || name == Opm::ParserKeywords::START::keywordName )
        {
            for ( size_t i = 0; i < keyword.size(); ++i )
            {
                flush();
                if ( auto date = dateFromRecord( keyword.getRecord( i ) ) )
                    m_date = date;
                else
                    RiaLogging::warning( "Production network: Invalid date in " + name + ", ignored" );
            }
        }
        else if ( name == Opm::ParserKeywords::TSTEP::keywordName )
        {
            if ( !m_date )
            {
                RiaLogging::warning( "Production network: TSTEP found before the start date, ignored" );
                return;
            }

            flush();
            double days = 0.0;
            for ( size_t i = 0; i < keyword.size(); ++i )
            {
                for ( double step : keyword.getRecord( i ).getItem( 0 ).getData<double>() )
                {
                    days += step;
                }
            }
            *m_date += static_cast<std::time_t>( days * 24.0 * 3600.0 );
        }
        else if ( name == Opm::ParserKeywords::GRUPTREE::keywordName )
        {
            using KW = Opm::ParserKeywords::GRUPTREE;
            for ( const auto& record : keyword )
            {
                const auto child  = record.getItem<KW::CHILD_GROUP>().getTrimmedString( 0 );
                const auto parent = record.getItem<KW::PARENT_GROUP>().getTrimmedString( 0 );
                m_gruptree.set( child, { parent, {} } );
            }
            m_changedGruptree = true;
        }
        else if ( name == Opm::ParserKeywords::BRANPROP::keywordName )
        {
            using KW = Opm::ParserKeywords::BRANPROP;
            for ( const auto& record : keyword )
            {
                const auto child  = record.getItem<KW::DOWNTREE_NODE>().getTrimmedString( 0 );
                const auto parent = record.getItem<KW::UPTREE_NODE>().getTrimmedString( 0 );
                const int  vfp    = record.getItem<KW::VFP_TABLE>().get<int>( 0 );
                if ( vfp == removeBranchVfpNumber )
                {
                    m_branprop.removeSubtree( child );
                    continue;
                }

                std::map<std::string, double> properties;
                properties[vfpTableKey] = vfp;
                if ( auto alq = doubleValue( record.getItem<KW::ALQ>() ) ) properties[alqKey] = *alq;
                m_branprop.set( child, { parent, properties } );
            }
            m_changedBranprop = true;
        }
        else if ( name == Opm::ParserKeywords::WELSPECS::keywordName )
        {
            using KW = Opm::ParserKeywords::WELSPECS;
            for ( const auto& record : keyword )
            {
                const auto well  = record.getItem<KW::WELL>().getTrimmedString( 0 );
                const auto group = record.getItem<KW::GROUP>().getTrimmedString( 0 );
                m_welspecs.set( well, { group, {} } );
            }
            m_changedWelspecs = true;
        }
        else if ( name == Opm::ParserKeywords::GRUPNET::keywordName )
        {
            using KW = Opm::ParserKeywords::GRUPNET;
            for ( const auto& record : keyword )
            {
                const auto node = record.getItem<KW::NAME>().getTrimmedString( 0 );
                if ( auto pressure = doubleValue( record.getItem<KW::TERMINAL_PRESSURE>() ) )
                {
                    m_grupnetPressure[node] = *pressure;
                }
            }
            m_changedGruptree = true;
        }
        else if ( name == Opm::ParserKeywords::NODEPROP::keywordName )
        {
            using KW = Opm::ParserKeywords::NODEPROP;
            for ( const auto& record : keyword )
            {
                const auto node = record.getItem<KW::NAME>().getTrimmedString( 0 );
                if ( auto pressure = doubleValue( record.getItem<KW::PRESSURE>() ) )
                {
                    m_nodepropPressure[node] = *pressure;
                }
            }
            m_changedBranprop = true;
        }
    }

    // Stores the current trees with the current date if any network keyword was seen since the last date
    void flush()
    {
        const bool writeGruptree = m_changedGruptree || m_changedWelspecs;
        const bool writeBranprop = m_changedBranprop || m_changedWelspecs;
        if ( !writeGruptree && !writeBranprop ) return;

        m_changedGruptree = m_changedBranprop = m_changedWelspecs = false;

        if ( !m_date )
        {
            if ( !m_missingDateReported )
            {
                RiaLogging::warning( "Production network: Network keywords found before the start date, ignored" );
                m_missingDateReported = true;
            }
            m_gruptree = {};
            m_branprop = {};
            m_welspecs = {};
            m_grupnetPressure.clear();
            m_nodepropPressure.clear();
            return;
        }

        if ( writeGruptree )
        {
            auto tree = createTree( m_gruptree, RigGroupNetwork::EdgeKeyword::GRUPTREE, m_grupnetPressure, true );
            if ( !tree.edges.empty() ) m_network.addTree( RigGroupNetwork::TreeType::GRUPTREE, tree );
        }

        if ( writeBranprop && !m_branprop.values.empty() )
        {
            auto tree = createTree( m_branprop, RigGroupNetwork::EdgeKeyword::BRANPROP, m_nodepropPressure, false );
            if ( !tree.edges.empty() ) m_network.addTree( RigGroupNetwork::TreeType::BRANPROP, tree );
        }
    }

    RigGroupNetwork network() const { return m_network; }

private:
    RigGroupNetwork::TreeRevision createTree( const EdgeMap&                       groupEdges,
                                              RigGroupNetwork::EdgeKeyword         keyword,
                                              const std::map<std::string, double>& nodePressure,
                                              bool                                 isStandardNetwork ) const
    {
        EdgeMap edges = groupEdges;

        std::set<std::string> nodes;
        for ( const auto& [child, value] : edges.values )
        {
            nodes.insert( child );
            nodes.insert( value.parent );
        }

        if ( isStandardNetwork )
        {
            for ( const auto& well : m_welspecs.order )
            {
                const auto& group = m_welspecs.values.at( well ).parent;
                if ( !nodes.contains( group ) && group != fieldName )
                {
                    edges.set( group, { fieldName, {} } );
                    nodes.insert( group );
                    nodes.insert( fieldName );
                }
                else if ( group == fieldName )
                {
                    nodes.insert( fieldName );
                }
            }
        }

        // Children of each parent, in the order they were defined
        std::map<std::string, std::vector<std::string>> children;
        for ( const auto& child : edges.order )
        {
            children[edges.values.at( child ).parent].push_back( child );
        }

        std::map<std::string, std::vector<std::string>> wellsOfGroup;
        for ( const auto& well : m_welspecs.order )
        {
            const auto& group = m_welspecs.values.at( well ).parent;
            if ( nodes.contains( group ) ) wellsOfGroup[group].push_back( well );
        }

        std::vector<std::string> roots;
        for ( const auto& child : edges.order )
        {
            const auto& parent = edges.values.at( child ).parent;
            if ( !edges.values.contains( parent ) && std::find( roots.begin(), roots.end(), parent ) == roots.end() )
            {
                roots.push_back( parent );
            }
        }
        if ( roots.empty() && nodes.contains( fieldName ) ) roots.push_back( fieldName );

        auto nodeProperties = [&nodePressure]( const std::string& node )
        {
            std::map<std::string, double> properties;
            if ( auto it = nodePressure.find( node ); it != nodePressure.end() ) properties[terminalPressureKey] = it->second;
            return properties;
        };

        RigGroupNetwork::TreeRevision tree;
        tree.date = *m_date;

        // Breadth first from the roots, so parents are listed before their children. Nodes on cycles
        // are not reachable from a root and are left out.
        std::set<std::string>   visited;
        std::deque<std::string> queue;
        for ( const auto& root : roots )
        {
            tree.edges.push_back( { root, "", keyword, nodeProperties( root ) } );
            visited.insert( root );
            queue.push_back( root );
        }

        while ( !queue.empty() )
        {
            const std::string node = queue.front();
            queue.pop_front();

            if ( auto it = children.find( node ); it != children.end() )
            {
                for ( const auto& child : it->second )
                {
                    if ( !visited.insert( child ).second ) continue;

                    auto properties = edges.values.at( child ).properties;
                    properties.merge( nodeProperties( child ) );
                    tree.edges.push_back( { child, node, keyword, properties } );
                    queue.push_back( child );
                }
            }

            if ( auto it = wellsOfGroup.find( node ); it != wellsOfGroup.end() )
            {
                for ( const auto& well : it->second )
                {
                    tree.edges.push_back( { well, node, RigGroupNetwork::EdgeKeyword::WELSPECS, {} } );
                }
            }
        }

        return tree;
    }

private:
    std::optional<std::time_t> m_date;

    EdgeMap                       m_gruptree;
    EdgeMap                       m_branprop;
    EdgeMap                       m_welspecs;
    std::map<std::string, double> m_grupnetPressure;
    std::map<std::string, double> m_nodepropPressure;

    bool m_changedGruptree     = false;
    bool m_changedBranprop     = false;
    bool m_changedWelspecs     = false;
    bool m_missingDateReported = false;
    bool m_isInAction          = false;
    bool m_actionReported      = false;

    RigGroupNetwork m_network;
};

} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGroupNetwork RifGroupNetworkReader::readFromDeck( const Opm::Deck& deck )
{
    NetworkAccumulator accumulator;
    for ( const auto& keyword : deck )
    {
        accumulator.processKeyword( keyword );
    }
    accumulator.flush();

    return accumulator.network();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<RigGroupNetwork, QString> RifGroupNetworkReader::readFromFile( const QString& deckFileName )
{
    if ( deckFileName.isEmpty() ) return std::unexpected( "No deck file" );
    if ( !std::filesystem::exists( deckFileName.toStdString() ) )
    {
        return std::unexpected( QString( "Deck file '%1' does not exist" ).arg( deckFileName ) );
    }

    try
    {
        // Only the keywords needed for the network are parsed. Other keywords are skipped. Some keywords
        // without network data are included as they have caused parse errors when skipped.
        Opm::Parser parser( false );

        const std::vector<Opm::ParserKeyword> parserKeywords = { Opm::ParserKeywords::START(),
                                                                 Opm::ParserKeywords::DATES(),
                                                                 Opm::ParserKeywords::TSTEP(),
                                                                 Opm::ParserKeywords::GRUPTREE(),
                                                                 Opm::ParserKeywords::BRANPROP(),
                                                                 Opm::ParserKeywords::WELSPECS(),
                                                                 Opm::ParserKeywords::GRUPNET(),
                                                                 Opm::ParserKeywords::NODEPROP(),
                                                                 Opm::ParserKeywords::ACTIONX(),
                                                                 Opm::ParserKeywords::ENDACTIO(),
                                                                 Opm::ParserKeywords::INCLUDE(),
                                                                 Opm::ParserKeywords::PATHS(),
                                                                 Opm::ParserKeywords::TUNING(),
                                                                 Opm::ParserKeywords::SLAVES(),
                                                                 Opm::ParserKeywords::ECHO() };
        for ( const auto& keyword : parserKeywords )
        {
            parser.addParserKeyword( keyword );
        }

        Opm::ParseContext parseContext( Opm::InputErrorAction::WARN );
        const auto        deck = parser.parseFile( deckFileName.toStdString(), parseContext );

        return readFromDeck( deck );
    }
    catch ( const std::exception& e )
    {
        return std::unexpected( QString( "Failed to parse '%1': %2" ).arg( deckFileName ).arg( QString::fromStdString( e.what() ) ) );
    }
    catch ( ... )
    {
        return std::unexpected( QString( "Failed to parse '%1'" ).arg( deckFileName ) );
    }
}
