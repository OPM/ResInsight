/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicExportToLasFileFeature.h"

#include "RicExportFeatureImpl.h"
#include "RicExportToLasFileResampleUi.h"
#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"

#include "RiaApplication.h"

#include "RigLasFileExporter.h"
#include "RigWellLogCurveData.h"

#include "RimWellLogCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicExportToLasFileFeature, "RicExportToLasFileFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RicExportToLasFileFeature::exportToLasFiles( const QString&        exportFolder,
                                                                  const QString&        exportPrefix,
                                                                  const RimWellLogPlot* plotWindow,
                                                                  bool                  exportTvdRkb,
                                                                  bool                  capitalizeFileNames,
                                                                  bool                  alwaysOverwrite,
                                                                  double                resampleInterval,
                                                                  bool                  convertCurveUnits )
{
    std::vector<RimWellLogCurve*> allCurves;
    std::vector<RimQwtPlot*>      plots = plotWindow->visiblePlots();

    for ( RimQwtPlot* plot : plots )
    {
        RimWellLogTrack* track = dynamic_cast<RimWellLogTrack*>( plot );
        if ( track )
        {
            std::vector<RimWellLogCurve*> curves = track->visibleCurves();
            allCurves.insert( allCurves.end(), curves.begin(), curves.end() );
        }
    }

    std::vector<QString> wellNames;
    std::vector<double>  rkbDiffs;
    {
        RigLasFileExporter lasExporter( allCurves );
        lasExporter.wellPathsAndRkbDiff( &wellNames, &rkbDiffs );

        return exportToLasFiles( exportFolder,
                                 exportPrefix,
                                 allCurves,
                                 wellNames,
                                 rkbDiffs,
                                 capitalizeFileNames,
                                 alwaysOverwrite,
                                 resampleInterval,
                                 convertCurveUnits );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RicExportToLasFileFeature::exportToLasFiles( const QString&                exportFolder,
                                                                  const QString&                filePrefix,
                                                                  std::vector<RimWellLogCurve*> curves,
                                                                  const std::vector<QString>&   wellNames,
                                                                  const std::vector<double>&    rkbDiffs,
                                                                  bool                          capitalizeFileNames,
                                                                  bool                          alwaysOverwrite,
                                                                  double                        resampleInterval,
                                                                  bool                          convertCurveUnits )
{
    RigLasFileExporter lasExporter( curves );

    std::vector<QString> writtenFiles;

    if ( resampleInterval > 0.0 )
    {
        lasExporter.setResamplingInterval( resampleInterval );
    }

    if ( !rkbDiffs.empty() )
    {
        lasExporter.setRkbDiffs( wellNames, rkbDiffs );
    }

    writtenFiles =
        lasExporter.writeToFolder( exportFolder, filePrefix, capitalizeFileNames, alwaysOverwrite, convertCurveUnits );

    // Remember the path to next time
    RiaApplication::instance()->setLastUsedDialogDirectory( "WELL_LOGS_DIR", exportFolder );
    return writtenFiles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportToLasFileFeature::isCommandEnabled()
{
    if ( RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot() ) return false;
    if ( RicWellLogPlotCurveFeatureImpl::parentWellRftPlot() ) return false;

    return RicWellLogPlotCurveFeatureImpl::selectedWellLogCurves().size() > 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportToLasFileFeature::onActionTriggered( bool isChecked )
{
    this->disableModelChangeContribution();

    if ( RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot() ) return;

    std::vector<RimWellLogCurve*> curves = RicWellLogPlotCurveFeatureImpl::selectedWellLogCurves();
    if ( curves.size() == 0 ) return;

    QString defaultDir =
        RiaApplication::instance()->lastUsedDialogDirectoryWithFallbackToProjectFolder( "WELL_LOGS_DIR" );

    RigLasFileExporter           lasExporter( curves );
    RicExportToLasFileResampleUi featureUi;
    featureUi.exportFolder = defaultDir;

    if ( RicWellLogPlotCurveFeatureImpl::parentWellBoreStabilityPlot() )
    {
        featureUi.filePrefix         = "WBS_";
        featureUi.capitalizeFileName = true;
        featureUi.exportTvdrkb       = true;
        featureUi.setUnitConversionOptionEnabled( true );
    }

    {
        std::vector<QString> wellNames;
        std::vector<double>  rkbDiffs;
        lasExporter.wellPathsAndRkbDiff( &wellNames, &rkbDiffs );
        featureUi.setRkbDiffs( wellNames, rkbDiffs );
    }

    caf::PdmUiPropertyViewDialog propertyDialog( nullptr,
                                                 &featureUi,
                                                 "Export Curve Data to LAS file(s)",
                                                 "",
                                                 QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
    RicExportFeatureImpl::configureForExport( propertyDialog.dialogButtonBox() );
    propertyDialog.resize( QSize( 400, 360 ) );

    if ( propertyDialog.exec() == QDialog::Accepted && !featureUi.exportFolder().isEmpty() )
    {
        double resampleInterval = 0.0;
        if ( featureUi.activateResample )
        {
            resampleInterval = featureUi.resampleInterval();
        }

        std::vector<QString> wellNames;
        std::vector<double>  rkbDiffs;
        if ( featureUi.exportTvdrkb )
        {
            lasExporter.wellPathsAndRkbDiff( &wellNames, &rkbDiffs );
            std::vector<double> userDefRkbDiff;
            featureUi.tvdrkbDiffForWellPaths( &userDefRkbDiff );
            rkbDiffs = userDefRkbDiff;
        }

        exportToLasFiles( featureUi.exportFolder(),
                          featureUi.filePrefix(),
                          curves,
                          wellNames,
                          rkbDiffs,
                          featureUi.capitalizeFileName,
                          false,
                          resampleInterval,
                          featureUi.curveUnitConversion() ==
                              RicExportToLasFileResampleUi::CurveUnitConversion::EXPORT_WITH_STANDARD_UNITS );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportToLasFileFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Export To LAS Files..." );
}
