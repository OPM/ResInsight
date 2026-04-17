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

#include "RigPadModel.h"

#include "RifOpmFlowDeckFile.h"
#include "RigModelPaddingSettings.h"

#include "RiaLogging.h"

#include "cvfVector3.h"

#include "opm/input/eclipse/Deck/Deck.hpp"
#include "opm/input/eclipse/Deck/DeckKeyword.hpp"
#include "opm/input/eclipse/Deck/DeckSection.hpp"
#include "opm/input/eclipse/Deck/FileDeck.hpp"
#include "opm/input/eclipse/EclipseState/Grid/SatfuncPropertyInitializers.hpp"
#include "opm/input/eclipse/EclipseState/Runspec.hpp"
#include "opm/input/eclipse/EclipseState/Tables/TableManager.hpp"

#include "opm/input/eclipse/Parser/ParserKeywords/A.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/B.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/C.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/D.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/E.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/F.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/M.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/N.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/O.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/P.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/S.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/W.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{

using GridSize = cvf::Vec3st;

//--------------------------------------------------------------------------------------------------
/// Section names for tracking which section a keyword belongs to
//--------------------------------------------------------------------------------------------------
static const std::unordered_set<std::string> sectionNames = { "RUNSPEC", "GRID", "EDIT", "PROPS", "REGIONS", "SOLUTION", "SUMMARY", "SCHEDULE" };

static bool isSectionKeyword( const std::string& name )
{
    return sectionNames.count( name ) > 0;
}

//--------------------------------------------------------------------------------------------------
/// Read NX/NY/NZ from DIMENS via FileDeck
//--------------------------------------------------------------------------------------------------
std::expected<GridSize, QString> getDimens( Opm::FileDeck& fileDeck )
{
    auto idx = fileDeck.find( Opm::ParserKeywords::DIMENS::keywordName );
    if ( !idx.has_value() )
    {
        return std::unexpected( "Failed to read grid dimensions from DIMENS keyword" );
    }

    const auto& kw = fileDeck[idx.value()];
    GridSize    gridSize( kw.getRecord( 0 ).getItem( "NX" ).get<int>( 0 ),
                       kw.getRecord( 0 ).getItem( "NY" ).get<int>( 0 ),
                       kw.getRecord( 0 ).getItem( "NZ" ).get<int>( 0 ) );

    if ( gridSize.x() == 0 || gridSize.y() == 0 || gridSize.z() == 0 )
    {
        return std::unexpected( "Grid dimensions from DIMENS keyword contain zero values" );
    }

    return gridSize;
}

//--------------------------------------------------------------------------------------------------
/// Modify DIMENS, EQLDIMS, SPECGRID NZ in-place
//--------------------------------------------------------------------------------------------------
int extendDimens( const GridSize& gridSize, const RigModelPaddingSettings& settings, Opm::FileDeck& fileDeck, int nzNew )
{
    int upperEquilnum = settings.upperEquilnum();

    // Update DIMENS NZ
    auto dimIdx = fileDeck.find( Opm::ParserKeywords::DIMENS::keywordName );
    if ( dimIdx.has_value() )
    {
        auto&             NZit    = const_cast<Opm::DeckItem&>( fileDeck[dimIdx.value()].getRecord( 0 ).getItem( "NZ" ) );
        std::vector<int>& dimData = NZit.getData<int>();
        dimData[0]                = nzNew;
    }

    // Update EQLDIMS NTEQUL
    auto eqlIdx = fileDeck.find( Opm::ParserKeywords::EQLDIMS::keywordName );
    if ( eqlIdx.has_value() )
    {
        auto& eqldims = const_cast<Opm::DeckItem&>( fileDeck[eqlIdx.value()].getRecord( 0 ).getItem( "NTEQUL" ) );

        std::vector<int>& data = eqldims.getData<int>();

        std::vector<Opm::value::status>& value_status = const_cast<std::vector<Opm::value::status>&>( eqldims.getValueStatus() );

        if ( value_status[0] == Opm::value::status::deck_value )
        {
            data[0] += 1;
        }
        else
        {
            value_status[0] = Opm::value::status::deck_value;
            data[0]         = 2;
        }

        upperEquilnum = data[0];
    }

    // Update SPECGRID NZ
    auto specIdx = fileDeck.find( Opm::ParserKeywords::SPECGRID::keywordName );
    if ( specIdx.has_value() )
    {
        auto&             NZit     = const_cast<Opm::DeckItem&>( fileDeck[specIdx.value()].getRecord( 0 ).getItem( "NZ" ) );
        std::vector<int>& specData = NZit.getData<int>();
        specData[0]                = nzNew;
    }

    return upperEquilnum;
}

//--------------------------------------------------------------------------------------------------
/// Template: extend DeckItem data via swap
//--------------------------------------------------------------------------------------------------
template <typename T>
void extendPropertyArray( const GridSize&                gridSize,
                          const RigModelPaddingSettings& settings,
                          const Opm::type_tag            type,
                          const T                        default_value,
                          const Opm::DeckItem&           property )
{
    if ( property.getType() != type )
    {
        return;
    }

    const auto input_nc =
        static_cast<std::size_t>( gridSize.x() ) * static_cast<std::size_t>( gridSize.y() ) * static_cast<std::size_t>( gridSize.z() );

    auto& value = const_cast<Opm::DeckItem&>( property ).template getData<T>();
    if ( std::size( value ) != input_nc )
    {
        return;
    }

    auto& status = const_cast<std::vector<Opm::value::status>&>( property.getValueStatus() );

    const auto output_nc = static_cast<std::size_t>( gridSize.x() ) * static_cast<std::size_t>( gridSize.y() ) *
                           static_cast<std::size_t>( settings.nzUpper() + gridSize.z() + settings.nzLower() );

    const auto value_offset = static_cast<std::size_t>( gridSize.x() ) * static_cast<std::size_t>( gridSize.y() ) *
                              static_cast<std::size_t>( settings.nzUpper() );

    auto remapped_value  = std::vector<T>( output_nc, default_value );
    auto remapped_status = std::vector<Opm::value::status>( output_nc, Opm::value::status::deck_value );

    std::copy( value.begin(), value.end(), remapped_value.begin() + value_offset );
    std::copy( status.begin(), status.end(), remapped_status.begin() + value_offset );

    value.swap( remapped_value );
    status.swap( remapped_status );
}

//--------------------------------------------------------------------------------------------------
/// Iterate keywords in GRID section, extend matching data arrays (padmodel lines 417-441)
//--------------------------------------------------------------------------------------------------
template <typename T>
void extendGridSection( Opm::FileDeck& fileDeck, const GridSize& gridSize, const RigModelPaddingSettings& settings, const Opm::type_tag type )
{
    bool inGridSection = false;

    for ( auto it = fileDeck.start(); it != fileDeck.stop(); ++it )
    {
        const auto& keyword = fileDeck[it];

        if ( keyword.name() == "GRID" )
        {
            inGridSection = true;
            continue;
        }
        if ( inGridSection && isSectionKeyword( keyword.name() ) )
        {
            break;
        }

        if ( !inGridSection ) continue;
        if ( keyword.size() != 1 || !keyword.isDataKeyword() ) continue;

        T default_value{};
        if ( keyword.name() == "PORO" )
        {
            default_value = static_cast<T>( settings.upperPorosity() );
        }
        else if ( keyword.name() == "NTG" )
        {
            default_value = T{ 1 };
        }

        extendPropertyArray( gridSize, settings, type, default_value, keyword.getRecord( 0 ).getItem( 0 ) );
    }
}

//--------------------------------------------------------------------------------------------------
/// Shift BCCON K1/K2 based on direction (padmodel lines 443-474)
//--------------------------------------------------------------------------------------------------
void extendBCCon( Opm::FileDeck& fileDeck, const RigModelPaddingSettings& settings )
{
    for ( auto it = fileDeck.start(); it != fileDeck.stop(); ++it )
    {
        const auto& kw = fileDeck[it];
        if ( kw.name() != "BCCON" ) continue;

        for ( size_t r = 0; r < kw.size(); ++r )
        {
            const auto& record = kw.getRecord( r );

            int& K1 = const_cast<int&>( record.getItem( "K1" ).getData<int>()[0] );
            int& K2 = const_cast<int&>( record.getItem( "K2" ).getData<int>()[0] );

            const auto direction = record.getItem( "DIRECTION" ).getTrimmedString( 0 );

            // OPM-Flow BCCON convention: "Z" (or "Z+") is the positive K direction,
            // i.e. the bottom face of the grid. After padding, these faces must be
            // shifted by the total number of added layers.
            if ( direction == "Z" || direction == "Z+" )
            {
                const int shift = settings.nzUpper() + settings.nzLower();
                K1 += shift;
                K2 += shift;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Shift FAULTS IZ1/IZ2 by nz_upper (padmodel lines 476-488)
//--------------------------------------------------------------------------------------------------
void extendFaults( const RigModelPaddingSettings& settings, Opm::FileDeck& fileDeck )
{
    for ( auto it = fileDeck.start(); it != fileDeck.stop(); ++it )
    {
        const auto& kw = fileDeck[it];
        if ( kw.name() != Opm::ParserKeywords::FAULTS::keywordName ) continue;

        for ( size_t r = 0; r < kw.size(); ++r )
        {
            const auto& face = kw.getRecord( r );

            const_cast<int&>( face.getItem<Opm::ParserKeywords::FAULTS::IZ1>().getData<int>().front() ) += settings.nzUpper();
            const_cast<int&>( face.getItem<Opm::ParserKeywords::FAULTS::IZ2>().getData<int>().front() ) += settings.nzUpper();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Shift K1/K2 in ADD/BOX/EQUALS/MAXVALUE/MINVALUE/MULTIPLY/OPERATE
//--------------------------------------------------------------------------------------------------
void extendOperators( const RigModelPaddingSettings& settings, Opm::FileDeck& fileDeck )
{
    const std::vector<std::string> opKws = { Opm::ParserKeywords::ADD::keywordName,
                                             Opm::ParserKeywords::BOX::keywordName,
                                             Opm::ParserKeywords::EQUALS::keywordName,
                                             Opm::ParserKeywords::MAXVALUE::keywordName,
                                             Opm::ParserKeywords::MINVALUE::keywordName,
                                             Opm::ParserKeywords::MULTIPLY::keywordName,
                                             Opm::ParserKeywords::OPERATE::keywordName };

    std::unordered_set<std::string> opKwSet( opKws.begin(), opKws.end() );

    for ( auto it = fileDeck.start(); it != fileDeck.stop(); ++it )
    {
        const auto& kw = fileDeck[it];
        if ( opKwSet.count( kw.name() ) == 0 ) continue;

        for ( size_t r = 0; r < kw.size(); ++r )
        {
            const auto& record = kw.getRecord( r );
            for ( const auto* kname : { "K1", "K2" } )
            {
                // Some keywords may not have K1/K2 items - check by name
                if ( !record.hasItem( kname ) ) continue;

                const auto& k = record.getItem( kname );
                if ( !k.defaultApplied( 0 ) )
                {
                    const_cast<Opm::DeckItem&>( k ).getData<int>().front() += settings.nzUpper();
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Set inactive cells active, zero perm, set poro (padmodel lines 518-595)
//--------------------------------------------------------------------------------------------------
void extendSpecialGridProperties( Opm::FileDeck& fileDeck, const GridSize& gridSize, const RigModelPaddingSettings& settings )
{
    // Find ACTNUM keyword
    std::vector<int>*    actnumPtr = nullptr;
    std::vector<double>* poroPtr   = nullptr;
    std::vector<double>* permxPtr  = nullptr;
    std::vector<double>* permyPtr  = nullptr;
    std::vector<double>* permzPtr  = nullptr;
    std::vector<double>* ntgPtr    = nullptr;

    bool inGridSection = false;

    for ( auto it = fileDeck.start(); it != fileDeck.stop(); ++it )
    {
        const auto& kw = fileDeck[it];

        if ( kw.name() == "GRID" )
        {
            inGridSection = true;
            continue;
        }
        if ( inGridSection && isSectionKeyword( kw.name() ) ) break;
        if ( !inGridSection ) continue;

        if ( kw.name() == "ACTNUM" && kw.isDataKeyword() )
        {
            actnumPtr = &const_cast<std::vector<int>&>( kw.getIntData() );
        }
        else if ( kw.name() == "PORO" && kw.isDataKeyword() )
        {
            poroPtr = const_cast<std::vector<double>*>( &kw.getRawDoubleData() );
        }
        else if ( kw.name() == "PERMX" && kw.isDataKeyword() )
        {
            permxPtr = const_cast<std::vector<double>*>( &kw.getRawDoubleData() );
        }
        else if ( kw.name() == "PERMY" && kw.isDataKeyword() )
        {
            permyPtr = const_cast<std::vector<double>*>( &kw.getRawDoubleData() );
        }
        else if ( kw.name() == "PERMZ" && kw.isDataKeyword() )
        {
            permzPtr = const_cast<std::vector<double>*>( &kw.getRawDoubleData() );
        }
        else if ( kw.name() == "NTG" && kw.isDataKeyword() )
        {
            ntgPtr = const_cast<std::vector<double>*>( &kw.getRawDoubleData() );
        }
    }

    if ( actnumPtr == nullptr ) return;

    auto& actnum = *actnumPtr;

    // Fix value_status for ACTNUM
    for ( auto it = fileDeck.start(); it != fileDeck.stop(); ++it )
    {
        const auto& kw = fileDeck[it];
        if ( kw.name() == "ACTNUM" && kw.isDataKeyword() )
        {
            auto& actnum_status = const_cast<std::vector<Opm::value::status>&>( kw.getValueStatus() );
            actnum_status.assign( actnum.size(), Opm::value::status::deck_value );
            break;
        }
    }

    for ( size_t i = 0; i < actnum.size(); ++i )
    {
        if ( actnum[i] == 1 ) continue;

        actnum[i] = 1;

        if ( poroPtr != nullptr ) ( *poroPtr )[i] = settings.upperPorosity();
        if ( permxPtr != nullptr ) ( *permxPtr )[i] = 0.0;
        if ( permyPtr != nullptr ) ( *permyPtr )[i] = 0.0;
        if ( permzPtr != nullptr ) ( *permzPtr )[i] = 0.0;
        if ( ntgPtr != nullptr ) ( *ntgPtr )[i] = 1.0;
    }
}

//--------------------------------------------------------------------------------------------------
/// Per-column ZCORN + COORD vertical pillars
//--------------------------------------------------------------------------------------------------
void extendGRDECL( Opm::FileDeck& fileDeck, const GridSize& gridSize, const RigModelPaddingSettings& settings, int nzNew )
{
    const int    nx           = static_cast<int>( gridSize.x() );
    const int    ny           = static_cast<int>( gridSize.y() );
    const int    nz           = static_cast<int>( gridSize.z() );
    const int    nz_upper     = settings.nzUpper();
    const int    nz_lower     = settings.nzLower();
    const double top_upper    = settings.topUpper();
    const double bottom_lower = settings.bottomLower();
    const double min_dist     = settings.minLayerThickness();
    const bool   no_gap       = settings.fillGaps();
    const int    nz_new       = nzNew;

    // Find COORD and ZCORN in GRID section
    std::vector<double>*             coordPtr       = nullptr;
    std::vector<double>*             zcornPtr       = nullptr;
    std::vector<Opm::value::status>* zcornStatusPtr = nullptr;

    bool inGridSection = false;

    for ( auto it = fileDeck.start(); it != fileDeck.stop(); ++it )
    {
        const auto& kw = fileDeck[it];

        if ( kw.name() == "GRID" )
        {
            inGridSection = true;
            continue;
        }
        if ( inGridSection && isSectionKeyword( kw.name() ) ) break;
        if ( !inGridSection ) continue;

        if ( kw.name() == "COORD" && kw.isDataKeyword() )
        {
            coordPtr = const_cast<std::vector<double>*>( &kw.getRawDoubleData() );
        }
        else if ( kw.name() == "ZCORN" && kw.isDataKeyword() )
        {
            zcornPtr       = const_cast<std::vector<double>*>( &kw.getRawDoubleData() );
            zcornStatusPtr = const_cast<std::vector<Opm::value::status>*>( &kw.getValueStatus() );
        }
    }

    if ( coordPtr == nullptr || zcornPtr == nullptr || zcornStatusPtr == nullptr ) return;

    auto& coord        = *coordPtr;
    auto& zcorn        = *zcornPtr;
    auto& zcorn_status = *zcornStatusPtr;

    // Make pillars vertical
    if ( settings.verticalPillars() )
    {
        const int ncoord = static_cast<int>( coord.size() );
        const int nxny   = ncoord / 6;

        assert( ( nx + 1 ) * ( ny + 1 ) == nxny );
        assert( ncoord % 6 == 0 );

        for ( int i = 0; i < nxny; ++i )
        {
            coord[6 * i + 3] = coord[6 * i + 0];
            coord[6 * i + 4] = coord[6 * i + 1];
        }
    }

    // Extend ZCORN per-column (padmodel lines 638-718)
    // Always extend ZCORN when padding is requested. The monotonic_zcorn flag
    // controls whether monotonic enforcement (dz < min_dist) is applied.
    {
        const bool use_monotonic = settings.monotonicZcorn();
        auto       zcorn_status_new =
            std::vector<Opm::value::status>( (size_t)( 2 * nx ) * ( 2 * ny ) * ( 2 * nz_new ), Opm::value::status::deck_value );
        auto zcorn_new = std::vector<double>( (size_t)( 2 * nx ) * ( 2 * ny ) * ( 2 * nz_new ), 0.0 );

        for ( int i = 0; i < 2 * nx; ++i )
        {
            for ( int j = 0; j < 2 * ny; ++j )
            {
                double minz = 1e20;
                double maxz = -1e20;

                for ( int k = 0; k < 2 * nz; ++k )
                {
                    const int    index = i + j * ( 2 * nx ) + k * ( 2 * nx ) * ( 2 * ny );
                    const double z     = zcorn[index];
                    if ( z > 0.0 )
                    {
                        minz = std::min( minz, z );
                        maxz = std::max( maxz, z );
                    }
                }

                const double dz_upper = ( nz_upper > 0 ) ? std::max( 0.0, ( minz - top_upper ) / nz_upper ) : 0.0;
                const double dz_lower = ( nz_lower > 0 ) ? std::max( 0.0, ( bottom_lower - maxz ) / nz_lower ) : 0.0;

                std::vector<double> zcornvert( 2 * nz_new );
                if ( nz_upper > 0 )
                {
                    zcornvert[0] = std::min( top_upper, minz );
                }
                else
                {
                    zcornvert[0] = minz;
                }

                double z_prev = zcornvert[0];
                for ( int k = 1; k < 2 * nz_new; k++ )
                {
                    const int k_old   = k - 2 * nz_upper;
                    const int ind_old = i + j * ( 2 * nx ) + k_old * ( 2 * nx ) * ( 2 * ny );

                    if ( k_old >= 0 && k_old < 2 * nz )
                    {
                        double z  = zcorn[ind_old];
                        double dz = z - z_prev;

                        if ( use_monotonic && dz < min_dist )
                        {
                            dz = 0;
                        }

                        if ( z == 0.0 )
                        {
                            dz = 0;
                        }

                        if ( no_gap )
                        {
                            if ( k % 2 == 0 )
                            {
                                dz = 0;
                            }
                        }

                        zcornvert[k] = zcornvert[k - 1] + dz;
                        z_prev       = zcornvert[k];
                    }
                    else
                    {
                        double dz_cell = dz_lower;
                        if ( k < 2 * nz_upper )
                        {
                            dz_cell = dz_upper;
                        }

                        if ( k % 2 == 1 )
                        {
                            zcornvert[k] = zcornvert[k - 1] + dz_cell;
                        }
                        else
                        {
                            zcornvert[k] = zcornvert[k - 1];
                        }

                        z_prev = zcornvert[k];
                    }
                }

                for ( int k = 0; k < 2 * nz_new; k++ )
                {
                    int ind        = i + j * ( 2 * nx ) + k * ( 2 * nx ) * ( 2 * ny );
                    zcorn_new[ind] = zcornvert[k];
                }
            }
        }

        zcorn_status = zcorn_status_new;
        zcorn        = zcorn_new;
    }
}

//--------------------------------------------------------------------------------------------------
/// Compute keyword-specific default values from saturation function tables.
/// Modeled on padmodel.cpp PropsSectionDefaultValue (lines 101-196).
//--------------------------------------------------------------------------------------------------
std::unordered_map<std::string, double> computePropsDefaultValues( const Opm::Deck& deck )
{
    using namespace std::string_literals;

    const auto tables = Opm::TableManager{ deck };
    const auto rspec  = Opm::Runspec{ deck };

    const auto rtep =
        Opm::satfunc::getRawTableEndpoints( tables, rspec.phases(), rspec.saturationFunctionControls().minimumRelpermMobilityThreshold() );

    const auto rfunc = Opm::satfunc::getRawFunctionValues( tables, rspec.phases(), rtep );

    auto cvrtPress = [&usys = deck.getActiveUnitSystem()]( const double p ) { return usys.from_si( Opm::UnitSystem::measure::pressure, p ); };

    std::unordered_map<std::string, double> defaults;
    defaults.insert( {
        // Horizontally scaled end-points for gas
        std::pair{ "SGL"s, rtep.connate.gas.front() },
        std::pair{ "SGLPC"s, rtep.connate.gas.front() },
        std::pair{ "SGCR"s, rtep.critical.gas.front() },
        std::pair{ "SGU"s, rtep.maximum.gas.front() },

        // Horizontally scaled end-points for oil
        std::pair{ "SOGCR"s, rtep.critical.oil_in_gas.front() },
        std::pair{ "SOWCR"s, rtep.critical.oil_in_water.front() },

        // Horizontally scaled end-points for water
        std::pair{ "SWL"s, rtep.connate.water.front() },
        std::pair{ "SWLPC"s, rtep.connate.water.front() },
        std::pair{ "SWCR"s, rtep.critical.water.front() },
        std::pair{ "SWU"s, rtep.maximum.water.front() },

        // Vertically scaled end-points for gas
        std::pair{ "KRGR"s, rfunc.krg.r.front() },
        std::pair{ "KRG"s, rfunc.krg.max.front() },
        std::pair{ "PCG"s, cvrtPress( rfunc.pc.g.front() ) },

        // Vertically scaled end-points for oil
        std::pair{ "KRORG"s, rfunc.kro.rg.front() },
        std::pair{ "KRORW"s, rfunc.kro.rw.front() },
        std::pair{ "KRO"s, rfunc.kro.max.front() },

        // Vertically scaled end-points for water
        std::pair{ "KRWR"s, rfunc.krw.r.front() },
        std::pair{ "KRW"s, rfunc.krw.max.front() },
        std::pair{ "PCW"s, cvrtPress( rfunc.pc.w.front() ) },
    } );

    return defaults;
}

//--------------------------------------------------------------------------------------------------
/// Look up the default value for a PROPS keyword, handling imbibition (I-prefix)
/// and directional (X/Y/Z suffix) variants via regex stripping.
/// Falls back to 0.0 for unknown keywords, except SWATINIT which defaults to 1.0.
//--------------------------------------------------------------------------------------------------
static const std::unordered_map<std::string, double> fallbackDefaults = { { "SWATINIT", 1.0 } };

static const std::regex epsKwRegEx( R"(I?([SKP].+?)([XYZ]-?)?$)" );

double lookupPropsDefault( const std::unordered_map<std::string, double>& defaults, const std::string& keyword )
{
    std::smatch m;
    if ( std::regex_match( keyword, m, epsKwRegEx ) )
    {
        auto it = defaults.find( m[1].str() );
        if ( it != defaults.end() )
        {
            return it->second;
        }
    }

    auto fbIt = fallbackDefaults.find( keyword );
    return ( fbIt != fallbackDefaults.end() ) ? fbIt->second : 0.0;
}

//--------------------------------------------------------------------------------------------------
/// Extend PROPS section data keywords (padmodel lines 721-738)
//--------------------------------------------------------------------------------------------------
void extendProps( const GridSize& gridSize, const RigModelPaddingSettings& settings, Opm::FileDeck& fileDeck, const Opm::Deck* deck )
{
    // Compute keyword-specific defaults from saturation function tables if Deck is available
    std::unordered_map<std::string, double> propsDefaults;
    if ( deck != nullptr )
    {
        try
        {
            propsDefaults = computePropsDefaultValues( *deck );
        }
        catch ( ... )
        {
            // If table computation fails, fall back to empty map (will use 0.0 defaults)
        }
    }

    bool inPropsSection = false;

    for ( auto it = fileDeck.start(); it != fileDeck.stop(); ++it )
    {
        const auto& keyword = fileDeck[it];

        if ( keyword.name() == "PROPS" )
        {
            inPropsSection = true;
            continue;
        }
        if ( inPropsSection && isSectionKeyword( keyword.name() ) ) break;
        if ( !inPropsSection ) continue;

        if ( keyword.size() != 1 || !keyword.isDataKeyword() ) continue;

        // Only extend double-valued data keywords
        const auto& item = keyword.getRecord( 0 ).getItem( 0 );
        if ( item.getType() != Opm::type_tag::fdouble ) continue;

        const auto input_nc = static_cast<size_t>( gridSize.x() * gridSize.y() * gridSize.z() );
        if ( item.data_size() != input_nc ) continue;

        double default_value = ( deck != nullptr ) ? lookupPropsDefault( propsDefaults, keyword.name() ) : 0.0;

        extendPropertyArray( gridSize, settings, Opm::type_tag::fdouble, default_value, item );
    }
}

//--------------------------------------------------------------------------------------------------
/// Extend region int arrays with actnum_old filtering (padmodel lines 741-811)
//--------------------------------------------------------------------------------------------------
void extendRegions( Opm::FileDeck&                 fileDeck,
                    const GridSize&                gridSize,
                    const RigModelPaddingSettings& settings,
                    const std::vector<int>&        actnum_old,
                    int                            upperEquilnum )
{
    const auto input_nc = static_cast<size_t>( gridSize.x() * gridSize.y() * gridSize.z() );

    const auto output_nc = static_cast<std::size_t>( gridSize.x() ) * static_cast<std::size_t>( gridSize.y() ) *
                           static_cast<std::size_t>( settings.nzUpper() + gridSize.z() + settings.nzLower() );

    const auto value_offset = static_cast<std::size_t>( gridSize.x() ) * static_cast<std::size_t>( gridSize.y() ) *
                              static_cast<std::size_t>( settings.nzUpper() );

    bool inRegionsSection = false;

    for ( auto it = fileDeck.start(); it != fileDeck.stop(); ++it )
    {
        const auto& keyword = fileDeck[it];

        if ( keyword.name() == "REGIONS" )
        {
            inRegionsSection = true;
            continue;
        }
        if ( inRegionsSection && isSectionKeyword( keyword.name() ) ) break;
        if ( !inRegionsSection ) continue;

        if ( keyword.size() != 1 || !keyword.isDataKeyword() ) continue;

        const auto& record = keyword.getRecord( 0 );
        if ( record.size() != 1 ) continue;

        const auto& item = record.getItem( 0 );
        if ( item.getType() != Opm::type_tag::integer || item.data_size() != input_nc ) continue;

        auto& value = const_cast<Opm::DeckItem&>( item ).getData<int>();

        const auto default_value = [&value, upperEquilnum, is_eqlnum = keyword.name() == "EQLNUM"]()
        {
            if ( is_eqlnum )
            {
                return upperEquilnum;
            }

            return std::max( 1, *std::min_element( value.begin(), value.end() ) );
        }();

        auto& status = const_cast<std::vector<Opm::value::status>&>( item.getValueStatus() );

        auto remapped_value  = std::vector<int>( output_nc, default_value );
        auto remapped_status = std::vector<Opm::value::status>( output_nc, Opm::value::status::deck_value );

        for ( auto i = decltype( input_nc ){ 0 }; i != input_nc; ++i )
        {
            if ( actnum_old[i] == 1 )
            {
                remapped_value[value_offset + i]  = value[i];
                remapped_status[value_offset + i] = status[i];
            }
        }

        value.swap( remapped_value );
        status.swap( remapped_status );
    }
}

//--------------------------------------------------------------------------------------------------
/// Add EQUIL record with GOC=WOC=0 (padmodel lines 813-830)
//--------------------------------------------------------------------------------------------------
void extendEQUIL( Opm::DeckKeyword& equil )
{
    using Kw = Opm::ParserKeywords::EQUIL;

    assert( !equil.empty() );

    auto record = equil.getRecord( 0 );

    double& goc = const_cast<double&>( record.getItem<Kw::GOC>().getData<double>()[0] );
    double& woc = const_cast<double&>( record.getItem<Kw::OWC>().getData<double>()[0] );

    goc = woc = 0.0;

    equil.addRecord( std::move( record ) );
}

} // namespace

//--------------------------------------------------------------------------------------------------
/// Extend RSVD/RVVD/RTEMPVD/PBVD/PDVD tables (padmodel lines 832-867)
//--------------------------------------------------------------------------------------------------
void RigPadModel::extendDepthTable( const RigModelPaddingSettings& settings, Opm::DeckKeyword& propertyVD )
{
    assert( !propertyVD.empty() );

    auto newPropertyVD = propertyVD.emptyStructuralCopy();

    for ( size_t t = 0; t < propertyVD.size(); ++t )
    {
        const auto& table = propertyVD.getRecord( t );
        auto        i     = std::vector{ table.getItem( 0 ).emptyStructuralCopy() };

        const auto& values = table.getItem( 0 ).getData<double>();

        // Need at least one depth-property pair for safe indexing
        if ( values.size() < 2 ) continue;

        if ( settings.topUpper() < values.front() )
        {
            i.front().push_back( settings.topUpper() );
            i.front().push_back( values[0 + 1] );
        }

        for ( const auto& value : values )
        {
            i.front().push_back( value );
        }

        if ( settings.bottomLower() > values[values.size() - 2] )
        {
            i.front().push_back( settings.bottomLower() );
            i.front().push_back( values.back() );
        }

        newPropertyVD.addRecord( Opm::DeckRecord{ std::move( i ) } );
    }

    {
        auto first = newPropertyVD.getRecord( 0 );
        newPropertyVD.addRecord( std::move( first ) );
    }

    propertyVD = std::move( newPropertyVD );
}

namespace
{

//--------------------------------------------------------------------------------------------------
/// Extend EQUIL + depth tables in SOLUTION section (padmodel lines 869-886)
//--------------------------------------------------------------------------------------------------
void extendSolution( const RigModelPaddingSettings& settings, Opm::FileDeck& fileDeck )
{
    bool inSolutionSection = false;

    for ( auto it = fileDeck.start(); it != fileDeck.stop(); ++it )
    {
        const auto& keyword = fileDeck[it];

        if ( keyword.name() == "SOLUTION" )
        {
            inSolutionSection = true;
            continue;
        }
        if ( inSolutionSection && isSectionKeyword( keyword.name() ) ) break;
        if ( !inSolutionSection ) continue;

        if ( keyword.name() == "EQUIL" )
        {
            extendEQUIL( const_cast<Opm::DeckKeyword&>( keyword ) );
        }
        else if ( keyword.name() == "RSVD" || keyword.name() == "RVVD" || keyword.name() == "RTEMPVD" || keyword.name() == "PBVD" ||
                  keyword.name() == "PDVD" )
        {
            RigPadModel::extendDepthTable( settings, const_cast<Opm::DeckKeyword&>( keyword ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Shift COMPDAT K1/K2, COMPSEGS K, WSEED K
//--------------------------------------------------------------------------------------------------
void extendSchedule( Opm::FileDeck& fileDeck, const RigModelPaddingSettings& settings )
{
    const int nz_upper = settings.nzUpper();

    bool inScheduleSection = false;

    for ( auto it = fileDeck.start(); it != fileDeck.stop(); ++it )
    {
        const auto& records = fileDeck[it];

        if ( records.name() == "SCHEDULE" )
        {
            inScheduleSection = true;
            continue;
        }
        if ( !inScheduleSection ) continue;

        if ( records.name() != "COMPDAT" && records.name() != "WSEED" && records.name() != "COMPSEGS" )
        {
            continue;
        }

        if ( records.name() == "WSEED" )
        {
            for ( size_t r = 0; r < records.size(); ++r )
            {
                const auto& record = records.getRecord( r );
                int&        K      = const_cast<int&>( record.getItem( "K" ).getData<int>()[0] );
                K += nz_upper;
            }
        }
        else if ( records.name() == "COMPSEGS" )
        {
            bool is_first = true;
            for ( size_t r = 0; r < records.size(); ++r )
            {
                if ( is_first )
                {
                    is_first = false;
                    continue;
                }
                const auto& record = records.getRecord( r );
                int&        K      = const_cast<int&>( record.getItem( "K" ).getData<int>()[0] );
                K += nz_upper;
            }
        }
        else
        {
            // COMPDAT
            for ( size_t r = 0; r < records.size(); ++r )
            {
                const auto& record = records.getRecord( r );
                int&        K1     = const_cast<int&>( record.getItem( "K1" ).getData<int>()[0] );
                int&        K2     = const_cast<int&>( record.getItem( "K2" ).getData<int>()[0] );
                K1 += nz_upper;
                K2 += nz_upper;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Get ACTNUM before property extension for region filtering
//--------------------------------------------------------------------------------------------------
std::vector<int> getActnumOld( Opm::FileDeck& fileDeck, const GridSize& gridSize )
{
    bool inGridSection = false;

    for ( auto it = fileDeck.start(); it != fileDeck.stop(); ++it )
    {
        const auto& kw = fileDeck[it];

        if ( kw.name() == "GRID" )
        {
            inGridSection = true;
            continue;
        }
        if ( inGridSection && isSectionKeyword( kw.name() ) ) break;
        if ( !inGridSection ) continue;

        if ( kw.name() == "ACTNUM" && kw.isDataKeyword() )
        {
            return kw.getIntData();
        }
    }

    // No ACTNUM: all cells active
    return std::vector<int>( static_cast<size_t>( gridSize.x() ) * gridSize.y() * gridSize.z(), 1 );
}

//--------------------------------------------------------------------------------------------------
/// Check if a section exists in the FileDeck
//--------------------------------------------------------------------------------------------------
bool hasSection( Opm::FileDeck& fileDeck, const std::string& sectionName )
{
    return fileDeck.find( sectionName ).has_value();
}

} // namespace

//--------------------------------------------------------------------------------------------------
/// Main entry point (matching padmodel's manipulate_deck() lines 931-1024)
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigPadModel::extendGrid( RifOpmFlowDeckFile& deckFile, const RigModelPaddingSettings& settings )
{
    if ( !settings.isEnabled() )
    {
        return {};
    }

    auto* fileDeck = deckFile.fileDeck();
    if ( fileDeck == nullptr )
    {
        return std::unexpected( "No FileDeck available for model padding" );
    }

    auto gridSizeResult = getDimens( *fileDeck );
    if ( !gridSizeResult.has_value() )
    {
        return std::unexpected( gridSizeResult.error() );
    }

    const auto gridSize = gridSizeResult.value();

    if ( settings.nzUpper() == 0 && settings.nzLower() == 0 )
    {
        RiaLogging::info( "Model padding enabled but no padding layers specified - skipping" );
        return {};
    }

    const int nzNew = static_cast<int>( gridSize.z() ) + settings.nzUpper() + settings.nzLower();

    RiaLogging::info(
        QString( "Applying model padding: %1 upper layers, %2 lower layers" ).arg( settings.nzUpper() ).arg( settings.nzLower() ) );
    RiaLogging::info(
        QString( "Grid dimensions: %1 x %2 x %3 -> %1 x %2 x %4" ).arg( gridSize.x() ).arg( gridSize.y() ).arg( gridSize.z() ).arg( nzNew ) );

    // Save actnum before any extensions for region filtering
    auto actnumOld = getActnumOld( *fileDeck, gridSize );

    // Step 1: DIMENS, EQLDIMS, SPECGRID
    const int upperEquilnum = extendDimens( gridSize, settings, *fileDeck, nzNew );

    // Step 2: BCCON K-shift
    extendBCCon( *fileDeck, settings );

    // Step 3: Extend GRID section data arrays (double and int)
    extendGridSection<double>( *fileDeck, gridSize, settings, Opm::type_tag::fdouble );
    extendGridSection<int>( *fileDeck, gridSize, settings, Opm::type_tag::integer );

    // Step 4: Special grid properties (ACTNUM + inactive cell handling)
    extendSpecialGridProperties( *fileDeck, gridSize, settings );

    // Step 5: FAULTS K-shift
    extendFaults( settings, *fileDeck );

    // Step 6: COORD + ZCORN per-column
    extendGRDECL( *fileDeck, gridSize, settings, nzNew );

    // Step 7: Extend PROPS section arrays
    if ( hasSection( *fileDeck, "PROPS" ) )
    {
        extendProps( gridSize, settings, *fileDeck, deckFile.deck() );
    }

    // Step 8: Extend REGIONS with actnum filtering
    if ( hasSection( *fileDeck, "REGIONS" ) )
    {
        extendRegions( *fileDeck, gridSize, settings, actnumOld, upperEquilnum );
    }

    // Step 9: Extend SOLUTION (EQUIL + depth tables)
    if ( hasSection( *fileDeck, "SOLUTION" ) )
    {
        extendSolution( settings, *fileDeck );
    }

    // Step 10: Extend SCHEDULE (COMPDAT/COMPSEGS/WSEED)
    if ( hasSection( *fileDeck, "SCHEDULE" ) )
    {
        extendSchedule( *fileDeck, settings );
    }

    // Step 11: Operators K-shift (ADD/BOX/EQUALS etc.)
    extendOperators( settings, *fileDeck );

    RiaLogging::info( QString( "Model padding applied successfully: %1 x %2 x %3" ).arg( gridSize.x() ).arg( gridSize.y() ).arg( nzNew ) );

    return {};
}

//--------------------------------------------------------------------------------------------------
/// Extend a property array with default values for padding layers (exposed for testing)
//--------------------------------------------------------------------------------------------------
std::vector<double> RigPadModel::extendPropertyArray( const std::vector<double>& original,
                                                      int                        nx,
                                                      int                        ny,
                                                      int                        nz,
                                                      int                        nzUpper,
                                                      int                        nzLower,
                                                      double                     upperDefault,
                                                      double                     lowerDefault )
{
    int    xyCount = nx * ny;
    int    newNz   = nz + nzUpper + nzLower;
    size_t newSize = static_cast<size_t>( xyCount ) * newNz;

    std::vector<double> result;
    result.reserve( newSize );

    for ( int k = 0; k < nzUpper; k++ )
    {
        for ( int i = 0; i < xyCount; i++ )
        {
            result.push_back( upperDefault );
        }
    }

    result.insert( result.end(), original.begin(), original.end() );

    for ( int k = 0; k < nzLower; k++ )
    {
        for ( int i = 0; i < xyCount; i++ )
        {
            result.push_back( lowerDefault );
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/// Public static: compute the proper default value for a PROPS keyword from the Deck's
/// saturation function tables. Used by extendProps() and exposed for unit testing.
//--------------------------------------------------------------------------------------------------
double RigPadModel::getPropsDefaultValue( const Opm::Deck& deck, const std::string& keyword )
{
    auto defaults = computePropsDefaultValues( deck );
    return lookupPropsDefault( defaults, keyword );
}
