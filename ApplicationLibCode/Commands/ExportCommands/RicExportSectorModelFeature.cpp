/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025   Equinor ASA
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

#include "RicExportSectorModelFeature.h"

#include "RiaDefines.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "RicExportEclipseSectorModelFeature.h"
#include "RicExportSectorModelUi.h"

#include "RigSimulationInputSettings.h"
#include "RigSimulationInputTool.h"

#include "Jobs/RimJobCollection.h"
#include "Jobs/RimKeywordBcprop.h"
#include "Jobs/RimOpmFlowJob.h"
#include "RimDialogData.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimProject.h"
#include "RimTools.h"
#include "Riu3DMainWindowTools.h"
#include "Tools/RimEclipseViewTools.h"

#include "RiuMainWindowBase.h"
#include "RiuPropertyViewWizard.h"

#include "opm/input/eclipse/Deck/DeckKeyword.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"

#include <QAction>
#include <QDir>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicExportSectorModelFeature, "RicExportSectorModelFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportSectorModelFeature::isCommandEnabled() const
{
    return RicExportEclipseSectorModelFeature::selectedView() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSectorModelFeature::onActionTriggered( bool isChecked )
{
    auto view = RicExportEclipseSectorModelFeature::selectedView();
    if ( view == nullptr ) return;

    auto eCase = view->eclipseCase();
    if ( ( eCase == nullptr ) || ( eCase->eclipseCaseData() == nullptr ) ) return;

    auto exportSettings = RimProject::current()->dialogData()->createExportSectorModelUi();
    exportSettings->setEclipseView( view );

    auto parent = RiaGuiApplication::activeMainWindow();

    RiuPropertyViewWizard wizard( parent,
                                  exportSettings,
                                  "Sector Model Export" + RiaDefines::betaFeaturePostfix(),
                                  exportSettings->pageNames(),
                                  exportSettings->pageSubTitles() );

    if ( wizard.exec() == QDialog::Accepted )
    {
        doExport( exportSettings, view );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSectorModelFeature::doExport( RicExportSectorModelUi* exportSettings, RimEclipseView* view )
{
    // Convert UI settings to RigSimulationInputSettings
    RigSimulationInputSettings settings;
    settings.setMin( exportSettings->min() );
    settings.setMax( exportSettings->max() );
    settings.setRefinement( exportSettings->refinement() );

    std::vector<Opm::DeckRecord> bcpropKeywords;
    for ( auto bcprop : exportSettings->bcpropKeywords() )
    {
        Opm::DeckKeyword kw     = bcprop->keyword();
        const auto&      record = kw.getRecord( 0 );
        bcpropKeywords.push_back( record );
    }
    settings.setBcpropKeywords( bcpropKeywords );
    settings.setBoundaryCondition( exportSettings->boundaryCondition() );
    settings.setPorvMultiplier( exportSettings->porvMultiplier() );

    // Get input deck file name from eclipse case
    QFileInfo fi( view->eclipseCase()->gridFileName() );
    QString   dataFileName = fi.absolutePath() + "/" + fi.completeBaseName() + ".DATA";
    settings.setInputDeckFileName( dataFileName );
    settings.setOutputDeckFileName( exportSettings->exportDeckFilename() );

    cvf::ref<cvf::UByteArray> cellVisibility = RimEclipseViewTools::createVisibilityBasedOnBoxSelection( view,
                                                                                                         exportSettings->gridBoxSelection(),
                                                                                                         exportSettings->min(),
                                                                                                         exportSettings->max(),
                                                                                                         exportSettings->wellPadding() );
    if ( auto result = RigSimulationInputTool::exportSimulationInput( *view->eclipseCase(), settings, cellVisibility.p() ); !result )
    {
        RiaLogging::error( QString( "Failed to export sector model to DATA file: %1" ).arg( result.error() ) );
    }
    else
    {
        RiaLogging::info( QString( "Successfully exported sector model to DATA file: %1" ).arg( exportSettings->exportDeckFilename() ) );
    }

    auto jobColl = RimTools::jobCollection();
    if ( exportSettings->shouldCreateSimulationJob() )
    {
        auto job = new RimOpmFlowJob();

        job->setInputDataFile( exportSettings->exportDeckFilename() );
        job->setName( exportSettings->newSimulationJobName() );
        job->setWorkingDirectory( exportSettings->newSimulationJobFolder() );

        QDir d;
        d.mkpath( exportSettings->newSimulationJobFolder() );

        jobColl->addNewJob( job );

        Riu3DMainWindowTools::selectAsCurrentItem( job );
    }

    if ( exportSettings->startSimulationJobAfterExport() )
    {
        for ( auto job : jobColl->jobsMatchingKeyValue( RimOpmFlowJob::jobInputFileKey(), exportSettings->exportDeckFilename() ) )
        {
            job->execute();
        }
        Riu3DMainWindowTools::selectAsCurrentItem( jobColl );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSectorModelFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Export Sector Model" + RiaDefines::betaFeaturePostfix() );
}
