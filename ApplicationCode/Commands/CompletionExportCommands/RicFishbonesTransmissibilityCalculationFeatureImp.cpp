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
#include "RicMswExportInfo.h"
#include "RicWellPathExportCompletionDataFeatureImpl.h"
#include "RicWellPathExportMswCompletionsImpl.h"

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

#include <cafPdmObject.h>
#include <cafPdmPointer.h>

//==================================================================================================
///
//==================================================================================================
struct WellBorePartForTransCalc
{
    WellBorePartForTransCalc( cvf::Vec3d     lengthsInCell,
                              double         wellRadius,
                              double         skinFactor,
                              bool           isMainBore,
                              const QString& metaData )
        : lengthsInCell( lengthsInCell )
        , wellRadius( wellRadius )
        , skinFactor( skinFactor )
        , isMainBore( isMainBore )
        , metaData( metaData )
        , intersectionWithWellMeasuredDepth( HUGE_VAL )
        , lateralIndex( cvf::UNDEFINED_SIZE_T )
    {
    }

    cvf::Vec3d lengthsInCell;
    double     wellRadius;
    double     skinFactor;
    QString    metaData;
    bool       isMainBore;

    double intersectionWithWellMeasuredDepth;
    size_t lateralIndex;

    void setSourcePdmObject( const caf::PdmObject* sourcePdmObj )
    {
        this->sourcePdmObject = const_cast<caf::PdmObject*>( sourcePdmObj );
    }
    caf::PdmPointer<caf::PdmObject> sourcePdmObject;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData>
    RicFishbonesTransmissibilityCalculationFeatureImp::generateFishboneCompdatValuesUsingAdjustedCellVolume(
        const RimWellPath*                       wellPath,
        const RicExportCompletionDataSettingsUi& settings )
{
    std::vector<RigCompletionData> completionData;

    if ( !wellPath || !wellPath->completions() || !wellPath->wellPathGeometry() )
    {
        return completionData;
    }

    std::map<size_t, std::vector<WellBorePartForTransCalc>> wellBorePartsInCells; // wellBore = main bore or fishbone
                                                                                  // lateral
    findFishboneLateralsWellBoreParts( wellBorePartsInCells, wellPath, settings );
    findFishboneImportedLateralsWellBoreParts( wellBorePartsInCells, wellPath, settings );

    const RigActiveCellInfo* activeCellInfo =
        settings.caseToApply->eclipseCaseData()->activeCellInfo( RiaDefines::MATRIX_MODEL );

    for ( const auto& cellAndWellBoreParts : wellBorePartsInCells )
    {
        size_t                                       globalCellIndex = cellAndWellBoreParts.first;
        const std::vector<WellBorePartForTransCalc>& wellBoreParts   = cellAndWellBoreParts.second;

        bool cellIsActive = activeCellInfo->isActive( globalCellIndex );
        if ( !cellIsActive ) continue;

        // Find main bore and number of laterals

        size_t        numberOfLaterals  = 0;
        CellDirection mainBoreDirection = DIR_I;
        for ( const auto& wellBorePart : wellBoreParts )
        {
            if ( !wellBorePart.isMainBore )
            {
                numberOfLaterals++;
            }
            else
            {
                mainBoreDirection =
                    RicWellPathExportCompletionDataFeatureImpl::calculateCellMainDirection( settings.caseToApply,
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

            RigCompletionData completion( wellPath->completions()->wellNameForExport(),
                                          RigCompletionDataGridCell( globalCellIndex, settings.caseToApply->mainGrid() ),
                                          wellBorePart.intersectionWithWellMeasuredDepth );
            completion.setSecondOrderingValue( wellBorePart.lateralIndex );

            double transmissibility = 0.0;
            if ( wellBorePart.isMainBore )
            {
                // No change in transmissibility for main bore
                auto transmissibilityAndPermeability =
                    RicWellPathExportCompletionDataFeatureImpl::calculateTransmissibilityData( settings.caseToApply,
                                                                                               wellPath,
                                                                                               wellBorePart.lengthsInCell,
                                                                                               wellBorePart.skinFactor,
                                                                                               wellBorePart.wellRadius,
                                                                                               globalCellIndex,
                                                                                               settings.useLateralNTG );

                transmissibility = transmissibilityAndPermeability.connectionFactor();
            }
            else
            {
                // Adjust transmissibility for fishbone laterals
                auto transmissibilityAndPermeability =
                    RicWellPathExportCompletionDataFeatureImpl::calculateTransmissibilityData( settings.caseToApply,
                                                                                               wellPath,
                                                                                               wellBorePart.lengthsInCell,
                                                                                               wellBorePart.skinFactor,
                                                                                               wellBorePart.wellRadius,
                                                                                               globalCellIndex,
                                                                                               settings.useLateralNTG,
                                                                                               numberOfLaterals,
                                                                                               mainBoreDirection );

                transmissibility = transmissibilityAndPermeability.connectionFactor();
            }

            CellDirection direction =
                RicWellPathExportCompletionDataFeatureImpl::calculateCellMainDirection( settings.caseToApply,
                                                                                        globalCellIndex,
                                                                                        wellBorePart.lengthsInCell );

            completion.setTransAndWPImultBackgroundDataFromFishbone( transmissibility,
                                                                     wellBorePart.skinFactor,
                                                                     wellBorePart.wellRadius * 2,
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
void RicFishbonesTransmissibilityCalculationFeatureImp::findFishboneLateralsWellBoreParts(
    std::map<size_t, std::vector<WellBorePartForTransCalc>>& wellBorePartsInCells,
    const RimWellPath*                                       wellPath,
    const RicExportCompletionDataSettingsUi&                 settings )
{
    if ( !wellPath || !wellPath->wellPathGeometry() ) return;

    // Generate data
    const RigEclipseCaseData* caseData = settings.caseToApply()->eclipseCaseData();
    RicMswExportInfo          exportInfo =
        RicWellPathExportMswCompletionsImpl::generateFishbonesMswExportInfo( settings.caseToApply(), wellPath, false );

    RiaEclipseUnitTools::UnitSystem unitSystem = caseData->unitsType();
    bool                            isMainBore = false;

    for ( std::shared_ptr<RicMswSegment> location : exportInfo.wellSegmentLocations() )
    {
        for ( std::shared_ptr<RicMswCompletion> completion : location->completions() )
        {
            for ( std::shared_ptr<RicMswSubSegment> segment : completion->subSegments() )
            {
                for ( std::shared_ptr<RicMswSubSegmentCellIntersection> intersection : segment->intersections() )
                {
                    double  diameter = location->holeDiameter();
                    QString completionMetaData =
                        ( location->label() +
                          QString( ": Sub: %1 Lateral: %2" ).arg( location->subIndex() ).arg( completion->index() ) );

                    WellBorePartForTransCalc wellBorePart = WellBorePartForTransCalc( intersection->lengthsInCell(),
                                                                                      diameter / 2.0,
                                                                                      location->skinFactor(),
                                                                                      isMainBore,
                                                                                      completionMetaData );

                    wellBorePart.intersectionWithWellMeasuredDepth = location->endMD();
                    wellBorePart.lateralIndex                      = completion->index();
                    wellBorePart.setSourcePdmObject( location->sourcePdmObject() );

                    wellBorePartsInCells[intersection->globalCellIndex()].push_back( wellBorePart );
                }
            }
        }
    }

    {
        // Note that it is not supported to export main bore perforation intervals for Imported Laterals, only for
        // fishbones defined by ResInsight. It is not trivial to define the open section of the main bore for imported
        // laterals.

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
                                         settings,
                                         skinFactor,
                                         holeRadius,
                                         startMD,
                                         endMD,
                                         fishboneDefinition );
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
    const RicExportCompletionDataSettingsUi&                 settings )
{
    RiaEclipseUnitTools::UnitSystem unitSystem = settings.caseToApply->eclipseCaseData()->unitsType();

    if ( !wellPath ) return;
    if ( !wellPath->wellPathGeometry() ) return;

    bool   isMainBore = false;
    double holeRadius = wellPath->fishbonesCollection()->wellPathCollection()->holeDiameter( unitSystem ) / 2.0;
    double skinFactor = wellPath->fishbonesCollection()->wellPathCollection()->skinFactor();

    for ( const RimFishboneWellPath* fishbonesPath : wellPath->fishbonesCollection()->wellPathCollection()->wellPaths() )
    {
        if ( !fishbonesPath->isChecked() )
        {
            continue;
        }

        std::vector<WellPathCellIntersectionInfo> intersectedCells =
            RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( settings.caseToApply->eclipseCaseData(),
                                                                              fishbonesPath->coordinates(),
                                                                              fishbonesPath->measuredDepths() );

        for ( const auto& cellIntersectionInfo : intersectedCells )
        {
            QString                  completionMetaData = fishbonesPath->name();
            WellBorePartForTransCalc wellBorePart =
                WellBorePartForTransCalc( cellIntersectionInfo.intersectionLengthsInCellCS,
                                          holeRadius,
                                          skinFactor,
                                          isMainBore,
                                          completionMetaData );
            wellBorePart.intersectionWithWellMeasuredDepth = cellIntersectionInfo.startMD;

            wellBorePartsInCells[cellIntersectionInfo.globCellIndex].push_back( wellBorePart );
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
    double                                                   endMeasuredDepth,
    const RimFishbonesMultipleSubs*                          fishbonesDefintions )
{
    if ( !wellPath ) return;
    if ( !wellPath->wellPathGeometry() ) return;
    if ( !wellPath->fishbonesCollection() ) return;

    bool isMainBore = true;

    std::pair<std::vector<cvf::Vec3d>, std::vector<double>> fishbonePerfWellPathCoords =
        wellPath->wellPathGeometry()->clippedPointSubset( startMeasuredDepth, endMeasuredDepth );

    std::vector<WellPathCellIntersectionInfo> intersectedCellsIntersectionInfo =
        RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( settings.caseToApply->eclipseCaseData(),
                                                                          fishbonePerfWellPathCoords.first,
                                                                          fishbonePerfWellPathCoords.second );

    for ( const auto& cellIntersectionInfo : intersectedCellsIntersectionInfo )
    {
        QString                  completionMetaData = wellPath->name() + " main bore";
        WellBorePartForTransCalc wellBorePart = WellBorePartForTransCalc( cellIntersectionInfo.intersectionLengthsInCellCS,
                                                                          holeRadius,
                                                                          skinFactor,
                                                                          isMainBore,
                                                                          completionMetaData );

        wellBorePart.intersectionWithWellMeasuredDepth = cellIntersectionInfo.startMD;

        wellBorePart.setSourcePdmObject( fishbonesDefintions );
        wellBorePartsInCells[cellIntersectionInfo.globCellIndex].push_back( wellBorePart );
    }
}
