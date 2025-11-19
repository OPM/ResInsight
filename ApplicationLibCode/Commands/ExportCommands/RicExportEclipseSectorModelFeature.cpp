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
#include "RigBoundingBoxIjk.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseCaseDataTools.h"
#include "RigEclipseResultTools.h"
#include "RigGridExportAdapter.h"
#include "RigMainGrid.h"
#include "RigResdataGridConverter.h"
#include "RigSimulationInputSettings.h"
#include "RigSimulationInputTool.h"
#include "Well/RigSimWellData.h"
#include "Well/RigWellResultFrame.h"
#include "Well/RigWellResultPoint.h"

#include "Riu3DMainWindowTools.h"
#include "RiuPropertyViewTabWidget.h"
#include "RiuViewer.h"

#include "cafCmdFeatureManager.h"
#include "cafProgressInfo.h"
#include "cafSelectionManager.h"

#include "opm/input/eclipse/Deck/DeckItem.hpp"
#include "opm/input/eclipse/Deck/DeckKeyword.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"
#include "opm/input/eclipse/Parser/ParserKeyword.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/C.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/E.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/O.hpp"
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
        // Convert UI settings to RigSimulationInputSettings
        RigSimulationInputSettings settings;
        settings.setMin( exportSettings.min() );
        settings.setMax( exportSettings.max() );
        settings.setRefinement( exportSettings.refinement() );

        std::vector<Opm::DeckRecord> bcpropKeywords;
        for ( auto bcprop : exportSettings.m_bcpropKeywords )
        {
            Opm::DeckKeyword kw     = bcprop->keyword();
            const auto&      record = kw.getRecord( 0 );
            bcpropKeywords.push_back( record );
        }
        settings.setBcpropKeywords( bcpropKeywords );
        settings.setBoundaryCondition( exportSettings.m_boundaryCondition() );

        // Get input deck file name from eclipse case
        QFileInfo fi( view->eclipseCase()->gridFileName() );
        QString   dataFileName = fi.absolutePath() + "/" + fi.completeBaseName() + ".DATA";
        settings.setInputDeckFileName( dataFileName );
        settings.setOutputDeckFileName( exportSettings.exportGridFilename() );

        cvf::ref<cvf::UByteArray> cellVisibility = createVisibilityBasedOnBoxSelection( view, exportSettings );
        if ( auto result = RigSimulationInputTool::exportSimulationInput( *view->eclipseCase(), settings, cellVisibility.p() ); !result )
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
std::pair<caf::VecIjk0, caf::VecIjk0> RicExportEclipseSectorModelFeature::getVisibleCellRange( RimEclipseView*        view,
                                                                                               const cvf::UByteArray& cellVisibillity )
{
    const RigMainGrid* mainGrid = view->eclipseCase()->mainGrid();
    caf::VecIjk0       max      = caf::VecIjk0::ZERO;
    caf::VecIjk0       min( mainGrid->cellCountI() - 1, mainGrid->cellCountJ() - 1, mainGrid->cellCountK() - 1 );

    size_t cellCount = mainGrid->cellCount();
    for ( size_t index = 0; index < cellCount; ++index )
    {
        if ( cellVisibillity[index] )
        {
            auto ijk = mainGrid->ijkFromCellIndex( index );
            if ( ijk.has_value() )
            {
                for ( int n = 0; n < 3; ++n )
                {
                    min[n] = std::min( min[n], ijk.value()[n] );
                    max[n] = std::max( max[n], ijk.value()[n] );
                }
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
            const caf::VecIjk0 minIjk   = caf::VecIjk0::ZERO;
            const caf::VecIjk0 maxIjk( mainGrid->cellCountI(), mainGrid->cellCountJ(), mainGrid->cellCountK() );
            return RigEclipseCaseDataTools::createVisibilityFromIjkBounds( caseData, minIjk, maxIjk );
        }
        default:
            return nullptr;
    }
}
