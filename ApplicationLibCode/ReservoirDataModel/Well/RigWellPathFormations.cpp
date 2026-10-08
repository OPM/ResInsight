/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RigWellPathFormations.h"

#include "RiaWellLogTrackDefines.h"

#include <QStringList>

#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <optional>

namespace
{
using FormationLevel = RiaDefines::WellLogTrackFormationLevel;
using NamesAndDepths = std::pair<std::vector<QString>, std::vector<double>>;

// Picks closer than this are considered to be at the same depth
struct DepthComp
{
    bool operator()( double depth1, double depth2 ) const
    {
        if ( std::abs( depth1 - depth2 ) < 0.1 ) return false;
        return depth1 < depth2;
    }
};

enum class PickPosition
{
    TOP,
    BASE
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<double> pickDepth( const RigWellPathFormation& formation, PickPosition position, RiaDefines::DepthType depthType )
{
    const bool isTop = position == PickPosition::TOP;

    switch ( depthType )
    {
        case RiaDefines::DepthType::MEASURED_DEPTH:
            return isTop ? formation.mdTop : formation.mdBase;
        case RiaDefines::DepthType::TRUE_VERTICAL_DEPTH:
            return isTop ? formation.tvdTop : formation.tvdBase;
        default:
            return std::nullopt;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString pickName( const RigWellPathFormation& formation, PickPosition position )
{
    return formation.formationName + ( position == PickPosition::TOP ? " Top" : " Base" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
NamesAndDepths fluidPicks( const std::vector<RigWellPathFormation>& fluids, RiaDefines::DepthType depthType )
{
    std::map<double, QString, DepthComp> picks;

    // Bases first, so a top at the same depth replaces the base
    for ( auto position : { PickPosition::BASE, PickPosition::TOP } )
    {
        for ( const auto& fluid : fluids )
        {
            auto depth = pickDepth( fluid, position, depthType );
            if ( !depth ) return {};

            picks[*depth] = pickName( fluid, position );
        }
    }

    NamesAndDepths result;
    for ( const auto& [depth, name] : picks )
    {
        result.first.push_back( name );
        result.second.push_back( depth );
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
NamesAndDepths allPicksWithoutDuplicateDepths( const std::vector<std::pair<RigWellPathFormation, FormationLevel>>& formations,
                                               RiaDefines::DepthType                                               depthType )
{
    NamesAndDepths              result;
    std::set<double, DepthComp> usedDepths;

    for ( auto position : { PickPosition::TOP, PickPosition::BASE } )
    {
        for ( const auto& [formation, level] : formations )
        {
            auto depth = pickDepth( formation, position, depthType );
            if ( !depth ) return {};

            if ( usedDepths.insert( *depth ).second )
            {
                result.first.push_back( pickName( formation, position ) );
                result.second.push_back( *depth );
            }
        }
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/// At each depth, keep the pick from the most detailed level not exceeding maxLevel
//--------------------------------------------------------------------------------------------------
NamesAndDepths picksUpToLevel( const std::vector<std::pair<RigWellPathFormation, FormationLevel>>& formations,
                               FormationLevel                                                      maxLevel,
                               RiaDefines::DepthType                                               depthType )
{
    std::map<double, std::pair<FormationLevel, QString>, DepthComp> picks;

    for ( auto position : { PickPosition::TOP, PickPosition::BASE } )
    {
        for ( const auto& [formation, level] : formations )
        {
            auto depth = pickDepth( formation, position, depthType );
            if ( !depth ) return {};

            if ( level > maxLevel ) continue;

            auto it = picks.find( *depth );
            if ( it == picks.end() )
            {
                picks.emplace( *depth, std::make_pair( level, pickName( formation, position ) ) );
            }
            else if ( it->second.first < level )
            {
                it->second = { level, pickName( formation, position ) };
            }
        }
    }

    NamesAndDepths result;
    for ( const auto& [depth, levelAndName] : picks )
    {
        result.first.push_back( levelAndName.second );
        result.second.push_back( depth );
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool isFluid( const QString& formationName )
{
    const QString name = formationName.trimmed();
    return name == "OIL" || name == "GAS" || name == "WATER";
}

//--------------------------------------------------------------------------------------------------
/// Group names are all upper case. Otherwise the number of dots in the numeric part gives the level,
/// e.g. "Ile 2" is level 1 and "Ile 2.1" is level 2.
//--------------------------------------------------------------------------------------------------
FormationLevel detectLevel( const QString& formationName )
{
    const QString name = formationName.trimmed();

    if ( std::none_of( name.begin(), name.end(), []( QChar c ) { return c.isLower(); } ) )
    {
        return FormationLevel::GROUP;
    }

    auto containsDigit = []( const QString& word ) { return std::any_of( word.begin(), word.end(), []( QChar c ) { return c.isDigit(); } ); };
    auto containsLetter = []( const QString& word )
    { return std::any_of( word.begin(), word.end(), []( QChar c ) { return c.isLetter(); } ); };

    std::vector<QString> levelDescriptorCandidates;
    for ( const QString& word : name.split( ' ' ) )
    {
        if ( containsDigit( word ) ) levelDescriptorCandidates.push_back( word );
    }

    if ( levelDescriptorCandidates.empty() ) return FormationLevel::LEVEL0;

    if ( levelDescriptorCandidates.size() > 1 )
    {
        std::erase_if( levelDescriptorCandidates, containsLetter );
    }

    if ( levelDescriptorCandidates.size() != 1 ) return FormationLevel::UNKNOWN;

    const QString levelDescriptor = levelDescriptorCandidates.front().split( '+' ).front();

    static const std::array levels = { FormationLevel::LEVEL1,
                                       FormationLevel::LEVEL2,
                                       FormationLevel::LEVEL3,
                                       FormationLevel::LEVEL4,
                                       FormationLevel::LEVEL5,
                                       FormationLevel::LEVEL6,
                                       FormationLevel::LEVEL7,
                                       FormationLevel::LEVEL8,
                                       FormationLevel::LEVEL9,
                                       FormationLevel::LEVEL10 };

    const auto dotCount = static_cast<size_t>( levelDescriptor.count( '.' ) );
    return dotCount < levels.size() ? levels[dotCount] : FormationLevel::UNKNOWN;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellPathFormations::RigWellPathFormations( const std::vector<RigWellPathFormation>& formations, const QString& filePath, const QString& key )
    : m_filePath( filePath )
    , m_keyInFile( key )
{
    for ( const auto& formation : formations )
    {
        if ( isFluid( formation.formationName ) )
        {
            m_fluids.push_back( formation );
        }
        else
        {
            auto level = detectLevel( formation.formationName );
            m_formationsLevelsPresent.insert( level );
            m_formations.emplace_back( formation, level );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<QString>, std::vector<double>>
    RigWellPathFormations::depthAndFormationNamesUpToLevel( FormationLevel level, bool includeFluids, RiaDefines::DepthType depthType ) const
{
    NamesAndDepths result;
    if ( includeFluids )
    {
        result = fluidPicks( m_fluids, depthType );
    }

    NamesAndDepths formationResult;
    if ( level == FormationLevel::ALL )
    {
        formationResult = allPicksWithoutDuplicateDepths( m_formations, depthType );
    }
    else if ( level != FormationLevel::NONE )
    {
        formationResult = picksUpToLevel( m_formations, level, depthType );
    }

    result.first.insert( result.first.end(), formationResult.first.begin(), formationResult.first.end() );
    result.second.insert( result.second.end(), formationResult.second.begin(), formationResult.second.end() );

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
/// Returns one (name, top, base) range per formation whose level does not exceed the given level,
/// for use as shaded zone regions. Each zone is clipped against the more detailed zones inside it, so
/// parent and child zones are not drawn on top of each other.
//--------------------------------------------------------------------------------------------------
std::vector<std::tuple<QString, double, double>> RigWellPathFormations::depthRangesUpToLevel( FormationLevel        level,
                                                                                              RiaDefines::DepthType depthType ) const
{
    struct Zone
    {
        QString        name;
        double         top;
        double         base;
        FormationLevel level;
    };

    std::vector<Zone> zones;
    if ( level == FormationLevel::NONE ) return {};

    for ( const auto& [formation, formationLevel] : m_formations )
    {
        if ( level != FormationLevel::ALL && formationLevel > level ) continue;

        auto top  = pickDepth( formation, PickPosition::TOP, depthType );
        auto base = pickDepth( formation, PickPosition::BASE, depthType );
        if ( !top || !base ) continue;

        zones.push_back( { formation.formationName, *top, *base, formationLevel } );
    }

    // UNKNOWN has no place in the hierarchy, so it neither hides nor is hidden by other zones
    auto isMoreDetailed = []( FormationLevel candidate, FormationLevel reference )
    { return candidate <= FormationLevel::LEVEL10 && reference <= FormationLevel::LEVEL10 && candidate > reference; };

    const double minThickness = 0.1;

    std::vector<std::tuple<QString, double, double>> result;
    for ( const auto& zone : zones )
    {
        const double lower = std::min( zone.top, zone.base );
        const double upper = std::max( zone.top, zone.base );

        std::vector<std::pair<double, double>> covered;
        for ( const auto& other : zones )
        {
            if ( !isMoreDetailed( other.level, zone.level ) ) continue;
            covered.emplace_back( std::min( other.top, other.base ), std::max( other.top, other.base ) );
        }
        std::sort( covered.begin(), covered.end() );

        // Keep the parts of the zone not covered by a more detailed zone, in the zone's own direction
        auto addPart = [&]( double from, double to )
        {
            if ( to - from <= minThickness ) return;
            if ( zone.top <= zone.base )
                result.emplace_back( zone.name, from, to );
            else
                result.emplace_back( zone.name, to, from );
        };

        double current = lower;
        for ( const auto& [coveredLower, coveredUpper] : covered )
        {
            if ( coveredLower >= upper ) break;
            if ( coveredUpper <= current ) continue;

            addPart( current, std::min( coveredLower, upper ) );
            current = std::max( current, coveredUpper );
        }
        addPart( current, upper );
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigWellPathFormations::FormationLevel> RigWellPathFormations::formationsLevelsPresent() const
{
    return { m_formationsLevelsPresent.begin(), m_formationsLevelsPresent.end() };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigWellPathFormations::filePath() const
{
    return m_filePath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigWellPathFormations::keyInFile() const
{
    return m_keyInFile;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigWellPathFormations::formationNamesCount() const
{
    return m_formations.size() + m_fluids.size();
}

//--------------------------------------------------------------------------------------------------
/// Returns the non-fluid formation at index, in the order the input data was given
//--------------------------------------------------------------------------------------------------
const RigWellPathFormation& RigWellPathFormations::formationAt( size_t index ) const
{
    return m_formations[index].first;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigWellPathFormations::formationCount() const
{
    return m_formations.size();
}
