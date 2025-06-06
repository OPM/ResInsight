/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicCreateTemporaryLgrFeature.h"

#include "RiaCellDividingTools.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "ExportCommands/RicExportLgrFeature.h"
#include "ExportCommands/RicExportLgrUi.h"
#include "RicDeleteTemporaryLgrsFeature.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimGridCollection.h"
#include "RimMainPlotCollection.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCompletions.h"

#include <QAction>
#include <QDir>

#include <cafSelectionManagerTools.h>
#include <cafUtils.h>
#include <cafVecIjk.h>

#include <set>

CAF_CMD_SOURCE_INIT( RicCreateTemporaryLgrFeature, "RicCreateTemporaryLgrFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateTemporaryLgrFeature::createLgrsForWellPaths( std::vector<RimWellPath*>                          wellPaths,
                                                           RimEclipseCase*                                    eclipseCase,
                                                           size_t                                             timeStep,
                                                           caf::VecIjk                                        lgrCellCounts,
                                                           Lgr::SplitType                                     splitType,
                                                           const std::set<RigCompletionData::CompletionType>& completionTypes,
                                                           QStringList*                                       wellsIntersectingOtherLgrs )
{
    auto               eclipseCaseData        = eclipseCase->eclipseCaseData();
    RigActiveCellInfo* activeCellInfo         = eclipseCaseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    RigActiveCellInfo* fractureActiveCellInfo = eclipseCaseData->activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL );

    auto lgrs = RicExportLgrFeature::buildLgrsForWellPaths( wellPaths,
                                                            eclipseCase,
                                                            timeStep,
                                                            lgrCellCounts,
                                                            splitType,
                                                            completionTypes,
                                                            wellsIntersectingOtherLgrs );

    for ( const auto& lgr : lgrs )
    {
        createLgr( lgr, eclipseCase->eclipseCaseData()->mainGrid() );

        size_t lgrCellCount = lgr.cellCount();

        activeCellInfo->addLgr( lgrCellCount );
        if ( fractureActiveCellInfo->reservoirActiveCellCount() > 0 )
        {
            fractureActiveCellInfo->addLgr( lgrCellCount );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateTemporaryLgrFeature::updateViews( RimEclipseCase* eclipseCase )
{
    RiaGuiApplication* guiApp = nullptr;
    if ( RiaGuiApplication::isRunning() )
    {
        guiApp = RiaGuiApplication::instance();
    }

    if ( guiApp ) RiaGuiApplication::clearAllSelections();

    deleteAllCachedData( eclipseCase );
    RimMainPlotCollection::current()->deleteAllCachedData();
    computeCachedData( eclipseCase );

    for ( auto view : eclipseCase->reservoirViews() )
    {
        if ( view && view->gridCollection() )
        {
            view->gridCollection()->syncFromMainEclipseGrid();
        }
    }

    RimMainPlotCollection::current()->wellLogPlotCollection()->loadDataAndUpdateAllPlots();

    if ( guiApp ) eclipseCase->createDisplayModelAndUpdateAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateTemporaryLgrFeature::isCommandEnabled() const
{
    std::vector<RimWellPathCompletions*> completions = caf::selectedObjectsByTypeStrict<RimWellPathCompletions*>();
    std::vector<RimWellPath*>            wellPaths   = caf::selectedObjectsByTypeStrict<RimWellPath*>();

    return !completions.empty() || !wellPaths.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateTemporaryLgrFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimWellPath*> wellPaths = RicExportLgrFeature::selectedWellPaths();
    if ( wellPaths.empty() ) return;

    QString dialogTitle = "Create Temporary LGR";

    RimEclipseCase* defaultEclipseCase = nullptr;
    int             defaultTimeStep    = 0;
    auto            activeView         = dynamic_cast<RimEclipseView*>( RiaApplication::instance()->activeGridView() );
    if ( activeView )
    {
        defaultEclipseCase = activeView->eclipseCase();
        defaultTimeStep    = activeView->currentTimeStep();
    }

    auto dialogData = RicExportLgrFeature::openDialog( dialogTitle, defaultEclipseCase, defaultTimeStep, true );
    if ( dialogData )
    {
        auto       eclipseCase     = dialogData->caseToApply();
        auto       lgrCellCounts   = dialogData->lgrCellCount();
        size_t     timeStep        = dialogData->timeStep();
        auto       splitType       = dialogData->splitType();
        const auto completionTypes = dialogData->completionTypes();

        RicDeleteTemporaryLgrsFeature::deleteAllTemporaryLgrs( eclipseCase );

        QStringList wellsIntersectingOtherLgrs;

        createLgrsForWellPaths( wellPaths, eclipseCase, timeStep, lgrCellCounts, splitType, completionTypes, &wellsIntersectingOtherLgrs );

        updateViews( eclipseCase );

        if ( !wellsIntersectingOtherLgrs.empty() )
        {
            RiaLogging::errorInMessageBox( nullptr,
                                           "LGR cells intersected",
                                           "At least one completion intersects with an LGR. No LGR(s) for those cells "
                                           "are produced" );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateTemporaryLgrFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Temporary LGR" );
    actionToSetup->setIcon( QIcon( ":/TempLGR16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
/// Todo: Guarding, caching LGR corner nodes calculations
//--------------------------------------------------------------------------------------------------
void RicCreateTemporaryLgrFeature::createLgr( const LgrInfo& lgrInfo, RigMainGrid* mainGrid )
{
    int    lgrId          = lgrInfo.id;
    size_t totalCellCount = mainGrid->totalCellCount();
    size_t lgrCellCount   = lgrInfo.cellCount();

    // Create local grid and set properties
    RigLocalGrid* localGrid = new RigLocalGrid( mainGrid );
    localGrid->setAsTempGrid( true );
    localGrid->setAssociatedWellPathName( lgrInfo.associatedWellPathName.toStdString() );
    localGrid->setGridId( lgrId );
    localGrid->setIndexToStartOfCells( totalCellCount );
    localGrid->setGridName( lgrInfo.name.toStdString() );
    localGrid->setGridPointDimensions( cvf::Vec3st( lgrInfo.sizes.i() + 1, lgrInfo.sizes.j() + 1, lgrInfo.sizes.k() + 1 ) );
    mainGrid->addLocalGrid( localGrid );

    size_t cellStartIndex = mainGrid->totalCellCount();
    size_t nodeStartIndex = mainGrid->nodes().size();

    // Resize global cell and node arrays
    {
        RigCell defaultCell;
        defaultCell.setHostGrid( localGrid );
        mainGrid->reservoirCells().resize( cellStartIndex + lgrCellCount, defaultCell );
        mainGrid->nodes().resize( nodeStartIndex + lgrCellCount * 8, cvf::Vec3d( 0, 0, 0 ) );
    }

    auto   lgrSizePerMainCell = lgrInfo.sizesPerMainGridCell();
    size_t gridLocalCellIndex = 0;

    // Loop through all new LGR cells
    for ( size_t lgrK = 0; lgrK < lgrInfo.sizes.k(); lgrK++ )
    {
        size_t mainK = lgrInfo.mainGridStartCell.k() + lgrK / lgrSizePerMainCell.k();

        for ( size_t lgrJ = 0; lgrJ < lgrInfo.sizes.j(); lgrJ++ )
        {
            size_t mainJ = lgrInfo.mainGridStartCell.j() + lgrJ / lgrSizePerMainCell.j();

            for ( size_t lgrI = 0; lgrI < lgrInfo.sizes.i(); lgrI++, gridLocalCellIndex++ )
            {
                size_t mainI = lgrInfo.mainGridStartCell.i() + lgrI / lgrSizePerMainCell.i();

                size_t mainCellIndex = mainGrid->cellIndexFromIJK( mainI, mainJ, mainK );
                auto&  mainGridCell  = mainGrid->cell( mainCellIndex );
                mainGridCell.setSubGrid( localGrid );

                RigCell& cell = mainGrid->cell( cellStartIndex + gridLocalCellIndex );
                cell.setGridLocalCellIndex( gridLocalCellIndex );
                cell.setParentCellIndex( mainCellIndex );

                // Corner coordinates
                {
                    size_t                    cIdx;
                    std::array<cvf::Vec3d, 8> vertices;
                    mainGrid->cellCornerVertices( mainCellIndex, vertices.data() );

                    auto cellCounts = lgrInfo.sizesPerMainGridCell();
                    auto lgrCoords = RiaCellDividingTools::createHexCornerCoords( vertices, cellCounts.i(), cellCounts.j(), cellCounts.k() );

                    size_t subI = lgrI % lgrSizePerMainCell.i();
                    size_t subJ = lgrJ % lgrSizePerMainCell.j();
                    size_t subK = lgrK % lgrSizePerMainCell.k();

                    size_t subIndex = subI + subJ * lgrSizePerMainCell.i() + subK * lgrSizePerMainCell.i() * lgrSizePerMainCell.j();

                    for ( cIdx = 0; cIdx < 8; ++cIdx )
                    {
                        auto& node = mainGrid->nodes()[nodeStartIndex + gridLocalCellIndex * 8 + cIdx];
                        node.set( lgrCoords[subIndex * 8 + cIdx] );
                        cell.cornerIndices()[cIdx] = nodeStartIndex + gridLocalCellIndex * 8 + cIdx;
                    }
                }
            }
        }
    }

    localGrid->setParentGrid( mainGrid );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateTemporaryLgrFeature::deleteAllCachedData( RimEclipseCase* eclipseCase )
{
    if ( eclipseCase )
    {
        std::vector<RiaDefines::ResultCatType> categoriesToExclude = { RiaDefines::ResultCatType::GENERATED };

        RigCaseCellResultsData* cellResultsDataMatrix = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
        if ( cellResultsDataMatrix )
        {
            cellResultsDataMatrix->freeAllocatedResultsData( categoriesToExclude, std::nullopt );
        }

        RigCaseCellResultsData* cellResultsDataFracture = eclipseCase->results( RiaDefines::PorosityModelType::FRACTURE_MODEL );
        if ( cellResultsDataFracture )
        {
            cellResultsDataFracture->freeAllocatedResultsData( categoriesToExclude, std::nullopt );
        }

        RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();
        if ( eclipseCaseData )
        {
            eclipseCaseData->clearWellCellsInGridCache();
            eclipseCaseData->setVirtualPerforationTransmissibilities( nullptr );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateTemporaryLgrFeature::computeCachedData( RimEclipseCase* eclipseCase )
{
    if ( eclipseCase )
    {
        RigCaseCellResultsData* cellResultsDataMatrix   = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
        RigCaseCellResultsData* cellResultsDataFracture = eclipseCase->results( RiaDefines::PorosityModelType::FRACTURE_MODEL );

        RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();
        if ( eclipseCaseData )
        {
            eclipseCaseData->mainGrid()->computeCachedData();
            eclipseCase->computeActiveCellsBoundingBox();
        }

        if ( cellResultsDataMatrix )
        {
            cellResultsDataMatrix->computeDepthRelatedResults();
            cellResultsDataMatrix->computeCellVolumes();
        }

        if ( cellResultsDataFracture )
        {
            cellResultsDataFracture->computeDepthRelatedResults();
        }
    }
}
