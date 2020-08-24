/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicImportFormationNamesFeature.h"

#include "RiaApplication.h"
#include "RigEclipseCaseData.h"
#include "RigFemPartResultsCollection.h"
#include "RigFormationNames.h"
#include "RigGeoMechCaseData.h"
#include "Rim3dView.h"
#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimFormationNames.h"
#include "RimFormationNamesCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechView.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include "RimColorLegend.h"
#include "RimColorLegendCollection.h"
#include "RimColorLegendItem.h"
#include "RimRegularLegendConfig.h"

#include "Riu3DMainWindowTools.h"
#include "RiuFileDialogTools.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT( RicImportFormationNamesFeature, "RicImportFormationNamesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFormationNames* RicImportFormationNamesFeature::importFormationFiles( const QStringList& fileNames )
{
    RimProject*                  proj        = RimProject::current();
    RimFormationNamesCollection* fomNameColl = proj->activeOilField()->formationNamesCollection();
    if ( !fomNameColl )
    {
        fomNameColl                                      = new RimFormationNamesCollection;
        proj->activeOilField()->formationNamesCollection = fomNameColl;
    }

    // For each file, find existing Formation names item, or create new
    std::vector<RimFormationNames*> formationNames = fomNameColl->importFiles( fileNames );
    fomNameColl->updateConnectedEditors();

    for ( int i = 0; i < fileNames.size(); i++ )
    {
        auto colors = formationNames[i]->formationNamesData()->formationColors();

        bool anyValidColor = false;
        for ( const auto& color : colors )
        {
            if ( color.isValid() )
            {
                anyValidColor = true;
                break;
            }
        }

        if ( anyValidColor )
        {
            QString baseName = QFileInfo( fileNames[i] ).baseName();
            RicImportFormationNamesFeature::addCustomColorLegend( baseName, formationNames[i] );
        }
    }

    return formationNames.back();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportFormationNamesFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// If only one formation file is imported, the formation will automatically be set in the active
/// view�s case. Import of LYR files with colors create custom color legends according to color
/// definition on each file. However, color legend must be set by the user.
//--------------------------------------------------------------------------------------------------
void RicImportFormationNamesFeature::onActionTriggered( bool isChecked )
{
    RiaApplication* app        = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory( "BINARY_GRID" );

    QString filterText =
        QString( "Formation Names description File (*.lyr);;FMU Layer Zone Table(%1);;All Files (*.*)" )
            .arg( RimFormationNames::layerZoneTableFileName() );

    QStringList fileNames = RiuFileDialogTools::getOpenFileNames( Riu3DMainWindowTools::mainWindowWidget(),
                                                                  "Import Formation Names",
                                                                  defaultDir,
                                                                  filterText );

    if ( fileNames.isEmpty() ) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory( "BINARY_GRID", QFileInfo( fileNames.last() ).absolutePath() );

    // Find or create the FormationNamesCollection
    RimFormationNames* formationNames = importFormationFiles( fileNames );

    // If we have more than 1 formation file, do not modify selected formation for the case in active view.
    if ( fileNames.size() > 1 ) return;

    if ( formationNames )
    {
        RimProject* proj = RimProject::current();

        std::vector<RimCase*> cases;
        proj->allCases( cases );

        // Legend name is base name of the one formation file, c.f. RicImportFormationNamesFeature::importFormationFiles()
        QString legendName = QFileInfo( fileNames.last() ).baseName();

        if ( !cases.empty() )
        {
            Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
            if ( activeView )
            {
                RimCase* ownerCase = activeView->ownerCase();
                if ( ownerCase )
                {
                    ownerCase->setFormationNames( formationNames );
                    ownerCase->updateFormationNamesData();

                    setFormationCellResultAndLegend( activeView, legendName );
                    ownerCase->updateConnectedEditors();
                }
            }
        }

        if ( formationNames )
        {
            Riu3DMainWindowTools::selectAsCurrentItem( formationNames );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportFormationNamesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/FormationCollection16x16.png" ) );
    actionToSetup->setText( "Import Formations" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportFormationNamesFeature::addCustomColorLegend( QString& name, RimFormationNames* rimFormationNames )
{
    RigFormationNames* rigFormationNames = rimFormationNames->formationNamesData();
    if ( !rigFormationNames ) return;

    const std::vector<QString>&      formationNames  = rigFormationNames->formationNames();
    const std::vector<cvf::Color3f>& formationColors = rigFormationNames->formationColors();

    // return if no formation names or colors (latter e.g. in case of FMU input or LYR without colors)
    if ( formationNames.empty() || formationColors.empty() ) return;

    RimColorLegend* colorLegend = new RimColorLegend;
    colorLegend->setColorLegendName( name );

    for ( size_t i = 0; i < formationColors.size(); i++ )
    {
        RimColorLegendItem* colorLegendItem = new RimColorLegendItem;

        colorLegendItem->setValues( formationNames[i], (int)i, formationColors[i] );

        colorLegend->appendColorLegendItem( colorLegendItem );
    }

    RimProject* proj = RimProject::current();

    RimColorLegendCollection* colorLegendCollection = proj->colorLegendCollection;

    colorLegendCollection->appendCustomColorLegend( colorLegend );
    colorLegendCollection->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportFormationNamesFeature::setFormationCellResultAndLegend( Rim3dView* activeView, QString& legendName )
{
    RimRegularLegendConfig* legendConfig = nullptr;

    RimEclipseView* eclView = dynamic_cast<RimEclipseView*>( activeView );
    if ( eclView )
    {
        eclView->cellResult()->setResultType( RiaDefines::ResultCatType::FORMATION_NAMES );
        eclView->cellResult()->setResultVariable( RiaDefines::activeFormationNamesResultName() );

        legendConfig = eclView->cellResult()->legendConfig();

        eclView->cellResult()->updateUiFieldsFromActiveResult();
        eclView->cellResult()->loadDataAndUpdate();
        eclView->updateAllRequiredEditors();
        eclView->updateDisplayModelForCurrentTimeStepAndRedraw();
    }

    RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>( activeView );
    if ( geoMechView )
    {
        legendConfig = geoMechView->cellResult()->legendConfig();
    }

    if ( legendConfig )
    {
        RimColorLegendCollection* colorLegendCollection = RimProject::current()->colorLegendCollection;

        RimColorLegend* legend = colorLegendCollection->findByName( legendName );
        if ( legend )
        {
            legendConfig->setColorLegend( legend );
        }
    }
}
