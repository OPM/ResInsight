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
