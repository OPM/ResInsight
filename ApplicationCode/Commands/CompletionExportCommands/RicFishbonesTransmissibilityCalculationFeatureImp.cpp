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

#include "RimFishboneWellPath.h"
#include "RimFishboneWellPathCollection.h"
#include "RimFishbonesCollection.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimWellPath.h"
#include "RimWellPathCompletions.h"
#include "RigWellLogExtractor.h"

//==================================================================================================
/// 
//==================================================================================================
struct WellBorePartForTransCalc
{
    WellBorePartForTransCalc(cvf::Vec3d lengthsInCell, double wellRadius, double skinFactor, bool isMainBore, QString metaData)
        : lengthsInCell(lengthsInCell), wellRadius(wellRadius), skinFactor(skinFactor), isMainBore(isMainBore), metaData(metaData)
    {
        intersectionWithWellMeasuredDepth = HUGE_VAL;
        lateralIndex = cvf::UNDEFINED_SIZE_T;
    }

    cvf::Vec3d lengthsInCell;
    double     wellRadius;
    double     skinFactor;
    QString    metaData;
    bool       isMainBore;

    double      intersectionWithWellMeasuredDepth;
    size_t      lateralIndex;
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFishbonesTransmissibilityCalculationFeatureImp::findFishboneLateralsWellBoreParts(std::map<size_t, std::vector<WellBorePartForTransCalc> >& wellBorePartsInCells, 
                                                                                          const RimWellPath* wellPath, 
                                                                                          const RicExportCompletionDataSettingsUi& settings)
{
    // Generate data
    const RigEclipseCaseData* caseData = settings.caseToApply()->eclipseCaseData();
    std::vector<WellSegmentLocation> locations = RicWellPathExportCompletionDataFeatureImpl::findWellSegmentLocations(settings.caseToApply, wellPath);

    RiaEclipseUnitTools::UnitSystem unitSystem = caseData->unitsType();
    bool isMainBore = false;

    for (const WellSegmentLocation& location : locations)
    {
        for (const WellSegmentLateral& lateral : location.laterals)
        {
            for (const WellSegmentLateralIntersection& intersection : lateral.intersections)
            {
                double diameter = location.fishbonesSubs->holeDiameter(unitSystem);
                QString completionMetaData = (location.fishbonesSubs->generatedName() + QString(": Sub: %1 Lateral: %2").arg(location.subIndex).arg(lateral.lateralIndex));
                WellBorePartForTransCalc wellBorePart = WellBorePartForTransCalc(intersection.lengthsInCell, 
                                                                                 diameter / 2, 
                                                                                 location.fishbonesSubs->skinFactor(), 
                                                                                 isMainBore,
                                                                                 completionMetaData);

                wellBorePart.intersectionWithWellMeasuredDepth = location.measuredDepth;
                wellBorePart.lateralIndex = lateral.lateralIndex;

                wellBorePartsInCells[intersection.globalCellIndex].push_back(wellBorePart);

            }
        }
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicFishbonesTransmissibilityCalculationFeatureImp::generateFishboneCompdatValuesUsingAdjustedCellVolume(const RimWellPath* wellPath, 
                                                                                                                                               const RicExportCompletionDataSettingsUi& settings)
{
    std::map<size_t, std::vector<WellBorePartForTransCalc> > wellBorePartsInCells; //wellBore = main bore or fishbone lateral
    findFishboneLateralsWellBoreParts(wellBorePartsInCells, wellPath, settings);
    findFishboneImportedLateralsWellBoreParts(wellBorePartsInCells, wellPath, settings);
    if (!wellBorePartsInCells.empty())
    {
        findMainWellBoreParts(wellBorePartsInCells, wellPath, settings);
    }

    std::vector<RigCompletionData> completionData;

    const RigActiveCellInfo* activeCellInfo = settings.caseToApply->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL);

    for (const auto& cellAndWellBoreParts : wellBorePartsInCells)
    {
        size_t globalCellIndex = cellAndWellBoreParts.first;
        const std::vector<WellBorePartForTransCalc>& wellBoreParts = cellAndWellBoreParts.second;

        bool cellIsActive = activeCellInfo->isActive(globalCellIndex);
        if (!cellIsActive) continue;

        // Find main bore and number of laterals

        size_t numberOfLaterals = 0;
        CellDirection mainBoreDirection = DIR_I;
        for (const auto& wellBorePart : wellBoreParts)
        {
            if (!wellBorePart.isMainBore)
            {
                numberOfLaterals++;
            }
            else
            {
                mainBoreDirection = RicWellPathExportCompletionDataFeatureImpl::calculateDirectionInCell(settings.caseToApply,
                                                                                                     globalCellIndex,
                                                                                                     wellBorePart.lengthsInCell);
            }
        }
        
        for (WellBorePartForTransCalc wellBorePart : wellBoreParts)
        {
            RigCompletionData completion(wellPath->completions()->wellNameForExport(), RigCompletionDataGridCell(globalCellIndex, settings.caseToApply->mainGrid()), wellBorePart.intersectionWithWellMeasuredDepth);
            completion.setSecondOrderingValue(wellBorePart.lateralIndex);

            double transmissibility = 0.0;
            if (wellBorePart.isMainBore)
            {
                //No change in transmissibility for main bore
                transmissibility = RicWellPathExportCompletionDataFeatureImpl::calculateTransmissibility(settings.caseToApply,
                                                                                                            wellPath,
                                                                                                            wellBorePart.lengthsInCell,
                                                                                                            wellBorePart.skinFactor,
                                                                                                            wellBorePart.wellRadius,
                                                                                                            globalCellIndex,
                                                                                                            settings.useLateralNTG);

            }
            else
            {
                //Adjust transmissibility for fishbone laterals
                transmissibility = RicWellPathExportCompletionDataFeatureImpl::calculateTransmissibility(settings.caseToApply,
                                                                                                     wellPath,
                                                                                                     wellBorePart.lengthsInCell,
                                                                                                     wellBorePart.skinFactor,
                                                                                                     wellBorePart.wellRadius,
                                                                                                     globalCellIndex,
                                                                                                     settings.useLateralNTG,
                                                                                                     numberOfLaterals,
                                                                                                     mainBoreDirection);

            }

            CellDirection direction = RicWellPathExportCompletionDataFeatureImpl::calculateDirectionInCell(settings.caseToApply, 
                                                                                                       globalCellIndex, 
                                                                                                       wellBorePart.lengthsInCell);

            completion.setTransAndWPImultBackgroundDataFromFishbone(transmissibility,  
                                                                    wellBorePart.skinFactor, 
                                                                    wellBorePart.wellRadius *2, 
                                                                    direction,
                                                                    wellBorePart.isMainBore);

            completion.addMetadata(wellBorePart.metaData, QString::number(transmissibility));
            
            completionData.push_back(completion);
        }
    }
    return completionData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFishbonesTransmissibilityCalculationFeatureImp::findFishboneImportedLateralsWellBoreParts(std::map<size_t, std::vector<WellBorePartForTransCalc> >& wellBorePartsInCells, 
                                                                                                  const RimWellPath* wellPath,
                                                                                                  const RicExportCompletionDataSettingsUi& settings)
{
    RiaEclipseUnitTools::UnitSystem unitSystem = settings.caseToApply->eclipseCaseData()->unitsType();
    std::set<size_t> wellPathCells = RicFishbonesTransmissibilityCalculationFeatureImp::findIntersectedCells(settings.caseToApply()->eclipseCaseData(), 
                                                                                                   wellPath->wellPathGeometry()->m_wellPathPoints);
    bool isMainBore = false;

    double diameter = wellPath->fishbonesCollection()->wellPathCollection()->holeDiameter(unitSystem);
    for (const RimFishboneWellPath* fishbonesPath : wellPath->fishbonesCollection()->wellPathCollection()->wellPaths())
    {
        std::vector<WellPathCellIntersectionInfo> intersectedCells = RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath(settings.caseToApply->eclipseCaseData(), 
                                                                                                                              fishbonesPath->coordinates(),
                                                                                                                              fishbonesPath->measuredDepths());
        for (auto& cell : intersectedCells)
        {
            if (wellPathCells.count(cell.globCellIndex) ) continue;

            double skinFactor = wellPath->fishbonesCollection()->wellPathCollection()->skinFactor();
            QString completionMetaData = fishbonesPath->name();
            WellBorePartForTransCalc wellBorePart = WellBorePartForTransCalc(cell.intersectionLengthsInCellCS, 
                                                                             diameter / 2, 
                                                                             skinFactor, 
                                                                             isMainBore,
                                                                             completionMetaData);
            wellBorePart.intersectionWithWellMeasuredDepth = cell.startMD;

            wellBorePartsInCells[cell.globCellIndex].push_back(wellBorePart); 
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicFishbonesTransmissibilityCalculationFeatureImp::findMainWellBoreParts(std::map<size_t, std::vector<WellBorePartForTransCalc>>& wellBorePartsInCells,
                                                                              const RimWellPath* wellPath,
                                                                              const RicExportCompletionDataSettingsUi& settings)
{
    RiaEclipseUnitTools::UnitSystem unitSystem = settings.caseToApply->eclipseCaseData()->unitsType();
    bool isMainBore = true;
    double holeDiameter = wellPath->fishbonesCollection()->mainBoreDiameter(unitSystem);

    std::vector<double> wellPathMD = wellPath->wellPathGeometry()->m_measuredDepths;
    double wellPathEndMD = 0.0;
    if (wellPathMD.size() > 1) wellPathEndMD = wellPathMD.back();

    std::pair< std::vector<cvf::Vec3d>, std::vector<double> > fishbonePerfWellPathCoords = wellPath->wellPathGeometry()->clippedPointSubset(wellPath->fishbonesCollection()->startMD(),
                                                                                                                                            wellPathEndMD);

    std::vector<WellPathCellIntersectionInfo> intersectedCellsIntersectionInfo = RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath(settings.caseToApply->eclipseCaseData(),
                                                                                                                                          fishbonePerfWellPathCoords.first,
                                                                                                                                          fishbonePerfWellPathCoords.second);

    for (auto& cell : intersectedCellsIntersectionInfo)
    {
        double skinFactor = wellPath->fishbonesCollection()->mainBoreSkinFactor();
        QString completionMetaData = wellPath->name() + " main bore";
        WellBorePartForTransCalc wellBorePart = WellBorePartForTransCalc(cell.intersectionLengthsInCellCS,
                                                                         holeDiameter / 2,
                                                                         skinFactor,
                                                                         isMainBore,
                                                                         completionMetaData);

        wellBorePart.intersectionWithWellMeasuredDepth = cell.startMD;

        wellBorePartsInCells[cell.globCellIndex].push_back(wellBorePart);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<size_t> RicFishbonesTransmissibilityCalculationFeatureImp::findIntersectedCells(const RigEclipseCaseData* caseData, const std::vector<cvf::Vec3d>& coords)
{
    std::set<size_t> cells;

    std::vector<HexIntersectionInfo> intersections = RigWellPathIntersectionTools::findRawHexCellIntersections(caseData->mainGrid(), coords);
    for (auto intersection : intersections)
    {
        cells.insert(intersection.m_hexIndex);
    }

    return cells;
}
