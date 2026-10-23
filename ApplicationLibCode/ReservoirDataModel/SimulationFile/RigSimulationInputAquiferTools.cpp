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

#include "RigSimulationInputAquiferTools.h"

#include "RifOpmDeckTools.h"
#include "RifOpmFlowDeckFile.h"

#include "RimEclipseCase.h"

#include "RigBoundingBoxIjk.h"
#include "RigSimulationInputSettings.h"
#include "RigSimulationInputTool.h"

#include "opm/input/eclipse/Deck/DeckItem.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/A.hpp"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputAquiferTools::replaceAquconKeywordIndices( RimEclipseCase*                   eclipseCase,
                                                                                          const RigSimulationInputSettings& settings,
                                                                                          RifOpmFlowDeckFile&               deckFile )
{
    return RigSimulationInputTool::replaceKeywordWithBoxIndices( "AQUCON", eclipseCase, settings, deckFile, processAquconRecord );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputAquiferTools::replaceAquanconKeywordIndices( RimEclipseCase*                   eclipseCase,
                                                                                            const RigSimulationInputSettings& settings,
                                                                                            RifOpmFlowDeckFile&               deckFile )
{
    return RigSimulationInputTool::replaceKeywordWithBoxIndices( "AQUANCON", eclipseCase, settings, deckFile, processAquanconRecord );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputAquiferTools::replaceAqunumKeywordIndices( RimEclipseCase*                   eclipseCase,
                                                                                          const RigSimulationInputSettings& settings,
                                                                                          RifOpmFlowDeckFile&               deckFile )
{
    return RigSimulationInputTool::replaceKeywordWithBoxIndices( "AQUNUM", eclipseCase, settings, deckFile, processAqunumRecord );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<Opm::DeckRecord, QString> RigSimulationInputAquiferTools::processAquconRecord( const Opm::DeckRecord& record,
                                                                                             const caf::VecIjk0&    min,
                                                                                             const caf::VecIjk0&    max,
                                                                                             const RigRefinement&   refinement )
{
    // AQUCON format: ID I1 I2 J1 J2 K1 K2 CONNECT_FACE ...
    // Items: 0=ID, 1=I1, 2=I2, 3=J1, 4=J2, 5=K1, 6=K2, 7+=other parameters
    if ( record.size() < 1 )
    {
        return std::unexpected( QString( "AQUCON record has insufficient items (expected at least 1, got %1)" ).arg( record.size() ) );
    }

    using A = Opm::ParserKeywords::AQUCON;

    std::vector<Opm::DeckItem> items;

    // Copy aquifer ID (first item)
    items.push_back( record.getItem( 0 ) );

    if ( record.size() >= 7 && record.getItem( 1 ).hasValue( 0 ) && record.getItem( 2 ).hasValue( 0 ) && record.getItem( 3 ).hasValue( 0 ) &&
         record.getItem( 4 ).hasValue( 0 ) && record.getItem( 5 ).hasValue( 0 ) && record.getItem( 6 ).hasValue( 0 ) )
    {
        // Transform IJK box coordinates (items 1-6: I1, I2, J1, J2, K1, K2)
        // Note: AQUCON uses 1-based Eclipse coordinates
        caf::VecIjk1 origMin = RigSimulationInputTool::extractIjk( record, 1, 3, 5 ); // I1, J1, K1
        caf::VecIjk1 origMax = RigSimulationInputTool::extractIjk( record, 2, 4, 6 ); // I2, J2, K2

        // Create input bounding box (0-based, inclusive)
        RigBoundingBoxIjk<caf::VecIjk0> inputBox( origMin.toZeroBased(), origMax.toZeroBased() );

        // Get aquifer ID for logging
        int     aquiferId   = record.getItem( 0 ).get<int>( 0 );
        QString recordIdent = QString( "for aquifer %1" ).arg( aquiferId );

        // Transform box to sector coordinates
        auto transformResult = RigSimulationInputTool::transformBoxToSectorCoordinates( inputBox,
                                                                                        min,
                                                                                        max,
                                                                                        refinement,
                                                                                        QString::fromStdString( A::keywordName ),
                                                                                        recordIdent );
        if ( !transformResult )
        {
            return std::unexpected( transformResult.error() );
        }

        items.push_back( RifOpmDeckTools::item( A::I1::itemName, static_cast<int>( transformResult->min().x() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( A::I2::itemName, static_cast<int>( transformResult->max().x() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( A::J1::itemName, static_cast<int>( transformResult->min().y() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( A::J2::itemName, static_cast<int>( transformResult->max().y() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( A::K1::itemName, static_cast<int>( transformResult->min().z() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( A::K2::itemName, static_cast<int>( transformResult->max().z() + 1 ) ) );

        // Copy remaining items (CONNECT_FACE and other parameters)
        for ( size_t i = 7; i < record.size(); ++i )
        {
            items.push_back( record.getItem( i ) );
        }
    }
    else
    {
        // No box definition or incomplete box definition - copy remaining items as-is
        for ( size_t i = 1; i < record.size(); ++i )
        {
            items.push_back( record.getItem( i ) );
        }
    }

    return Opm::DeckRecord{ std::move( items ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<Opm::DeckRecord, QString> RigSimulationInputAquiferTools::processAquanconRecord( const Opm::DeckRecord& record,
                                                                                               const caf::VecIjk0&    min,
                                                                                               const caf::VecIjk0&    max,
                                                                                               const RigRefinement&   refinement )
{
    // AQUANCON format: ID I1 I2 J1 J2 K1 K2 AQU_FACE ...
    // Items: 0=ID, 1=I1, 2=I2, 3=J1, 4=J2, 5=K1, 6=K2, 7+=other parameters
    if ( record.size() < 1 )
    {
        return std::unexpected( QString( "AQUANCON record has insufficient items (expected at least 1, got %1)" ).arg( record.size() ) );
    }

    using A = Opm::ParserKeywords::AQUANCON;

    std::vector<Opm::DeckItem> items;

    // Copy aquifer ID (first item)
    items.push_back( record.getItem( 0 ) );

    if ( record.size() >= 7 && record.getItem( 1 ).hasValue( 0 ) && record.getItem( 2 ).hasValue( 0 ) && record.getItem( 3 ).hasValue( 0 ) &&
         record.getItem( 4 ).hasValue( 0 ) && record.getItem( 5 ).hasValue( 0 ) && record.getItem( 6 ).hasValue( 0 ) )
    {
        // Transform IJK box coordinates (items 1-6: I1, I2, J1, J2, K1, K2)
        // Note: AQUANCON uses 1-based Eclipse coordinates
        caf::VecIjk1 origMin = RigSimulationInputTool::extractIjk( record, 1, 3, 5 ); // I1, J1, K1
        caf::VecIjk1 origMax = RigSimulationInputTool::extractIjk( record, 2, 4, 6 ); // I2, J2, K2

        // Create input bounding box (0-based, inclusive)
        RigBoundingBoxIjk<caf::VecIjk0> inputBox( origMin.toZeroBased(), origMax.toZeroBased() );

        // Get aquifer ID for logging
        int     aquiferId   = record.getItem( 0 ).get<int>( 0 );
        QString recordIdent = QString( "for aquifer %1" ).arg( aquiferId );

        // Transform box to sector coordinates
        auto transformResult = RigSimulationInputTool::transformBoxToSectorCoordinates( inputBox,
                                                                                        min,
                                                                                        max,
                                                                                        refinement,
                                                                                        QString::fromStdString( A::keywordName ),
                                                                                        recordIdent );
        if ( !transformResult )
        {
            return std::unexpected( transformResult.error() );
        }

        items.push_back( RifOpmDeckTools::item( A::I1::itemName, static_cast<int>( transformResult->min().x() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( A::I2::itemName, static_cast<int>( transformResult->max().x() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( A::J1::itemName, static_cast<int>( transformResult->min().y() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( A::J2::itemName, static_cast<int>( transformResult->max().y() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( A::K1::itemName, static_cast<int>( transformResult->min().z() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( A::K2::itemName, static_cast<int>( transformResult->max().z() + 1 ) ) );

        // Copy remaining items (AQUFACE and other parameters)
        for ( size_t i = 7; i < record.size(); ++i )
        {
            items.push_back( record.getItem( i ) );
        }
    }
    else
    {
        // No box definition or incomplete box definition - copy remaining items as-is
        for ( size_t i = 1; i < record.size(); ++i )
        {
            items.push_back( record.getItem( i ) );
        }
    }

    return Opm::DeckRecord{ std::move( items ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<Opm::DeckRecord, QString> RigSimulationInputAquiferTools::processAqunumRecord( const Opm::DeckRecord& record,
                                                                                             const caf::VecIjk0&    min,
                                                                                             const caf::VecIjk0&    max,
                                                                                             const RigRefinement&   refinement )
{
    // AQUNUM format: ID I J K AREA ...
    // Items: 0=ID, 1=I, 2=J, 3=K, 4+=other parameters
    if ( record.size() < 1 )
    {
        return std::unexpected( QString( "AQUNUM record has insufficient items (expected at least 1, got %1)" ).arg( record.size() ) );
    }

    using A = Opm::ParserKeywords::AQUNUM;

    std::vector<Opm::DeckItem> items;

    // Copy aquifer ID (first item)
    items.push_back( record.getItem( 0 ) );

    if ( record.size() >= 4 && record.getItem( 1 ).hasValue( 0 ) && record.getItem( 2 ).hasValue( 0 ) && record.getItem( 3 ).hasValue( 0 ) )
    {
        // Transform IJK coordinates (items 1-6: I1, I2, J1, J2, K1, K2)
        // Note: AQUNUM uses 1-based Eclipse coordinates
        caf::VecIjk1 origCell = RigSimulationInputTool::extractIjk( record, 1, 2, 3 ); // I, J, K

        // Create input bounding box (0-based, inclusive)
        RigBoundingBoxIjk<caf::VecIjk0> inputBox( origCell.toZeroBased(), origCell.toZeroBased() );

        // Get aquifer ID for logging
        int     aquiferId   = record.getItem( 0 ).get<int>( 0 );
        QString recordIdent = QString( "for aquifer %1" ).arg( aquiferId );

        // Transform box to sector coordinates
        auto transformResult = RigSimulationInputTool::transformBoxToSectorCoordinates( inputBox,
                                                                                        min,
                                                                                        max,
                                                                                        refinement,
                                                                                        QString::fromStdString( A::keywordName ),
                                                                                        recordIdent );
        if ( !transformResult )
        {
            return std::unexpected( transformResult.error() );
        }

        items.push_back( RifOpmDeckTools::item( A::I::itemName, static_cast<int>( transformResult->min().x() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( A::J::itemName, static_cast<int>( transformResult->min().y() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( A::K::itemName, static_cast<int>( transformResult->min().z() + 1 ) ) );

        // Copy remaining items (AREA and other parameters)
        for ( size_t i = 4; i < record.size(); ++i )
        {
            items.push_back( record.getItem( i ) );
        }
    }
    else
    {
        // No box definition or incomplete box definition - copy remaining items as-is
        for ( size_t i = 1; i < record.size(); ++i )
        {
            items.push_back( record.getItem( i ) );
        }
    }

    return Opm::DeckRecord{ std::move( items ) };
}
