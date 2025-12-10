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

#include "RigSimulationInputTool.h"

#include "RiaLogging.h"

#include "RiaResultNames.h"
#include "RigSimulationInputSettings.h"

#include "RifEclipseInputFileTools.h"
#include "RifOpmDeckTools.h"
#include "RifOpmFlowDeckFile.h"

#include "ProjectDataModel/Jobs/RimKeywordFactory.h"
#include "RimEclipseCase.h"

#include "RigBoundingBoxIjk.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultTools.h"
#include "RigGridExportAdapter.h"
#include "RigResdataGridConverter.h"
#include "Well/RigSimWellData.h"
#include "Well/RigWellResultFrame.h"
#include "Well/RigWellResultPoint.h"

#include "opm/input/eclipse/Deck/DeckItem.hpp"
#include "opm/input/eclipse/Deck/DeckKeyword.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"
#include "opm/input/eclipse/Parser/ParserKeyword.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/A.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/B.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/C.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/E.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/M.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/O.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/S.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/W.hpp"

#include <QFileInfo>

#include <set>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputTool::exportSimulationInput( RimEclipseCase&                   eclipseCase,
                                                                            const RigSimulationInputSettings& settings,
                                                                            cvf::UByteArray*                  visibility )
{
    // Load the deck file
    QString inputDeckFileName = settings.inputDeckFileName();

    RifOpmFlowDeckFile deckFile;
    if ( !deckFile.loadDeck( inputDeckFileName.toStdString() ) )
    {
        return std::unexpected( QString( "Unable to load deck file '%1'" ).arg( inputDeckFileName ) );
    }

    QString   outputDeckFileName = settings.outputDeckFileName();
    QFileInfo exportGridInfo( outputDeckFileName );
    QString   outputFolder = exportGridInfo.absolutePath();
    QString   outputFile   = exportGridInfo.completeBaseName() + ".DATA";

    if ( auto result = updateCornerPointGridInDeckFile( &eclipseCase, settings, deckFile ); !result )
    {
        return result;
    }

    // Generate BORDNUM result based on visibility
    if ( visibility && visibility->size() > 0 )
    {
        RigEclipseResultTools::generateBorderResult( &eclipseCase, visibility, RiaResultNames::bordnum() );
        if ( settings.boundaryCondition() == RiaModelExportDefines::BoundaryCondition::BCCON_BCPROP )
        {
            // Generate BCCON result to assign values 1-6 based on which face of the box the border cells are on
            RigEclipseResultTools::generateBcconResult( &eclipseCase, settings.min(), settings.max() );
        }
        else if ( settings.boundaryCondition() == RiaModelExportDefines::BoundaryCondition::OPERNUM_OPERATER )
        {
            // Generate OPERNUM result based on BORDNUM (border cells get max existing OPERNUM + 1)
            int operNumRegion = RigEclipseResultTools::generateOperNumResult( &eclipseCase );

            if ( auto result = addOperNumRegionAndOperater( &eclipseCase, settings, deckFile, operNumRegion, settings.porvMultiplier() );
                 !result )
            {
                return result;
            }
        }
    }

    if ( auto result = replaceKeywordValuesInDeckFile( &eclipseCase, settings, deckFile ); !result )
    {
        return result;
    }

    if ( auto result = replaceEqualsKeywordIndices( &eclipseCase, settings, deckFile ); !result )
    {
        return result;
    }

    if ( auto result = replaceCopyKeywordIndices( &eclipseCase, settings, deckFile ); !result )
    {
        return result;
    }

    if ( auto result = replaceMultiplyKeywordIndices( &eclipseCase, settings, deckFile ); !result )
    {
        return result;
    }

    if ( auto result = replaceAddKeywordIndices( &eclipseCase, settings, deckFile ); !result )
    {
        return result;
    }

    if ( auto result = replaceBoxKeywordIndices( &eclipseCase, settings, deckFile ); !result )
    {
        return result;
    }

    if ( auto result = replaceAquconKeywordIndices( &eclipseCase, settings, deckFile ); !result )
    {
        return result;
    }

    if ( auto result = updateWelldimsKeyword( &eclipseCase, settings, deckFile ); !result )
    {
        return result;
    }

    if ( auto result = addBorderBoundaryConditions( &eclipseCase, settings, deckFile ); !result )
    {
        return result;
    }

    if ( auto result = addFaultsToDeckFile( &eclipseCase, settings, deckFile ); !result )
    {
        return result;
    }

    if ( auto result = filterAndUpdateWellKeywords( &eclipseCase, settings, deckFile ); !result )
    {
        return result;
    }

    // Remove SKIP keywords that were used as placeholders for filtered-out keywords
    deckFile.removeKeywords( "SKIP" );

    // TODO: fix this..
    deckFile.removeKeywords( "MAPAXES" );

    // Save the modified deck file to the export directory
    if ( !deckFile.saveDeckInline( outputFolder.toStdString(), outputFile.toStdString() ) )
    {
        return std::unexpected( QString( "Failed to save modified deck file to '%1/%2'" ).arg( outputFolder ).arg( outputFile ) );
    }

    RiaLogging::info( QString( "Saved modified deck file to '%1/%2'" ).arg( outputFolder ).arg( outputFile ) );
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputTool::updateCornerPointGridInDeckFile( RimEclipseCase*                   eclipseCase,
                                                                                      const RigSimulationInputSettings& settings,
                                                                                      RifOpmFlowDeckFile&               deckFile )
{
    // Get grid bounds for extraction

    RigGridExportAdapter gridAdapter( eclipseCase->eclipseCaseData(), settings.min(), settings.max(), settings.refinement() );

    std::vector<float> coordArray;
    std::vector<float> zcornArray;
    std::vector<int>   actnumArray;

    RigResdataGridConverter::convertGridToCornerPointArrays( gridAdapter, coordArray, zcornArray, actnumArray );

    auto keywords = deckFile.keywords( false );

    // Sector dimensions (after refinement)
    std::vector<int> dimens = { static_cast<int>( gridAdapter.cellCountI() ),
                                static_cast<int>( gridAdapter.cellCountJ() ),
                                static_cast<int>( gridAdapter.cellCountK() ) };

    if ( !deckFile.replaceKeywordData( "DIMENS", dimens ) )
    {
        return std::unexpected( "Failed to replace DIMENS keyword in deck file" );
    }

    // SPECGRID has the same dimensions plus NUMRES and COORD_TYPE
    // Format: NX NY NZ (use defaults for NUMRES (1) and  COORD_TYPE: 'F'=Cartesian)
    if ( std::find( keywords.begin(), keywords.end(), "SPECGRID" ) == keywords.end() )
    {
        Opm::DeckKeyword newKw( Opm::ParserKeyword( "SPECGRID" ) );
        deckFile.addKeyword( "GRID", newKw );
    }

    std::vector<int> specgrid = { static_cast<int>( gridAdapter.cellCountI() ),
                                  static_cast<int>( gridAdapter.cellCountJ() ),
                                  static_cast<int>( gridAdapter.cellCountK() ) };

    if ( !deckFile.replaceKeywordData( "SPECGRID", specgrid ) )
    {
        return std::unexpected( "Failed to replace SPECGRID keyword in deck file" );
    }

    auto convertToDoubleVector = []( const std::vector<float>& vec )
    {
        std::vector<double> outVec;
        outVec.reserve( vec.size() );
        for ( float f : vec )
            outVec.push_back( f );
        return outVec;
    };
    std::vector<double> coords = convertToDoubleVector( coordArray );
    std::vector<double> zcorn  = convertToDoubleVector( zcornArray );

    if ( std::find( keywords.begin(), keywords.end(), "ACTNUM" ) == keywords.end() )
    {
        Opm::DeckKeyword newKw( Opm::ParserKeyword( "ACTNUM" ) );
        deckFile.addKeyword( "GRID", newKw );
    }

    if ( !deckFile.replaceKeywordData( "COORD", coords ) )
    {
        return std::unexpected( "Failed to replace COORD keyword in deck file" );
    }

    if ( !deckFile.replaceKeywordData( "ZCORN", zcorn ) )
    {
        return std::unexpected( "Failed to replace ZCORN keyword in deck file" );
    }

    if ( !deckFile.replaceKeywordData( "ACTNUM", actnumArray ) )
    {
        return std::unexpected( "Failed to replace ACTNUM keyword in deck file" );
    }

    // TODO: deal with map axis
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputTool::replaceKeywordValuesInDeckFile( RimEclipseCase*                   eclipseCase,
                                                                                     const RigSimulationInputSettings& settings,
                                                                                     RifOpmFlowDeckFile&               deckFile )
{
    // Extract and replace keyword data for all keywords in the deck
    auto keywords = deckFile.keywords( false );
    RiaLogging::info( QString( "Processing %1 keywords from deck file" ).arg( keywords.size() ) );

    for ( const auto& keywordStdStr : keywords )
    {
        QString keyword = QString::fromStdString( keywordStdStr );

        // Skip special keywords that aren't cell properties
        if ( keyword.startsWith( "DATES" ) || keyword == "SCHEDULE" || keyword == "GRID" || keyword == "PROPS" || keyword == "SOLUTION" ||
             keyword == "RUNSPEC" || keyword == "SUMMARY" )
        {
            continue;
        }

        // Try to extract keyword data
        auto result = RifEclipseInputFileTools::extractKeywordData( eclipseCase->eclipseCaseData(),
                                                                    keyword,
                                                                    settings.min(),
                                                                    settings.max(),
                                                                    settings.refinement() );
        if ( result )
        {
            // Replace keyword values in deck with extracted data
            if ( deckFile.replaceKeywordData( keywordStdStr, result.value() ) )
            {
                RiaLogging::info(
                    QString( "Successfully replaced data for keyword '%1' (%2 values)" ).arg( keyword ).arg( result.value().size() ) );
            }
            else
            {
                RiaLogging::warning( QString( "Failed to replace keyword '%1' in deck" ).arg( keyword ) );
            }
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputTool::addBorderBoundaryConditions( RimEclipseCase*                   eclipseCase,
                                                                                  const RigSimulationInputSettings& settings,
                                                                                  RifOpmFlowDeckFile&               deckFile )
{
    // Generate border cell faces
    auto borderCellFaces = RigEclipseResultTools::generateBorderCellFaces( eclipseCase );

    if ( !borderCellFaces.empty() )
    {
        // Transform border cell face coordinates to sector-relative coordinates
        for ( auto& face : borderCellFaces )
        {
            auto transformResult =
                RigGridExportAdapter::transformIjkToSectorCoordinates( face.ijk, settings.min(), settings.max(), settings.refinement(), true );

            if ( !transformResult )
            {
                RiaLogging::warning( QString( "Failed to transform border cell face at (%1, %2, %3): %4" )
                                         .arg( face.ijk.x() )
                                         .arg( face.ijk.y() )
                                         .arg( face.ijk.z() )
                                         .arg( transformResult.error() ) );
                continue;
            }

            // Update the IJK coordinates to sector-relative (1-based Eclipse coordinates)
            // Note: RigGridExportAdapter::transformIjkToSectorCoordinates returns 1-based coordinates, but we need to convert back to
            // 0-based for the BorderCellFace struct
            face.ijk = transformResult->toZeroBased();
        }

        // Create BCCON keyword using the factory
        Opm::DeckKeyword bcconKw = RimKeywordFactory::bcconKeyword( borderCellFaces );

        // Replace BCCON keyword in GRID section
        if ( !deckFile.replaceKeyword( "GRID", bcconKw ) )
        {
            return std::unexpected( "Failed to replace BCCON keyword in deck file" );
        }

        // Build BCPROP records from the settings configuration
        std::vector<Opm::DeckRecord> bcpropRecords = settings.bcpropKeywords();

        // Create BCPROP keyword using the factory
        Opm::DeckKeyword bcpropKw = RimKeywordFactory::bcpropKeyword( borderCellFaces, bcpropRecords );

        // Replace BCPROP keyword in GRID section
        if ( !deckFile.replaceKeyword( Opm::ParserKeywords::SCHEDULE::keywordName, bcpropKw ) )
        {
            return std::unexpected( "Failed to replace BCPROP keyword in deck file" );
        }
    }
    else
    {
        RiaLogging::warning( "No border cells found - skipping BCCON/BCPROP keyword generation" );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
/// Generic helper for processing keywords with box indices (EQUALS, COPY, MULTIPLY, ADD)
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputTool::replaceKeywordWithBoxIndices( const std::string&                keywordName,
                                                                                   RimEclipseCase*                   eclipseCase,
                                                                                   const RigSimulationInputSettings& settings,
                                                                                   RifOpmFlowDeckFile&               deckFile,
                                                                                   RecordProcessorFunc               processorFunc )
{
    auto keywordsWithIndices = deckFile.findAllKeywordsWithIndices( keywordName );
    if ( !keywordsWithIndices.empty() )
    {
        RiaLogging::info( QString( "Processing %1 occurrence(s) of %2 keyword" ).arg( keywordsWithIndices.size() ).arg( keywordName.c_str() ) );

        // Process in reverse order so indices remain valid after modifications
        for ( auto it = keywordsWithIndices.rbegin(); it != keywordsWithIndices.rend(); ++it )
        {
            const Opm::FileDeck::Index& index = it->first;
            const Opm::DeckKeyword&     kw    = it->second;

            // Create new keyword with transformed coordinates
            Opm::DeckKeyword transformedKeyword( kw.location(), kw.name() );

            try
            {
                for ( size_t recordIdx = 0; recordIdx < kw.size(); ++recordIdx )
                {
                    const auto& record = kw.getRecord( recordIdx );
                    auto        result = processorFunc( record, settings.min(), settings.max(), settings.refinement() );

                    if ( result )
                    {
                        transformedKeyword.addRecord( std::move( result.value() ) );
                    }
                    else
                    {
                        RiaLogging::warning( QString( "Failed to process %1 record: %2" ).arg( keywordName.c_str() ).arg( result.error() ) );
                    }
                }

                // Replace with transformed keyword
                if ( transformedKeyword.size() > 0 )
                {
                    RiaLogging::info( QString( "Got %1 keywords." ).arg( transformedKeyword.size() ) );
                    deckFile.replaceKeywordAtIndex( index, transformedKeyword );
                }
                else
                {
                    // If all records failed, remove the keyword
                    deckFile.replaceKeywordAtIndex( index, Opm::DeckKeyword( kw.location(), "SKIP" ) );
                }
            }
            catch ( std::exception& e )
            {
                return std::unexpected( QString( "Exception processing %1 keyword: %2" ).arg( keywordName.c_str() ).arg( e.what() ) );
            }
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputTool::replaceEqualsKeywordIndices( RimEclipseCase*                   eclipseCase,
                                                                                  const RigSimulationInputSettings& settings,
                                                                                  RifOpmFlowDeckFile&               deckFile )
{
    return replaceKeywordWithBoxIndices( "EQUALS", eclipseCase, settings, deckFile, processEqualsRecord );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputTool::replaceMultiplyKeywordIndices( RimEclipseCase*                   eclipseCase,
                                                                                    const RigSimulationInputSettings& settings,
                                                                                    RifOpmFlowDeckFile&               deckFile )
{
    return replaceKeywordWithBoxIndices( "MULTIPLY", eclipseCase, settings, deckFile, processMultiplyRecord );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputTool::replaceCopyKeywordIndices( RimEclipseCase*                   eclipseCase,
                                                                                const RigSimulationInputSettings& settings,
                                                                                RifOpmFlowDeckFile&               deckFile )
{
    return replaceKeywordWithBoxIndices( "COPY", eclipseCase, settings, deckFile, processCopyRecord );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputTool::replaceAddKeywordIndices( RimEclipseCase*                   eclipseCase,
                                                                               const RigSimulationInputSettings& settings,
                                                                               RifOpmFlowDeckFile&               deckFile )
{
    return replaceKeywordWithBoxIndices( "ADD", eclipseCase, settings, deckFile, processAddRecord );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputTool::replaceAquconKeywordIndices( RimEclipseCase*                   eclipseCase,
                                                                                  const RigSimulationInputSettings& settings,
                                                                                  RifOpmFlowDeckFile&               deckFile )
{
    return replaceKeywordWithBoxIndices( "AQUCON", eclipseCase, settings, deckFile, processAquconRecord );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputTool::replaceBoxKeywordIndices( RimEclipseCase*                   eclipseCase,
                                                                               const RigSimulationInputSettings& settings,
                                                                               RifOpmFlowDeckFile&               deckFile )
{
    return replaceKeywordWithBoxIndices( "BOX", eclipseCase, settings, deckFile, processBoxRecord );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputTool::updateWelldimsKeyword( RimEclipseCase*                   eclipseCase,
                                                                            const RigSimulationInputSettings& settings,
                                                                            RifOpmFlowDeckFile&               deckFile )
{
    auto wellDims = deckFile.welldims();
    if ( wellDims.empty() ) return std::unexpected( QString( "Missing WELLDIMS keyword" ) );

    // Scale the max connections with the refinement
    int newMaxConnections = wellDims[1] * static_cast<int>( settings.refinement().x() * settings.refinement().y() * settings.refinement().z() );

    deckFile.setWelldims( wellDims[0], newMaxConnections, wellDims[2], wellDims[3] );

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputTool::addFaultsToDeckFile( RimEclipseCase*                   eclipseCase,
                                                                          const RigSimulationInputSettings& settings,
                                                                          RifOpmFlowDeckFile&               deckFile )
{
    // Create FAULTS keyword using the factory
    Opm::DeckKeyword faultsKw =
        RimKeywordFactory::faultsKeyword( eclipseCase->mainGrid(), settings.min(), settings.max(), settings.refinement() );

    // Replace FAULTS keyword in GRID section
    if ( !deckFile.replaceKeyword( "GRID", faultsKw ) )
    {
        return std::unexpected( "Failed to replace FAULTS keyword in deck file" );
    }

    RiaLogging::info( "Successfully replaced FAULTS keyword in deck file" );
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigSimWellData*>
    RigSimulationInputTool::findIntersectingWells( RimEclipseCase* eclipseCase, const cvf::Vec3st& min, const cvf::Vec3st& max )
{
    std::vector<RigSimWellData*> intersectingWells;

    if ( !eclipseCase || !eclipseCase->eclipseCaseData() ) return intersectingWells;

    const auto& wellResults = eclipseCase->eclipseCaseData()->wellResults();

    for ( size_t wellIdx = 0; wellIdx < wellResults.size(); ++wellIdx )
    {
        const RigSimWellData* wellData = wellResults[wellIdx].p();
        if ( !wellData ) continue;

        bool intersects = false;

        // Check all time steps for this well
        for ( const auto& wellFrame : wellData->m_wellCellsTimeSteps )
        {
            // Check all result points in this frame
            auto resultPoints = wellFrame.allResultPoints();
            for ( const auto& point : resultPoints )
            {
                // Get IJK if available
                auto ijkOpt = point.cellIjk();
                if ( !ijkOpt.has_value() ) continue;

                const auto& ijk = ijkOpt.value();

                // Check if point is within bounding box (inclusive)
                if ( static_cast<size_t>( ijk.i() ) >= min.x() && static_cast<size_t>( ijk.i() ) <= max.x() &&
                     static_cast<size_t>( ijk.j() ) >= min.y() && static_cast<size_t>( ijk.j() ) <= max.y() &&
                     static_cast<size_t>( ijk.k() ) >= min.z() && static_cast<size_t>( ijk.k() ) <= max.z() )
                {
                    intersects = true;
                    break;
                }
            }
            if ( intersects ) break;
        }

        if ( intersects )
        {
            // const_cast is safe here - we only need write access to collect well pointers
            intersectingWells.push_back( const_cast<RigSimWellData*>( wellData ) );
        }
    }

    return intersectingWells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<Opm::DeckRecord, QString> RigSimulationInputTool::processWelspecsRecord( const Opm::DeckRecord&            record,
                                                                                       const std::string&                wellName,
                                                                                       const RigSimulationInputSettings& settings )
{
    // WELSPECS format: WELL GROUP HEAD_I HEAD_J REF_DEPTH ...
    // Items: 0=WELL, 1=GROUP, 2=HEAD_I, 3=HEAD_J
    if ( record.size() < 4 )
    {
        return std::unexpected( QString( "WELSPECS record for well %1 has insufficient items (expected at least 4, got %2)" )
                                    .arg( wellName.c_str() )
                                    .arg( record.size() ) );
    }

    std::vector<Opm::DeckItem> items;

    // Copy well name and group
    items.push_back( record.getItem( 0 ) );
    items.push_back( record.getItem( 1 ) );

    // Transform HEAD_I and HEAD_J
    // Note: HEAD coordinates might be outside sector even if well has completions inside
    // Clamp to sector bounds before transformation
    int origI = record.getItem( 2 ).get<int>( 0 ) - 1; // Convert to 0-based
    int origJ = record.getItem( 3 ).get<int>( 0 ) - 1;

    // Clamp to sector bounds
    size_t clampedI = std::max( settings.min().x(), std::min( settings.max().x(), static_cast<size_t>( origI ) ) );
    size_t clampedJ = std::max( settings.min().y(), std::min( settings.max().y(), static_cast<size_t>( origJ ) ) );

    // Transform with clamped coordinates (this will always succeed)
    // Center the coordinate in the refined cell block
    size_t sectorI = ( clampedI - settings.min().x() ) * settings.refinement().x() + ( settings.refinement().x() + 1 ) / 2;
    size_t sectorJ = ( clampedJ - settings.min().y() ) * settings.refinement().y() + ( settings.refinement().y() + 1 ) / 2;

    if ( origI < static_cast<int>( settings.min().x() ) || origI > static_cast<int>( settings.max().x() ) ||
         origJ < static_cast<int>( settings.min().y() ) || origJ > static_cast<int>( settings.max().y() ) )
    {
        RiaLogging::info( QString( "Well %1 HEAD position (%2, %3) outside sector, clamped to (%4, %5)" )
                              .arg( wellName.c_str() )
                              .arg( origI + 1 )
                              .arg( origJ + 1 )
                              .arg( sectorI )
                              .arg( sectorJ ) );
    }

    using W = Opm::ParserKeywords::WELSPECS;
    items.push_back( RifOpmDeckTools::item( W::HEAD_I::itemName, static_cast<int>( sectorI ) ) );
    items.push_back( RifOpmDeckTools::item( W::HEAD_J::itemName, static_cast<int>( sectorJ ) ) );

    // Copy remaining items
    for ( size_t i = 4; i < record.size(); ++i )
    {
        items.push_back( record.getItem( i ) );
    }

    return Opm::DeckRecord{ std::move( items ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<Opm::DeckRecord, QString> RigSimulationInputTool::processCompdatRecord( const Opm::DeckRecord&            record,
                                                                                      const std::string&                wellName,
                                                                                      const RigSimulationInputSettings& settings )
{
    // COMPDAT format: WELL I J K1 K2 STATE ...
    // Items: 0=WELL, 1=I, 2=J, 3=K1, 4=K2
    if ( record.size() < 5 )
    {
        return std::unexpected( QString( "COMPDAT record for well %1 has insufficient items (expected at least 5, got %2)" )
                                    .arg( wellName.c_str() )
                                    .arg( record.size() ) );
    }

    std::vector<Opm::DeckItem> items;

    // Copy well name
    items.push_back( record.getItem( 0 ) );

    // Transform I, J, K1, K2
    int origI  = record.getItem( 1 ).get<int>( 0 ) - 1; // Convert to 0-based
    int origJ  = record.getItem( 2 ).get<int>( 0 ) - 1;
    int origK1 = record.getItem( 3 ).get<int>( 0 ) - 1;
    int origK2 = record.getItem( 4 ).get<int>( 0 ) - 1;

    // Transform K1
    caf::VecIjk0 origIjkK1( origI, origJ, origK1 );
    auto         transformResultK1 =
        RigGridExportAdapter::transformIjkToSectorCoordinates( origIjkK1, settings.min(), settings.max(), settings.refinement(), true );

    // Transform K2
    caf::VecIjk0 origIjkK2( origI, origJ, origK2 );
    auto         transformResultK2 =
        RigGridExportAdapter::transformIjkToSectorCoordinates( origIjkK2, settings.min(), settings.max(), settings.refinement(), true );

    if ( !transformResultK1 )
    {
        return std::unexpected(
            QString( "COMPDAT K1 coordinate for well %1 is out of sector bounds: %2" ).arg( wellName.c_str() ).arg( transformResultK1.error() ) );
    }

    if ( !transformResultK2 )
    {
        return std::unexpected(
            QString( "COMPDAT K2 coordinate for well %1 is out of sector bounds: %2" ).arg( wellName.c_str() ).arg( transformResultK2.error() ) );
    }

    using C = Opm::ParserKeywords::COMPDAT;
    items.push_back( RifOpmDeckTools::item( C::I::itemName, static_cast<int>( transformResultK1->x() ) ) );
    items.push_back( RifOpmDeckTools::item( C::J::itemName, static_cast<int>( transformResultK1->y() ) ) );
    items.push_back( RifOpmDeckTools::item( C::K1::itemName, static_cast<int>( transformResultK1->z() ) ) );
    items.push_back( RifOpmDeckTools::item( C::K2::itemName, static_cast<int>( transformResultK2->z() ) ) );

    // Copy remaining items
    for ( size_t i = 5; i < record.size(); ++i )
    {
        items.push_back( record.getItem( i ) );
    }

    return Opm::DeckRecord{ std::move( items ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<Opm::DeckRecord, QString> RigSimulationInputTool::processCompsegsRecord( const Opm::DeckRecord&            record,
                                                                                       const std::string&                wellName,
                                                                                       bool                              isWellNameRecord,
                                                                                       const RigSimulationInputSettings& settings )
{
    // COMPSEGS format: first record is well name, subsequent records are segment data
    // Well name record: just copy as-is
    if ( isWellNameRecord )
    {
        return Opm::DeckRecord( record );
    }

    // Segment record format: I J K BRANCH START_MD END_MD ...
    // Items: 0=I, 1=J, 2=K
    if ( record.size() < 3 )
    {
        return std::unexpected( QString( "COMPSEGS segment record for well %1 has insufficient items (expected at least 3, got %2)" )
                                    .arg( wellName.c_str() )
                                    .arg( record.size() ) );
    }

    std::vector<Opm::DeckItem> items;

    // Transform I, J, K (first three items)
    int origI = record.getItem( 0 ).get<int>( 0 ) - 1; // Convert to 0-based
    int origJ = record.getItem( 1 ).get<int>( 0 ) - 1;
    int origK = record.getItem( 2 ).get<int>( 0 ) - 1;

    caf::VecIjk0 origIjk( origI, origJ, origK );
    auto         transformResult =
        RigGridExportAdapter::transformIjkToSectorCoordinates( origIjk, settings.min(), settings.max(), settings.refinement(), true );

    if ( !transformResult )
    {
        return std::unexpected(
            QString( "COMPSEGS segment coordinate for well %1 is out of sector bounds: %2" ).arg( wellName.c_str() ).arg( transformResult.error() ) );
    }

    // Add transformed I, J, K
    items.push_back( RifOpmDeckTools::item( "I", static_cast<int>( transformResult->x() ) ) );
    items.push_back( RifOpmDeckTools::item( "J", static_cast<int>( transformResult->y() ) ) );
    items.push_back( RifOpmDeckTools::item( "K", static_cast<int>( transformResult->z() ) ) );

    // Copy remaining items
    for ( size_t i = 3; i < record.size(); ++i )
    {
        items.push_back( record.getItem( i ) );
    }

    return Opm::DeckRecord{ std::move( items ) };
}

//--------------------------------------------------------------------------------------------------
/// Helper function to transform bounding box from global to sector coordinates
/// Performs intersection, clamping, and coordinate transformation
/// Returns bounding box with 0-based sector-relative coordinates
//--------------------------------------------------------------------------------------------------
std::expected<RigBoundingBoxIjk, QString> RigSimulationInputTool::transformBoxToSectorCoordinates( const RigBoundingBoxIjk& inputBox,
                                                                                                   const caf::VecIjk0&      sectorMin,
                                                                                                   const caf::VecIjk0&      sectorMax,
                                                                                                   const cvf::Vec3st&       refinement,
                                                                                                   const QString&           keywordName,
                                                                                                   const QString& recordIdentifier )
{
    // Create sector bounding box
    RigBoundingBoxIjk sectorBox( sectorMin, sectorMax );
    // cvf::Vec3st( sectorMin.x(), sectorMin.y(), sectorMin.z() ),
    //                              cvf::Vec3st( sectorMax.x(), sectorMax.y(), sectorMax.z() ) );

    // Check if boxes overlap and get intersection
    auto intersection = inputBox.intersection( sectorBox );
    if ( !intersection )
    {
        // No overlap with sector - skip this record
        const auto& origMin = inputBox.min();
        const auto& origMax = inputBox.max();
        RiaLogging::info( QString( "%1 record%2 [%3-%4, %5-%6, %7-%8] does not overlap with sector - skipping" )
                              .arg( keywordName )
                              .arg( recordIdentifier.isEmpty() ? "" : " " + recordIdentifier )
                              .arg( origMin.x() + 1 )
                              .arg( origMax.x() + 1 )
                              .arg( origMin.y() + 1 )
                              .arg( origMax.y() + 1 )
                              .arg( origMin.z() + 1 )
                              .arg( origMax.z() + 1 ) );
        return std::unexpected( QString( "%1 record does not overlap with sector" ).arg( keywordName ) );
    }

    // Get the clamped coordinates from the intersection
    cvf::Vec3st clampedMin = intersection->min();
    cvf::Vec3st clampedMax = intersection->max();

    // Log if clamping occurred (partial overlap)
    const auto& origMin = inputBox.min();
    const auto& origMax = inputBox.max();
    if ( origMin.x() != clampedMin.x() || origMax.x() != clampedMax.x() || origMin.y() != clampedMin.y() || origMax.y() != clampedMax.y() ||
         origMin.z() != clampedMin.z() || origMax.z() != clampedMax.z() )
    {
        RiaLogging::info(
            QString( "%1 record%2 partially overlaps sector, clamped from [%3-%4, %5-%6, %7-%8] to [%9-%10, %11-%12, %13-%14]" )
                .arg( keywordName )
                .arg( recordIdentifier.isEmpty() ? "" : " " + recordIdentifier )
                .arg( origMin.x() + 1 )
                .arg( origMax.x() + 1 )
                .arg( origMin.y() + 1 )
                .arg( origMax.y() + 1 )
                .arg( origMin.z() + 1 )
                .arg( origMax.z() + 1 )
                .arg( clampedMin.x() + 1 )
                .arg( clampedMax.x() + 1 )
                .arg( clampedMin.y() + 1 )
                .arg( clampedMax.y() + 1 )
                .arg( clampedMin.z() + 1 )
                .arg( clampedMax.z() + 1 ) );
    }

    // Transform clamped coordinates to sector-relative coordinates
    auto transformResult1 =
        RigGridExportAdapter::transformIjkToSectorCoordinates( caf::VecIjk0( clampedMin.x(), clampedMin.y(), clampedMin.z() ),
                                                               sectorMin,
                                                               sectorMax,
                                                               refinement,
                                                               false,
                                                               false );
    auto transformResult2 =
        RigGridExportAdapter::transformIjkToSectorCoordinates( caf::VecIjk0( clampedMax.x(), clampedMax.y(), clampedMax.z() ),
                                                               sectorMin,
                                                               sectorMax,
                                                               refinement,
                                                               false,
                                                               true );

    if ( !transformResult1 )
    {
        return std::unexpected( QString( "%1 min coordinate is out of sector bounds: %2" ).arg( keywordName ).arg( transformResult1.error() ) );
    }

    if ( !transformResult2 )
    {
        return std::unexpected( QString( "%1 max coordinate is out of sector bounds: %2" ).arg( keywordName ).arg( transformResult2.error() ) );
    }

    // Return bounding box with 0-based sector-relative coordinates
    return RigBoundingBoxIjk( cvf::Vec3st( transformResult1->x() - 1, transformResult1->y() - 1, transformResult1->z() - 1 ),
                              cvf::Vec3st( transformResult2->x() - 1, transformResult2->y() - 1, transformResult2->z() - 1 ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<Opm::DeckRecord, QString> RigSimulationInputTool::processEqualsRecord( const Opm::DeckRecord& record,
                                                                                     const caf::VecIjk0&    min,
                                                                                     const caf::VecIjk0&    max,
                                                                                     const cvf::Vec3st&     refinement )
{
    // EQUALS format: FIELD VALUE I1 I2 J1 J2 K1 K2
    // Items: 0=FIELD, 1=VALUE, 2=I1, 3=I2, 4=J1, 5=J2, 6=K1, 7=K2
    if ( record.size() < 2 )
    {
        return std::unexpected( QString( "EQUALS record has insufficient items (expected at least 2, got %1)" ).arg( record.size() ) );
    }

    std::vector<Opm::DeckItem> items;

    // Copy field name and value (first two items)
    items.push_back( record.getItem( 0 ) );
    items.push_back( record.getItem( 1 ) );

    if ( record.size() >= 8 && record.getItem( 2 ).hasValue( 0 ) && record.getItem( 3 ).hasValue( 0 ) && record.getItem( 4 ).hasValue( 0 ) &&
         record.getItem( 5 ).hasValue( 0 ) && record.getItem( 6 ).hasValue( 0 ) && record.getItem( 7 ).hasValue( 0 ) )
    {
        // Transform IJK box coordinates (items 2-7: I1, I2, J1, J2, K1, K2)
        // Note: EQUALS uses 1-based Eclipse coordinates
        int origI1 = record.getItem( 2 ).get<int>( 0 ) - 1; // Convert to 0-based
        int origI2 = record.getItem( 3 ).get<int>( 0 ) - 1;
        int origJ1 = record.getItem( 4 ).get<int>( 0 ) - 1;
        int origJ2 = record.getItem( 5 ).get<int>( 0 ) - 1;
        int origK1 = record.getItem( 6 ).get<int>( 0 ) - 1;
        int origK2 = record.getItem( 7 ).get<int>( 0 ) - 1;

        // Create input bounding box (0-based, inclusive)
        RigBoundingBoxIjk inputBox( cvf::Vec3st( origI1, origJ1, origK1 ), cvf::Vec3st( origI2, origJ2, origK2 ) );

        // Get field name for logging
        QString fieldName   = QString::fromStdString( record.getItem( 0 ).get<std::string>( 0 ) );
        QString recordIdent = QString( "for field '%1'" ).arg( fieldName );

        // Transform box to sector coordinates
        auto transformResult = transformBoxToSectorCoordinates( inputBox, min, max, refinement, "EQUALS", recordIdent );
        if ( !transformResult )
        {
            return std::unexpected( transformResult.error() );
        }

        using E = Opm::ParserKeywords::EQUALS;
        items.push_back( RifOpmDeckTools::item( E::I1::itemName, static_cast<int>( transformResult->min().x() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( E::I2::itemName, static_cast<int>( transformResult->max().x() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( E::J1::itemName, static_cast<int>( transformResult->min().y() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( E::J2::itemName, static_cast<int>( transformResult->max().y() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( E::K1::itemName, static_cast<int>( transformResult->min().z() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( E::K2::itemName, static_cast<int>( transformResult->max().z() + 1 ) ) );
    }

    return Opm::DeckRecord{ std::move( items ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<Opm::DeckRecord, QString> RigSimulationInputTool::processMultiplyRecord( const Opm::DeckRecord& record,
                                                                                       const caf::VecIjk0&    min,
                                                                                       const caf::VecIjk0&    max,
                                                                                       const cvf::Vec3st&     refinement )
{
    // MULTIPLY format: FIELD FACTOR I1 I2 J1 J2 K1 K2
    // Items: 0=FIELD, 1=FACTOR, 2=I1, 3=I2, 4=J1, 5=J2, 6=K1, 7=K2
    if ( record.size() < 8 )
    {
        return std::unexpected( QString( "MULTIPLY record has insufficient items (expected at least 8, got %1)" ).arg( record.size() ) );
    }

    std::vector<Opm::DeckItem> items;

    // Copy field name and factor (first two items)
    items.push_back( record.getItem( 0 ) );
    items.push_back( record.getItem( 1 ) );

    if ( record.getItem( 2 ).hasValue( 0 ) && record.getItem( 3 ).hasValue( 0 ) && record.getItem( 4 ).hasValue( 0 ) &&
         record.getItem( 5 ).hasValue( 0 ) && record.getItem( 6 ).hasValue( 0 ) && record.getItem( 7 ).hasValue( 0 ) )
    {
        // Transform IJK box coordinates (items 2-7: I1, I2, J1, J2, K1, K2)
        // Note: MULTIPLY uses 1-based Eclipse coordinates
        int origI1 = record.getItem( 2 ).get<int>( 0 ) - 1; // Convert to 0-based
        int origI2 = record.getItem( 3 ).get<int>( 0 ) - 1;
        int origJ1 = record.getItem( 4 ).get<int>( 0 ) - 1;
        int origJ2 = record.getItem( 5 ).get<int>( 0 ) - 1;
        int origK1 = record.getItem( 6 ).get<int>( 0 ) - 1;
        int origK2 = record.getItem( 7 ).get<int>( 0 ) - 1;

        // Create input bounding box (0-based, inclusive)
        RigBoundingBoxIjk inputBox( cvf::Vec3st( origI1, origJ1, origK1 ), cvf::Vec3st( origI2, origJ2, origK2 ) );

        // Get field name for logging
        QString fieldName   = QString::fromStdString( record.getItem( 0 ).get<std::string>( 0 ) );
        QString recordIdent = QString( "for field '%1'" ).arg( fieldName );

        // Transform box to sector coordinates
        auto transformResult = transformBoxToSectorCoordinates( inputBox, min, max, refinement, "MULTIPLY", recordIdent );
        if ( !transformResult )
        {
            return std::unexpected( transformResult.error() );
        }

        using M = Opm::ParserKeywords::MULTIPLY;
        items.push_back( RifOpmDeckTools::item( M::I1::itemName, static_cast<int>( transformResult->min().x() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( M::I2::itemName, static_cast<int>( transformResult->max().x() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( M::J1::itemName, static_cast<int>( transformResult->min().y() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( M::J2::itemName, static_cast<int>( transformResult->max().y() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( M::K1::itemName, static_cast<int>( transformResult->min().z() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( M::K2::itemName, static_cast<int>( transformResult->max().z() + 1 ) ) );
    }

    return Opm::DeckRecord{ std::move( items ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<Opm::DeckRecord, QString> RigSimulationInputTool::processAddRecord( const Opm::DeckRecord& record,
                                                                                  const caf::VecIjk0&    min,
                                                                                  const caf::VecIjk0&    max,
                                                                                  const cvf::Vec3st&     refinement )
{
    // ADD format: FIELD SHIFT I1 I2 J1 J2 K1 K2
    // Items: 0=FIELD, 1=SHIFT, 2=I1, 3=I2, 4=J1, 5=J2, 6=K1, 7=K2
    if ( record.size() < 8 )
    {
        return std::unexpected( QString( "ADD record has insufficient items (expected at least 8, got %1)" ).arg( record.size() ) );
    }

    std::vector<Opm::DeckItem> items;

    // Copy field name and shift value (first two items)
    items.push_back( record.getItem( 0 ) );
    items.push_back( record.getItem( 1 ) );

    if ( record.getItem( 2 ).hasValue( 0 ) && record.getItem( 3 ).hasValue( 0 ) && record.getItem( 4 ).hasValue( 0 ) &&
         record.getItem( 5 ).hasValue( 0 ) && record.getItem( 6 ).hasValue( 0 ) && record.getItem( 7 ).hasValue( 0 ) )
    {
        // Transform IJK box coordinates (items 2-7: I1, I2, J1, J2, K1, K2)
        // Note: ADD uses 1-based Eclipse coordinates
        int origI1 = record.getItem( 2 ).get<int>( 0 ) - 1; // Convert to 0-based
        int origI2 = record.getItem( 3 ).get<int>( 0 ) - 1;
        int origJ1 = record.getItem( 4 ).get<int>( 0 ) - 1;
        int origJ2 = record.getItem( 5 ).get<int>( 0 ) - 1;
        int origK1 = record.getItem( 6 ).get<int>( 0 ) - 1;
        int origK2 = record.getItem( 7 ).get<int>( 0 ) - 1;

        // Create input bounding box (0-based, inclusive)
        RigBoundingBoxIjk inputBox( cvf::Vec3st( origI1, origJ1, origK1 ), cvf::Vec3st( origI2, origJ2, origK2 ) );

        // Get field name for logging
        QString fieldName   = QString::fromStdString( record.getItem( 0 ).get<std::string>( 0 ) );
        QString recordIdent = QString( "for field '%1'" ).arg( fieldName );

        // Transform box to sector coordinates
        auto transformResult = transformBoxToSectorCoordinates( inputBox, min, max, refinement, "ADD", recordIdent );
        if ( !transformResult )
        {
            return std::unexpected( transformResult.error() );
        }

        using A = Opm::ParserKeywords::ADD;
        items.push_back( RifOpmDeckTools::item( A::I1::itemName, static_cast<int>( transformResult->min().x() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( A::I2::itemName, static_cast<int>( transformResult->max().x() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( A::J1::itemName, static_cast<int>( transformResult->min().y() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( A::J2::itemName, static_cast<int>( transformResult->max().y() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( A::K1::itemName, static_cast<int>( transformResult->min().z() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( A::K2::itemName, static_cast<int>( transformResult->max().z() + 1 ) ) );
    }

    return Opm::DeckRecord{ std::move( items ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<Opm::DeckRecord, QString> RigSimulationInputTool::processAquconRecord( const Opm::DeckRecord& record,
                                                                                     const caf::VecIjk0&    min,
                                                                                     const caf::VecIjk0&    max,
                                                                                     const cvf::Vec3st&     refinement )
{
    // AQUCON format: ID I1 I2 J1 J2 K1 K2 CONNECT_FACE ...
    // Items: 0=ID, 1=I1, 2=I2, 3=J1, 4=J2, 5=K1, 6=K2, 7+=other parameters
    if ( record.size() < 1 )
    {
        return std::unexpected( QString( "AQUCON record has insufficient items (expected at least 1, got %1)" ).arg( record.size() ) );
    }

    std::vector<Opm::DeckItem> items;

    // Copy aquifer ID (first item)
    items.push_back( record.getItem( 0 ) );

    if ( record.size() >= 7 && record.getItem( 1 ).hasValue( 0 ) && record.getItem( 2 ).hasValue( 0 ) && record.getItem( 3 ).hasValue( 0 ) &&
         record.getItem( 4 ).hasValue( 0 ) && record.getItem( 5 ).hasValue( 0 ) && record.getItem( 6 ).hasValue( 0 ) )
    {
        // Transform IJK box coordinates (items 1-6: I1, I2, J1, J2, K1, K2)
        // Note: AQUCON uses 1-based Eclipse coordinates
        int origI1 = record.getItem( 1 ).get<int>( 0 ) - 1; // Convert to 0-based
        int origI2 = record.getItem( 2 ).get<int>( 0 ) - 1;
        int origJ1 = record.getItem( 3 ).get<int>( 0 ) - 1;
        int origJ2 = record.getItem( 4 ).get<int>( 0 ) - 1;
        int origK1 = record.getItem( 5 ).get<int>( 0 ) - 1;
        int origK2 = record.getItem( 6 ).get<int>( 0 ) - 1;

        // Create input bounding box (0-based, inclusive)
        RigBoundingBoxIjk inputBox( cvf::Vec3st( origI1, origJ1, origK1 ), cvf::Vec3st( origI2, origJ2, origK2 ) );

        // Get aquifer ID for logging
        int     aquiferId   = record.getItem( 0 ).get<int>( 0 );
        QString recordIdent = QString( "for aquifer %1" ).arg( aquiferId );

        // Transform box to sector coordinates
        auto transformResult = transformBoxToSectorCoordinates( inputBox, min, max, refinement, "AQUCON", recordIdent );
        if ( !transformResult )
        {
            return std::unexpected( transformResult.error() );
        }

        using A = Opm::ParserKeywords::AQUCON;
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
std::expected<Opm::DeckRecord, QString> RigSimulationInputTool::processCopyRecord( const Opm::DeckRecord& record,
                                                                                   const caf::VecIjk0&    min,
                                                                                   const caf::VecIjk0&    max,
                                                                                   const cvf::Vec3st&     refinement )
{
    // COPY format: SRC_ARRAY TARGET_ARRAY I1 I2 J1 J2 K1 K2
    // Items: 0=SRC_ARRAY, 1=TARGET_ARRAY, 2=I1, 3=I2, 4=J1, 5=J2, 6=K1, 7=K2
    if ( record.size() < 2 )
    {
        return std::unexpected( QString( "COPY record has insufficient items (expected at least 2, got %1)" ).arg( record.size() ) );
    }

    std::vector<Opm::DeckItem> items;

    // Copy source and target array names (first two items)
    items.push_back( record.getItem( 0 ) );
    items.push_back( record.getItem( 1 ) );

    // Check if IJK coordinates are specified and have values
    if ( record.size() >= 8 && record.getItem( 2 ).hasValue( 0 ) && record.getItem( 3 ).hasValue( 0 ) && record.getItem( 4 ).hasValue( 0 ) &&
         record.getItem( 5 ).hasValue( 0 ) && record.getItem( 6 ).hasValue( 0 ) && record.getItem( 7 ).hasValue( 0 ) )
    {
        // Transform IJK box coordinates (items 2-7: I1, I2, J1, J2, K1, K2)
        // Note: COPY uses 1-based Eclipse coordinates
        int origI1 = record.getItem( 2 ).get<int>( 0 ) - 1; // Convert to 0-based
        int origI2 = record.getItem( 3 ).get<int>( 0 ) - 1;
        int origJ1 = record.getItem( 4 ).get<int>( 0 ) - 1;
        int origJ2 = record.getItem( 5 ).get<int>( 0 ) - 1;
        int origK1 = record.getItem( 6 ).get<int>( 0 ) - 1;
        int origK2 = record.getItem( 7 ).get<int>( 0 ) - 1;

        // Create input bounding box (0-based, inclusive)
        RigBoundingBoxIjk inputBox( cvf::Vec3st( origI1, origJ1, origK1 ), cvf::Vec3st( origI2, origJ2, origK2 ) );

        // Get array names for logging
        QString srcArray    = QString::fromStdString( record.getItem( 0 ).get<std::string>( 0 ) );
        QString targetArray = QString::fromStdString( record.getItem( 1 ).get<std::string>( 0 ) );
        QString recordIdent = QString( "for %1->%2" ).arg( srcArray ).arg( targetArray );

        // Transform box to sector coordinates
        auto transformResult = transformBoxToSectorCoordinates( inputBox, min, max, refinement, "COPY", recordIdent );
        if ( !transformResult )
        {
            return std::unexpected( transformResult.error() );
        }

        using C = Opm::ParserKeywords::COPY;
        items.push_back( RifOpmDeckTools::item( C::I1::itemName, static_cast<int>( transformResult->min().x() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( C::I2::itemName, static_cast<int>( transformResult->max().x() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( C::J1::itemName, static_cast<int>( transformResult->min().y() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( C::J2::itemName, static_cast<int>( transformResult->max().y() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( C::K1::itemName, static_cast<int>( transformResult->min().z() + 1 ) ) );
        items.push_back( RifOpmDeckTools::item( C::K2::itemName, static_cast<int>( transformResult->max().z() + 1 ) ) );
    }
    else
    {
        // No IJK coordinates specified - copy applies to entire grid
        // Just copy remaining items as-is (they will be default values or not present)
        for ( size_t i = 2; i < record.size(); ++i )
        {
            items.push_back( record.getItem( i ) );
        }
    }

    return Opm::DeckRecord{ std::move( items ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<Opm::DeckRecord, QString> RigSimulationInputTool::processBoxRecord( const Opm::DeckRecord& record,
                                                                                  const caf::VecIjk0&    min,
                                                                                  const caf::VecIjk0&    max,
                                                                                  const cvf::Vec3st&     refinement )
{
    // BOX format: I1 I2 J1 J2 K1 K2
    // Items: 0=I1, 1=I2, 2=J1, 3=J2, 4=K1, 5=K2
    if ( record.size() < 6 )
    {
        return std::unexpected( QString( "BOX record has insufficient items (expected at least 6, got %1)" ).arg( record.size() ) );
    }

    std::vector<Opm::DeckItem> items;

    // Transform IJK box coordinates (items 0-5: I1, I2, J1, J2, K1, K2)
    // Note: BOX uses 1-based Eclipse coordinates
    int origI1 = record.getItem( 0 ).get<int>( 0 ) - 1; // Convert to 0-based
    int origI2 = record.getItem( 1 ).get<int>( 0 ) - 1;
    int origJ1 = record.getItem( 2 ).get<int>( 0 ) - 1;
    int origJ2 = record.getItem( 3 ).get<int>( 0 ) - 1;
    int origK1 = record.getItem( 4 ).get<int>( 0 ) - 1;
    int origK2 = record.getItem( 5 ).get<int>( 0 ) - 1;

    // Create input bounding box (0-based, inclusive)
    RigBoundingBoxIjk inputBox( cvf::Vec3st( origI1, origJ1, origK1 ), cvf::Vec3st( origI2, origJ2, origK2 ) );

    // Transform box to sector coordinates
    auto transformResult = transformBoxToSectorCoordinates( inputBox, min, max, refinement, "BOX", "" );
    if ( !transformResult )
    {
        return std::unexpected( transformResult.error() );
    }

    using B = Opm::ParserKeywords::BOX;
    items.push_back( RifOpmDeckTools::item( B::I1::itemName, static_cast<int>( transformResult->min().x() + 1 ) ) );
    items.push_back( RifOpmDeckTools::item( B::I2::itemName, static_cast<int>( transformResult->max().x() + 1 ) ) );
    items.push_back( RifOpmDeckTools::item( B::J1::itemName, static_cast<int>( transformResult->min().y() + 1 ) ) );
    items.push_back( RifOpmDeckTools::item( B::J2::itemName, static_cast<int>( transformResult->max().y() + 1 ) ) );
    items.push_back( RifOpmDeckTools::item( B::K1::itemName, static_cast<int>( transformResult->min().z() + 1 ) ) );
    items.push_back( RifOpmDeckTools::item( B::K2::itemName, static_cast<int>( transformResult->max().z() + 1 ) ) );

    return Opm::DeckRecord{ std::move( items ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputTool::filterAndUpdateWellKeywords( RimEclipseCase*                   eclipseCase,
                                                                                  const RigSimulationInputSettings& settings,
                                                                                  RifOpmFlowDeckFile&               deckFile )
{
    // Find wells that intersect with the sector
    auto intersectingWells = findIntersectingWells( eclipseCase, settings.min(), settings.max() );

    if ( intersectingWells.empty() )
    {
        RiaLogging::info( "No wells intersect with the selected sector - no well filtering needed" );
        return {};
    }

    // Create set of valid well names for fast lookup
    std::set<std::string> validWellNames;
    for ( const auto& wellData : intersectingWells )
    {
        if ( wellData )
        {
            validWellNames.insert( wellData->m_wellName.toStdString() );
        }
    }

    RiaLogging::info( QString( "Found %1 wells intersecting with sector: %2" )
                          .arg( validWellNames.size() )
                          .arg( QString::fromStdString(
                              [&validWellNames]()
                              {
                                  std::string names;
                                  for ( const auto& name : validWellNames )
                                  {
                                      if ( !names.empty() ) names += ", ";
                                      names += name;
                                  }
                                  return names;
                              }() ) ) );

    // List of well-related keywords to filter (keywords that reference well names)
    std::vector<std::string> wellKeywords = { "COMPDAT",
                                              "COMPLUMP",
                                              "COMPORD",
                                              "COMPSEGS",
                                              "WCONHIST",
                                              "WCONINJH",
                                              "WCONINJE",
                                              "WCONPROD",
                                              "WELSEGS",
                                              "WELSPECS",
                                              "WELOPEN",
                                              "WELTARG",
                                              "WPAVEDEP",
                                              "WRFTPLT",
                                              "WTRACER",
                                              "WTEST" };

    // Process each type of well keyword
    // Use findAllKeywordsWithIndices to get all occurrences with their positions
    for ( const auto& keywordName : wellKeywords )
    {
        auto keywordsWithIndices = deckFile.findAllKeywordsWithIndices( keywordName );
        if ( keywordsWithIndices.empty() ) continue;

        RiaLogging::info(
            QString( "Processing %1 occurrence(s) of keyword '%2'" ).arg( keywordsWithIndices.size() ).arg( keywordName.c_str() ) );

        int replacedCount = 0;
        int removedCount  = 0;

        // Process in reverse order so indices remain valid after modifications
        for ( auto it = keywordsWithIndices.rbegin(); it != keywordsWithIndices.rend(); ++it )
        {
            const Opm::FileDeck::Index& index   = it->first;
            const Opm::DeckKeyword&     keyword = it->second;

            // Create new empty keyword with same name and location
            Opm::DeckKeyword filteredKeyword( keyword.location(), keyword.name() );

            // Track current well for COMPSEGS/WELSEGS (where only first record has well name)
            std::string currentSegmentWell;
            bool        keepSegmentRecords = false;

            try
            {
                for ( size_t recordIdx = 0; recordIdx < keyword.size(); ++recordIdx )
                {
                    const auto& record = keyword.getRecord( recordIdx );

                    // First item in well keywords is typically the well name
                    if ( record.size() == 0 ) continue;

                    const auto& wellNameItem = record.getItem( 0 );

                    // Check if first item is a string (well name) or other type (segment data)
                    bool isWellNameRecord = wellNameItem.hasValue( 0 ) && ( wellNameItem.getType() == Opm::type_tag::string );

                    std::string wellName;
                    if ( isWellNameRecord )
                    {
                        wellName = wellNameItem.get<std::string>( 0 );

                        // For COMPSEGS/WELSEGS, update the current well context
                        if ( keywordName == "COMPSEGS" || keywordName == "WELSEGS" )
                        {
                            currentSegmentWell = wellName;
                            keepSegmentRecords = ( validWellNames.find( wellName ) != validWellNames.end() );
                        }
                    }
                    else if ( keywordName == "COMPSEGS" || keywordName == "WELSEGS" )
                    {
                        // Segment data record - use the current well context
                        wellName = currentSegmentWell;
                    }

                    // Check if this well is in our valid set
                    if ( ( isWellNameRecord && validWellNames.find( wellName ) != validWellNames.end() ) ||
                         ( !isWellNameRecord && ( keywordName == "COMPSEGS" || keywordName == "WELSEGS" ) && keepSegmentRecords ) )
                    {
                        // For keywords with IJK coordinates, we need to transform them
                        if ( keywordName == "WELSPECS" )
                        {
                            auto result = processWelspecsRecord( record, wellName, settings );
                            if ( result )
                            {
                                filteredKeyword.addRecord( std::move( result.value() ) );
                            }
                            else
                            {
                                RiaLogging::warning(
                                    QString( "Failed to process WELSPECS for well %1: %2" ).arg( wellName.c_str() ).arg( result.error() ) );
                            }
                        }
                        else if ( keywordName == "COMPDAT" )
                        {
                            auto result = processCompdatRecord( record, wellName, settings );
                            if ( result )
                            {
                                filteredKeyword.addRecord( std::move( result.value() ) );
                            }
                            else
                            {
                                RiaLogging::warning(
                                    QString( "Failed to process COMPDAT for well %1: %2" ).arg( wellName.c_str() ).arg( result.error() ) );
                            }
                        }
                        else if ( keywordName == "COMPSEGS" )
                        {
                            auto result = processCompsegsRecord( record, wellName, isWellNameRecord, settings );
                            if ( result )
                            {
                                filteredKeyword.addRecord( std::move( result.value() ) );
                            }
                            else
                            {
                                RiaLogging::warning(
                                    QString( "Failed to process COMPSEGS for well %1: %2" ).arg( wellName.c_str() ).arg( result.error() ) );
                            }
                        }
                        else if ( keywordName == "WELSEGS" )
                        {
                            // WELSEGS: just copy all records for now
                            filteredKeyword.addRecord( Opm::DeckRecord( record ) );
                        }
                        else
                        {
                            // For other keywords (WCONHIST, WELTARG, etc.), just copy if well name matches
                            filteredKeyword.addRecord( Opm::DeckRecord( record ) );
                        }
                    }
                }
            }
            catch ( std::exception& e )
            {
                RiaLogging::warning( QString( "EXCEPTION for keyword '%1': %2" ).arg( keyword.name().c_str() ).arg( e.what() ) );
            }

            // Replace or remove this keyword occurrence in place
            if ( filteredKeyword.size() == 0 )
            {
                // Keyword is empty after filtering - remove it
                deckFile.replaceKeywordAtIndex( index, Opm::DeckKeyword( keyword.location(), "SKIP" ) );
                removedCount++;
            }
            else
            {
                // Replace with filtered version
                deckFile.replaceKeywordAtIndex( index, filteredKeyword );
                replacedCount++;
            }
        }

        RiaLogging::info(
            QString( "Processed keyword '%1': %2 replaced, %3 removed" ).arg( keywordName.c_str() ).arg( replacedCount ).arg( removedCount ) );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputTool::addOperNumRegionAndOperater( RimEclipseCase*                   eclipseCase,
                                                                                  const RigSimulationInputSettings& settings,
                                                                                  RifOpmFlowDeckFile&               deckFile,
                                                                                  int                               operNumRegion,
                                                                                  double                            porvMultiplier )
{
    // Update REGDIMS and add OPERATER keyword for OPERNUM regions
    // Get the OPERNUM region number that was assigned to border cells

    RiaLogging::info( QString( "Using OPERNUM region %1 for border cells" ).arg( operNumRegion ) );

    // Ensure REGDIMS keyword exists
    if ( !deckFile.ensureRegdimsKeyword() )
    {
        return std::unexpected( "Failed to ensure REGDIMS keyword exists in RUNSPEC section" );
    }

    // Read current REGDIMS values
    auto regdimsValues = deckFile.regdims();
    if ( regdimsValues.empty() )
    {
        // TODO: improve this?
        // Use defaults if reading failed
        regdimsValues = { 1, 1, 0, 0, 0, 1, 0 }; // NTFIP NMFIPR NRFREG NTFREG MAX_ETRACK NTCREG MAX_OPERNUM
    }

    // Update MAX_OPERNUM (item 7) to the new region number
    if ( regdimsValues.size() < 7 )
    {
        regdimsValues.resize( 7, 0 );
    }
    regdimsValues[6] = operNumRegion; // Index 6 is item 7 (MAX_OPERNUM)

    // Update REGDIMS in deck
    if ( !deckFile.setRegdims( regdimsValues[0],
                               regdimsValues[1],
                               regdimsValues[2],
                               regdimsValues[3],
                               regdimsValues[4],
                               regdimsValues[5],
                               regdimsValues[6] ) )
    {
        return std::unexpected( "Failed to update REGDIMS keyword in deck file" );
    }

    // Check if OPERNUM keyword already exists in deck
    auto opernumKeyword = deckFile.findKeyword( "OPERNUM" );
    if ( !opernumKeyword.has_value() )
    {
        auto keywords = deckFile.keywords( false );
        if ( std::find( keywords.begin(), keywords.end(), "OPERNUM" ) == keywords.end() )
        {
            Opm::DeckKeyword newKw( ( Opm::ParserKeywords::OPERNUM() ) );
            deckFile.addKeyword( "GRID", newKw );
        }

        // Try to extract keyword data
        auto result = RifEclipseInputFileTools::extractKeywordData( eclipseCase->eclipseCaseData(),
                                                                    "OPERNUM",
                                                                    settings.min(),
                                                                    settings.max(),
                                                                    settings.refinement() );
        if ( result )
        {
            // Replace keyword values in deck with extracted data
            if ( deckFile.replaceKeywordData( "OPERNUM", result.value() ) )
            {
                RiaLogging::info( "Added replaced opernum value in existing position." );
            }
            else
            {
                return std::unexpected( "Unable to replace OPERNUM values" );
            }
        }
        else
        {
            return std::unexpected( "No OPERNUM result found." );
        }
    }
    else
    {
        RiaLogging::info( "OPERNUM keyword already exists in deck, skipping INCLUDE" );
    }

    // Create OPERATER keyword to multiply pore volume in border region
    // OPERATER format: TARGET_ARRAY REGION_NUMBER OPERATION ARRAY_PARAMETER PARAM1 PARAM2 REGION_NAME
    // Example: OPERATER / PORV 1 MULTX PORV 1.0e6 1* 1* /
    Opm::DeckKeyword operaterKw =
        RimKeywordFactory::operaterKeyword( "PORV", operNumRegion, "MULTX", "PORV", static_cast<float>( porvMultiplier ) );

    // Add OPERATER keyword to EDIT section
    if ( !deckFile.replaceKeyword( "EDIT", operaterKw ) )
    {
        return std::unexpected( "Failed to add OPERATER keyword to EDIT section" );
    }

    RiaLogging::info( QString( "Added OPERATER keyword to multiply PORV in region %1 by %2" ).arg( operNumRegion ).arg( porvMultiplier ) );

    return {};
}
