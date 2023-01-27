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

#include "RicCreateDepthAdjustedLasFilesFeature.h"

#include "RiaLogging.h"

#include "RicCreateDepthAdjustedLasFilesImpl.h"
#include "RicCreateDepthAdjustedLasFilesUi.h"
#include "RicExportFeatureImpl.h"

#include "RigEclipseWellLogExtractor.h"
#include "RigGeoMechWellLogExtractor.h"

#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellLogFile.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "cafPdmUiPropertyViewDialog.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCreateDepthAdjustedLasFilesFeature, "RicCreateDepthAdjustedLasFilesFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateDepthAdjustedLasFilesFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateDepthAdjustedLasFilesFeature::onActionTriggered( bool isChecked )
{
    RicCreateDepthAdjustedLasFilesUi featureUi;
    featureUi.setDefaultValues();

    caf::PdmUiPropertyViewDialog propertyDialog( nullptr, &featureUi, "Create Depth Adjusted LAS file(s)", "" );
    RicExportFeatureImpl::configureForExport( propertyDialog.dialogButtonBox() );
    propertyDialog.setWindowFlag( Qt::WindowCloseButtonHint, true );
    propertyDialog.resize( QSize( 850, 500 ) );

    if ( propertyDialog.exec() == QDialog::Accepted )
    {
        if ( !featureUi.hasValidSelections() )
        {
            RiaLogging::warning( featureUi.invalidSelectionsLogString() );
        }
        else
        {
            RimCase*                  selectedCase             = featureUi.selectedCase();
            RimWellPath*              sourceWell               = featureUi.sourceWell();
            RimWellLogFile*           sourceWellLogFile        = sourceWell->wellLogFiles()[0];
            std::vector<RimWellPath*> destinationWells         = featureUi.destinationWells().ptrReferencedObjects();
            std::vector<QString>      selectedResultProperties = featureUi.selectedResultProperties();
            QString                   exportFolder             = featureUi.exportFolder();

            RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( selectedCase );
            RimGeoMechCase* geomCase    = dynamic_cast<RimGeoMechCase*>( selectedCase );

            if ( eclipseCase == nullptr && geomCase == nullptr )
            {
                RiaLogging::warning( QString( "The selected case is invalid" ) );
                return;
            }

            if ( eclipseCase != nullptr )
            {
                createDepthAdjustedWellLogFileFromEclipseCase( eclipseCase, sourceWell, destinationWells, selectedResultProperties, exportFolder );
            }
            else if ( geomCase != nullptr )
            {
                createDepthAdjustedWellLogFileFromGeoMechCase( geomCase, sourceWell, destinationWells, selectedResultProperties, exportFolder );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateDepthAdjustedLasFilesFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create Depth Adjusted LAS Files..." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateDepthAdjustedLasFilesFeature::createDepthAdjustedWellLogFileFromEclipseCase( RimEclipseCase*                 eclipseCase,
                                                                                           RimWellPath*                    sourceWell,
                                                                                           const std::vector<RimWellPath*> destinationWells,
                                                                                           const std::vector<QString>& selectedResultProperties,
                                                                                           const QString&              exportFolder )
{
    if ( sourceWell->wellLogFiles().empty() ) return;

    RimWellLogPlotCollection*            wellLogCollection   = RimMainPlotCollection::current()->wellLogPlotCollection();
    cvf::ref<RigEclipseWellLogExtractor> sourceWellExtractor = wellLogCollection->findOrCreateExtractor( sourceWell, eclipseCase );
    if ( sourceWellExtractor.isNull() )
    {
        RiaLogging::info( QString( "Could not create RigEclipseWellLogExtractor for %1" ).arg( sourceWell->name() ) );
        return;
    }
    const double rkbDiff = sourceWellExtractor->wellPathGeometry()->rkbDiff();
    RicCreateDepthAdjustedLasFilesImpl::createDestinationWellsLasFiles( eclipseCase,
                                                                        sourceWell,
                                                                        destinationWells,
                                                                        selectedResultProperties,
                                                                        exportFolder,
                                                                        rkbDiff );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateDepthAdjustedLasFilesFeature::createDepthAdjustedWellLogFileFromGeoMechCase( RimGeoMechCase*                 geoMechCase,
                                                                                           RimWellPath*                    sourceWell,
                                                                                           const std::vector<RimWellPath*> destinationWells,
                                                                                           const std::vector<QString>& selectedResultProperties,
                                                                                           const QString&              exportFolder )
{
    if ( sourceWell->wellLogFiles().empty() ) return;

    auto*                                wellLogFileData     = sourceWell->wellLogFiles()[0]->wellLogFileData();
    RimWellLogPlotCollection*            wellLogCollection   = RimMainPlotCollection::current()->wellLogPlotCollection();
    cvf::ref<RigGeoMechWellLogExtractor> sourceWellExtractor = wellLogCollection->findOrCreateExtractor( sourceWell, geoMechCase );
    if ( sourceWellExtractor.isNull() )
    {
        RiaLogging::info( QString( "Could not create RigGeoMechWellLogExtractor for %1" ).arg( sourceWell->name() ) );
        return;
    }
    const double rkbDiff = sourceWellExtractor->wellPathGeometry()->rkbDiff();
    RicCreateDepthAdjustedLasFilesImpl::createDestinationWellsLasFiles( geoMechCase,
                                                                        sourceWell,
                                                                        destinationWells,
                                                                        selectedResultProperties,
                                                                        exportFolder,
                                                                        rkbDiff );
}
