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
#include "RicTransmissibilityCalculator.h"
#include "RicWellPathExportCompletionDataFeatureImpl.h"

#include "RigActiveCellInfo.h"
#include "RigCompletionData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "Well/RigWellLogExtractor.h"
#include "Well/RigWellPath.h"
#include "Well/RigWellPathIntersectionTools.h"

#include "RimFishbones.h"
#include "RimFishbonesCollection.h"
#include "RimMswCompletionParameters.h"
#include "RimWellPath.h"
#include "RimWellPathCompletions.h"

#include <cafPdmObject.h>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicFishbonesTransmissibilityCalculationFeatureImp::generateFishboneCompdatValuesUsingAdjustedCellVolume(
    const RimWellPath*                       wellPath,
    const RicExportCompletionDataSettingsUi& settings )
{
    std::vector<RigCompletionData> completionData;

    if ( !wellPath || !wellPath->completions() || !wellPath->wellPathGeometry() )
    {
        return completionData;
    }

    auto wellBorePartsInCells = findFishboneLateralsWellBoreParts( wellPath, settings.caseToApply() );

    const RigActiveCellInfo* activeCellInfo =
        settings.caseToApply->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    for ( const auto& cellAndWellBoreParts : wellBorePartsInCells )
    {
        size_t                                       globalCellIndex = cellAndWellBoreParts.first;
        const std::vector<WellBorePartForTransCalc>& wellBoreParts   = cellAndWellBoreParts.second;

        bool cellIsActive = activeCellInfo->isActive( globalCellIndex );
        if ( !cellIsActive ) continue;

        // Find main bore and number of laterals

        size_t numberOfLaterals  = 0;
        auto   mainBoreDirection = RigCompletionData::CellDirection::DIR_I;
        for ( const auto& wellBorePart : wellBoreParts )
        {
            if ( !wellBorePart.isMainBore )
            {
                numberOfLaterals++;
            }
            else
            {
                mainBoreDirection = RicTransmissibilityCalculator::calculateCellMainDirection( settings.caseToApply,
                                                                                               globalCellIndex,
                                                                                               wellBorePart.lengthsInCell );
            }
        }

        for ( WellBorePartForTransCalc wellBorePart : wellBoreParts )
        {
            if ( wellBorePart.isMainBore && settings.excludeMainBoreForFishbones() )
            {
                continue;
            }

            RigCompletionData completion( wellPath->completionSettings()->wellNameForExport(),
                                          RigCompletionDataGridCell( globalCellIndex, settings.caseToApply->mainGrid() ),
                                          wellBorePart.intersectionWithWellMeasuredDepth );
            completion.setSecondOrderingValue( wellBorePart.lateralIndex );

            double transmissibility = 0.0;
            double kh               = RigCompletionData::defaultValue();
            if ( wellBorePart.isMainBore )
            {
                // No change in transmissibility for main bore
                auto transmissibilityAndPermeability =
                    RicTransmissibilityCalculator::calculateTransmissibilityData( settings.caseToApply,
                                                                                  wellPath,
                                                                                  wellBorePart.lengthsInCell,
                                                                                  wellBorePart.skinFactor,
                                                                                  wellBorePart.wellRadius,
                                                                                  globalCellIndex,
                                                                                  settings.useLateralNTG );

                transmissibility = transmissibilityAndPermeability.connectionFactor();
                kh               = transmissibilityAndPermeability.kh();
            }
            else
            {
                // Adjust transmissibility for fishbone laterals
                auto transmissibilityAndPermeability = RicTransmissibilityCalculator::calculateTransmissibilityData( settings.caseToApply,
                                                                                                                     wellPath,
                                                                                                                     wellBorePart.lengthsInCell,
                                                                                                                     wellBorePart.skinFactor,
                                                                                                                     wellBorePart.wellRadius,
                                                                                                                     globalCellIndex,
                                                                                                                     settings.useLateralNTG,
                                                                                                                     numberOfLaterals,
                                                                                                                     mainBoreDirection );

                transmissibility = transmissibilityAndPermeability.connectionFactor();
                kh               = transmissibilityAndPermeability.kh();
            }

            auto direction =
                RicTransmissibilityCalculator::calculateCellMainDirection( settings.caseToApply, globalCellIndex, wellBorePart.lengthsInCell );

            completion.setTransAndWPImultBackgroundDataFromFishbone( transmissibility,
                                                                     wellBorePart.skinFactor,
                                                                     wellBorePart.wellRadius * 2,
                                                                     kh,
                                                                     direction,
                                                                     wellBorePart.isMainBore );

            completion.addMetadata( wellBorePart.metaData, QString::number( transmissibility ) );
            completion.setSourcePdmObject( wellBorePart.sourcePdmObject );
            completionData.push_back( completion );
        }
    }
    return completionData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<size_t, std::vector<WellBorePartForTransCalc>>
    RicFishbonesTransmissibilityCalculationFeatureImp::findFishboneLateralsWellBoreParts( const RimWellPath*    wellPath,
                                                                                          const RimEclipseCase* eclipseCase )
{
    std::map<size_t, std::vector<WellBorePartForTransCalc>> wellBorePartsInCells;
    if ( !wellPath || !wellPath->wellPathGeometry() || wellPath->wellPathGeometry()->measuredDepths().size() < 2 )
        return wellBorePartsInCells;

    const RigEclipseCaseData*     caseData   = eclipseCase->eclipseCaseData();
    RiaDefines::EclipseUnitSystem unitSystem = caseData->unitsType();

    // Build lateral WellBoreParts directly from RimFishbones — no MSW tree needed.
    for ( const RimFishbones* subs : wellPath->completions()->fishbonesCollection()->activeFishbonesSubs() )
    {
        const double holeDiameter = subs->holeDiameter( unitSystem );
        const double skinFactor   = subs->skinFactor();

        for ( const auto& [subIndex, lateralIndex] : subs->installedLateralIndices() )
        {
            auto lateralCoordMDPairs = subs->coordsAndMDForLateral( subIndex, lateralIndex );
            if ( lateralCoordMDPairs.empty() ) continue;

            std::vector<cvf::Vec3d> coords;
            std::vector<double>     mds;
            for ( const auto& [coord, md] : lateralCoordMDPairs )
            {
                coords.push_back( coord );
                mds.push_back( md );
            }

            auto intersections = RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( caseData, wellPath->name(), coords, mds );

            for ( const auto& cellIntInfo : intersections )
            {
                QString completionMetaData = QString( "Sub segment: Sub: %1 Lateral: %2" ).arg( subIndex + 1 ).arg( lateralIndex + 1 );

                WellBorePartForTransCalc wellBorePart( cellIntInfo.intersectionLengthsInCellCS,
                                                       holeDiameter / 2.0,
                                                       skinFactor,
                                                       false,
                                                       completionMetaData );

                wellBorePart.intersectionWithWellMeasuredDepth = cellIntInfo.endMD;
                wellBorePart.lateralIndex                      = lateralIndex;
                wellBorePart.setSourcePdmObject( subs );

                wellBorePartsInCells[cellIntInfo.globCellIndex].push_back( wellBorePart );
            }
        }
    }

    {
        // Note that it is not supported to export main bore perforation intervals for Imported Laterals, only for
        // fishbones defined by ResInsight. It is not trivial to define the open section of the main bore for
        // imported laterals.

        if ( wellPath->fishbonesCollection()->isChecked() )
        {
            double holeRadius = wellPath->fishbonesCollection()->mainBoreDiameter( unitSystem ) / 2.0;
            double skinFactor = wellPath->fishbonesCollection()->mainBoreSkinFactor();

            for ( const auto& fishboneDefinition : wellPath->fishbonesCollection()->activeFishbonesSubs() )
            {
                double startMD = fishboneDefinition->startMD();
                double endMD   = fishboneDefinition->endMD();

                if ( fabs( startMD - endMD ) < 1e-3 )
                {
                    // Start and end md are close, adjust to be sure we get an intersection along the well path
                    startMD -= 0.5;
                    endMD += 0.5;
                }

                appendMainWellBoreParts( wellBorePartsInCells,
                                         wellPath,
                                         eclipseCase->eclipseCaseData(),
                                         skinFactor,
                                         holeRadius,
                                         startMD,
                                         endMD,
                                         fishboneDefinition );
            }
        }
    }

    return wellBorePartsInCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicFishbonesTransmissibilityCalculationFeatureImp::appendMainWellBoreParts(
    std::map<size_t, std::vector<WellBorePartForTransCalc>>& wellBorePartsInCells,
    const RimWellPath*                                       wellPath,
    const RigEclipseCaseData*                                eclipseCaseData,
    double                                                   skinFactor,
    double                                                   holeRadius,
    double                                                   startMeasuredDepth,
    double                                                   endMeasuredDepth,
    const RimFishbones*                                      fishbonesDefinitions )
{
    if ( !wellPath ) return;
    if ( !wellPath->wellPathGeometry() ) return;
    if ( !wellPath->fishbonesCollection() ) return;

    bool isMainBore = true;

    std::pair<std::vector<cvf::Vec3d>, std::vector<double>> fishbonePerfWellPathCoords =
        wellPath->wellPathGeometry()->clippedPointSubset( startMeasuredDepth, endMeasuredDepth );

    std::vector<WellPathCellIntersectionInfo> intersectedCellsIntersectionInfo =
        RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( eclipseCaseData,
                                                                          wellPath->name(),
                                                                          fishbonePerfWellPathCoords.first,
                                                                          fishbonePerfWellPathCoords.second );

    for ( const auto& cellIntersectionInfo : intersectedCellsIntersectionInfo )
    {
        QString                  completionMetaData = wellPath->completionSettings()->wellNameForExport() + " main bore";
        WellBorePartForTransCalc wellBorePart =
            WellBorePartForTransCalc( cellIntersectionInfo.intersectionLengthsInCellCS, holeRadius, skinFactor, isMainBore, completionMetaData );

        wellBorePart.intersectionWithWellMeasuredDepth = cellIntersectionInfo.startMD;

        wellBorePart.setSourcePdmObject( fishbonesDefinitions );
        wellBorePartsInCells[cellIntersectionInfo.globCellIndex].push_back( wellBorePart );
    }
}
