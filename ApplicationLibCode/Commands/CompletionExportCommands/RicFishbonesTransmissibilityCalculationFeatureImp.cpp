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
#include "RicMswBranch.h"
#include "RicMswCompletions.h"
#include "RicMswExportInfo.h"
#include "RicMswSegment.h"
#include "RicWellPathExportCompletionDataFeatureImpl.h"
#include "RicWellPathExportMswCompletionsImpl.h"

#include "RigActiveCellInfo.h"
#include "RigCompletionData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigWellLogExtractor.h"
#include "RigWellPath.h"
#include "RigWellPathIntersectionTools.h"

#include "RimFishbones.h"
#include "RimFishbonesCollection.h"
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

    const RigActiveCellInfo* activeCellInfo =
        settings.caseToApply->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

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
                    RicWellPathExportCompletionDataFeatureImpl::calculateTransmissibilityData( settings.caseToApply,
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
                kh               = transmissibilityAndPermeability.kh();
            }

            CellDirection direction =
                RicWellPathExportCompletionDataFeatureImpl::calculateCellMainDirection( settings.caseToApply,
                                                                                        globalCellIndex,
                                                                                        wellBorePart.lengthsInCell );

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
void RicFishbonesTransmissibilityCalculationFeatureImp::findFishboneLateralsWellBoreParts(
    std::map<size_t, std::vector<WellBorePartForTransCalc>>& wellBorePartsInCells,
    const RimWellPath*                                       wellPath,
    const RicExportCompletionDataSettingsUi&                 settings )
{
    if ( !wellPath || !wellPath->wellPathGeometry() ) return;

    // Generate data
    const RigEclipseCaseData* caseData = settings.caseToApply()->eclipseCaseData();

    auto                          mswParameters = wellPath->fishbonesCollection()->mswParameters();
    RiaDefines::EclipseUnitSystem unitSystem    = caseData->unitsType();

    RicMswExportInfo exportInfo( wellPath,
                                 unitSystem,
                                 wellPath->fishbonesCollection()->startMD(),
                                 mswParameters->lengthAndDepth().text(),
                                 mswParameters->pressureDrop().text() );
    exportInfo.setLinerDiameter( mswParameters->linerDiameter( unitSystem ) );
    exportInfo.setRoughnessFactor( mswParameters->roughnessFactor( unitSystem ) );

    RicWellPathExportMswCompletionsImpl::generateFishbonesMswExportInfoForWell( settings.caseToApply(),
                                                                                wellPath,
                                                                                &exportInfo,
                                                                                exportInfo.mainBoreBranch() );

    bool isMainBore = false;

    for ( auto mainBoreSegment : exportInfo.mainBoreBranch()->segments() )
    {
        for ( auto mainBoreCompletion : mainBoreSegment->completions() )
        {
            for ( auto completionSegment : mainBoreCompletion->segments() )
            {
                for ( auto completion : completionSegment->completions() )
                {
                    for ( auto segment : completion->segments() )
                    {
                        for ( auto intersection : segment->intersections() )
                        {
                            double  diameter           = segment->holeDiameter();
                            QString completionMetaData = ( segment->label() + QString( ": Sub: %1 Lateral: %2" )
                                                                                  .arg( segment->subIndex() + 1 )
                                                                                  .arg( completion->index() + 1 ) );

                            WellBorePartForTransCalc wellBorePart =
                                WellBorePartForTransCalc( intersection->lengthsInCell(),
                                                          diameter / 2.0,
                                                          segment->skinFactor(),
                                                          isMainBore,
                                                          completionMetaData );

                            wellBorePart.intersectionWithWellMeasuredDepth = segment->endMD();
                            wellBorePart.lateralIndex                      = completion->index();
                            wellBorePart.setSourcePdmObject( segment->sourcePdmObject() );

                            wellBorePartsInCells[intersection->globalCellIndex()].push_back( wellBorePart );
                        }
                    }
                }
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
void RicFishbonesTransmissibilityCalculationFeatureImp::appendMainWellBoreParts(
    std::map<size_t, std::vector<WellBorePartForTransCalc>>& wellBorePartsInCells,
    const RimWellPath*                                       wellPath,
    const RicExportCompletionDataSettingsUi&                 settings,
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
        RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( settings.caseToApply->eclipseCaseData(),
                                                                          wellPath->name(),
                                                                          fishbonePerfWellPathCoords.first,
                                                                          fishbonePerfWellPathCoords.second );

    for ( const auto& cellIntersectionInfo : intersectedCellsIntersectionInfo )
    {
        QString completionMetaData            = wellPath->completionSettings()->wellNameForExport() + " main bore";
        WellBorePartForTransCalc wellBorePart = WellBorePartForTransCalc( cellIntersectionInfo.intersectionLengthsInCellCS,
                                                                          holeRadius,
                                                                          skinFactor,
                                                                          isMainBore,
                                                                          completionMetaData );

        wellBorePart.intersectionWithWellMeasuredDepth = cellIntersectionInfo.startMD;

        wellBorePart.setSourcePdmObject( fishbonesDefinitions );
        wellBorePartsInCells[cellIntersectionInfo.globCellIndex].push_back( wellBorePart );
    }
}
