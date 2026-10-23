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

#include "RigSimulationInputNncTools.h"

#include "RiaLogging.h"

#include "RifOpmFlowDeckFile.h"

#include "ProjectDataModel/Jobs/RimKeywordFactory.h"
#include "RimEclipseCase.h"

#include "RigBoundingBoxIjk.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigSimulationInputSettings.h"
#include "RigSimulationInputTool.h"

#include "opm/input/eclipse/Deck/DeckItem.hpp"
#include "opm/input/eclipse/Deck/DeckKeyword.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigSimulationInputNncTools::NNCConnection>
    RigSimulationInputNncTools::extractDeckEditNncConnections( RifOpmFlowDeckFile& deckFile, const RigMainGrid& mainGrid )
{
    std::vector<NNCConnection> connections;

    // Find all EDITNNC keywords in deck (there may be multiple)
    auto keywords = deckFile.findAllKeywordsWithIndices( "EDITNNC" );

    for ( const auto& keywordPair : keywords )
    {
        const auto& keyword = keywordPair.second;

        // Each keyword can have multiple records
        for ( size_t recordIdx = 0; recordIdx < keyword.size(); ++recordIdx )
        {
            const auto& record = keyword.getRecord( recordIdx );

            // Read IJK coordinates directly from deck (1-based) and convert to 0-based
            caf::VecIjk1 ijk1( record.getItem( 0 ).get<int>( 0 ), record.getItem( 1 ).get<int>( 0 ), record.getItem( 2 ).get<int>( 0 ) );
            caf::VecIjk1 ijk2( record.getItem( 3 ).get<int>( 0 ), record.getItem( 4 ).get<int>( 0 ), record.getItem( 5 ).get<int>( 0 ) );
            double       transMult = record.getItem( 6 ).get<double>( 0 );

            // Convert IJK to global cell indices
            size_t c1Idx = mainGrid.cellIndexFromIJK( ijk1.toZeroBased() );
            size_t c2Idx = mainGrid.cellIndexFromIJK( ijk2.toZeroBased() );

            // Validate indices
            if ( c1Idx == cvf::UNDEFINED_SIZE_T || c2Idx == cvf::UNDEFINED_SIZE_T )
            {
                RiaLogging::warning( std::format( "Invalid EDITNNC connection in deck: ({})-({})", ijk1.toString(), ijk2.toString() ) );
                continue;
            }

            connections.push_back( { c1Idx, c2Idx, transMult } );
        }
    }

    return connections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigSimulationInputNncTools::NNCConnection>
    RigSimulationInputNncTools::filterInternalSectorConnections( const std::vector<NNCConnection>& allConnections,
                                                                 const RigMainGrid&                mainGrid,
                                                                 const caf::VecIjk0&               min,
                                                                 const caf::VecIjk0&               max )
{
    std::vector<NNCConnection> filteredConnections;

    RigBoundingBoxIjk sectorBounds( min, max );

    for ( const auto& conn : allConnections )
    {
        // Get IJK for both cells
        auto ijk1 = mainGrid.ijkFromCellIndex( conn.c1GlobIdx );
        auto ijk2 = mainGrid.ijkFromCellIndex( conn.c2GlobIdx );

        // Only keep if BOTH cells are inside sector bounds
        if ( ijk1 && ijk2 && sectorBounds.contains( *ijk1 ) && sectorBounds.contains( *ijk2 ) )
        {
            filteredConnections.push_back( conn );
        }
    }

    return filteredConnections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<RigSimulationInputNncTools::TransformedNNCConnection, QString>
    RigSimulationInputNncTools::transformNNCToSectorCoordinates( const NNCConnection& connection,
                                                                 const RigMainGrid&   mainGrid,
                                                                 const caf::VecIjk0&  min,
                                                                 const RigRefinement& refinement )
{
    // Get global IJK for both cells
    auto gijk1 = mainGrid.ijkFromCellIndex( connection.c1GlobIdx );
    if ( !gijk1 )
    {
        return std::unexpected( QString( "Invalid cell index: %1" ).arg( connection.c1GlobIdx ) );
    }

    auto gijk2 = mainGrid.ijkFromCellIndex( connection.c2GlobIdx );
    if ( !gijk2 )
    {
        return std::unexpected( QString( "Invalid cell index: %1" ).arg( connection.c2GlobIdx ) );
    }

    // Transform to sector coordinates using utility method
    TransformedNNCConnection transformed;
    transformed.cell1            = RigSimulationInputTool::transformToSectorCoordinates( *gijk1, min, refinement );
    transformed.cell2            = RigSimulationInputTool::transformToSectorCoordinates( *gijk2, min, refinement );
    transformed.transmissibility = connection.transmissibility;

    return transformed;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigSimulationInputNncTools::TransformedNNCConnection>
    RigSimulationInputNncTools::refineEditNncConnection( const NNCConnection& connection,
                                                         const RigMainGrid&   mainGrid,
                                                         const caf::VecIjk0&  min,
                                                         const RigRefinement& refinement )
{
    // Get global IJK for both cells
    auto gijk1 = mainGrid.ijkFromCellIndex( connection.c1GlobIdx );
    auto gijk2 = mainGrid.ijkFromCellIndex( connection.c2GlobIdx );

    if ( !gijk1 || !gijk2 )
    {
        return {}; // Return empty on error
    }

    // For EDITNNC: Connect subcells at same relative positions within coarse cells
    // Example: refinement (2,2,1) creates 4 connections:
    //   (0,0,0) → (0,0,0)
    //   (0,1,0) → (0,1,0)
    //   (1,0,0) → (1,0,0)
    //   (1,1,0) → (1,1,0)
    // Each connection gets the SAME TRANSMUL (no division)

    // Get base sector coordinates for both cells (start of refined block)
    caf::VecIjk0 baseCell1 = RigSimulationInputTool::transformToSectorCoordinates( *gijk1, min, refinement );
    caf::VecIjk0 baseCell2 = RigSimulationInputTool::transformToSectorCoordinates( *gijk2, min, refinement );

    // Get subcell counts for each cell
    size_t rel1I = gijk1->i() - min.i();
    size_t rel1J = gijk1->j() - min.j();
    size_t rel1K = gijk1->k() - min.k();

    size_t subcellsI = refinement.subcellCount( RigRefinement::DimI, rel1I );
    size_t subcellsJ = refinement.subcellCount( RigRefinement::DimJ, rel1J );
    size_t subcellsK = refinement.subcellCount( RigRefinement::DimK, rel1K );

    std::vector<TransformedNNCConnection> refined;
    for ( size_t subI = 0; subI < subcellsI; ++subI )
    {
        for ( size_t subJ = 0; subJ < subcellsJ; ++subJ )
        {
            for ( size_t subK = 0; subK < subcellsK; ++subK )
            {
                // Add subcell offset to base coordinates
                TransformedNNCConnection conn;
                conn.cell1 = caf::VecIjk0( baseCell1.i() + subI, baseCell1.j() + subJ, baseCell1.k() + subK );

                // SAME relative position in cell2
                conn.cell2 = caf::VecIjk0( baseCell2.i() + subI, baseCell2.j() + subJ, baseCell2.k() + subK );

                conn.transmissibility = connection.transmissibility; // Same TRAN_MULT for all
                refined.push_back( conn );
            }
        }
    }

    return refined;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputNncTools::exportEditNncKeyword( RimEclipseCase*                   eclipseCase,
                                                                               const RigSimulationInputSettings& settings,
                                                                               RifOpmFlowDeckFile&               deckFile )
{
    RigMainGrid* mainGrid = eclipseCase->eclipseCaseData()->mainGrid();
    if ( !mainGrid ) return std::unexpected( "No main grid available" );

    // Step 1: Extract EDITNNC from deck only
    auto allConnections = extractDeckEditNncConnections( deckFile, *mainGrid );

    if ( allConnections.empty() )
    {
        RiaLogging::info( "No EDITNNC keywords found in deck - skipping EDITNNC export" );
        return {};
    }

    // Step 2: Filter for internal sector
    auto sectorConnections = filterInternalSectorConnections( allConnections, *mainGrid, settings.min(), settings.max() );

    if ( sectorConnections.empty() )
    {
        RiaLogging::info( "No internal EDITNNC connections in sector - skipping EDITNNC export" );
        return {};
    }

    RiaLogging::info(
        std::format( "Found {} internal EDITNNC connections in sector (out of {} total)", sectorConnections.size(), allConnections.size() ) );

    // Step 3: Transform and refine
    bool hasRef = settings.hasRefinement();

    std::vector<TransformedNNCConnection> transformedConnections;

    if ( hasRef )
    {
        // Refine to subcells with corresponding positions
        for ( const auto& conn : sectorConnections )
        {
            auto refined = refineEditNncConnection( conn, *mainGrid, settings.min(), settings.refinement() );
            transformedConnections.insert( transformedConnections.end(), refined.begin(), refined.end() );
        }
    }
    else
    {
        // Simple coordinate transformation, no refinement
        for ( const auto& conn : sectorConnections )
        {
            auto result = transformNNCToSectorCoordinates( conn, *mainGrid, settings.min(), settings.refinement() );
            if ( result )
            {
                transformedConnections.push_back( *result );
            }
            else
            {
                RiaLogging::warning( result.error().toStdString() );
            }
        }
    }

    if ( transformedConnections.empty() )
    {
        return std::unexpected( "Failed to transform any EDITNNC connections" );
    }

    RiaLogging::info( std::format( "Exporting {} EDITNNC connections to sector model", transformedConnections.size() ) );

    // Step 4: Create keyword
    auto editnncKw = RimKeywordFactory::editnncKeyword( transformedConnections );

    // Step 5: Replace/add to EDIT section (not GRID!)
    deckFile.replaceKeyword( "EDIT", editnncKw );

    return {};
}
