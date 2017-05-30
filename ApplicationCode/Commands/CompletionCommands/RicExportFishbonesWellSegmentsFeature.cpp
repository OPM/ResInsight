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

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"
#include "cafPdmUiPropertyViewDialog.h"

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
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportFishbonesWellSegmentsFeature::generateWelsegsTable(RifEclipseDataTableFormatter& formatter, const RimWellPath* wellPath, const RicExportWellSegmentsSettingsUi& settings, const std::vector<WellSegmentLocation>& locations)
{
    formatter.keyword("WELSEGS");

    const WellSegmentLocation& firstLocation = locations[0];

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
        formatter.add(firstLocation.trueVerticalDepth);
        formatter.add(firstLocation.measuredDepth);
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
        WellSegmentLocation previousLocation = firstLocation;
        formatter.comment("Main stem");

        double depth = 0;
        double length = 0;

        for (size_t i = 0; i < locations.size(); ++i)
        {
            const WellSegmentLocation& location = locations[i];

            if (settings.lengthAndDepth() == RicExportWellSegmentsSettingsUi::INC)
            {
                depth = location.trueVerticalDepth - previousLocation.trueVerticalDepth;
                length = location.fishbonesSubs->locationOfSubs()[location.subIndex] - previousLocation.fishbonesSubs->locationOfSubs()[previousLocation.subIndex];
            }
            else
            {
                depth += location.trueVerticalDepth - previousLocation.trueVerticalDepth;
                length += location.fishbonesSubs->locationOfSubs()[location.subIndex] - previousLocation.fishbonesSubs->locationOfSubs()[previousLocation.subIndex];
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

            previousLocation = location;
        }
    }

    {
        formatter.comment("Laterals");
        formatter.comment("Diam: MSW - Tubing Radius");
        formatter.comment("Rough: MSW - Open Hole Roughness Factor");
        for (const WellSegmentLocation& location : locations)
        {
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
void RicExportFishbonesWellSegmentsFeature::generateCompsegsTable(RifEclipseDataTableFormatter& formatter, const RimWellPath* wellPath, const RicExportWellSegmentsSettingsUi& settings, const std::vector<WellSegmentLocation>& locations)
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
            double length = 0;
            for (const WellSegmentLateralIntersection& intersection : lateral.intersections)
            {
                length += intersection.length;

                size_t i, j, k;
                grid->ijkFromCellIndex(intersection.cellIndex, &i, &j, &k);
                
                formatter.addZeroBasedCellIndex(i).addZeroBasedCellIndex(j).addZeroBasedCellIndex(k);
                formatter.add(lateral.branchNumber);
                formatter.add(length);
                formatter.add("1*");
                switch (intersection.direction)
                {
                case POS_I:
                case NEG_I:
                    formatter.add("I");
                    break;
                case POS_J:
                case NEG_J:
                    formatter.add("J");
                    break;
                case POS_K:
                case NEG_K:
                    formatter.add("K");
                    break;
                }
                formatter.add(-1);
                formatter.rowCompleted();
            }
        }
    }

    formatter.tableCompleted();
}
