/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RiuMenuBarBuildTools.h"

#include "RimEclipseCaseCollection.h"

#include "RiuToolTipMenu.h"
#include "RiuTools.h"

#include "cafCmdFeatureManager.h"
#include "cafCmdFeatureMenuBuilder.h"

#include "cvfAssert.h"

#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMenu* RiuMenuBarBuildTools::createDefaultFileMenu( QMenuBar* menuBar )
{
    caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();
    CVF_ASSERT( menuBar && cmdFeatureMgr );

    QMenu* fileMenu = new RiuToolTipMenu( menuBar );
    fileMenu->setTitle( "&File" );

    menuBar->addMenu( fileMenu );

    fileMenu->addAction( cmdFeatureMgr->action( "RicOpenProjectFeature" ) );
    fileMenu->addAction( cmdFeatureMgr->action( "RicOpenLastUsedFileFeature" ) );

    return fileMenu;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMenu* RiuMenuBarBuildTools::createDefaultEditMenu( QMenuBar* menuBar )
{
    caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();
    CVF_ASSERT( menuBar && cmdFeatureMgr );

    QMenu* editMenu = menuBar->addMenu( "&Edit" );
    editMenu->addAction( cmdFeatureMgr->action( "RicSnapshotViewToClipboardFeature" ) );
    editMenu->addSeparator();
    editMenu->addAction( cmdFeatureMgr->action( "RicShowMemoryCleanupDialogFeature" ) );
    editMenu->addSeparator();
    editMenu->addAction( cmdFeatureMgr->action( "RicEditPreferencesFeature" ) );

    return editMenu;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMenu* RiuMenuBarBuildTools::createDefaultViewMenu( QMenuBar* menuBar )
{
    caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();
    CVF_ASSERT( menuBar && cmdFeatureMgr );

    QMenu* viewMenu = menuBar->addMenu( "&View" );
    viewMenu->addAction( cmdFeatureMgr->action( "RicViewZoomAllFeature" ) );

    return viewMenu;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMenu* RiuMenuBarBuildTools::createDefaultHelpMenu( QMenuBar* menuBar )
{
    caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();
    CVF_ASSERT( menuBar && cmdFeatureMgr );

    QMenu* helpMenu = menuBar->addMenu( "&Help" );
    helpMenu->addAction( cmdFeatureMgr->action( "RicHelpAboutFeature" ) );
    helpMenu->addAction( cmdFeatureMgr->action( "RicHelpCommandLineFeature" ) );
    helpMenu->addAction( cmdFeatureMgr->action( "RicHelpSummaryCommandLineFeature" ) );
    helpMenu->addSeparator();
    helpMenu->addAction( cmdFeatureMgr->action( "RicHelpOpenUsersGuideFeature" ) );
    helpMenu->addAction( cmdFeatureMgr->action( "RicSearchHelpFeature" ) );
    helpMenu->addAction( cmdFeatureMgr->action( "RicSearchIssuesHelpFeature" ) );
    helpMenu->addAction( cmdFeatureMgr->action( "RicCreateNewIssueHelpFeature" ) );

    return helpMenu;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMenuBarBuildTools::addImportMenuWithActions( QObject* parent, QMenu* menu )
{
    caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();

    if ( !parent || !menu || !cmdFeatureMgr ) return;

    QMenu* importMenu = menu->addMenu( "&Import" );

    QMenu* importEclipseMenu = importMenu->addMenu( QIcon( ":/Case48x48.png" ), "Eclipse Cases" );
    caf::CmdFeatureMenuBuilder::appendToMenu( importEclipseMenu, RimEclipseCaseCollection::importMenuFeatureNames() );

    QMenu* importRoffMenu = importMenu->addMenu( QIcon( ":/Case48x48.png" ), "Roff Grid Models" );
    importRoffMenu->addAction( cmdFeatureMgr->action( "RicImportRoffCaseFeature" ) );

    importMenu->addSeparator();
    QMenu* importSummaryMenu = importMenu->addMenu( QIcon( ":/SummaryCase.svg" ), "Summary Cases" );
    importSummaryMenu->addAction( cmdFeatureMgr->action( "RicImportSummaryCaseFeature" ) );
    importSummaryMenu->addAction( cmdFeatureMgr->action( "RicImportSummaryCasesFeature" ) );
    importSummaryMenu->addAction( cmdFeatureMgr->action( "RicImportSummaryGroupFeature" ) );
    importSummaryMenu->addAction( cmdFeatureMgr->action( "RicImportEnsembleFeature" ) );

    importMenu->addSeparator();
    QMenu* importGeoMechMenu = importMenu->addMenu( QIcon( ":/GeoMechCase24x24.png" ), "Geo Mechanical Cases" );
    importGeoMechMenu->addAction( cmdFeatureMgr->action( "RicImportGeoMechCaseFeature" ) );
    importGeoMechMenu->addAction( cmdFeatureMgr->action( "RicImportGeoMechCaseTimeStepFilterFeature" ) );
    importGeoMechMenu->addAction( cmdFeatureMgr->action( "RicImportElementPropertyFeature" ) );

    importMenu->addSeparator();
    QMenu* importWellMenu = importMenu->addMenu( QIcon( ":/Well.svg" ), "Well Data" );
    importWellMenu->addAction( cmdFeatureMgr->action( "RicWellPathsImportFileFeature" ) );
    importWellMenu->addAction( cmdFeatureMgr->action( "RicWellPathsImportOsduFeature" ) );
    importWellMenu->addAction( cmdFeatureMgr->action( "RicWellLogsImportFileFeature" ) );
    importWellMenu->addAction( cmdFeatureMgr->action( "RicWellPathFormationsImportFileFeature" ) );
    importWellMenu->addAction( cmdFeatureMgr->action( "RicImportWellMeasurementsFeature" ) );

    importMenu->addSeparator();
    importMenu->addAction( cmdFeatureMgr->action( "RicImportObservedDataFeature" ) );
    importMenu->addAction( cmdFeatureMgr->action( "RicImportObservedFmuDataFeature" ) );
    importMenu->addAction( cmdFeatureMgr->action( "RicImportPressureDepthDataFeature" ) );
    importMenu->addAction( cmdFeatureMgr->action( "RicImportFormationNamesFeature" ) );
    importMenu->addAction( cmdFeatureMgr->action( "RicImportSurfacesFeature" ) );
    importMenu->addAction( cmdFeatureMgr->action( "RicImportSeismicFeature" ) );

    RiuTools::enableAllActionsOnShow( parent, importMenu );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMenuBarBuildTools::addSaveProjectActions( QMenu* menu )
{
    caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();

    if ( !menu || !cmdFeatureMgr ) return;

    menu->addAction( cmdFeatureMgr->action( "RicSaveProjectFeature" ) );
    menu->addAction( cmdFeatureMgr->action( "RicSaveProjectAsFeature" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMenuBarBuildTools::addCloseAndExitActions( QMenu* menu )
{
    caf::CmdFeatureManager* cmdFeatureMgr = caf::CmdFeatureManager::instance();

    if ( !menu || !cmdFeatureMgr ) return;

    menu->addAction( cmdFeatureMgr->action( "RicCloseProjectFeature" ) );
    menu->addSeparator();
    menu->addAction( cmdFeatureMgr->action( "RicExitApplicationFeature" ) );
}
