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
#include "RifOpmFlowDeckFile.h"
#include "RifReaderEclipseOutput.h"

#include "ProjectDataModel/Jobs/RimKeywordBcprop.h"
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

#include "Riu3DMainWindowTools.h"
#include "RiuPropertyViewTabWidget.h"
#include "RiuViewer.h"

#include "cafCmdFeatureManager.h"
#include "cafProgressInfo.h"
#include "cafSelectionManager.h"

#include "opm/input/eclipse/Deck/DeckKeyword.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"

#include <QAction>
#include <QDir>
#include <QFileInfo>

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

    // Generate BORDNUM result based on the selected grid box
    auto bordnumVisibility = createVisibilityBasedOnBoxSelection( view, exportSettings );
    if ( !bordnumVisibility.isNull() )
    {
        RigEclipseResultTools::generateBorderResult( view->eclipseCase(), bordnumVisibility, RiaResultNames::bordnum() );

        // Generate OPERNUM result based on BORDNUM (border cells get max existing OPERNUM + 1)
        RigEclipseResultTools::generateOperNumResult( view->eclipseCase() );
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
        exportSimulationInput( view, exportSettings );
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

    // Automatically add BORDNUM to the keywords list if not already present
    if ( std::find( keywords.begin(), keywords.end(), RiaResultNames::bordnum() ) == keywords.end() )
    {
        keywords.push_back( RiaResultNames::bordnum() );
    }

    // Automatically add OPERNUM to the keywords list if not already present
    if ( std::find( keywords.begin(), keywords.end(), RiaResultNames::opernum() ) == keywords.end() )
    {
        keywords.push_back( RiaResultNames::opernum() );
    }

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
void RicExportEclipseSectorModelFeature::exportSimulationInput( RimEclipseView* view, const RicExportEclipseSectorModelUi& exportSettings )
{
    // Load the deck file
    QFileInfo fi( view->eclipseCase()->gridFileName() );
    QString   dataFileName = fi.absolutePath() + "/" + fi.completeBaseName() + ".DATA";

    RifOpmFlowDeckFile deckFile;
    if ( !deckFile.loadDeck( dataFileName.toStdString() ) )
    {
        RiaLogging::error( QString( "Unable to load deck file '%1'" ).arg( dataFileName ) );
        return;
    }

    // Only change values when exporting to modified box: original values should just work (tm) for full grid box
    if ( exportSettings.exportGridBox() != RicExportEclipseSectorModelUi::GridBoxSelection::FULL_GRID_BOX )
    {
        // Get grid bounds for extraction
        RigGridExportAdapter gridAdapter( view->eclipseCase()->eclipseCaseData(),
                                          exportSettings.min(),
                                          exportSettings.max(),
                                          exportSettings.refinement() );
        // Grid dimensions (after refinement)
        std::vector<float> coordArray;
        std::vector<float> zcornArray;
        std::vector<int>   actnumArray;

        RigResdataGridConverter::convertGridToCornerPointArrays( gridAdapter, coordArray, zcornArray, actnumArray );

        std::vector<int> dimens = { static_cast<int>( gridAdapter.cellCountI() ),
                                    static_cast<int>( gridAdapter.cellCountJ() ),
                                    static_cast<int>( gridAdapter.cellCountK() ) };

        deckFile.replaceKeywordData( "DIMENS", dimens );

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

        deckFile.replaceKeywordData( "COORD", coords );
        deckFile.replaceKeywordData( "ZCORN", zcorn );
        deckFile.replaceKeywordData( "ACTNUM", actnumArray );

        // TODO: deal with map axis

        // Extract and replace keyword data for all keywords in the deck
        auto keywords = deckFile.keywords( false );
        RiaLogging::info( QString( "Processing %1 keywords from deck file" ).arg( keywords.size() ) );

        for ( const auto& keywordStdStr : keywords )
        {
            QString keyword = QString::fromStdString( keywordStdStr );

            // Skip special keywords that aren't cell properties
            if ( keyword.startsWith( "DATES" ) || keyword == "SCHEDULE" || keyword == "GRID" || keyword == "PROPS" ||
                 keyword == "SOLUTION" || keyword == "RUNSPEC" || keyword == "SUMMARY" )
            {
                continue;
            }

            // Try to extract keyword data
            auto result = RifEclipseInputFileTools::extractKeywordData( view->eclipseCase()->eclipseCaseData(),
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

        // Generate border cell faces
        auto borderCellFaces = RigEclipseResultTools::generateBorderCellFaces( view->eclipseCase() );

        if ( !borderCellFaces.empty() )
        {
            // Add BCCON keyword
            if ( !deckFile.addBcconKeyword( "GRID", borderCellFaces ) )
            {
                RiaLogging::error( "Failed to add BCCON keyword to deck file" );
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

            // Add BCPROP keyword
            if ( !deckFile.addBcpropKeyword( "GRID", borderCellFaces, bcpropRecords ) )
            {
                RiaLogging::error( "Failed to add BCPROP keyword to deck file" );
            }
        }
        else
        {
            RiaLogging::warning( "No border cells found - skipping BCCON/BCPROP keyword generation" );
        }
    }

    // Save the modified deck file to the export directory
    QFileInfo exportGridInfo( exportSettings.exportGridFilename() );
    QString   outputFolder = exportGridInfo.absolutePath();
    QString   outputFile   = exportGridInfo.completeBaseName() + ".DATA";
    if ( !deckFile.saveDeck( outputFolder.toStdString(), outputFile.toStdString() ) )
    {
        RiaLogging::error( QString( "Failed to save modified deck file to '%1/%2'" ).arg( outputFolder ).arg( outputFile ) );
    }
    else
    {
        RiaLogging::info( QString( "Saved modified deck file to '%1/%2'" ).arg( outputFolder ).arg( outputFile ) );
    }
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
            // For active cells, we need to create a visibility array based on active cells
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
            // Use the current total cell visibility from the view
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
            // For full grid, create visibility for all cells
            const RigMainGrid* mainGrid = caseData->mainGrid();
            const cvf::Vec3st  minIjk   = cvf::Vec3st::ZERO;
            const cvf::Vec3st  maxIjk   = mainGrid->cellCounts();
            return RigEclipseCaseDataTools::createVisibilityFromIjkBounds( caseData, minIjk, maxIjk );
        }
        default:
            return nullptr;
    }
}
