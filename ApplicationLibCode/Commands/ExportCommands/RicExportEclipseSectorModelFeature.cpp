/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RicExportEclipseSectorModelFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaResultNames.h"

#include "RicExportEclipseSectorModelUi.h"
#include "RicExportFeatureImpl.h"

#include "RifEclipseInputFileTools.h"
#include "RifOpmDeckTools.h"
#include "RifOpmFlowDeckFile.h"
#include "RifReaderEclipseOutput.h"

#include "ProjectDataModel/Jobs/RimKeywordBcprop.h"
#include "ProjectDataModel/Jobs/RimKeywordFactory.h"
#include "Rim3dView.h"
#include "RimDialogData.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"
#include "RimProject.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseCaseDataTools.h"
#include "RigEclipseResultTools.h"
#include "RigGridExportAdapter.h"
#include "RigMainGrid.h"
#include "RigResdataGridConverter.h"
#include "Well/RigSimWellData.h"
#include "Well/RigWellResultFrame.h"
#include "Well/RigWellResultPoint.h"

#include "Riu3DMainWindowTools.h"
#include "RiuPropertyViewTabWidget.h"
#include "RiuViewer.h"

#include "cafCmdFeatureManager.h"
#include "cafProgressInfo.h"
#include "cafSelectionManager.h"

#include "opm/common/OpmLog/KeywordLocation.hpp"
#include "opm/input/eclipse/Deck/DeckItem.hpp"
#include "opm/input/eclipse/Deck/DeckKeyword.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"
#include "opm/input/eclipse/Parser/ParserKeyword.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/C.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/S.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/W.hpp"
#include "opm/input/eclipse/Utility/Typetools.hpp"

#include <QAction>
#include <QDir>
#include <QFileInfo>

#include <set>
#include <string>

CAF_CMD_SOURCE_INIT( RicExportEclipseSectorModelFeature, "RicExportEclipseInputGridFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelFeature::openDialogAndExecuteCommand( RimEclipseView* view )
{
    if ( !view ) return;

    RigEclipseCaseData* caseData = view->eclipseCase()->eclipseCaseData();

    cvf::UByteArray cellVisibility;
    view->calculateCurrentTotalCellVisibility( &cellVisibility, view->currentTimeStep() );

    const auto& [min, max] = getVisibleCellRange( view, cellVisibility );

    RicExportEclipseSectorModelUi* exportSettings = RimProject::current()->dialogData()->exportSectorModelUi();
    exportSettings->setCaseData( caseData, view, min, max );

    exportSettings->applyBoundaryDefaults();
    exportSettings->removeInvalidKeywords();

    RiuPropertyViewTabWidget propertyDialog( Riu3DMainWindowTools::mainWindowWidget(),
                                             exportSettings,
                                             "Export Eclipse Sector Model",
                                             exportSettings->tabNames() );

    RicExportFeatureImpl::configureForExport( propertyDialog.dialogButtonBox() );

    if ( propertyDialog.exec() == QDialog::Accepted )
    {
        executeCommand( view, *exportSettings, "ExportInputGrid" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelFeature::executeCommand( RimEclipseView*                      view,
                                                         const RicExportEclipseSectorModelUi& exportSettings,
                                                         const QString&                       logPrefix )
{
    int resultProgressPercentage = exportSettings.exportParameters() ? std::min( (int)exportSettings.selectedKeywords().size(), 20 ) : 0;

    int faultsProgressPercentage = exportSettings.exportFaults() ? 10 : 0;

    int gridProgressPercentage = 100 - resultProgressPercentage - faultsProgressPercentage;
    caf::ProgressInfo progress( gridProgressPercentage + resultProgressPercentage + faultsProgressPercentage, "Export Eclipse Sector Model" );

    CVF_ASSERT( exportSettings.refinement().x() > 0u && exportSettings.refinement().y() > 0u && exportSettings.refinement().z() > 0u );

    if ( exportSettings.exportGrid() )
    {
        auto task = progress.task( "Export Grid", gridProgressPercentage );
        exportGrid( view, exportSettings );
    }

    if ( exportSettings.exportParameters() != RicExportEclipseSectorModelUi::EXPORT_NO_RESULTS )
    {
        auto task = progress.task( "Export Properties", resultProgressPercentage );
        exportParameters( view, exportSettings );
    }

    if ( exportSettings.exportFaults() != RicExportEclipseSectorModelUi::EXPORT_NO_RESULTS )
    {
        auto task = progress.task( "Export Faults", faultsProgressPercentage );
        exportFaults( view, exportSettings );
    }

    // Export simulation input if enabled
    if ( exportSettings.m_exportSimulationInput() )
    {
        if ( auto result = exportSimulationInput( *view, *view->eclipseCase(), exportSettings ); !result )
        {
            RiaLogging::error( QString( "Failed to export simulation input: %1" ).arg( result.error() ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelFeature::exportGrid( RimEclipseView* view, const RicExportEclipseSectorModelUi& exportSettings )
{
    cvf::UByteArray cellVisibility;
    view->calculateCurrentTotalCellVisibility( &cellVisibility, view->currentTimeStep() );
    getVisibleCellRange( view, cellVisibility );

    const cvf::UByteArray* cellVisibilityForActnum = exportSettings.makeInvisibleCellsInactive() ? &cellVisibility : nullptr;

    bool worked = RifEclipseInputFileTools::exportGrid( exportSettings.exportGridFilename(),
                                                        view->eclipseCase()->eclipseCaseData(),
                                                        exportSettings.exportInLocalCoordinates(),
                                                        cellVisibilityForActnum,
                                                        exportSettings.min(),
                                                        exportSettings.max(),
                                                        exportSettings.refinement() );

    if ( !worked )
    {
        RiaLogging::error( QString( "Unable to write grid to '%1'" ).arg( exportSettings.exportGridFilename() ) );
    }
    else
    {
        if ( view->eclipseCase()->eclipseCaseData()->gridCount() > 1u )
        {
            RiaLogging::warning( "Grid has LGRs but ResInsight only supports exporting the Main Grid" );
        }

        QFileInfo info( exportSettings.exportGridFilename() );
        RiaApplication::instance()->setLastUsedDialogDirectory( "EXPORT_INPUT_GRID", info.absolutePath() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelFeature::exportParameters( RimEclipseView* view, const RicExportEclipseSectorModelUi& exportSettings )
{
    std::vector<QString> keywords = exportSettings.selectedKeywords;

    if ( exportSettings.exportParameters == RicExportEclipseSectorModelUi::EXPORT_TO_SEPARATE_FILE_PER_RESULT )
    {
        QFileInfo info( exportSettings.exportGridFilename() );
        QDir      dirPath = info.absoluteDir();
        for ( const QString& keyword : keywords )
        {
            QString fileName = dirPath.absoluteFilePath( keyword + ".GRDECL" );
            bool    worked   = RifEclipseInputFileTools::exportKeywords( fileName,
                                                                    view->eclipseCase()->eclipseCaseData(),
                                                                         { keyword },
                                                                    exportSettings.writeEchoKeywords(),
                                                                    exportSettings.min(),
                                                                    exportSettings.max(),
                                                                    exportSettings.refinement() );
            if ( !worked )
            {
                RiaLogging::error( QString( "Unable to write results to '%1'" ).arg( fileName ) );
            }
        }
    }
    else
    {
        QString fileName = exportSettings.exportParametersFilename();
        if ( exportSettings.exportParameters() == RicExportEclipseSectorModelUi::EXPORT_TO_GRID_FILE )
        {
            fileName = exportSettings.exportGridFilename();
        }

        bool worked = RifEclipseInputFileTools::exportKeywords( fileName,
                                                                view->eclipseCase()->eclipseCaseData(),
                                                                keywords,
                                                                exportSettings.writeEchoKeywords(),
                                                                exportSettings.min(),
                                                                exportSettings.max(),
                                                                exportSettings.refinement() );

        if ( !worked )
        {
            RiaLogging::error( QString( "Unable to write results to '%1'" ).arg( fileName ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelFeature::exportFaults( RimEclipseView* view, const RicExportEclipseSectorModelUi& exportSettings )
{
    if ( exportSettings.exportFaults == RicExportEclipseSectorModelUi::EXPORT_TO_SEPARATE_FILE_PER_RESULT )
    {
        for ( const auto& faultInView : view->faultCollection()->faults() )
        {
            auto    rigFault = faultInView->faultGeometry();
            QString fileName = QString( "%1.GRDECL" ).arg( rigFault->name() );
            RifEclipseInputFileTools::saveFault( fileName,
                                                 view->eclipseCase()->mainGrid(),
                                                 rigFault->faultFaces(),
                                                 rigFault->name(),
                                                 exportSettings.min(),
                                                 exportSettings.max(),
                                                 exportSettings.refinement() );
        }
    }
    else
    {
        QString             fileName = exportSettings.exportFaultsFilename();
        QIODevice::OpenMode openFlag = QIODevice::Truncate;
        if ( exportSettings.exportParameters() == RicExportEclipseSectorModelUi::EXPORT_TO_GRID_FILE )
        {
            openFlag = QIODevice::Append;
            fileName = exportSettings.exportGridFilename();
        }
        QFile exportFile( fileName );

        if ( !exportFile.open( QIODevice::Text | QIODevice::WriteOnly | openFlag ) )
        {
            RiaLogging::error( "Could not open the file : " + fileName );
        }

        QTextStream stream( &exportFile );
        RifEclipseInputFileTools::saveFaults( stream,
                                              view->eclipseCase()->mainGrid(),
                                              exportSettings.min(),
                                              exportSettings.max(),
                                              exportSettings.refinement() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RicExportEclipseSectorModelFeature::exportSimulationInput( RimEclipseView& view,
                                                                                        RimEclipseCase& eclipseCase,
                                                                                        const RicExportEclipseSectorModelUi& exportSettings )
{
    // Load the deck file
    QFileInfo fi( eclipseCase.gridFileName() );
    QString   dataFileName = fi.absolutePath() + "/" + fi.completeBaseName() + ".DATA";

    RifOpmFlowDeckFile deckFile;
    if ( !deckFile.loadDeck( dataFileName.toStdString() ) )
    {
        return std::unexpected( QString( "Unable to load deck file '%1'" ).arg( dataFileName ) );
    }

    QFileInfo exportGridInfo( exportSettings.exportGridFilename() );
    QString   outputFolder = exportGridInfo.absolutePath();
    QString   outputFile   = exportGridInfo.completeBaseName() + ".DATA";

    // Only change values when exporting to modified box: original values should just work (tm) for full grid box
    if ( exportSettings.exportGridBox() != RicExportEclipseSectorModelUi::GridBoxSelection::FULL_GRID_BOX )
    {
        if ( auto result = updateCornerPointGridInDeckFile( &eclipseCase, exportSettings, deckFile ); !result )
        {
            return result;
        }

        // Generate BORDNUM result based on the selected grid box
        auto bordnumVisibility = createVisibilityBasedOnBoxSelection( &view, exportSettings );
        if ( !bordnumVisibility.isNull() )
        {
            RigEclipseResultTools::generateBorderResult( &eclipseCase, bordnumVisibility, RiaResultNames::bordnum() );
            if ( exportSettings.m_boundaryCondition() == RicExportEclipseSectorModelUi::BoundaryCondition::BCCON_BCPROP )
            {
                // Generate BCCON result to assign values 1-6 based on which face of the box the border cells are on
                RigEclipseResultTools::generateBcconResult( &eclipseCase, exportSettings.min(), exportSettings.max() );
            }
            else if ( exportSettings.m_boundaryCondition() == RicExportEclipseSectorModelUi::BoundaryCondition::OPERNUM_OPERATER )
            {
                // Generate OPERNUM result based on BORDNUM (border cells get max existing OPERNUM + 1)
                RigEclipseResultTools::generateOperNumResult( &eclipseCase );

                if ( auto result = addOperNumRegionAndOperater( &eclipseCase, exportSettings, deckFile ); !result )
                {
                    return result;
                }
            }
        }

        if ( auto result = replaceKeywordValuesInDeckFile( &eclipseCase, exportSettings, deckFile ); !result )
        {
            return result;
        }

        if ( auto result = addBorderBoundaryConditions( &eclipseCase, exportSettings, deckFile ); !result )
        {
            return result;
        }

        if ( auto result = addFaultsToDeckFile( &eclipseCase, exportSettings, deckFile ); !result )
        {
            return result;
        }

        if ( auto result = filterAndUpdateWellKeywords( &eclipseCase, exportSettings, deckFile ); !result )
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
std::expected<void, QString>
    RicExportEclipseSectorModelFeature::updateCornerPointGridInDeckFile( RimEclipseCase*                      eclipseCase,
                                                                         const RicExportEclipseSectorModelUi& exportSettings,
                                                                         RifOpmFlowDeckFile&                  deckFile )
{
    // Get grid bounds for extraction

    RigGridExportAdapter gridAdapter( eclipseCase->eclipseCaseData(), exportSettings.min(), exportSettings.max(), exportSettings.refinement() );

    std::vector<float> coordArray;
    std::vector<float> zcornArray;
    std::vector<int>   actnumArray;

    RigResdataGridConverter::convertGridToCornerPointArrays( gridAdapter, coordArray, zcornArray, actnumArray );

    // Sector dimensions (after refinement)
    std::vector<int> dimens = { static_cast<int>( gridAdapter.cellCountI() ),
                                static_cast<int>( gridAdapter.cellCountJ() ),
                                static_cast<int>( gridAdapter.cellCountK() ) };

    if ( !deckFile.replaceKeywordData( "DIMENS", dimens ) )
    {
        return std::unexpected( "Failed to replace DIMENS keyword in deck file" );
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
std::expected<void, QString>
    RicExportEclipseSectorModelFeature::replaceKeywordValuesInDeckFile( RimEclipseCase*                      eclipseCase,
                                                                        const RicExportEclipseSectorModelUi& exportSettings,
                                                                        RifOpmFlowDeckFile&                  deckFile )
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
                                                                    exportSettings.min(),
                                                                    exportSettings.max(),
                                                                    exportSettings.refinement() );
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
        else
        {
            // Not all keywords will have data - this is expected
            RiaLogging::debug(
                QString( "Could not extract data for keyword '%1': %2" ).arg( keyword ).arg( QString::fromStdString( result.error() ) ) );
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RicExportEclipseSectorModelFeature::addBorderBoundaryConditions( RimEclipseCase* eclipseCase,
                                                                                              const RicExportEclipseSectorModelUi& exportSettings,
                                                                                              RifOpmFlowDeckFile& deckFile )
{
    // Generate border cell faces
    auto borderCellFaces = RigEclipseResultTools::generateBorderCellFaces( eclipseCase );

    if ( !borderCellFaces.empty() )
    {
        // Transform border cell face coordinates to sector-relative coordinates
        for ( auto& face : borderCellFaces )
        {
            auto transformResult =
                transformIjkToSectorCoordinates( face.ijk, exportSettings.min(), exportSettings.max(), exportSettings.refinement() );

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
            // Note: transformIjkToSectorCoordinates returns 1-based coordinates, but we need to convert back to 0-based
            // for the BorderCellFace struct
            face.ijk = cvf::Vec3st( transformResult->x() - 1, transformResult->y() - 1, transformResult->z() - 1 );
        }

        // Create BCCON keyword using the factory
        Opm::DeckKeyword bcconKw = RimKeywordFactory::bcconKeyword( borderCellFaces );

        // Replace BCCON keyword in GRID section
        if ( !deckFile.replaceKeyword( "GRID", bcconKw ) )
        {
            return std::unexpected( "Failed to replace BCCON keyword in deck file" );
        }

        // Build BCPROP records from the UI configuration
        std::vector<Opm::DeckRecord> bcpropRecords;
        for ( const auto& bcprop : exportSettings.m_bcpropKeywords )
        {
            if ( bcprop != nullptr )
            {
                Opm::DeckKeyword kw     = bcprop->keyword();
                const auto&      record = kw.getRecord( 0 );
                bcpropRecords.push_back( record );
            }
        }

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
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RicExportEclipseSectorModelFeature::addFaultsToDeckFile( RimEclipseCase*                      eclipseCase,
                                                                                      const RicExportEclipseSectorModelUi& exportSettings,
                                                                                      RifOpmFlowDeckFile&                  deckFile )
{
    // Create FAULTS keyword using the factory
    Opm::DeckKeyword faultsKw =
        RimKeywordFactory::faultsKeyword( eclipseCase->mainGrid(), exportSettings.min(), exportSettings.max(), exportSettings.refinement() );

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
std::pair<cvf::Vec3st, cvf::Vec3st> RicExportEclipseSectorModelFeature::getVisibleCellRange( RimEclipseView*        view,
                                                                                             const cvf::UByteArray& cellVisibillity )
{
    const RigMainGrid* mainGrid = view->eclipseCase()->mainGrid();
    cvf::Vec3st        max      = cvf::Vec3st::ZERO;
    cvf::Vec3st        min      = cvf::Vec3st( mainGrid->cellCountI() - 1, mainGrid->cellCountJ() - 1, mainGrid->cellCountK() - 1 );

    size_t cellCount = mainGrid->cellCount();
    for ( size_t index = 0; index < cellCount; ++index )
    {
        if ( cellVisibillity[index] )
        {
            cvf::Vec3st ijk;
            mainGrid->ijkFromCellIndex( index, &ijk[0], &ijk[1], &ijk[2] );
            for ( int n = 0; n < 3; ++n )
            {
                min[n] = std::min( min[n], ijk[n] );
                max[n] = std::max( max[n], ijk[n] );
            }
        }
    }
    return std::make_pair( min, max );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportEclipseSectorModelFeature::isCommandEnabled() const
{
    return selectedView() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelFeature::onActionTriggered( bool isChecked )
{
    RimEclipseView* view = RicExportEclipseSectorModelFeature::selectedView();
    openDialogAndExecuteCommand( view );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Export Eclipse Sector Model" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView* RicExportEclipseSectorModelFeature::selectedView() const
{
    auto contextViewer = dynamic_cast<RiuViewer*>( caf::CmdFeatureManager::instance()->currentContextMenuTargetWidget() );
    if ( contextViewer != nullptr )
    {
        // Command is triggered from viewer
        Rim3dView* activeView = RiaApplication::instance()->activeMainOrComparisonGridView();
        return dynamic_cast<RimEclipseView*>( activeView );
    }

    // Command triggered from project tree or file menu
    auto view = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimEclipseView>();
    return view;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::UByteArray>
    RicExportEclipseSectorModelFeature::createVisibilityBasedOnBoxSelection( RimEclipseView*                      view,
                                                                             const RicExportEclipseSectorModelUi& exportSettings )
{
    RigEclipseCaseData* caseData = view->eclipseCase()->eclipseCaseData();

    switch ( exportSettings.exportGridBox() )
    {
        case RicExportEclipseSectorModelUi::VISIBLE_WELLS_BOX:
        {
            auto [minWellCells, maxWellCells] =
                RicExportEclipseSectorModelUi::computeVisibleWellCells( view, caseData, exportSettings.m_visibleWellsPadding() );
            return RigEclipseCaseDataTools::createVisibilityFromIjkBounds( caseData, minWellCells, maxWellCells );
        }
        case RicExportEclipseSectorModelUi::ACTIVE_CELLS_BOX:
        {
            // For active cells, we need to create a
            // visibility array based on active cells
            auto   activeCellInfo       = caseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
            auto   activeReservoirCells = activeCellInfo->activeReservoirCellIndices();
            size_t totalCellCount       = caseData->mainGrid()->cellCount();

            cvf::ref<cvf::UByteArray> visibility = new cvf::UByteArray( totalCellCount );
            visibility->setAll( false );

            for ( auto activeCellIdx : activeReservoirCells )
            {
                visibility->set( activeCellIdx.value(), true );
            }
            return visibility;
        }
        case RicExportEclipseSectorModelUi::VISIBLE_CELLS_BOX:
        {
            // Use the current total cell visibility
            // from the view
            cvf::ref<cvf::UByteArray> cellVisibility = new cvf::UByteArray();
            view->calculateCurrentTotalCellVisibility( cellVisibility.p(), view->currentTimeStep() );
            return cellVisibility;
        }
        case RicExportEclipseSectorModelUi::MANUAL_SELECTION:
        {
            return RigEclipseCaseDataTools::createVisibilityFromIjkBounds( caseData, exportSettings.min(), exportSettings.max() );
        }
        case RicExportEclipseSectorModelUi::FULL_GRID_BOX:
        {
            // For full grid, create visibility for
            // all cells
            const RigMainGrid* mainGrid = caseData->mainGrid();
            const cvf::Vec3st  minIjk   = cvf::Vec3st::ZERO;
            const cvf::Vec3st  maxIjk   = mainGrid->cellCounts();
            return RigEclipseCaseDataTools::createVisibilityFromIjkBounds( caseData, minIjk, maxIjk );
        }
        default:
            return nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigSimWellData*>
    RicExportEclipseSectorModelFeature::findIntersectingWells( RimEclipseCase* eclipseCase, const cvf::Vec3st& min, const cvf::Vec3st& max )
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
                // TODO: bug? min/max is zero-indexed?
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
std::expected<cvf::Vec3st, QString> RicExportEclipseSectorModelFeature::transformIjkToSectorCoordinates( const cvf::Vec3st& originalIjk,
                                                                                                         const cvf::Vec3st& min,
                                                                                                         const cvf::Vec3st& max,
                                                                                                         const cvf::Vec3st& refinement )
{
    // Check if original IJK is within the sector bounds
    if ( originalIjk.x() < min.x() || originalIjk.x() > max.x() || originalIjk.y() < min.y() || originalIjk.y() > max.y() ||
         originalIjk.z() < min.z() || originalIjk.z() > max.z() )
    {
        return std::unexpected( QString( "IJK coordinates (%1, %2, %3) are outside sector bounds [(%4, %5, %6), (%7, %8, %9)]" )
                                    .arg( originalIjk.x() )
                                    .arg( originalIjk.y() )
                                    .arg( originalIjk.z() )
                                    .arg( min.x() )
                                    .arg( min.y() )
                                    .arg( min.z() )
                                    .arg( max.x() )
                                    .arg( max.y() )
                                    .arg( max.z() ) );
    }

    // Transform to sector-relative coordinates with refinement
    // Eclipse uses 1-based indexing, so we'll return 1-based coordinates
    cvf::Vec3st sectorIjk;
    sectorIjk.x() = ( originalIjk.x() - min.x() ) * refinement.x() + 1;
    sectorIjk.y() = ( originalIjk.y() - min.y() ) * refinement.y() + 1;
    sectorIjk.z() = ( originalIjk.z() - min.z() ) * refinement.z() + 1;

    return sectorIjk;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<Opm::DeckRecord, QString>
    RicExportEclipseSectorModelFeature::processWelspecsRecord( const Opm::DeckRecord&               record,
                                                               const std::string&                   wellName,
                                                               const RicExportEclipseSectorModelUi& exportSettings )
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
    size_t clampedI = std::max( exportSettings.min().x(), std::min( exportSettings.max().x(), static_cast<size_t>( origI ) ) );
    size_t clampedJ = std::max( exportSettings.min().y(), std::min( exportSettings.max().y(), static_cast<size_t>( origJ ) ) );

    // Transform with clamped coordinates (this will always succeed)
    size_t sectorI = ( clampedI - exportSettings.min().x() ) * exportSettings.refinement().x() + 1;
    size_t sectorJ = ( clampedJ - exportSettings.min().y() ) * exportSettings.refinement().y() + 1;

    if ( origI < static_cast<int>( exportSettings.min().x() ) || origI > static_cast<int>( exportSettings.max().x() ) ||
         origJ < static_cast<int>( exportSettings.min().y() ) || origJ > static_cast<int>( exportSettings.max().y() ) )
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
std::expected<Opm::DeckRecord, QString>
    RicExportEclipseSectorModelFeature::processCompdatRecord( const Opm::DeckRecord&               record,
                                                              const std::string&                   wellName,
                                                              const RicExportEclipseSectorModelUi& exportSettings )
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
    cvf::Vec3st origIjkK1( origI, origJ, origK1 );
    auto        transformResultK1 =
        transformIjkToSectorCoordinates( origIjkK1, exportSettings.min(), exportSettings.max(), exportSettings.refinement() );

    // Transform K2
    cvf::Vec3st origIjkK2( origI, origJ, origK2 );
    auto        transformResultK2 =
        transformIjkToSectorCoordinates( origIjkK2, exportSettings.min(), exportSettings.max(), exportSettings.refinement() );

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
std::expected<Opm::DeckRecord, QString>
    RicExportEclipseSectorModelFeature::processCompsegsRecord( const Opm::DeckRecord&               record,
                                                               const std::string&                   wellName,
                                                               bool                                 isWellNameRecord,
                                                               const RicExportEclipseSectorModelUi& exportSettings )
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

    cvf::Vec3st origIjk( origI, origJ, origK );
    auto transformResult = transformIjkToSectorCoordinates( origIjk, exportSettings.min(), exportSettings.max(), exportSettings.refinement() );

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
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RicExportEclipseSectorModelFeature::filterAndUpdateWellKeywords( RimEclipseCase* eclipseCase,
                                                                                              const RicExportEclipseSectorModelUi& exportSettings,
                                                                                              RifOpmFlowDeckFile& deckFile )
{
    // Find wells that intersect with the sector
    auto intersectingWells = findIntersectingWells( eclipseCase, exportSettings.min(), exportSettings.max() );

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
                                              "WCONPROD",
                                              "WELSEGS",
                                              "WELSPECS",
                                              "WELTARG",
                                              "WPAVEDEP",
                                              "WRFTPLT",
                                              "WTRACER" };

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
                            auto result = processWelspecsRecord( record, wellName, exportSettings );
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
                            auto result = processCompdatRecord( record, wellName, exportSettings );
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
                            auto result = processCompsegsRecord( record, wellName, isWellNameRecord, exportSettings );
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
std::expected<void, QString> RicExportEclipseSectorModelFeature::addOperNumRegionAndOperater( RimEclipseCase* eclipseCase,
                                                                                              const RicExportEclipseSectorModelUi& exportSettings,
                                                                                              RifOpmFlowDeckFile& deckFile )
{
    // Update REGDIMS and add OPERATER keyword for OPERNUM regions
    // Get the OPERNUM region number that was assigned to border cells
    int maxOperNum    = RigEclipseResultTools::findMaxOperNumValue( eclipseCase );
    int operNumRegion = maxOperNum + 1; // Border cells use this region number

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
        // Add INCLUDE for OPERNUM.GRDECL in REGIONS section
        if ( !deckFile.addIncludeKeyword( "REGIONS", "OPERNUM", "OPERNUM.GRDECL" ) )
        {
            return std::unexpected( "Failed to add INCLUDE for OPERNUM in REGIONS section" );
        }
        RiaLogging::info( "Added INCLUDE 'OPERNUM.GRDECL' to REGIONS section" );
    }
    else
    {
        RiaLogging::info( "OPERNUM keyword already exists in deck, skipping INCLUDE" );
    }

    // Create OPERATER keyword to multiply pore volume in border region
    // OPERATER format: TARGET_ARRAY REGION_NUMBER OPERATION ARRAY_PARAMETER PARAM1 PARAM2 REGION_NAME
    // Example: OPERATER / PORV 1 MULTX PORV 1.0e6 1* 1* /
    float            porvMultiplier = static_cast<float>( exportSettings.m_porvMultiplier() );
    Opm::DeckKeyword operaterKw     = RimKeywordFactory::operaterKeyword( "PORV", operNumRegion, "MULTX", "PORV", porvMultiplier );

    // Add OPERATER keyword to EDIT section
    if ( !deckFile.replaceKeyword( "EDIT", operaterKw ) )
    {
        return std::unexpected( "Failed to add OPERATER keyword to EDIT section" );
    }

    RiaLogging::info( QString( "Added OPERATER keyword to multiply PORV in region %1 by %2" ).arg( operNumRegion ).arg( porvMultiplier ) );

    return {};
}
