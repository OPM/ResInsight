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

#include "RicExportFishbonesWellSegmentsFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimProject.h"
#include "RimFishboneWellPathCollection.h"
#include "RimFishbonesCollection.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimWellPath.h"
#include "RimEclipseCase.h"

#include "RigMainGrid.h"
#include "RigEclipseCaseData.h"
#include "RigWellPath.h"

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"
#include "cafPdmUiPropertyViewDialog.h"

#include "cvfMath.h"

#include <QAction>
#include <QMessageBox>
#include <QDir>


CAF_CMD_SOURCE_INIT(RicExportFishbonesWellSegmentsFeature, "RicExportFishbonesWellSegmentsFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportFishbonesWellSegmentsFeature::onActionTriggered(bool isChecked)
{
    RimFishbonesCollection* fishbonesCollection = selectedFishbonesCollection();
    RimWellPath* wellPath = selectedWellPath();
    CVF_ASSERT(fishbonesCollection);
    CVF_ASSERT(wellPath);

    RiaApplication* app = RiaApplication::instance();

    QString projectFolder = app->currentProjectPath();
    QString defaultDir = RiaApplication::instance()->lastUsedDialogDirectoryWithFallback("COMPLETIONS", projectFolder);

    RicExportWellSegmentsSettingsUi exportSettings;
    std::vector<RimCase*> cases;
    app->project()->allCases(cases);
    for (auto c : cases)
    {
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(c);
        if (eclipseCase != nullptr)
        {
            exportSettings.caseToApply = eclipseCase;
            break;
        }
    }

    exportSettings.fileName = QDir(defaultDir).filePath("WellSegments");

    caf::PdmUiPropertyViewDialog propertyDialog(RiuMainWindow::instance(), &exportSettings, "Export Completion Data", "");
    if (propertyDialog.exec() == QDialog::Accepted)
    {
        RiaApplication::instance()->setLastUsedDialogDirectory("COMPLETIONS", QFileInfo(exportSettings.fileName).absolutePath());

        std::vector<RimFishbonesMultipleSubs*> fishbonesSubs;
        for (RimFishbonesMultipleSubs* subs : fishbonesCollection->fishbonesSubs)
        {
            fishbonesSubs.push_back(subs);
        }

        exportWellSegments(wellPath, fishbonesSubs, exportSettings);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFishbonesCollection* RicExportFishbonesWellSegmentsFeature::selectedFishbonesCollection()
{
    RimFishbonesCollection* objToFind = nullptr;
    
    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (objHandle)
    {
        objHandle->firstAncestorOrThisOfType(objToFind);
    }

    return objToFind;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RicExportFishbonesWellSegmentsFeature::selectedWellPath()
{
    RimWellPath* objToFind = nullptr;
    
    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (objHandle)
    {
        objHandle->firstAncestorOrThisOfType(objToFind);
    }

    return objToFind;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportFishbonesWellSegmentsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Well Segments");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicExportFishbonesWellSegmentsFeature::isCommandEnabled()
{
    if (selectedFishbonesCollection())
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportFishbonesWellSegmentsFeature::exportWellSegments(const RimWellPath* wellPath, const std::vector<RimFishbonesMultipleSubs*>& fishbonesSubs, const RicExportWellSegmentsSettingsUi& settings)
{
    QFile exportFile(settings.fileName());

    if (settings.caseToApply() == nullptr)
    {
        RiaLogging::error("Export Well Segments: Cannot export completions data without specified eclipse case");
        return;
    }

    if (!exportFile.open(QIODevice::WriteOnly))
    {
        RiaLogging::error(QString("Export Well Segments: Could not open the file: %1").arg(settings.fileName()));
        return;
    }

    std::vector<WellSegmentLocation> locations = RicWellPathExportCompletionDataFeature::findWellSegmentLocations(settings.caseToApply, wellPath, fishbonesSubs);

    QTextStream stream(&exportFile);
    RifEclipseDataTableFormatter formatter(stream);
    generateWelsegsTable(formatter, wellPath, settings, locations);
    generateCompsegsTable(formatter, wellPath, settings, locations);
    generateWsegvalvTable(formatter, wellPath, settings, locations);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportFishbonesWellSegmentsFeature::generateWelsegsTable(RifEclipseDataTableFormatter& formatter,
                                                                 const RimWellPath* wellPath,
                                                                 const RicExportWellSegmentsSettingsUi& settings,
                                                                 const std::vector<WellSegmentLocation>& locations)
{
    formatter.keyword("WELSEGS");

    double startMD = wellPath->fishbonesCollection()->startMD();
    double startTVD = -wellPath->wellPathGeometry()->interpolatedPointAlongWellPath(startMD).z();

    {
        std::vector<RifEclipseOutputTableColumn> header = {
            RifEclipseOutputTableColumn("Name"),
            RifEclipseOutputTableColumn("Dep 1"),
            RifEclipseOutputTableColumn("Tlen 1"),
            RifEclipseOutputTableColumn("Vol 1"),
            RifEclipseOutputTableColumn("Len&Dep"),
            RifEclipseOutputTableColumn("PresDrop"),
        };
        formatter.header(header);

        formatter.add(wellPath->name());
        formatter.add(startTVD);
        formatter.add(startMD);
        formatter.add("1*");
        formatter.add(settings.lengthAndDepth().text()); 
        formatter.add(settings.pressureDrop().text());

        formatter.rowCompleted();
    }

    {
        std::vector<RifEclipseOutputTableColumn> header = {
            RifEclipseOutputTableColumn("First Seg"),
            RifEclipseOutputTableColumn("Last Seg"),
            RifEclipseOutputTableColumn("Branch Num"),
            RifEclipseOutputTableColumn("Outlet Seg"),
            RifEclipseOutputTableColumn("Length"),
            RifEclipseOutputTableColumn("Depth Change"),
            RifEclipseOutputTableColumn("Diam"),
            RifEclipseOutputTableColumn("Rough"),
        };
        formatter.header(header);
    }

    {
        formatter.comment("Main stem");

        double depth = 0;
        double length = 0;
        double previousMD = startMD;
        double previousTVD = startTVD;

        for (const WellSegmentLocation& location : locations)
        {
            if (settings.lengthAndDepth() == RicExportWellSegmentsSettingsUi::INC)
            {
                depth = location.trueVerticalDepth - previousTVD;
                length = location.fishbonesSubs->measuredDepth(location.subIndex) - previousMD;
            }
            else
            {
                depth += location.trueVerticalDepth - previousTVD;
                length += location.fishbonesSubs->measuredDepth(location.subIndex) - previousMD;
            }

            formatter.comment(QString("Segment for sub %1").arg(location.subIndex));
            formatter.add(location.segmentNumber).add(location.segmentNumber);
            formatter.add(1); // All segments on main stem are branch 1
            formatter.add(location.segmentNumber - 1); // All main stem segments are connected to the segment below them
            formatter.add(length);
            formatter.add(depth);
            formatter.add(-1.0); // FIXME : Diam of main stem?
            formatter.add(-1.0); // FIXME : Rough of main stem?
            formatter.rowCompleted();

            previousMD = location.measuredDepth;
            previousTVD = location.trueVerticalDepth;
        }
    }

    {
        formatter.comment("Laterals");
        formatter.comment("Diam: MSW - Tubing Radius");
        formatter.comment("Rough: MSW - Open Hole Roughness Factor");
        for (const WellSegmentLocation& location : locations)
        {
            formatter.comment("ICD");
            formatter.add(location.icdSegmentNumber).add(location.icdSegmentNumber);
            formatter.add(location.icdBranchNumber);
            formatter.add(location.segmentNumber);
            formatter.add(0.1); // ICDs have 0.1 length
            formatter.add(0); // Depth change
            formatter.add(-1.0); // Diam?
            formatter.add(-1.0); // Rough?
            formatter.rowCompleted();

            for (const WellSegmentLateral& lateral : location.laterals)
            {
                formatter.comment(QString("%1 : Sub index %2 - Lateral %3").arg(location.fishbonesSubs->name()).arg(location.subIndex).arg(lateral.lateralIndex));

                double depth = 0;
                double length = 0;

                for (const WellSegmentLateralIntersection& intersection : lateral.intersections)
                {
                    if (settings.lengthAndDepth() == RicExportWellSegmentsSettingsUi::INC)
                    {
                        depth = intersection.depth;
                        length = intersection.length;
                    }
                    else
                    {
                        depth += intersection.depth;
                        length += intersection.length;
                    }
                    formatter.add(intersection.segmentNumber);
                    formatter.add(intersection.segmentNumber);
                    formatter.add(lateral.branchNumber);
                    formatter.add(intersection.attachedSegmentNumber);
                    formatter.add(length);
                    formatter.add(depth);
                    formatter.add(location.fishbonesSubs->tubingDiameter());
                    formatter.add(location.fishbonesSubs->openHoleRoughnessFactor());
                    formatter.rowCompleted();
                }
            }
        }
    }

    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportFishbonesWellSegmentsFeature::generateCompsegsTable(RifEclipseDataTableFormatter& formatter,
                                                                  const RimWellPath* wellPath,
                                                                  const RicExportWellSegmentsSettingsUi& settings,
                                                                  const std::vector<WellSegmentLocation>& locations)
{
    RigMainGrid* grid = settings.caseToApply->eclipseCaseData()->mainGrid();
    formatter.keyword("COMPSEGS");
    {
        std::vector<RifEclipseOutputTableColumn> header = {
            RifEclipseOutputTableColumn("Name")
        };
        formatter.header(header);
        formatter.add(wellPath->name());
        formatter.rowCompleted();
    }

    {
        std::vector<RifEclipseOutputTableColumn> header = {
            RifEclipseOutputTableColumn("I"),
            RifEclipseOutputTableColumn("J"),
            RifEclipseOutputTableColumn("K"),
            RifEclipseOutputTableColumn("Branch no"),
            RifEclipseOutputTableColumn("Start Length"),
            RifEclipseOutputTableColumn("End Length"),
            RifEclipseOutputTableColumn("Dir Pen"),
            RifEclipseOutputTableColumn("End Range"),
            RifEclipseOutputTableColumn("Connection Depth")
        };
        formatter.header(header);
    }

    for (const WellSegmentLocation& location : locations)
    {
        for (const WellSegmentLateral& lateral : location.laterals)
        {
            double aggregatedLength = 0;
            for (const WellSegmentLateralIntersection& intersection : lateral.intersections)
            {
                size_t i, j, k;
                grid->ijkFromCellIndex(intersection.cellIndex, &i, &j, &k);
                
                formatter.addZeroBasedCellIndex(i).addZeroBasedCellIndex(j).addZeroBasedCellIndex(k);
                formatter.add(lateral.branchNumber);
                formatter.add(aggregatedLength);
                formatter.add(aggregatedLength + intersection.length);
                formatter.rowCompleted();

                aggregatedLength += intersection.length;
            }
        }
    }

    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportFishbonesWellSegmentsFeature::generateWsegvalvTable(RifEclipseDataTableFormatter& formatter,
                                                                  const RimWellPath* wellPath,
                                                                  const RicExportWellSegmentsSettingsUi& settings,
                                                                  const std::vector<WellSegmentLocation>& locations)
{
    {
        formatter.keyword("WSEGVALV");
        std::vector<RifEclipseOutputTableColumn> header = {
            RifEclipseOutputTableColumn("Well Name"),
            RifEclipseOutputTableColumn("Seg No"),
            RifEclipseOutputTableColumn("Cv"),
            RifEclipseOutputTableColumn("Ac"),
        };
        formatter.header(header);
    }
    for (const WellSegmentLocation& location : locations)
    {
        formatter.add(wellPath->name());
        formatter.add(location.icdSegmentNumber);
        formatter.add(location.fishbonesSubs->icdFlowCoefficient());

        double icdOrificeRadius = location.fishbonesSubs->icdOrificeDiameter() / 2;
        double icdArea = icdOrificeRadius * icdOrificeRadius * cvf::PI_D;
        formatter.add(icdArea * static_cast<double>(location.fishbonesSubs->icdCount()));
        formatter.rowCompleted();
    }
    formatter.tableCompleted();
}
