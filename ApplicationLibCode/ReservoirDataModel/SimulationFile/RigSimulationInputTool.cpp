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
#include "RiaStdStringTools.h"

#include "RifEclipseInputFileTools.h"
#include "RifOpmDeckTools.h"
#include "RifOpmFlowDeckFile.h"

#include "ProjectDataModel/Jobs/RimKeywordFactory.h"
#include "RimEclipseCase.h"

#include "RigBoundingBoxIjk.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultTools.h"
#include "RigGridExportAdapter.h"
#include "RigMainGrid.h"
#include "RigModelPaddingSettings.h"
#include "RigNonUniformRefinement.h"
#include "RigPadModel.h"
#include "RigResdataGridConverter.h"
#include "RigSimulationInputSettings.h"
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

    int noOfRemovedKeywords = 0;
    for ( auto& kwToRemove : settings.keywordsToRemove() )
    {
        noOfRemovedKeywords += deckFile.removeKeywords( kwToRemove );
    }
    if ( noOfRemovedKeywords > 0 )
    {
        RiaLogging::info( QString( "Removed %1 user specified keywords from deck file." ).arg( noOfRemovedKeywords ) );
    }

    if ( auto result = settings.validateBox(); !result )
    {
        return result;
    }

    if ( auto result = updateCornerPointGridInDeckFile( &eclipseCase, settings, deckFile ); !result )
    {
        return result;
    }

    auto croppedKeywords = cropDataKeywordsInDeckFile( &eclipseCase, settings, deckFile );

    if ( auto result = replaceKeywordValuesInDeckFile( &eclipseCase, settings, deckFile, croppedKeywords ); !result )
    {
        return result;
    }

    // Single-pass BOX/ENDBOX processing: crop data keywords, inject+transform EQUALS/COPY/ADD/MULTIPLY,
    // and transform BOX coordinates, all while tracking BOX/ENDBOX state.
    if ( auto result = transformKeywordsInDeckFile( &eclipseCase, settings, deckFile ); !result )
    {
        return result;
    }

    if ( auto result = replaceAquconKeywordIndices( &eclipseCase, settings, deckFile ); !result )
    {
        return result;
    }

    if ( auto result = replaceAquanconKeywordIndices( &eclipseCase, settings, deckFile ); !result )
    {
        return result;
    }

    if ( auto result = replaceAqunumKeywordIndices( &eclipseCase, settings, deckFile ); !result )
    {
        return result;
    }

    if ( auto result = updateWelldimsKeyword( &eclipseCase, settings, deckFile ); !result )
    {
        return result;
    }

    if ( auto result = addBorderBoundaryConditions( &eclipseCase, settings, visibility, deckFile ); !result )
    {
        return result;
    }

    if ( auto result = addFaultsToDeckFile( &eclipseCase, settings, deckFile ); !result )
    {
        return result;
    }

    auto validWellNames = wellNamesToInclude( &eclipseCase, settings );

    if ( auto result = filterAndUpdateWellKeywords( validWellNames, settings, deckFile ); !result )
    {
        return result;
    }

    if ( auto result = updateWellListKeywords( validWellNames, settings, deckFile ); !result )
    {
        return result;
    }

    if ( auto result = exportEditNncKeyword( &eclipseCase, settings, deckFile ); !result )
    {
        return result;
    }

    // Apply model padding if enabled
    if ( settings.paddingSettings().isEnabled() )
    {
        if ( auto result = applyModelPadding( deckFile, settings.paddingSettings() ); !result )
        {
            return result;
        }
    }

    // Remove SKIP keywords that were used as placeholders for filtered-out keywords
    deckFile.removeKeywords( "SKIP" );

    // TODO: fix this..
    deckFile.removeKeywords( "MAPAXES" );

    // Save the modified deck file to the export directory
    if ( !deckFile.saveDeck( outputFolder.toStdString(), outputFile.toStdString() ) )
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
    auto createGridAdapter = [&]()
    {
        if ( settings.hasNonUniformRefinement() )
        {
            return RigGridExportAdapter( eclipseCase->eclipseCaseData(), settings.min(), settings.max(), settings.nonUniformRefinement() );
        }
        return RigGridExportAdapter( eclipseCase->eclipseCaseData(), settings.min(), settings.max(), settings.refinement() );
    };

    RigGridExportAdapter gridAdapter = createGridAdapter();

    std::vector<float> coordArray;
    std::vector<float> zcornArray;
    std::vector<int>   actnumArray;

    RigResdataGridConverter::convertGridToCornerPointArrays( gridAdapter, coordArray, zcornArray, actnumArray );

    // Remove ACTNUM to handle deck where it appears more than once.
    deckFile.removeKeywords( "ACTNUM" );

    auto keywords = deckFile.keywords( false );

    // Sector dimensions (after refinement)
    int dimenNx = static_cast<int>( gridAdapter.cellCountI() );
    int dimenNy = static_cast<int>( gridAdapter.cellCountJ() );
    int dimenNz = static_cast<int>( gridAdapter.cellCountK() );

    if ( !deckFile.setDimens( dimenNx, dimenNy, dimenNz ) )
    {
        return std::unexpected( "Failed to replace DIMENS keyword in deck file" );
    }

    // SPECGRID has the same dimensions plus NUMRES and COORD_TYPE
    if ( std::find( keywords.begin(), keywords.end(), "SPECGRID" ) == keywords.end() )
    {
        Opm::DeckKeyword newKw( ( Opm::ParserKeywords::SPECGRID() ) );
        deckFile.addKeyword( "GRID", newKw );
    }

    if ( !deckFile.setSpecgrid( dimenNx, dimenNy, dimenNz ) )
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
        Opm::DeckKeyword newKw( ( Opm::ParserKeywords::ACTNUM() ) );
        newKw.setDataKeyword( true ); // needed due to bug(?) in OPM constructor
        deckFile.addKeyword( "GRID", newKw );
    }

    if ( !deckFile.replaceKeyword( "COORD", coords, true /*data kw*/ ) )
    {
        return std::unexpected( "Failed to replace COORD keyword in deck file" );
    }

    if ( !deckFile.replaceKeyword( "ZCORN", zcorn, true /*data kw*/ ) )
    {
        return std::unexpected( "Failed to replace ZCORN keyword in deck file" );
    }

    if ( !deckFile.replaceKeyword( "ACTNUM", actnumArray, true /*data kw*/ ) )
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
                                                                                     RifOpmFlowDeckFile&               deckFile,
                                                                                     const std::set<std::string>&      alreadyCropped )
{
    // Extract and replace keyword data for all keywords in the deck
    auto keywords = deckFile.keywords( false );
    RiaLogging::info( QString( "Processing %1 keywords from deck file" ).arg( keywords.size() ) );

    for ( const auto& keywordStdStr : keywords )
    {
        // Skip keywords already cropped from deck data
        if ( alreadyCropped.count( keywordStdStr ) > 0 ) continue;

        QString keyword = QString::fromStdString( keywordStdStr );

        // Skip special keywords that aren't cell properties
        if ( keyword.startsWith( "DATES" ) || keyword == "SCHEDULE" || keyword == "GRID" || keyword == "PROPS" || keyword == "SOLUTION" ||
             keyword == "RUNSPEC" || keyword == "SUMMARY" || keyword == "ACTNUM" )
        {
            continue;
        }

        // Try to extract keyword data
        auto result = settings.hasNonUniformRefinement() ? RifEclipseInputFileTools::extractKeywordData( eclipseCase->eclipseCaseData(),
                                                                                                         keyword,
                                                                                                         settings.min(),
                                                                                                         settings.max(),
                                                                                                         settings.nonUniformRefinement() )
                                                         : RifEclipseInputFileTools::extractKeywordData( eclipseCase->eclipseCaseData(),
                                                                                                         keyword,
                                                                                                         settings.min(),
                                                                                                         settings.max(),
                                                                                                         settings.refinement() );
        if ( result )
        {
            // Replace keyword values in deck with extracted data
            if ( deckFile.replaceKeyword( keywordStdStr, result.value(), true /* data kw */ ) )
            {
                RiaLogging::info(
                    QString( "Successfully replaced data for keyword '%1' (%2 values)" ).arg( keyword ).arg( result.value().size() ) );
            }
            else
            {
                RiaLogging::debug( QString( "'%1' is not a data keyword, skipping" ).arg( keyword ) );
            }
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<std::string> RigSimulationInputTool::cropDataKeywordsInDeckFile( RimEclipseCase*                   eclipseCase,
                                                                          const RigSimulationInputSettings& settings,
                                                                          RifOpmFlowDeckFile&               deckFile )
{
    std::set<std::string> croppedKeywords;

    RigMainGrid* mainGrid = eclipseCase->eclipseCaseData()->mainGrid();
    if ( !mainGrid ) return croppedKeywords;

    const size_t fullNx        = mainGrid->cellCountI();
    const size_t fullNy        = mainGrid->cellCountJ();
    const size_t fullNz        = mainGrid->cellCountK();
    const size_t fullCellCount = fullNx * fullNy * fullNz;

    const auto min        = settings.min();
    const auto max        = settings.max();
    const auto refinement = settings.refinement();

    // Build non-uniform refinement for cropping (handles both uniform and non-uniform cases)
    const bool hasNonUniform = settings.hasNonUniformRefinement();
    const auto nonUniformRef =
        hasNonUniform
            ? settings.nonUniformRefinement()
            : RigNonUniformRefinement::fromUniform( refinement,
                                                    cvf::Vec3st( max.x() - min.x() + 1, max.y() - min.y() + 1, max.z() - min.z() + 1 ) );

    auto keywords = deckFile.keywords( false );

    for ( const auto& name : keywords )
    {
        auto optKw = deckFile.findKeyword( name );
        if ( !optKw.has_value() ) continue;

        const auto& kw = optKw.value();
        if ( !kw.isDataKeyword() ) continue;
        if ( name == "ACTNUM" ) continue;
        if ( kw.size() != 1 ) continue;

        const auto& record = kw.getRecord( 0 );
        if ( record.size() != 1 ) continue;

        const auto& item = record.getItem( 0 );

        auto cropAndReplace = [&]<typename T>( const std::vector<T>& fullData )
        {
            std::vector<T> croppedData;

            size_t sectorNk = max.z() - min.z() + 1;
            size_t sectorNj = max.y() - min.y() + 1;
            size_t sectorNi = max.x() - min.x() + 1;

            for ( size_t origK = 0; origK < sectorNk; ++origK )
            {
                size_t mainK     = min.z() + origK;
                size_t subCountK = nonUniformRef.subcellCount( RigNonUniformRefinement::DimK, origK );
                for ( size_t rk = 0; rk < subCountK; ++rk )
                {
                    for ( size_t origJ = 0; origJ < sectorNj; ++origJ )
                    {
                        size_t mainJ     = min.y() + origJ;
                        size_t subCountJ = nonUniformRef.subcellCount( RigNonUniformRefinement::DimJ, origJ );
                        for ( size_t rj = 0; rj < subCountJ; ++rj )
                        {
                            for ( size_t origI = 0; origI < sectorNi; ++origI )
                            {
                                size_t mainI     = min.x() + origI;
                                size_t sourceIdx = mainI + mainJ * fullNx + mainK * fullNx * fullNy;
                                size_t subCountI = nonUniformRef.subcellCount( RigNonUniformRefinement::DimI, origI );

                                for ( size_t ri = 0; ri < subCountI; ++ri )
                                {
                                    croppedData.push_back( fullData[sourceIdx] );
                                }
                            }
                        }
                    }
                }
            }

            if ( deckFile.replaceKeyword( name, croppedData, true ) )
            {
                RiaLogging::info( QString( "Cropped data keyword '%1' from deck (%2 -> %3 values)" )
                                      .arg( QString::fromStdString( name ) )
                                      .arg( fullCellCount )
                                      .arg( croppedData.size() ) );
                croppedKeywords.insert( name );
            }
        };

        if ( item.getType() == Opm::type_tag::fdouble && item.data_size() == fullCellCount )
        {
            cropAndReplace( kw.getRawDoubleData() );
        }
        else if ( item.getType() == Opm::type_tag::integer && item.data_size() == fullCellCount )
        {
            cropAndReplace( kw.getIntData() );
        }
    }

    if ( !croppedKeywords.empty() )
    {
        RiaLogging::info( QString( "Cropped %1 data keywords directly from deck" ).arg( croppedKeywords.size() ) );
    }

    return croppedKeywords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputTool::addBorderBoundaryConditions( RimEclipseCase*                   eclipseCase,
                                                                                  const RigSimulationInputSettings& settings,
                                                                                  cvf::ref<cvf::UByteArray>         visibility,
                                                                                  RifOpmFlowDeckFile&               deckFile )
{
    // Return early if no visibility
    if ( visibility.isNull() || visibility->size() == 0 )
    {
        RiaLogging::info( "No visibility provided - skipping boundary conditions" );
        return {};
    }

    // Create grid adapter for refined grid operations
    auto createBorderGridAdapter = [&]()
    {
        if ( settings.hasNonUniformRefinement() )
        {
            return RigGridExportAdapter( eclipseCase->eclipseCaseData(),
                                         settings.min(),
                                         settings.max(),
                                         settings.nonUniformRefinement(),
                                         visibility.p() );
        }
        return RigGridExportAdapter( eclipseCase->eclipseCaseData(), settings.min(), settings.max(), settings.refinement(), visibility.p() );
    };
    RigGridExportAdapter gridAdapter = createBorderGridAdapter();

    // Create refined visibility array matching refined grid dimensions
    std::vector<int> refinedVisibility = createRefinedVisibility( gridAdapter );

    // Generate refined border result
    auto borderResult = RigEclipseResultTools::generateBorderResult( gridAdapter, refinedVisibility );

    if ( borderResult.empty() || borderResult.size() != gridAdapter.totalCells() )
    {
        RiaLogging::warning( "Failed to generate refined border result - skipping boundary conditions" );
        return {};
    }

    // Handle boundary condition based on settings
    if ( settings.boundaryCondition() == RiaModelExportDefines::BoundaryCondition::BCCON_BCPROP )
    {
        // Generate BCCON result for refined grid in memory
        auto bcconResult = RigEclipseResultTools::generateBcconResult( gridAdapter, borderResult );

        if ( bcconResult.empty() || bcconResult.size() != gridAdapter.totalCells() )
        {
            RiaLogging::warning( "Failed to generate refined BCCON result - skipping boundary conditions" );
            return {};
        }

        // Generate border cell faces using refined in-memory results
        auto borderCellFaces = RigEclipseResultTools::generateBorderCellFaces( gridAdapter, borderResult, bcconResult );

        if ( !borderCellFaces.empty() )
        {
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
    }
    else if ( settings.boundaryCondition() == RiaModelExportDefines::BoundaryCondition::OPERNUM_OPERATER )
    {
        // Find max OPERNUM value from the original grid
        int maxOperNum = RigEclipseResultTools::findMaxOperNumValue( eclipseCase );

        // Generate OPERNUM result directly on refined grid - border cells get max existing OPERNUM + 1
        // Returns a pair: [vector of OPERNUM values, operNumRegion value]
        // Uses existing OPERNUM data from eclipse case if available, refined to match the grid dimensions
        auto [refinedOperNumResult, operNumRegion] =
            RigEclipseResultTools::generateOperNumResult( eclipseCase, gridAdapter, borderResult, maxOperNum );

        CAF_ASSERT( borderResult.size() == refinedOperNumResult.size() );

        if ( operNumRegion != -1 )
        {
            if ( auto result =
                     addOperNumRegionAndOperater( deckFile, gridAdapter, refinedOperNumResult, operNumRegion, settings.porvMultiplier() );
                 !result )
            {
                return result;
            }
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
/// Create refined visibility array matching refined grid dimensions
/// Returns a vector with 1 for visible cells, 0 for invisible cells
//--------------------------------------------------------------------------------------------------
std::vector<int> RigSimulationInputTool::createRefinedVisibility( const RigGridExportAdapter& gridAdapter )
{
    size_t refinedNI  = gridAdapter.cellCountI();
    size_t refinedNJ  = gridAdapter.cellCountJ();
    size_t refinedNK  = gridAdapter.cellCountK();
    size_t totalCells = refinedNI * refinedNJ * refinedNK;

    std::vector<int> refinedVisibility( totalCells, 0 );

    // Lambda to calculate linear index from IJK coordinates
    auto linearIndex = [refinedNI, refinedNJ]( size_t i, size_t j, size_t k ) { return k * refinedNI * refinedNJ + j * refinedNI + i; };

    // Fill refined visibility array
    for ( size_t k = 0; k < refinedNK; ++k )
    {
        for ( size_t j = 0; j < refinedNJ; ++j )
        {
            for ( size_t i = 0; i < refinedNI; ++i )
            {
                size_t linearIdx             = linearIndex( i, j, k );
                refinedVisibility[linearIdx] = gridAdapter.isCellActive( i, j, k ) ? 1 : 0;
            }
        }
    }

    return refinedVisibility;
}

//--------------------------------------------------------------------------------------------------
/// Check if a record (EQUALS/COPY/ADD/MULTIPLY) has explicit box indices at items 2-7
//--------------------------------------------------------------------------------------------------
static bool recordHasExplicitBoxIndices( const Opm::DeckRecord& record )
{
    return record.size() >= 8 && record.getItem( 2 ).hasValue( 0 ) && record.getItem( 3 ).hasValue( 0 ) && record.getItem( 4 ).hasValue( 0 ) &&
           record.getItem( 5 ).hasValue( 0 ) && record.getItem( 6 ).hasValue( 0 ) && record.getItem( 7 ).hasValue( 0 );
}

//--------------------------------------------------------------------------------------------------
/// Create a new record with the original items 0-1 plus injected box indices
//--------------------------------------------------------------------------------------------------
static Opm::DeckRecord injectBoxIndicesIntoRecord( const Opm::DeckRecord& record, const std::array<int, 6>& boxIndices )
{
    std::vector<Opm::DeckItem> items;
    items.push_back( record.getItem( 0 ) );
    items.push_back( record.getItem( 1 ) );
    items.push_back( RifOpmDeckTools::item( "I1", boxIndices[0] ) );
    items.push_back( RifOpmDeckTools::item( "I2", boxIndices[1] ) );
    items.push_back( RifOpmDeckTools::item( "J1", boxIndices[2] ) );
    items.push_back( RifOpmDeckTools::item( "J2", boxIndices[3] ) );
    items.push_back( RifOpmDeckTools::item( "K1", boxIndices[4] ) );
    items.push_back( RifOpmDeckTools::item( "K2", boxIndices[5] ) );

    return Opm::DeckRecord{ std::move( items ) };
}

//--------------------------------------------------------------------------------------------------
/// Extract box indices (I1,I2,J1,J2,K1,K2) from items at positions 2-7 of a record
//--------------------------------------------------------------------------------------------------
static std::array<int, 6> extractBoxIndicesFromRecord( const Opm::DeckRecord& record )
{
    return { record.getItem( 2 ).get<int>( 0 ),
             record.getItem( 3 ).get<int>( 0 ),
             record.getItem( 4 ).get<int>( 0 ),
             record.getItem( 5 ).get<int>( 0 ),
             record.getItem( 6 ).get<int>( 0 ),
             record.getItem( 7 ).get<int>( 0 ) };
}

using RecordProcessorFunc = RigSimulationInputTool::RecordProcessorFunc;

struct Modification
{
    Opm::FileDeck::Index index;
    enum Action
    {
        Replace,
        Remove
    } action;
    Opm::DeckKeyword replacement;
};

//--------------------------------------------------------------------------------------------------
/// Process a BOX keyword entry: update box state and return a modification for the BOX keyword.
/// Returns std::unexpected on fatal exception.
//--------------------------------------------------------------------------------------------------
static std::expected<Modification, QString> processBoxKeywordEntry( const Opm::DeckKeyword&     kw,
                                                                    const Opm::FileDeck::Index& index,
                                                                    const caf::VecIjk0&         sectorMin,
                                                                    const caf::VecIjk0&         sectorMax,
                                                                    const cvf::Vec3st&          refinement,
                                                                    bool&                       insideBox,
                                                                    std::array<int, 6>&         activeBoxIndices,
                                                                    caf::VecIjk0&               boxMin,
                                                                    caf::VecIjk0&               boxMax )
{
    if ( insideBox )
    {
        RiaLogging::warning( "BOX keyword found while already inside a BOX context. Previous BOX will be overridden." );
    }

    insideBox = true;
    if ( kw.size() > 0 )
    {
        const auto& rec = kw.getRecord( 0 );
        if ( rec.size() >= 6 )
        {
            activeBoxIndices = { rec.getItem( 0 ).get<int>( 0 ),
                                 rec.getItem( 1 ).get<int>( 0 ),
                                 rec.getItem( 2 ).get<int>( 0 ),
                                 rec.getItem( 3 ).get<int>( 0 ),
                                 rec.getItem( 4 ).get<int>( 0 ),
                                 rec.getItem( 5 ).get<int>( 0 ) };

            // BOX items: I1, I2, J1, J2, K1, K2 (1-based) -> 0-based for data keyword cropping
            boxMin = caf::VecIjk0( activeBoxIndices[0] - 1, activeBoxIndices[2] - 1, activeBoxIndices[4] - 1 );
            boxMax = caf::VecIjk0( activeBoxIndices[1] - 1, activeBoxIndices[3] - 1, activeBoxIndices[5] - 1 );
        }
    }

    // Transform BOX coordinates via processBoxRecord
    try
    {
        if ( kw.size() > 0 )
        {
            auto result = RigSimulationInputTool::processBoxRecord( kw.getRecord( 0 ), sectorMin, sectorMax, refinement );
            if ( result )
            {
                Opm::DeckKeyword transformedKw( kw.location(), kw.name() );
                transformedKw.addRecord( std::move( result.value() ) );
                return Modification{ index, Modification::Replace, std::move( transformedKw ) };
            }
            else
            {
                RiaLogging::warning( QString( "Failed to process BOX record: %1" ).arg( result.error() ) );
                return Modification{ index, Modification::Remove, Opm::DeckKeyword{} };
            }
        }
    }
    catch ( std::exception& e )
    {
        return std::unexpected( QString( "Exception processing BOX keyword: %1" ).arg( e.what() ) );
    }

    return Modification{ index, Modification::Replace, kw };
}

//--------------------------------------------------------------------------------------------------
/// Crop a data keyword to the intersection of its BOX context and the sector.
/// Returns nullopt if no modification is needed (e.g. keyword has unexpected structure).
//--------------------------------------------------------------------------------------------------
static std::optional<Modification> cropDataKeywordInBoxContext( const Opm::DeckKeyword&        kw,
                                                                const Opm::FileDeck::Index&    index,
                                                                const caf::VecIjk0&            boxMin,
                                                                const caf::VecIjk0&            boxMax,
                                                                const caf::VecIjk0&            sectorMin,
                                                                const caf::VecIjk0&            sectorMax,
                                                                const cvf::Vec3st&             refinement,
                                                                const RigNonUniformRefinement* nonUniformRef = nullptr )
{
    const std::string& name = kw.name();

    // Compute intersection of box and sector
    caf::VecIjk0 intMin( std::max( boxMin.x(), sectorMin.x() ), std::max( boxMin.y(), sectorMin.y() ), std::max( boxMin.z(), sectorMin.z() ) );
    caf::VecIjk0 intMax( std::min( boxMax.x(), sectorMax.x() ), std::min( boxMax.y(), sectorMax.y() ), std::min( boxMax.z(), sectorMax.z() ) );

    if ( intMin.x() > intMax.x() || intMin.y() > intMax.y() || intMin.z() > intMax.z() )
    {
        RiaLogging::info(
            QString( "Removing data keyword '%1' inside BOX/ENDBOX: box does not intersect sector" ).arg( QString::fromStdString( name ) ) );
        return Modification{ index, Modification::Remove, Opm::DeckKeyword{} };
    }

    if ( kw.size() != 1 || kw.getRecord( 0 ).size() != 1 ) return std::nullopt;

    const auto& item = kw.getRecord( 0 ).getItem( 0 );

    const size_t boxNi = boxMax.x() - boxMin.x() + 1;
    const size_t boxNj = boxMax.y() - boxMin.y() + 1;

    auto cropBoxData = [&]<typename T>( const std::vector<T>& sourceData ) -> Modification
    {
        std::vector<T> croppedData;

        for ( size_t k = intMin.z(); k <= static_cast<size_t>( intMax.z() ); ++k )
        {
            size_t subCountK = nonUniformRef ? nonUniformRef->subcellCount( RigNonUniformRefinement::DimK, k - sectorMin.z() )
                                             : refinement.z();
            for ( size_t rk = 0; rk < subCountK; ++rk )
            {
                for ( size_t j = intMin.y(); j <= static_cast<size_t>( intMax.y() ); ++j )
                {
                    size_t subCountJ = nonUniformRef ? nonUniformRef->subcellCount( RigNonUniformRefinement::DimJ, j - sectorMin.y() )
                                                     : refinement.y();
                    for ( size_t rj = 0; rj < subCountJ; ++rj )
                    {
                        for ( size_t i = intMin.x(); i <= static_cast<size_t>( intMax.x() ); ++i )
                        {
                            size_t boxRelI = i - boxMin.x();
                            size_t boxRelJ = j - boxMin.y();
                            size_t boxRelK = k - boxMin.z();
                            size_t srcIdx  = boxRelI + boxRelJ * boxNi + boxRelK * boxNi * boxNj;

                            size_t subCountI = nonUniformRef ? nonUniformRef->subcellCount( RigNonUniformRefinement::DimI, i - sectorMin.x() )
                                                             : refinement.x();
                            for ( size_t ri = 0; ri < subCountI; ++ri )
                            {
                                croppedData.push_back( sourceData[srcIdx] );
                            }
                        }
                    }
                }
            }
        }

        Opm::DeckKeyword newKw = kw.emptyStructuralCopy();
        Opm::DeckRecord  record;

        if constexpr ( std::is_same_v<T, int> )
        {
            Opm::DeckItem newItem( name, int() );
            for ( const auto& v : croppedData )
                newItem.push_back( v );
            record.addItem( std::move( newItem ) );
        }
        else
        {
            std::vector<Opm::Dimension> activeDim;
            std::vector<Opm::Dimension> defaultDim;
            Opm::DeckItem               newItem( name, double(), activeDim, defaultDim );
            for ( const auto& v : croppedData )
                newItem.push_back( v );
            record.addItem( std::move( newItem ) );
        }

        newKw.addRecord( std::move( record ) );

        RiaLogging::info( QString( "Cropped data keyword '%1' inside BOX/ENDBOX (%2 -> %3 values)" )
                              .arg( QString::fromStdString( name ) )
                              .arg( sourceData.size() )
                              .arg( croppedData.size() ) );

        return Modification{ index, Modification::Replace, std::move( newKw ) };
    };

    if ( item.getType() == Opm::type_tag::integer )
    {
        return cropBoxData( kw.getIntData() );
    }
    else if ( item.getType() == Opm::type_tag::fdouble )
    {
        return cropBoxData( kw.getRawDoubleData() );
    }

    return std::nullopt;
}

//--------------------------------------------------------------------------------------------------
/// Process an EQUALS/COPY/ADD/MULTIPLY keyword: transform all records.
/// Returns std::unexpected on fatal exception.
//--------------------------------------------------------------------------------------------------
static std::expected<Modification, QString> processRecordBasedKeyword( const Opm::DeckKeyword&     kw,
                                                                       const Opm::FileDeck::Index& index,
                                                                       const RecordProcessorFunc&  processorFunc,
                                                                       bool                        insideBox,
                                                                       const std::array<int, 6>&   activeBoxIndices,
                                                                       const caf::VecIjk0&         sectorMin,
                                                                       const caf::VecIjk0&         sectorMax,
                                                                       const cvf::Vec3st&          refinement )
{
    const std::string& name = kw.name();

    try
    {
        Opm::DeckKeyword transformedKw( kw.location(), kw.name() );

        // Track "current operation box" for records inside BOX context
        std::array<int, 6> currentOpBox = activeBoxIndices;

        for ( size_t r = 0; r < kw.size(); ++r )
        {
            const auto& rec = kw.getRecord( r );

            Opm::DeckRecord recordToProcess = rec;

            if ( insideBox )
            {
                // Inside BOX: inject box indices into records without explicit coords
                if ( recordHasExplicitBoxIndices( rec ) )
                {
                    currentOpBox = extractBoxIndicesFromRecord( rec );
                }
                else
                {
                    recordToProcess = injectBoxIndicesIntoRecord( rec, currentOpBox );
                }
            }

            // Transform the record
            auto result = processorFunc( recordToProcess, sectorMin, sectorMax, refinement );
            if ( result )
            {
                transformedKw.addRecord( std::move( result.value() ) );
            }
            else
            {
                RiaLogging::warning( QString( "Failed to process %1 record: %2" ).arg( QString::fromStdString( name ) ).arg( result.error() ) );
            }
        }

        if ( transformedKw.size() > 0 )
        {
            return Modification{ index, Modification::Replace, std::move( transformedKw ) };
        }
        else
        {
            // All records failed - replace with SKIP
            return Modification{ index, Modification::Replace, Opm::DeckKeyword( kw.location(), "SKIP" ) };
        }
    }
    catch ( std::exception& e )
    {
        return std::unexpected( QString( "Exception processing %1 keyword: %2" ).arg( QString::fromStdString( name ) ).arg( e.what() ) );
    }
}

//--------------------------------------------------------------------------------------------------
/// Single-pass BOX/ENDBOX processing: tracks BOX/ENDBOX state and handles all keyword types
/// in one iteration. Crops data keywords to sector/box intersection, injects BOX indices into
/// EQUALS/COPY/ADD/MULTIPLY records and transforms their coordinates, and transforms BOX
/// coordinates. Replaces the previous multi-pass approach (expandBoxContextInDeckFile +
/// cropDataKeywordsInsideBoxContext + replace*KeywordIndices).
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputTool::transformKeywordsInDeckFile( RimEclipseCase*                   eclipseCase,
                                                                                  const RigSimulationInputSettings& settings,
                                                                                  RifOpmFlowDeckFile&               deckFile )
{
    Opm::FileDeck* fd = deckFile.fileDeck();
    if ( !fd ) return {};

    const auto sectorMin  = settings.min();
    const auto sectorMax  = settings.max();
    const auto refinement = settings.refinement();

    // Build non-uniform refinement pointer for functions that support it
    const RigNonUniformRefinement* nonUniformRefPtr = settings.hasNonUniformRefinement() ? &settings.nonUniformRefinement() : nullptr;

    // Map keyword names to their record processor functions
    const std::map<std::string, RecordProcessorFunc> recordProcessors = {
        { "EQUALS", processEqualsRecord },
        { "COPY", processCopyRecord },
        { "ADD", processAddRecord },
        { "MULTIPLY", processMultiplyRecord },
    };

    bool                      insideBox = false;
    std::array<int, 6>        activeBoxIndices{};
    caf::VecIjk0              boxMin( 0, 0, 0 );
    caf::VecIjk0              boxMax( 0, 0, 0 );
    std::vector<Modification> modifications;

    for ( auto it = fd->start(); it != fd->stop(); it++ )
    {
        const auto&        kw   = ( *fd )[it];
        const std::string& name = kw.name();

        if ( name == "BOX" )
        {
            auto result = processBoxKeywordEntry( kw, it, sectorMin, sectorMax, refinement, insideBox, activeBoxIndices, boxMin, boxMax );
            if ( !result ) return std::unexpected( result.error() );
            modifications.push_back( std::move( result.value() ) );
        }
        else if ( name == "ENDBOX" )
        {
            if ( !insideBox )
            {
                RiaLogging::warning( "ENDBOX keyword found without a matching BOX." );
            }
            insideBox = false;
        }
        else if ( insideBox && kw.isDataKeyword() )
        {
            auto result = cropDataKeywordInBoxContext( kw, it, boxMin, boxMax, sectorMin, sectorMax, refinement, nonUniformRefPtr );
            if ( result ) modifications.push_back( std::move( result.value() ) );
        }
        else if ( auto procIt = recordProcessors.find( name ); procIt != recordProcessors.end() )
        {
            auto result = processRecordBasedKeyword( kw, it, procIt->second, insideBox, activeBoxIndices, sectorMin, sectorMax, refinement );
            if ( !result ) return std::unexpected( result.error() );
            modifications.push_back( std::move( result.value() ) );
        }
    }

    if ( insideBox )
    {
        RiaLogging::warning( "BOX keyword found without a matching ENDBOX at end of deck." );
    }

    // Apply modifications in reverse order to maintain valid indices
    for ( auto modIt = modifications.rbegin(); modIt != modifications.rend(); ++modIt )
    {
        if ( modIt->action == Modification::Remove )
        {
            deckFile.removeKeywordAtIndex( modIt->index );
        }
        else
        {
            deckFile.replaceKeywordAtIndex( modIt->index, modIt->replacement );
        }
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
std::expected<void, QString> RigSimulationInputTool::replaceAquconKeywordIndices( RimEclipseCase*                   eclipseCase,
                                                                                  const RigSimulationInputSettings& settings,
                                                                                  RifOpmFlowDeckFile&               deckFile )
{
    return replaceKeywordWithBoxIndices( "AQUCON", eclipseCase, settings, deckFile, processAquconRecord );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputTool::replaceAquanconKeywordIndices( RimEclipseCase*                   eclipseCase,
                                                                                    const RigSimulationInputSettings& settings,
                                                                                    RifOpmFlowDeckFile&               deckFile )
{
    return replaceKeywordWithBoxIndices( "AQUANCON", eclipseCase, settings, deckFile, processAquanconRecord );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputTool::replaceAqunumKeywordIndices( RimEclipseCase*                   eclipseCase,
                                                                                  const RigSimulationInputSettings& settings,
                                                                                  RifOpmFlowDeckFile&               deckFile )
{
    return replaceKeywordWithBoxIndices( "AQUNUM", eclipseCase, settings, deckFile, processAqunumRecord );
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
    int refinementProduct = 1;
    if ( settings.hasNonUniformRefinement() )
    {
        const auto& nuRef = settings.nonUniformRefinement();
        // Use the maximum subcell product across all cell combinations
        size_t maxProduct = 1;
        for ( auto dim : { RigNonUniformRefinement::DimI, RigNonUniformRefinement::DimJ, RigNonUniformRefinement::DimK } )
        {
            size_t maxSubcells = 1;
            for ( size_t idx = 0; idx < nuRef.sectorSize( dim ); ++idx )
            {
                maxSubcells = std::max( maxSubcells, nuRef.subcellCount( dim, idx ) );
            }
            maxProduct *= maxSubcells;
        }
        refinementProduct = static_cast<int>( maxProduct );
    }
    else
    {
        refinementProduct = static_cast<int>( settings.refinement().x() * settings.refinement().y() * settings.refinement().z() );
    }
    int newMaxConnections = wellDims[1] * refinementProduct;

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
    // Remove FAULTS to handle deck where it appears more than once.
    deckFile.removeKeywords( "FAULTS" );

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
    RigSimulationInputTool::findIntersectingWells( RimEclipseCase* eclipseCase, const caf::VecIjk0& min, const caf::VecIjk0& max )
{
    std::vector<RigSimWellData*> intersectingWells;

    if ( !eclipseCase || !eclipseCase->eclipseCaseData() ) return intersectingWells;

    const auto& wellResults = eclipseCase->eclipseCaseData()->wellResults();

    // Create sector bounding box for intersection checks
    RigBoundingBoxIjk<caf::VecIjk0> sectorBox( min, max );

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
                if ( sectorBox.contains( ijk ) )
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
    size_t sectorI, sectorJ;
    if ( settings.hasNonUniformRefinement() )
    {
        const auto& nuRef = settings.nonUniformRefinement();
        size_t      relI  = clampedI - settings.min().x();
        size_t      relJ  = clampedJ - settings.min().y();
        sectorI = nuRef.cumulativeOffset( RigNonUniformRefinement::DimI, relI ) + nuRef.subcellCount( RigNonUniformRefinement::DimI, relI ) / 2;
        sectorJ = nuRef.cumulativeOffset( RigNonUniformRefinement::DimJ, relJ ) + nuRef.subcellCount( RigNonUniformRefinement::DimJ, relJ ) / 2;
    }
    else
    {
        sectorI = ( clampedI - settings.min().x() ) * settings.refinement().x() + ( settings.refinement().x() + 1 ) / 2;
        sectorJ = ( clampedJ - settings.min().y() ) * settings.refinement().y() + ( settings.refinement().y() + 1 ) / 2;
    }

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
        settings.hasNonUniformRefinement()
                    ? RigGridExportAdapter::transformIjkToSectorCoordinates( origIjkK1, settings.min(), settings.max(), settings.nonUniformRefinement(), true )
                    : RigGridExportAdapter::transformIjkToSectorCoordinates( origIjkK1, settings.min(), settings.max(), settings.refinement(), true );

    // Transform K2
    caf::VecIjk0 origIjkK2( origI, origJ, origK2 );
    auto         transformResultK2 =
        settings.hasNonUniformRefinement()
                    ? RigGridExportAdapter::transformIjkToSectorCoordinates( origIjkK2, settings.min(), settings.max(), settings.nonUniformRefinement(), true )
                    : RigGridExportAdapter::transformIjkToSectorCoordinates( origIjkK2, settings.min(), settings.max(), settings.refinement(), true );

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
    items.push_back( RifOpmDeckTools::item( C::I::itemName, static_cast<int>( transformResultK1->toOneBased().x() ) ) );
    items.push_back( RifOpmDeckTools::item( C::J::itemName, static_cast<int>( transformResultK1->toOneBased().y() ) ) );
    items.push_back( RifOpmDeckTools::item( C::K1::itemName, static_cast<int>( transformResultK1->toOneBased().z() ) ) );
    items.push_back( RifOpmDeckTools::item( C::K2::itemName, static_cast<int>( transformResultK2->toOneBased().z() ) ) );

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
    caf::VecIjk1 origIjk = extractIjk( record, 0, 1, 2 );

    auto transformResult = settings.hasNonUniformRefinement()
                               ? RigGridExportAdapter::transformIjkToSectorCoordinates( origIjk.toZeroBased(),
                                                                                        settings.min(),
                                                                                        settings.max(),
                                                                                        settings.nonUniformRefinement(),
                                                                                        true )
                               : RigGridExportAdapter::transformIjkToSectorCoordinates( origIjk.toZeroBased(),
                                                                                        settings.min(),
                                                                                        settings.max(),
                                                                                        settings.refinement(),
                                                                                        true );

    if ( !transformResult )
    {
        return std::unexpected(
            QString( "COMPSEGS segment coordinate for well %1 is out of sector bounds: %2" ).arg( wellName.c_str() ).arg( transformResult.error() ) );
    }

    // Add transformed I, J, K
    items.push_back( RifOpmDeckTools::item( "I", static_cast<int>( transformResult->toOneBased().x() ) ) );
    items.push_back( RifOpmDeckTools::item( "J", static_cast<int>( transformResult->toOneBased().y() ) ) );
    items.push_back( RifOpmDeckTools::item( "K", static_cast<int>( transformResult->toOneBased().z() ) ) );

    // Copy remaining items
    for ( size_t i = 3; i < record.size(); ++i )
    {
        items.push_back( record.getItem( i ) );
    }

    return Opm::DeckRecord{ std::move( items ) };
}

//--------------------------------------------------------------------------------------------------
/// Helper function to extract IJK coordinates from a deck record
/// Returns a VecIjk1 (1-based) constructed from values at the specified item indices
//--------------------------------------------------------------------------------------------------
caf::VecIjk1 RigSimulationInputTool::extractIjk( const Opm::DeckRecord& record, size_t indexI, size_t indexJ, size_t indexK )
{
    return caf::VecIjk1( record.getItem( indexI ).get<int>( 0 ), record.getItem( indexJ ).get<int>( 0 ), record.getItem( indexK ).get<int>( 0 ) );
}

//--------------------------------------------------------------------------------------------------
/// Helper function to transform bounding box from global to sector coordinates
/// Performs intersection, clamping, and coordinate transformation
/// Returns bounding box with 0-based sector-relative coordinates
//--------------------------------------------------------------------------------------------------
std::expected<RigBoundingBoxIjk<caf::VecIjk0>, QString>
    RigSimulationInputTool::transformBoxToSectorCoordinates( const RigBoundingBoxIjk<caf::VecIjk0>& inputBox,
                                                             const caf::VecIjk0&                    sectorMin,
                                                             const caf::VecIjk0&                    sectorMax,
                                                             const cvf::Vec3st&                     refinement,
                                                             const QString&                         keywordName,
                                                             const QString&                         recordIdentifier )
{
    // Create sector bounding box
    RigBoundingBoxIjk<caf::VecIjk0> sectorBox( sectorMin, sectorMax );

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
    caf::VecIjk0 clampedMin = intersection->min();
    caf::VecIjk0 clampedMax = intersection->max();

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
    auto transformResult1 = RigGridExportAdapter::transformIjkToSectorCoordinates( clampedMin, sectorMin, sectorMax, refinement, false, false );
    auto transformResult2 = RigGridExportAdapter::transformIjkToSectorCoordinates( clampedMax, sectorMin, sectorMax, refinement, false, true );

    if ( !transformResult1 )
    {
        return std::unexpected( QString( "%1 min coordinate is out of sector bounds: %2" ).arg( keywordName ).arg( transformResult1.error() ) );
    }

    if ( !transformResult2 )
    {
        return std::unexpected( QString( "%1 max coordinate is out of sector bounds: %2" ).arg( keywordName ).arg( transformResult2.error() ) );
    }

    // Return bounding box with 0-based sector-relative coordinates
    return RigBoundingBoxIjk<caf::VecIjk0>( *transformResult1, *transformResult2 );
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
        caf::VecIjk1 origMin = extractIjk( record, 2, 4, 6 ); // I1, J1, K1
        caf::VecIjk1 origMax = extractIjk( record, 3, 5, 7 ); // I2, J2, K2

        // Create input bounding box (0-based, inclusive)
        RigBoundingBoxIjk<caf::VecIjk0> inputBox( origMin.toZeroBased(), origMax.toZeroBased() );

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
    if ( record.size() < 2 )
    {
        return std::unexpected( QString( "MULTIPLY record has insufficient items (expected at least 2, got %1)" ).arg( record.size() ) );
    }

    std::vector<Opm::DeckItem> items;

    // Copy field name and factor (first two items)
    items.push_back( record.getItem( 0 ) );
    items.push_back( record.getItem( 1 ) );

    if ( record.size() >= 8 && record.getItem( 2 ).hasValue( 0 ) && record.getItem( 3 ).hasValue( 0 ) && record.getItem( 4 ).hasValue( 0 ) &&
         record.getItem( 5 ).hasValue( 0 ) && record.getItem( 6 ).hasValue( 0 ) && record.getItem( 7 ).hasValue( 0 ) )
    {
        // Transform IJK box coordinates (items 2-7: I1, I2, J1, J2, K1, K2)
        // Note: MULTIPLY uses 1-based Eclipse coordinates
        caf::VecIjk1 origMin = extractIjk( record, 2, 4, 6 ); // I1, J1, K1
        caf::VecIjk1 origMax = extractIjk( record, 3, 5, 7 ); // I2, J2, K2

        // Create input bounding box (0-based, inclusive)
        RigBoundingBoxIjk<caf::VecIjk0> inputBox( origMin.toZeroBased(), origMax.toZeroBased() );

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
    if ( record.size() < 2 )
    {
        return std::unexpected( QString( "ADD record has insufficient items (expected at least 2, got %1)" ).arg( record.size() ) );
    }

    std::vector<Opm::DeckItem> items;

    // Copy field name and shift value (first two items)
    items.push_back( record.getItem( 0 ) );
    items.push_back( record.getItem( 1 ) );

    if ( record.size() >= 8 && record.getItem( 2 ).hasValue( 0 ) && record.getItem( 3 ).hasValue( 0 ) && record.getItem( 4 ).hasValue( 0 ) &&
         record.getItem( 5 ).hasValue( 0 ) && record.getItem( 6 ).hasValue( 0 ) && record.getItem( 7 ).hasValue( 0 ) )
    {
        // Transform IJK box coordinates (items 2-7: I1, I2, J1, J2, K1, K2)
        // Note: ADD uses 1-based Eclipse coordinates
        caf::VecIjk1 origMin = extractIjk( record, 2, 4, 6 ); // I1, J1, K1
        caf::VecIjk1 origMax = extractIjk( record, 3, 5, 7 ); // I2, J2, K2

        // Create input bounding box (0-based, inclusive)
        RigBoundingBoxIjk<caf::VecIjk0> inputBox( origMin.toZeroBased(), origMax.toZeroBased() );

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

    using A = Opm::ParserKeywords::AQUCON;

    std::vector<Opm::DeckItem> items;

    // Copy aquifer ID (first item)
    items.push_back( record.getItem( 0 ) );

    if ( record.size() >= 7 && record.getItem( 1 ).hasValue( 0 ) && record.getItem( 2 ).hasValue( 0 ) && record.getItem( 3 ).hasValue( 0 ) &&
         record.getItem( 4 ).hasValue( 0 ) && record.getItem( 5 ).hasValue( 0 ) && record.getItem( 6 ).hasValue( 0 ) )
    {
        // Transform IJK box coordinates (items 1-6: I1, I2, J1, J2, K1, K2)
        // Note: AQUCON uses 1-based Eclipse coordinates
        caf::VecIjk1 origMin = extractIjk( record, 1, 3, 5 ); // I1, J1, K1
        caf::VecIjk1 origMax = extractIjk( record, 2, 4, 6 ); // I2, J2, K2

        // Create input bounding box (0-based, inclusive)
        RigBoundingBoxIjk<caf::VecIjk0> inputBox( origMin.toZeroBased(), origMax.toZeroBased() );

        // Get aquifer ID for logging
        int     aquiferId   = record.getItem( 0 ).get<int>( 0 );
        QString recordIdent = QString( "for aquifer %1" ).arg( aquiferId );

        // Transform box to sector coordinates
        auto transformResult =
            transformBoxToSectorCoordinates( inputBox, min, max, refinement, QString::fromStdString( A::keywordName ), recordIdent );
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
std::expected<Opm::DeckRecord, QString> RigSimulationInputTool::processAquanconRecord( const Opm::DeckRecord& record,
                                                                                       const caf::VecIjk0&    min,
                                                                                       const caf::VecIjk0&    max,
                                                                                       const cvf::Vec3st&     refinement )
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
        caf::VecIjk1 origMin = extractIjk( record, 1, 3, 5 ); // I1, J1, K1
        caf::VecIjk1 origMax = extractIjk( record, 2, 4, 6 ); // I2, J2, K2

        // Create input bounding box (0-based, inclusive)
        RigBoundingBoxIjk<caf::VecIjk0> inputBox( origMin.toZeroBased(), origMax.toZeroBased() );

        // Get aquifer ID for logging
        int     aquiferId   = record.getItem( 0 ).get<int>( 0 );
        QString recordIdent = QString( "for aquifer %1" ).arg( aquiferId );

        // Transform box to sector coordinates
        auto transformResult =
            transformBoxToSectorCoordinates( inputBox, min, max, refinement, QString::fromStdString( A::keywordName ), recordIdent );
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
std::expected<Opm::DeckRecord, QString> RigSimulationInputTool::processAqunumRecord( const Opm::DeckRecord& record,
                                                                                     const caf::VecIjk0&    min,
                                                                                     const caf::VecIjk0&    max,
                                                                                     const cvf::Vec3st&     refinement )
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
        caf::VecIjk1 origCell = extractIjk( record, 1, 2, 3 ); // I, J, K

        // Create input bounding box (0-based, inclusive)
        RigBoundingBoxIjk<caf::VecIjk0> inputBox( origCell.toZeroBased(), origCell.toZeroBased() );

        // Get aquifer ID for logging
        int     aquiferId   = record.getItem( 0 ).get<int>( 0 );
        QString recordIdent = QString( "for aquifer %1" ).arg( aquiferId );

        // Transform box to sector coordinates
        auto transformResult =
            transformBoxToSectorCoordinates( inputBox, min, max, refinement, QString::fromStdString( A::keywordName ), recordIdent );
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
        caf::VecIjk1 origMin = extractIjk( record, 2, 4, 6 ); // I1, J1, K1
        caf::VecIjk1 origMax = extractIjk( record, 3, 5, 7 ); // I2, J2, K2

        // Create input bounding box (0-based, inclusive)
        RigBoundingBoxIjk<caf::VecIjk0> inputBox( origMin.toZeroBased(), origMax.toZeroBased() );

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
    caf::VecIjk1 origMin = extractIjk( record, 0, 2, 4 ); // I1, J1, K1
    caf::VecIjk1 origMax = extractIjk( record, 1, 3, 5 ); // I2, J2, K2

    // Create input bounding box (0-based, inclusive)
    RigBoundingBoxIjk<caf::VecIjk0> inputBox( origMin.toZeroBased(), origMax.toZeroBased() );

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
std::set<std::string> RigSimulationInputTool::wellNamesToInclude( RimEclipseCase* eclipseCase, const RigSimulationInputSettings& settings )
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

    return validWellNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputTool::updateWellListKeywords( std::set<std::string>&            validWellNames,
                                                                             const RigSimulationInputSettings& settings,
                                                                             RifOpmFlowDeckFile&               deckFile )
{
    using W = Opm::ParserKeywords::WLIST;

    int replacedCount = 0;
    int removedCount  = 0;

    // gather existing lists

    auto keywordsWithIndices = deckFile.findAllKeywordsWithIndices( W::keywordName );
    if ( keywordsWithIndices.empty() ) return {};

    std::set<std::string> existingLists;

    // for each WLIST keyword
    for ( auto [index, kw] : keywordsWithIndices )
    {
        std::map<std::string, std::set<std::string>> wellLists;
        std::map<std::string, std::set<std::string>> deleteLists;

        // for each list operation in this keyword
        for ( size_t recordIdx = 0; recordIdx < kw.size(); recordIdx++ )
        {
            const auto& record = kw.getRecord( recordIdx );
            if ( record.size() < 3 ) continue;

            // the list name
            const auto& listNameItem = record.getItem( 0 );
            if ( !listNameItem.hasValue( 0 ) || listNameItem.getType() != Opm::type_tag::string ) continue;
            std::string listName = listNameItem.get<std::string>( 0 );

            // the list operation
            const auto& operationItem = record.getItem( 1 );
            if ( !operationItem.hasValue( 0 ) || operationItem.getType() != Opm::type_tag::string ) continue;
            std::string operationName = operationItem.get<std::string>( 0 );
            operationName             = RiaStdStringTools::toUpper( operationName );

            bool delOperation = operationName == "DEL";
            if ( operationName != "ADD" && operationName != "NEW" && !delOperation )
            {
                RiaLogging::warning( QString( "Unsupported %1 operation '%2' in list '%3', skipping" )
                                         .arg( W::keywordName.c_str() )
                                         .arg( operationName.c_str() )
                                         .arg( listName.c_str() ) );
                continue;
            }

            // the list of wells to do something with, only include the ones in our valid list
            const auto& wellsItem = record.getItem( 2 );
            for ( size_t i = 0; i < wellsItem.data_size(); i++ )
            {
                std::string wellName = wellsItem.get<std::string>( i );
                if ( validWellNames.contains( wellName ) )
                {
                    if ( delOperation )
                        deleteLists[listName].insert( wellName );
                    else
                        wellLists[listName].insert( wellName );
                }
            }
        }
        Opm::DeckKeyword newKw( kw.location(), kw.name() );

        for ( const auto& [listName, wells] : wellLists )
        {
            if ( wells.empty() ) continue;
            std::vector<Opm::DeckItem> items;
            items.push_back( RifOpmDeckTools::item( "NAME", listName ) );
            const std::string action = existingLists.contains( listName ) ? "ADD" : "NEW";
            items.push_back( RifOpmDeckTools::item( "ACTION", action ) );
            items.push_back( RifOpmDeckTools::item( "WELLS", wells ) );
            newKw.addRecord( Opm::DeckRecord{ std::move( items ) } );
            existingLists.insert( listName );
        }

        for ( const auto& [listName, wells] : deleteLists )
        {
            if ( wells.empty() ) continue;
            std::vector<Opm::DeckItem> items;
            items.push_back( RifOpmDeckTools::item( "NAME", listName ) );
            items.push_back( RifOpmDeckTools::item( "ACTION", "DEL" ) );
            items.push_back( RifOpmDeckTools::item( "WELLS", wells ) );
            newKw.addRecord( Opm::DeckRecord{ std::move( items ) } );
        }

        if ( newKw.size() > 0 )
        {
            // replace the first wlist kw with the new one, remove remaining kws
            deckFile.replaceKeywordAtIndex( index, std::move( newKw ) );
            replacedCount++;
        }
        else
        {
            // replace with SKIP kw to remove later
            deckFile.replaceKeywordAtIndex( index, Opm::DeckKeyword( kw.location(), "SKIP" ) );
            removedCount++;
        }
    }

    RiaLogging::info(
        QString( "Processed keyword '%1': %2 updated, %3 removed" ).arg( W::keywordName.c_str() ).arg( replacedCount ).arg( removedCount ) );

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputTool::filterAndUpdateWellKeywords( std::set<std::string>&            validWellNames,
                                                                                  const RigSimulationInputSettings& settings,
                                                                                  RifOpmFlowDeckFile&               deckFile )
{
    // List of well-related keywords to filter (keywords that reference well names)
    std::vector<std::string> wellKeywords = { "COMPDAT",  "COMPLUMP", "COMPORD",  "COMPSEGS", "WCONHIST", "WCONINJE", "WCONINJH",
                                              "WCONPROD", "WCYCLE",   "WDFAC",    "WDFACCOR", "WEFAC",    "WELCNTL",  "WELOPEN",
                                              "WELPI",    "WELSEGS",  "WELSPECS", "WELTARG",  "WINJCLN",  "WINJMULT", "WINJTEMP",
                                              "WLIFTOPT", "WPAVEDEP", "WRFTPLT",  "WTEMP",    "WTEST",    "WTRACER" };

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
                            keepSegmentRecords = ( validWellNames.contains( wellName ) );
                        }
                    }
                    else if ( keywordName == "COMPSEGS" || keywordName == "WELSEGS" )
                    {
                        // Segment data record - use the current well context
                        wellName = currentSegmentWell;
                    }

                    // Check if this well is in our valid set
                    if ( ( isWellNameRecord && validWellNames.contains( wellName ) ) ||
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
std::expected<void, QString> RigSimulationInputTool::addOperNumRegionAndOperater( RifOpmFlowDeckFile&         deckFile,
                                                                                  const RigGridExportAdapter& gridAdapter,
                                                                                  const std::vector<int>&     operNumResult,
                                                                                  int                         operNumRegion,
                                                                                  double                      porvMultiplier )
{
    RiaLogging::info(
        QString( "Adding OPERNUM region %1 with PORV multiplier %2 using refined grid" ).arg( operNumRegion ).arg( porvMultiplier ) );

    // Ensure REGDIMS keyword exists
    if ( !deckFile.ensureRegdimsKeyword() )
    {
        return std::unexpected( "Failed to ensure REGDIMS keyword exists in RUNSPEC section" );
    }

    // Read current REGDIMS values
    auto regdimsValues = deckFile.regdims();
    if ( regdimsValues.empty() )
    {
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
            newKw.setDataKeyword( true );
            deckFile.addKeyword( "GRID", newKw );
        }
    }

    // Replace keyword values in deck with refined exported data
    if ( deckFile.replaceKeyword( "OPERNUM", operNumResult, true /*data kw*/ ) )
    {
        RiaLogging::info( QString( "Replaced OPERNUM values for refined grid with dimensions %1x%2x%3" )
                              .arg( gridAdapter.cellCountI() )
                              .arg( gridAdapter.cellCountJ() )
                              .arg( gridAdapter.cellCountK() ) );
    }
    else
    {
        return std::unexpected( "Unable to replace OPERNUM values for refined grid" );
    }

    // Create OPERATER keyword to multiply pore volume in border region
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigSimulationInputTool::NNCConnection> RigSimulationInputTool::extractDeckEditNncConnections( RifOpmFlowDeckFile& deckFile,
                                                                                                          const RigMainGrid&  mainGrid )
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
                RiaLogging::warning( QString( "Invalid EDITNNC connection in deck: (%1)-(%2)" )
                                         .arg( QString::fromStdString( ijk1.toString() ) )
                                         .arg( QString::fromStdString( ijk2.toString() ) ) );
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
std::vector<RigSimulationInputTool::NNCConnection>
    RigSimulationInputTool::filterInternalSectorConnections( const std::vector<NNCConnection>& allConnections,
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
caf::VecIjk0 RigSimulationInputTool::transformToSectorCoordinates( const caf::VecIjk0& globalIjk,
                                                                   const caf::VecIjk0& min,
                                                                   const cvf::Vec3st&  refinement )
{
    // Transform global IJK to sector-relative coordinates with refinement
    // Formula matches RigGridExportAdapter::transformIjkToSectorCoordinates for box minimum coordinates
    // Returns 0-based sector coordinates
    return caf::VecIjk0( ( globalIjk.i() - min.i() ) * refinement.x(),
                         ( globalIjk.j() - min.j() ) * refinement.y(),
                         ( globalIjk.k() - min.k() ) * refinement.z() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<RigSimulationInputTool::TransformedNNCConnection, QString>
    RigSimulationInputTool::transformNNCToSectorCoordinates( const NNCConnection& connection,
                                                             const RigMainGrid&   mainGrid,
                                                             const caf::VecIjk0&  min,
                                                             const cvf::Vec3st&   refinement )
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
    transformed.cell1            = transformToSectorCoordinates( *gijk1, min, refinement );
    transformed.cell2            = transformToSectorCoordinates( *gijk2, min, refinement );
    transformed.transmissibility = connection.transmissibility;

    return transformed;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigSimulationInputTool::TransformedNNCConnection> RigSimulationInputTool::refineEditNncConnection( const NNCConnection& connection,
                                                                                                               const RigMainGrid&  mainGrid,
                                                                                                               const caf::VecIjk0& min,
                                                                                                               const cvf::Vec3st& refinement )
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
    caf::VecIjk0 baseCell1 = transformToSectorCoordinates( *gijk1, min, refinement );
    caf::VecIjk0 baseCell2 = transformToSectorCoordinates( *gijk2, min, refinement );

    std::vector<TransformedNNCConnection> refined;
    for ( size_t subI = 0; subI < refinement.x(); ++subI )
    {
        for ( size_t subJ = 0; subJ < refinement.y(); ++subJ )
        {
            for ( size_t subK = 0; subK < refinement.z(); ++subK )
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
std::expected<void, QString> RigSimulationInputTool::exportEditNncKeyword( RimEclipseCase*                   eclipseCase,
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
        QString( "Found %1 internal EDITNNC connections in sector (out of %2 total)" ).arg( sectorConnections.size() ).arg( allConnections.size() ) );

    // Step 3: Transform and refine
    bool hasRef = ( settings.refinement().x() != 1 || settings.refinement().y() != 1 || settings.refinement().z() != 1 ) ||
                  settings.hasNonUniformRefinement();

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
                RiaLogging::warning( result.error() );
            }
        }
    }

    if ( transformedConnections.empty() )
    {
        return std::unexpected( "Failed to transform any EDITNNC connections" );
    }

    RiaLogging::info( QString( "Exporting %1 EDITNNC connections to sector model" ).arg( transformedConnections.size() ) );

    // Step 4: Create keyword
    auto editnncKw = RimKeywordFactory::editnncKeyword( transformedConnections );

    // Step 5: Replace/add to EDIT section (not GRID!)
    deckFile.replaceKeyword( "EDIT", editnncKw );

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RigSimulationInputTool::applyModelPadding( RifOpmFlowDeckFile&            deckFile,
                                                                        const RigModelPaddingSettings& paddingSettings )
{
    return RigPadModel::extendGrid( deckFile, paddingSettings );
}
