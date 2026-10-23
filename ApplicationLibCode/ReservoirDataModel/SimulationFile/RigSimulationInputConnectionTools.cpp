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

#include "RigSimulationInputConnectionTools.h"

#include "RiaLogging.h"

#include "RifOpmDeckTools.h"

#include "RimEclipseCase.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigGridExportAdapter.h"
#include "RigMainGrid.h"
#include "RigSimulationInputSettings.h"
#include "RigSimulationInputTool.h"
#include "RigTransmissibilityEquations.h"

#include "opm/input/eclipse/Deck/DeckItem.hpp"
#include "opm/input/eclipse/Deck/DeckKeyword.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/C.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <tuple>

//--------------------------------------------------------------------------------------------------
/// Refined sub-cell index and its length fraction of the original cell
//--------------------------------------------------------------------------------------------------
namespace
{
struct RefinedSubcell
{
    size_t index    = 0;
    double fraction = 1.0;
};
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static size_t centerSubcellIndex( const RigRefinement& refinement, RigRefinement::Dimension dim, size_t sectorIndex )
{
    return refinement.cumulativeOffset( dim, sectorIndex ) + refinement.subcellCount( dim, sectorIndex ) / 2;
}

//--------------------------------------------------------------------------------------------------
/// All refined sub-cells of an original (sector-relative) cell index, with length fractions
//--------------------------------------------------------------------------------------------------
static std::vector<RefinedSubcell> allSubcells( const RigRefinement& refinement, RigRefinement::Dimension dim, size_t sectorIndex )
{
    const size_t offset     = refinement.cumulativeOffset( dim, sectorIndex );
    const auto&  cumulative = refinement.cumulativeFractions( dim, sectorIndex );

    std::vector<RefinedSubcell> subcells;
    double                      previous = 0.0;
    for ( size_t s = 0; s < cumulative.size(); ++s )
    {
        subcells.push_back( { offset + s, cumulative[s] - previous } );
        previous = cumulative[s];
    }
    return subcells;
}

//--------------------------------------------------------------------------------------------------
/// Penetration direction from COMPDAT DIR item (X -> I, Y -> J, Z or default -> K)
//--------------------------------------------------------------------------------------------------
static RigRefinement::Dimension compdatPenetrationDimension( const Opm::DeckRecord& record )
{
    using C = Opm::ParserKeywords::COMPDAT;
    if ( record.hasItem( C::DIR::itemName ) )
    {
        const auto& item = record.getItem( C::DIR::itemName );
        if ( item.hasValue( 0 ) )
        {
            const auto dir = item.get<std::string>( 0 );
            if ( !dir.empty() && ( dir[0] == 'X' || dir[0] == 'x' ) ) return RigRefinement::DimI;
            if ( !dir.empty() && ( dir[0] == 'Y' || dir[0] == 'y' ) ) return RigRefinement::DimJ;
        }
    }
    return RigRefinement::DimK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static bool hasExplicitDoubleValue( const Opm::DeckRecord& record, const std::string& itemName )
{
    if ( !record.hasItem( itemName ) ) return false;
    const auto& item = record.getItem( itemName );
    return item.hasValue( 0 ) && !item.defaultApplied( 0 );
}

//--------------------------------------------------------------------------------------------------
/// Length fraction of the center sub-cell of an original (sector-relative) cell index
//--------------------------------------------------------------------------------------------------
static double centerSubcellFraction( const RigRefinement& refinement, RigRefinement::Dimension dim, size_t sectorIndex )
{
    const auto&  cumulative = refinement.cumulativeFractions( dim, sectorIndex );
    const size_t center     = refinement.subcellCount( dim, sectorIndex ) / 2;
    if ( center >= cumulative.size() ) return 1.0;
    return cumulative[center] - ( center > 0 ? cumulative[center - 1] : 0.0 );
}

namespace
{
struct LateralRefinementCorrection
{
    double connectionFactorMultiplier = 1.0;
    double pressureEquivalentRadius   = 0.0;
};
} // namespace

//--------------------------------------------------------------------------------------------------
/// Correction of an explicit connection factor when the cell is refined perpendicular to the well.
/// Peaceman: CF = c * theta * K * h / ( ln( r0 / rw ) + S ). Lateral refinement keeps K * h, but reduces
/// the pressure equivalent radius r0, so CF_new = CF_old * ( ln( r0_old / rw ) + S ) / ( ln( r0_new / rw ) + S ).
/// r0_old is taken from the PR item if given, otherwise computed from the original cell.
//--------------------------------------------------------------------------------------------------
static std::optional<LateralRefinementCorrection>
    lateralRefinementCorrection( const Opm::DeckRecord&                                                            record,
                                 const std::string&                                                                wellName,
                                 RigRefinement::Dimension                                                          penetrationDim,
                                 double                                                                            fraction1,
                                 double                                                                            fraction2,
                                 const std::optional<RigSimulationInputConnectionTools::ConnectionCellProperties>& cellProperties )
{
    using C = Opm::ParserKeywords::COMPDAT;

    if ( fraction1 >= 1.0 && fraction2 >= 1.0 ) return std::nullopt;
    if ( !hasExplicitDoubleValue( record, C::CONNECTION_TRANSMISSIBILITY_FACTOR::itemName ) ) return std::nullopt;

    auto warning = [&]( const std::string& reason )
    {
        RiaLogging::warning(
            std::format( "COMPDAT for well {}: connection factor not corrected for refinement perpendicular to the well ({})", wellName, reason ) );
    };

    if ( !hasExplicitDoubleValue( record, C::DIAMETER::itemName ) )
    {
        warning( "no well diameter" );
        return std::nullopt;
    }
    const double wellRadius = 0.5 * record.getItem( C::DIAMETER::itemName ).get<double>( 0 );

    double skin = 0.0;
    if ( record.hasItem( C::SKIN::itemName ) && record.getItem( C::SKIN::itemName ).hasValue( 0 ) )
    {
        skin = record.getItem( C::SKIN::itemName ).get<double>( 0 );
    }

    // Peaceman radius of the original and refined cell, using the cell sizes and permeabilities perpendicular to the well
    std::optional<std::pair<double, double>> computedRadii;
    if ( cellProperties )
    {
        const int d1 = ( penetrationDim == RigRefinement::DimI ) ? 1 : 0;
        const int d2 = ( penetrationDim == RigRefinement::DimK ) ? 1 : 2;

        const double size1 = cellProperties->cellSize[d1];
        const double size2 = cellProperties->cellSize[d2];
        double       perm1 = cellProperties->permeability[d1];
        double       perm2 = cellProperties->permeability[d2];
        if ( perm1 <= 0.0 || perm2 <= 0.0 )
        {
            perm1 = 1.0;
            perm2 = 1.0;
        }

        if ( size1 > 0.0 && size2 > 0.0 )
        {
            const double coarse = RigTransmissibilityEquations::peacemanRadius( perm1, perm2, size1, size2 );
            const double fine   = RigTransmissibilityEquations::peacemanRadius( perm1, perm2, fraction1 * size1, fraction2 * size2 );
            computedRadii       = std::make_pair( coarse, fine );
        }
    }

    double originalRadius = 0.0;
    if ( hasExplicitDoubleValue( record, C::PR::itemName ) )
    {
        originalRadius = record.getItem( C::PR::itemName ).get<double>( 0 );
    }
    else if ( computedRadii )
    {
        originalRadius = computedRadii->first;
    }
    else
    {
        warning( "no pressure equivalent radius or cell geometry" );
        return std::nullopt;
    }

    // Peaceman radius is homogeneous in the cell sizes, so the ratio is exact when both directions are refined equally
    const double radiusRatio   = computedRadii ? computedRadii->second / computedRadii->first
                                               : std::sqrt( 0.5 * ( fraction1 * fraction1 + fraction2 * fraction2 ) );
    const double refinedRadius = originalRadius * radiusRatio;

    const double originalDenominator = std::log( originalRadius / wellRadius ) + skin;
    const double refinedDenominator  = std::log( refinedRadius / wellRadius ) + skin;
    if ( wellRadius <= 0.0 || originalDenominator <= 0.0 || refinedDenominator <= 0.0 )
    {
        warning(
            std::format( "refined pressure equivalent radius {:.4g} is too small compared to well radius {:.4g}", refinedRadius, wellRadius ) );
        return std::nullopt;
    }

    return LateralRefinementCorrection{ originalDenominator / refinedDenominator, refinedRadius };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigSimulationInputConnectionTools::ConnectionCellPropertiesProvider
    RigSimulationInputConnectionTools::connectionCellPropertiesProvider( RimEclipseCase* eclipseCase )
{
    if ( !eclipseCase || !eclipseCase->eclipseCaseData() || !eclipseCase->eclipseCaseData()->mainGrid() ) return {};

    auto* caseData       = eclipseCase->eclipseCaseData();
    auto* results        = caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    auto* activeCellInfo = caseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    std::array<const std::vector<double>*, 3> permeabilities = { nullptr, nullptr, nullptr };
    if ( results && activeCellInfo )
    {
        const std::array<QString, 3> permNames = { "PERMX", "PERMY", "PERMZ" };
        for ( size_t i = 0; i < 3; ++i )
        {
            RigEclipseResultAddress address( RiaDefines::ResultCatType::STATIC_NATIVE, permNames[i] );
            if ( results->ensureKnownResultLoaded( address ) && !results->cellScalarResults( address ).empty() )
            {
                permeabilities[i] = &results->cellScalarResults( address, 0 );
            }
        }
    }

    const RigMainGrid* mainGrid = caseData->mainGrid();

    return [mainGrid, activeCellInfo, permeabilities]( const caf::VecIjk0& ijk ) -> std::optional<ConnectionCellProperties>
    {
        if ( !mainGrid->isCellValid( ijk.i(), ijk.j(), ijk.k() ) ) return std::nullopt;

        const size_t   cellIndex = mainGrid->cellIndexFromIJK( ijk.i(), ijk.j(), ijk.k() );
        const RigCell& cell      = mainGrid->cell( cellIndex );
        if ( cell.isInvalid() ) return std::nullopt;

        using F = cvf::StructGridInterface;
        ConnectionCellProperties properties;
        properties.cellSize = cvf::Vec3d( ( cell.faceCenter( F::NEG_I ) - cell.faceCenter( F::POS_I ) ).length(),
                                          ( cell.faceCenter( F::NEG_J ) - cell.faceCenter( F::POS_J ) ).length(),
                                          ( cell.faceCenter( F::NEG_K ) - cell.faceCenter( F::POS_K ) ).length() );

        properties.permeability = cvf::Vec3d( 1.0, 1.0, 1.0 );
        if ( activeCellInfo && permeabilities[0] && permeabilities[1] && permeabilities[2] )
        {
            const size_t resultIndex = activeCellInfo->cellResultIndex( ReservoirCellIndex( cellIndex ) ).value();
            if ( resultIndex < permeabilities[0]->size() && resultIndex < permeabilities[1]->size() && resultIndex < permeabilities[2]->size() )
            {
                properties.permeability =
                    cvf::Vec3d( ( *permeabilities[0] )[resultIndex], ( *permeabilities[1] )[resultIndex], ( *permeabilities[2] )[resultIndex] );
            }
        }

        return properties;
    };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigSimulationInputConnectionTools::ConnectionKey::operator<( const ConnectionKey& other ) const
{
    return std::make_tuple( wellName, ijk.x(), ijk.y(), ijk.z() ) <
           std::make_tuple( other.wellName, other.ijk.x(), other.ijk.y(), other.ijk.z() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigSimulationInputConnectionTools::ConnectionDirectionMap
    RigSimulationInputConnectionTools::connectionDirections( const std::vector<Opm::DeckKeyword>& compdatKeywords )
{
    ConnectionDirectionMap directions;

    for ( const auto& keyword : compdatKeywords )
    {
        for ( size_t r = 0; r < keyword.size(); ++r )
        {
            const auto& record = keyword.getRecord( r );
            if ( record.size() < 5 ) continue;

            const auto& wellItem = record.getItem( 0 );
            if ( !wellItem.hasValue( 0 ) || wellItem.getType() != Opm::type_tag::string ) continue;

            const auto wellName = wellItem.get<std::string>( 0 );
            const int  i        = record.getItem( 1 ).get<int>( 0 ) - 1;
            const int  j        = record.getItem( 2 ).get<int>( 0 ) - 1;
            const int  k1       = record.getItem( 3 ).get<int>( 0 ) - 1;
            const int  k2       = record.getItem( 4 ).get<int>( 0 ) - 1;
            if ( i < 0 || j < 0 || k1 < 0 || k2 < k1 ) continue;

            const auto dim = compdatPenetrationDimension( record );
            for ( int k = k1; k <= k2; ++k )
            {
                directions[{ wellName, caf::VecIjk0( i, j, k ) }] = dim;
            }
        }
    }

    return directions;
}

//--------------------------------------------------------------------------------------------------
/// Cells where the well travels towards decreasing index along the connection penetration direction,
/// determined from the neighbouring COMPSEGS cells on the same branch sorted by measured depth
//--------------------------------------------------------------------------------------------------
RigSimulationInputConnectionTools::ConnectionKeySet
    RigSimulationInputConnectionTools::reversedSegmentCells( const std::vector<Opm::DeckKeyword>& compsegsKeywords,
                                                             const ConnectionDirectionMap&        directions )
{
    struct SegmentCell
    {
        double       startMd = 0.0;
        caf::VecIjk0 ijk;
    };

    ConnectionKeySet reversed;

    for ( const auto& keyword : compsegsKeywords )
    {
        if ( keyword.size() < 2 ) continue;

        const auto& wellItem = keyword.getRecord( 0 ).getItem( 0 );
        if ( !wellItem.hasValue( 0 ) || wellItem.getType() != Opm::type_tag::string ) continue;
        const auto wellName = wellItem.get<std::string>( 0 );

        std::map<int, std::vector<SegmentCell>> cellsPerBranch;
        for ( size_t r = 1; r < keyword.size(); ++r )
        {
            const auto& record = keyword.getRecord( r );
            if ( record.size() < 6 ) continue;
            if ( !record.getItem( 0 ).hasValue( 0 ) || !record.getItem( 1 ).hasValue( 0 ) || !record.getItem( 2 ).hasValue( 0 ) ) continue;

            const int i = record.getItem( 0 ).get<int>( 0 ) - 1;
            const int j = record.getItem( 1 ).get<int>( 0 ) - 1;
            const int k = record.getItem( 2 ).get<int>( 0 ) - 1;
            if ( i < 0 || j < 0 || k < 0 ) continue;

            const int    branch  = record.getItem( 3 ).hasValue( 0 ) ? record.getItem( 3 ).get<int>( 0 ) : 0;
            const double startMd = record.getItem( 4 ).hasValue( 0 ) ? record.getItem( 4 ).get<double>( 0 ) : 0.0;
            cellsPerBranch[branch].push_back( { startMd, caf::VecIjk0( i, j, k ) } );
        }

        for ( auto& [branch, cells] : cellsPerBranch )
        {
            std::stable_sort( cells.begin(), cells.end(), []( const SegmentCell& a, const SegmentCell& b ) { return a.startMd < b.startMd; } );

            for ( size_t c = 0; c < cells.size(); ++c )
            {
                const auto& ijk = cells[c].ijk;
                const auto  key = ConnectionKey{ wellName, ijk };

                auto dim = RigRefinement::DimK;
                if ( auto it = directions.find( key ); it != directions.end() ) dim = it->second;

                // Prefer the next cell along the well, fall back to the previous cell
                std::optional<bool> isReversed;
                if ( c + 1 < cells.size() && cells[c + 1].ijk[dim] != ijk[dim] )
                {
                    isReversed = cells[c + 1].ijk[dim] < ijk[dim];
                }
                else if ( c > 0 && cells[c - 1].ijk[dim] != ijk[dim] )
                {
                    isReversed = ijk[dim] < cells[c - 1].ijk[dim];
                }

                if ( isReversed.value_or( false ) ) reversed.insert( key );
            }
        }
    }

    return reversed;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<std::vector<Opm::DeckRecord>, QString>
    RigSimulationInputConnectionTools::processCompdatRecord( const Opm::DeckRecord&                  record,
                                                             const std::string&                      wellName,
                                                             const RigSimulationInputSettings&       settings,
                                                             const ConnectionCellPropertiesProvider& cellProperties )
{
    // COMPDAT format: WELL I J K1 K2 STATE ...
    // Items: 0=WELL, 1=I, 2=J, 3=K1, 4=K2
    if ( record.size() < 5 )
    {
        return std::unexpected( QString( "COMPDAT record for well %1 has insufficient items (expected at least 5, got %2)" )
                                    .arg( wellName.c_str() )
                                    .arg( record.size() ) );
    }

    int origI  = record.getItem( 1 ).get<int>( 0 ) - 1; // Convert to 0-based
    int origJ  = record.getItem( 2 ).get<int>( 0 ) - 1;
    int origK1 = record.getItem( 3 ).get<int>( 0 ) - 1;
    int origK2 = record.getItem( 4 ).get<int>( 0 ) - 1;

    const auto& refinement = settings.refinement();

    auto transformResultK1 = RigGridExportAdapter::transformIjkToSectorCoordinates( caf::VecIjk0( origI, origJ, origK1 ),
                                                                                    settings.min(),
                                                                                    settings.max(),
                                                                                    refinement,
                                                                                    true );
    if ( !transformResultK1 )
    {
        return std::unexpected(
            QString( "COMPDAT K1 coordinate for well %1 is out of sector bounds: %2" ).arg( wellName.c_str() ).arg( transformResultK1.error() ) );
    }

    auto transformResultK2 = RigGridExportAdapter::transformIjkToSectorCoordinates( caf::VecIjk0( origI, origJ, origK2 ),
                                                                                    settings.min(),
                                                                                    settings.max(),
                                                                                    refinement,
                                                                                    true );
    if ( !transformResultK2 )
    {
        return std::unexpected(
            QString( "COMPDAT K2 coordinate for well %1 is out of sector bounds: %2" ).arg( wellName.c_str() ).arg( transformResultK2.error() ) );
    }

    if ( origK2 < origK1 )
    {
        return std::unexpected(
            QString( "COMPDAT record for well %1 has K2 (%2) < K1 (%3)" ).arg( wellName.c_str() ).arg( origK2 + 1 ).arg( origK1 + 1 ) );
    }

    using C = Opm::ParserKeywords::COMPDAT;

    const auto   dim      = compdatPenetrationDimension( record );
    const size_t sectorI  = origI - settings.min().x();
    const size_t sectorJ  = origJ - settings.min().y();
    const size_t sectorK1 = origK1 - settings.min().z();
    const size_t sectorK2 = origK2 - settings.min().z();

    const bool hasExplicitFactors = hasExplicitDoubleValue( record, C::CONNECTION_TRANSMISSIBILITY_FACTOR::itemName ) ||
                                    hasExplicitDoubleValue( record, C::Kh::itemName );

    bool isKRefinedInRange = false;
    for ( size_t k = sectorK1; k <= sectorK2; ++k )
    {
        if ( refinement.subcellCount( RigRefinement::DimK, k ) > 1 ) isKRefinedInRange = true;
    }

    struct KSpan
    {
        size_t k1       = 0;
        size_t k2       = 0;
        double fraction = 1.0;
        size_t sectorK  = 0; // Original (sector-relative) K, used for the lateral refinement correction
    };

    std::vector<KSpan> kSpans;
    if ( !isKRefinedInRange )
    {
        kSpans.push_back( { refinement.cumulativeOffset( RigRefinement::DimK, sectorK1 ),
                            refinement.cumulativeOffset( RigRefinement::DimK, sectorK2 ),
                            1.0,
                            sectorK1 } );
    }
    else if ( dim == RigRefinement::DimK && !hasExplicitFactors )
    {
        // The simulator computes the connection factors, so the full refined range can be given in one record
        kSpans.push_back( { refinement.cumulativeOffset( RigRefinement::DimK, sectorK1 ),
                            refinement.cumulativeOffset( RigRefinement::DimK, sectorK2 + 1 ) - 1,
                            1.0,
                            sectorK1 } );
    }
    else if ( dim == RigRefinement::DimK )
    {
        for ( size_t k = sectorK1; k <= sectorK2; ++k )
        {
            for ( const auto& subcell : allSubcells( refinement, RigRefinement::DimK, k ) )
            {
                kSpans.push_back( { subcell.index, subcell.index, subcell.fraction, k } );
            }
        }
    }
    else
    {
        for ( size_t k = sectorK1; k <= sectorK2; ++k )
        {
            const size_t centerK = centerSubcellIndex( refinement, RigRefinement::DimK, k );
            kSpans.push_back( { centerK, centerK, 1.0, k } );
        }
    }

    const std::vector<RefinedSubcell> iSubcells =
        ( dim == RigRefinement::DimI )
            ? allSubcells( refinement, RigRefinement::DimI, sectorI )
            : std::vector<RefinedSubcell>{ { centerSubcellIndex( refinement, RigRefinement::DimI, sectorI ), 1.0 } };
    const std::vector<RefinedSubcell> jSubcells =
        ( dim == RigRefinement::DimJ )
            ? allSubcells( refinement, RigRefinement::DimJ, sectorJ )
            : std::vector<RefinedSubcell>{ { centerSubcellIndex( refinement, RigRefinement::DimJ, sectorJ ), 1.0 } };

    // Refinement perpendicular to the well, applies to the center sub-cell
    auto lateralCorrection = [&]( size_t sectorK ) -> std::optional<LateralRefinementCorrection>
    {
        double fraction1 = 1.0;
        double fraction2 = 1.0;
        if ( dim == RigRefinement::DimK )
        {
            fraction1 = centerSubcellFraction( refinement, RigRefinement::DimI, sectorI );
            fraction2 = centerSubcellFraction( refinement, RigRefinement::DimJ, sectorJ );
        }
        else
        {
            fraction1 = ( dim == RigRefinement::DimI ) ? centerSubcellFraction( refinement, RigRefinement::DimJ, sectorJ )
                                                       : centerSubcellFraction( refinement, RigRefinement::DimI, sectorI );
            fraction2 = centerSubcellFraction( refinement, RigRefinement::DimK, sectorK );
        }
        if ( fraction1 >= 1.0 && fraction2 >= 1.0 ) return std::nullopt;

        std::optional<ConnectionCellProperties> properties;
        if ( cellProperties )
        {
            properties = cellProperties( caf::VecIjk0( origI, origJ, sectorK + settings.min().z() ) );
        }
        return lateralRefinementCorrection( record, wellName, dim, fraction1, fraction2, properties );
    };

    auto createRecord =
        [&]( size_t i, size_t j, size_t k1, size_t k2, double fraction, const std::optional<LateralRefinementCorrection>& correction )
    {
        std::vector<Opm::DeckItem> items;
        items.push_back( record.getItem( 0 ) );
        items.push_back( RifOpmDeckTools::item( C::I::itemName, static_cast<int>( i + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( C::J::itemName, static_cast<int>( j + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( C::K1::itemName, static_cast<int>( k1 + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( C::K2::itemName, static_cast<int>( k2 + 1 ) ) );

        for ( size_t idx = 5; idx < record.size(); ++idx )
        {
            const auto& item       = record.getItem( idx );
            const bool  isCF       = item.name() == C::CONNECTION_TRANSMISSIBILITY_FACTOR::itemName;
            const bool  isScalable = isCF || item.name() == C::Kh::itemName;
            const bool  isExplicit = hasExplicitDoubleValue( record, item.name() );
            if ( isCF && isExplicit && correction )
            {
                items.push_back(
                    RifOpmDeckTools::item( item.name(), item.get<double>( 0 ) * fraction * correction->connectionFactorMultiplier ) );
            }
            else if ( item.name() == C::PR::itemName && isExplicit && correction )
            {
                items.push_back( RifOpmDeckTools::item( item.name(), correction->pressureEquivalentRadius ) );
            }
            else if ( fraction != 1.0 && isScalable && isExplicit )
            {
                items.push_back( RifOpmDeckTools::item( item.name(), item.get<double>( 0 ) * fraction ) );
            }
            else
            {
                items.push_back( item );
            }
        }

        return Opm::DeckRecord{ std::move( items ) };
    };

    std::vector<Opm::DeckRecord> records;
    for ( const auto& kSpan : kSpans )
    {
        const auto correction = lateralCorrection( kSpan.sectorK );
        for ( const auto& iSubcell : iSubcells )
        {
            for ( const auto& jSubcell : jSubcells )
            {
                records.push_back( createRecord( iSubcell.index,
                                                 jSubcell.index,
                                                 kSpan.k1,
                                                 kSpan.k2,
                                                 kSpan.fraction * iSubcell.fraction * jSubcell.fraction,
                                                 correction ) );
            }
        }
    }

    return records;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<std::vector<Opm::DeckRecord>, QString>
    RigSimulationInputConnectionTools::processCompsegsRecord( const Opm::DeckRecord&            record,
                                                              const std::string&                wellName,
                                                              bool                              isWellNameRecord,
                                                              const RigSimulationInputSettings& settings,
                                                              const ConnectionDirectionMap&     directions,
                                                              const ConnectionKeySet&           reversedCells )
{
    // COMPSEGS format: first record is well name, subsequent records are segment data
    // Well name record: just copy as-is
    if ( isWellNameRecord )
    {
        return std::vector<Opm::DeckRecord>{ Opm::DeckRecord( record ) };
    }

    // Segment record format: I J K BRANCH START_MD END_MD DIRECTION END_IJK ...
    // Items: 0=I, 1=J, 2=K, 4=START_MD, 5=END_MD, 6=DIRECTION
    if ( record.size() < 3 )
    {
        return std::unexpected( QString( "COMPSEGS segment record for well %1 has insufficient items (expected at least 3, got %2)" )
                                    .arg( wellName.c_str() )
                                    .arg( record.size() ) );
    }

    caf::VecIjk0 origIjk = RigSimulationInputTool::extractIjk( record, 0, 1, 2 ).toZeroBased();

    const auto& refinement = settings.refinement();

    auto transformResult = RigGridExportAdapter::transformIjkToSectorCoordinates( origIjk, settings.min(), settings.max(), refinement, true );

    if ( !transformResult )
    {
        return std::unexpected(
            QString( "COMPSEGS segment coordinate for well %1 is out of sector bounds: %2" ).arg( wellName.c_str() ).arg( transformResult.error() ) );
    }

    auto dim = RigRefinement::DimK;
    if ( auto it = directions.find( { wellName, origIjk } ); it != directions.end() )
    {
        dim = it->second;
    }

    const size_t sectorIndex = origIjk[dim] - settings.min()[dim];

    std::vector<RefinedSubcell> subcells = { { ( *transformResult )[dim], 1.0 } };

    using C = Opm::ParserKeywords::COMPSEGS;

    auto hasItemValue = [&record]( const std::string& itemName )
    { return record.hasItem( itemName ) && record.getItem( itemName ).hasValue( 0 ); };

    // A cell range is given by END_IJK. DIRECTION alone (with an MD interval) describes a single cell.
    const bool isCellRange = hasItemValue( C::END_IJK::itemName );
    const bool hasEndMd    = hasItemValue( C::DISTANCE_END::itemName );

    // A defaulted start MD is zero (only allowed for the first segment record)
    const double startMd = hasItemValue( C::DISTANCE_START::itemName ) ? record.getItem( C::DISTANCE_START::itemName ).get<double>( 0 ) : 0.0;
    const double endMd = hasEndMd ? record.getItem( C::DISTANCE_END::itemName ).get<double>( 0 ) : 0.0;

    // Whether the MD interval is divided between the sub-cells, otherwise the record is copied unchanged to each sub-cell
    bool splitMdInterval = false;

    if ( refinement.subcellCount( dim, sectorIndex ) > 1 )
    {
        if ( isCellRange )
        {
            RiaLogging::warning(
                std::format( "COMPSEGS record for well {} at ({}, {}, {}) uses a cell range (END_IJK), which is not supported "
                             "for refined sector export. Only the center refined sub-cell is assigned to the segment.",
                             wellName,
                             origIjk.x() + 1,
                             origIjk.y() + 1,
                             origIjk.z() + 1 ) );
        }
        else
        {
            // All refined sub-cells along the connection are completed by COMPDAT, so all must be assigned to a segment
            subcells = allSubcells( refinement, dim, sectorIndex );

            // Assign the MD interval in the order the well travels through the sub-cells
            if ( reversedCells.contains( { wellName, origIjk } ) )
            {
                std::reverse( subcells.begin(), subcells.end() );
            }

            splitMdInterval = hasEndMd;
            if ( !hasEndMd )
            {
                RiaLogging::warning( std::format( "COMPSEGS record for well {} at ({}, {}, {}) has no end MD. "
                                                  "The record is copied unchanged to each refined sub-cell.",
                                                  wellName,
                                                  origIjk.x() + 1,
                                                  origIjk.y() + 1,
                                                  origIjk.z() + 1 ) );
            }
        }
    }

    std::vector<Opm::DeckRecord> records;

    double startFraction = 0.0;
    for ( const auto& subcell : subcells )
    {
        caf::VecIjk0 refinedIjk = *transformResult;
        refinedIjk[dim]         = subcell.index;

        std::vector<Opm::DeckItem> items;
        items.push_back( RifOpmDeckTools::item( "I", static_cast<int>( refinedIjk.x() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( "J", static_cast<int>( refinedIjk.y() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( "K", static_cast<int>( refinedIjk.z() + 1 ) ) );

        const double endFraction = startFraction + subcell.fraction;
        for ( size_t i = 3; i < record.size(); ++i )
        {
            const auto& itemName = record.getItem( i ).name();
            if ( splitMdInterval && ( itemName == C::DISTANCE_START::itemName || itemName == C::DISTANCE_END::itemName ) )
            {
                const double fraction = ( itemName == C::DISTANCE_START::itemName ) ? startFraction : endFraction;
                items.push_back( RifOpmDeckTools::item( record.getItem( i ).name(), startMd + ( endMd - startMd ) * fraction ) );
            }
            else
            {
                items.push_back( record.getItem( i ) );
            }
        }
        startFraction = endFraction;

        records.push_back( Opm::DeckRecord{ std::move( items ) } );
    }

    return records;
}
