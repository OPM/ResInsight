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

#include "RicFishbonesTransmissibilityCalculationFeatureImp.h"

#include "RicExportCompletionDataSettingsUi.h"
#include "RicWellPathExportCompletionDataFeatureImpl.h"

#include "RigActiveCellInfo.h"
#include "RigCompletionData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigWellPath.h"
#include "RigWellPathIntersectionTools.h"

#include "RigWellLogExtractor.h"
#include "RimFishboneWellPath.h"
#include "RimFishboneWellPathCollection.h"
#include "RimFishbonesCollection.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimWellPath.h"
#include "RimWellPathCompletions.h"

//==================================================================================================
///
//==================================================================================================
struct WellBorePartForTransCalc
{
    WellBorePartForTransCalc(cvf::Vec3d lengthsInCell, double wellRadius, double skinFactor, bool isMainBore, QString metaData)
        : lengthsInCell(lengthsInCell)
        , wellRadius(wellRadius)
        , skinFactor(skinFactor)
        , isMainBore(isMainBore)
        , metaData(metaData)
        , intersectionWithWellMeasuredDepth(HUGE_VAL)
        , lateralIndex(cvf::UNDEFINED_SIZE_T)
    {
    }

    cvf::Vec3d lengthsInCell;
    double     wellRadius;
    double     skinFactor;
    QString    metaData;
    bool       isMainBore;

    double intersectionWithWellMeasuredDepth;
    size_t lateralIndex;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData>
    RicFishbonesTransmissibilityCalculationFeatureImp::generateFishboneCompdatValuesUsingAdjustedCellVolume(
        const RimWellPath*                       wellPath,
        const RicExportCompletionDataSettingsUi& settings)
{
    std::vector<RigCompletionData> completionData;

    if (!wellPath || !wellPath->completions())
    {
        return completionData;
    }

    std::map<size_t, std::vector<WellBorePartForTransCalc>> wellBorePartsInCells; // wellBore = main bore or fishbone lateral
    findFishboneLateralsWellBoreParts(wellBorePartsInCells, wellPath, settings);
    findFishboneImportedLateralsWellBoreParts(wellBorePartsInCells, wellPath, settings);

    const RigActiveCellInfo* activeCellInfo = settings.caseToApply->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL);

    for (const auto& cellAndWellBoreParts : wellBorePartsInCells)
    {
        size_t                                       globalCellIndex = cellAndWellBoreParts.first;
        const std::vector<WellBorePartForTransCalc>& wellBoreParts   = cellAndWellBoreParts.second;

        bool cellIsActive = activeCellInfo->isActive(globalCellIndex);
        if (!cellIsActive) continue;

        // Find main bore and number of laterals

        size_t        numberOfLaterals  = 0;
        CellDirection mainBoreDirection = DIR_I;
        for (const auto& wellBorePart : wellBoreParts)
        {
            if (!wellBorePart.isMainBore)
            {
                numberOfLaterals++;
            }
            else
            {
                mainBoreDirection = RicWellPathExportCompletionDataFeatureImpl::calculateDirectionInCell(
                    settings.caseToApply, globalCellIndex, wellBorePart.lengthsInCell);
            }
        }

        for (WellBorePartForTransCalc wellBorePart : wellBoreParts)
        {
            if (wellBorePart.isMainBore && settings.excludeMainBoreForFishbones())
            {
                continue;
            }

            RigCompletionData completion(wellPath->completions()->wellNameForExport(),
                                         RigCompletionDataGridCell(globalCellIndex, settings.caseToApply->mainGrid()),
                                         wellBorePart.intersectionWithWellMeasuredDepth);
            completion.setSecondOrderingValue(wellBorePart.lateralIndex);

            double transmissibility = 0.0;
            if (wellBorePart.isMainBore)
            {
                // No change in transmissibility for main bore
                transmissibility =
                    RicWellPathExportCompletionDataFeatureImpl::calculateTransmissibility(settings.caseToApply,
                                                                                          wellPath,
                                                                                          wellBorePart.lengthsInCell,
                                                                                          wellBorePart.skinFactor,
                                                                                          wellBorePart.wellRadius,
                                                                                          globalCellIndex,
                                                                                          settings.useLateralNTG);
            }
            else
            {
                // Adjust transmissibility for fishbone laterals
                transmissibility =
                    RicWellPathExportCompletionDataFeatureImpl::calculateTransmissibility(settings.caseToApply,
                                                                                          wellPath,
                                                                                          wellBorePart.lengthsInCell,
                                                                                          wellBorePart.skinFactor,
                                                                                          wellBorePart.wellRadius,
                                                                                          globalCellIndex,
                                                                                          settings.useLateralNTG,
                                                                                          numberOfLaterals,
                                                                                          mainBoreDirection);
            }

            CellDirection direction = RicWellPathExportCompletionDataFeatureImpl::calculateDirectionInCell(
                settings.caseToApply, globalCellIndex, wellBorePart.lengthsInCell);

            completion.setTransAndWPImultBackgroundDataFromFishbone(
                transmissibility, wellBorePart.skinFactor, wellBorePart.wellRadius * 2, direction, wellBorePart.isMainBore);

            completion.addMetadata(wellBorePart.metaData, QString::number(transmissibility));

            completionData.push_back(completion);
        }
    }
    return completionData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicFishbonesTransmissibilityCalculationFeatureImp::findFishboneLateralsWellBoreParts(
    std::map<size_t, std::vector<WellBorePartForTransCalc>>& wellBorePartsInCells,
    const RimWellPath*                                       wellPath,
    const RicExportCompletionDataSettingsUi&                 settings)
{
    if (!wellPath) return;

    // Generate data
    const RigEclipseCaseData* caseData = settings.caseToApply()->eclipseCaseData();
    RicMswExportInfo          exportInfo =
        RicWellPathExportCompletionDataFeatureImpl::generateFishbonesMswExportInfo(settings.caseToApply(), wellPath, false);

    RiaEclipseUnitTools::UnitSystem unitSystem = caseData->unitsType();
    bool                            isMainBore = false;

    for (const RicMswSegment& location : exportInfo.wellSegmentLocations())
    {
        for (const RicMswCompletion& completion : location.completions())
        {
            for (const RicMswSubSegment& segment : completion.subSegments())
            {
                for (const RicMswSubSegmentCellIntersection& intersection : segment.intersections())
                {
                    double  diameter = location.holeDiameter();
                    QString completionMetaData =
                        (location.label() + QString(": Sub: %1 Lateral: %2").arg(location.subIndex()).arg(completion.index()));

                    WellBorePartForTransCalc wellBorePart = WellBorePartForTransCalc(
                        intersection.lengthsInCell(), diameter / 2.0, location.skinFactor(), isMainBore, completionMetaData);

                    wellBorePart.intersectionWithWellMeasuredDepth = location.endMD();
                    wellBorePart.lateralIndex                      = completion.index();

                    wellBorePartsInCells[intersection.globalCellIndex()].push_back(wellBorePart);
                }
            }
        }
    }

    {
        // Note that it is not supported to export main bore perforation intervals for Imported Laterals, only for fishbones
        // defined by ResInsight. It is not trivial to define the open section of the main bore for imported laterals.

        if (wellPath->fishbonesCollection()->isChecked())
        {
            double holeRadius = wellPath->fishbonesCollection()->mainBoreDiameter(unitSystem) / 2.0;
            double skinFactor = wellPath->fishbonesCollection()->mainBoreSkinFactor();

            for (const auto& fishboneDefinition : wellPath->fishbonesCollection()->activeFishbonesSubs())
            {
                double startMD = fishboneDefinition->startOfSubMD();
                double endMD = fishboneDefinition->endOfSubMD();

                if (fabs(startMD - endMD) < 1e-3)
                {
                    // Start and end md are close, adjust to be sure we get an intersection along the well path
                    startMD -= 0.5;
                    endMD += 0.5;
                }
                
                appendMainWellBoreParts(wellBorePartsInCells,
                                        wellPath,
                                        settings,
                                        skinFactor,
                                        holeRadius,
                                        startMD,
                                        endMD);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicFishbonesTransmissibilityCalculationFeatureImp::findFishboneImportedLateralsWellBoreParts(
    std::map<size_t, std::vector<WellBorePartForTransCalc>>& wellBorePartsInCells,
    const RimWellPath*                                       wellPath,
    const RicExportCompletionDataSettingsUi&                 settings)
{
    RiaEclipseUnitTools::UnitSystem unitSystem = settings.caseToApply->eclipseCaseData()->unitsType();

    if (!wellPath) return;
    if (!wellPath->wellPathGeometry()) return;

    bool   isMainBore = false;
    double holeRadius = wellPath->fishbonesCollection()->wellPathCollection()->holeDiameter(unitSystem) / 2.0;
    double skinFactor = wellPath->fishbonesCollection()->wellPathCollection()->skinFactor();

    for (const RimFishboneWellPath* fishbonesPath : wellPath->fishbonesCollection()->wellPathCollection()->wellPaths())
    {
        if (!fishbonesPath->isChecked())
        {
            continue;
        }

        std::vector<WellPathCellIntersectionInfo> intersectedCells =
            RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath(
                settings.caseToApply->eclipseCaseData(), fishbonesPath->coordinates(), fishbonesPath->measuredDepths());

        for (const auto& cellIntersectionInfo : intersectedCells)
        {
            QString                  completionMetaData = fishbonesPath->name();
            WellBorePartForTransCalc wellBorePart       = WellBorePartForTransCalc(
                cellIntersectionInfo.intersectionLengthsInCellCS, holeRadius, skinFactor, isMainBore, completionMetaData);
            wellBorePart.intersectionWithWellMeasuredDepth = cellIntersectionInfo.startMD;

            wellBorePartsInCells[cellIntersectionInfo.globCellIndex].push_back(wellBorePart);
        }
    }

    // Note that it is not supported to export main bore perforation intervals for Imported Laterals, only for fishbones
    // defined by ResInsight
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicFishbonesTransmissibilityCalculationFeatureImp::appendMainWellBoreParts(
    std::map<size_t, std::vector<WellBorePartForTransCalc>>& wellBorePartsInCells,
    const RimWellPath*                                       wellPath,
    const RicExportCompletionDataSettingsUi&                 settings,
    double                                                   skinFactor,
    double                                                   holeRadius,
    double                                                   startMeasuredDepth,
    double                                                   endMeasuredDepth)
{
    if (!wellPath) return;
    if (!wellPath->wellPathGeometry()) return;
    if (!wellPath->fishbonesCollection()) return;

    bool isMainBore = true;

    std::pair<std::vector<cvf::Vec3d>, std::vector<double>> fishbonePerfWellPathCoords =
        wellPath->wellPathGeometry()->clippedPointSubset(startMeasuredDepth, endMeasuredDepth);

    std::vector<WellPathCellIntersectionInfo> intersectedCellsIntersectionInfo =
        RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath(
            settings.caseToApply->eclipseCaseData(), fishbonePerfWellPathCoords.first, fishbonePerfWellPathCoords.second);

    for (const auto& cellIntersectionInfo : intersectedCellsIntersectionInfo)
    {
        QString                  completionMetaData = wellPath->name() + " main bore";
        WellBorePartForTransCalc wellBorePart       = WellBorePartForTransCalc(
            cellIntersectionInfo.intersectionLengthsInCellCS, holeRadius, skinFactor, isMainBore, completionMetaData);

        wellBorePart.intersectionWithWellMeasuredDepth = cellIntersectionInfo.startMD;

        wellBorePartsInCells[cellIntersectionInfo.globCellIndex].push_back(wellBorePart);
    }
}
