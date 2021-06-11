/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimCompletionCellIntersectionCalc.h"

#include "RiaApplication.h"
#include "RiaDefines.h"

#include "RigCompletionData.h"
#include "RigVirtualPerforationTransmissibilities.h"
#include "RigWellPath.h"
#include "RigWellPathIntersectionTools.h"

#include "RimEclipseCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaDefines::WellPathComponentType> fromCompletionData( const std::vector<RigCompletionData>& data )
{
    std::vector<RiaDefines::WellPathComponentType> appCompletionTypes;

    for ( const auto& d : data )
    {
        switch ( d.completionType() )
        {
            case RigCompletionData::CompletionType::FRACTURE:
                appCompletionTypes.push_back( RiaDefines::WellPathComponentType::FRACTURE );
                break;
            case RigCompletionData::CompletionType::PERFORATION:
                appCompletionTypes.push_back( RiaDefines::WellPathComponentType::PERFORATION_INTERVAL );
                break;
            case RigCompletionData::CompletionType::FISHBONES:
                appCompletionTypes.push_back( RiaDefines::WellPathComponentType::FISHBONES );
                break;
            default:
                break;
        }
    }

    return appCompletionTypes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCompletionCellIntersectionCalc::calculateCompletionTypeResult( RimEclipseCase*      eclipseCase,
                                                                       std::vector<double>& completionTypeCellResult,
                                                                       size_t               timeStep )
{
    CVF_ASSERT( eclipseCase && eclipseCase->eclipseCaseData() );

    const RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();

    std::vector<const RimWellPath*> visibleWells;
    {
        RimProject* project = RiaApplication::instance()->project();
        if ( project->activeOilField()->wellPathCollection->isActive )
        {
            for ( const RimWellPath* wellPath : project->activeOilField()->wellPathCollection->allWellPaths() )
            {
                if ( wellPath->showWellPath() && wellPath->wellPathGeometry() )
                {
                    visibleWells.push_back( wellPath );
                }
            }
        }
    }

    // Set all intersected cells by well path cells to well path completion type. Override later with other completions
    // types
    for ( const RimWellPath* wellPath : visibleWells )
    {
        auto intersectedCells =
            RigWellPathIntersectionTools::findIntersectedGlobalCellIndices( eclipseCaseData,
                                                                            wellPath->wellPathGeometry()->wellPathPoints() );

        for ( auto& intersection : intersectedCells )
        {
            completionTypeCellResult[intersection] = static_cast<int>( RiaDefines::WellPathComponentType::WELL_PATH );
        }
    }

    auto completions = eclipseCase->computeAndGetVirtualPerforationTransmissibilities();
    if ( completions )
    {
        for ( const RimWellPath* wellPath : visibleWells )
        {
            for ( const auto& [globalCellIndex, completionsForCell] :
                  completions->multipleCompletionsPerEclipseCell( wellPath, timeStep ) )
            {
                RiaDefines::WellPathComponentType appCompletionType = RiaDefines::WellPathComponentType::WELL_PATH;

                auto appCompletionTypes = fromCompletionData( completionsForCell );

                if ( std::find( appCompletionTypes.begin(),
                                appCompletionTypes.end(),
                                RiaDefines::WellPathComponentType::FRACTURE ) != appCompletionTypes.end() )
                {
                    appCompletionType = RiaDefines::WellPathComponentType::FRACTURE;
                }
                else if ( std::find( appCompletionTypes.begin(),
                                     appCompletionTypes.end(),
                                     RiaDefines::WellPathComponentType::FISHBONES ) != appCompletionTypes.end() )
                {
                    appCompletionType = RiaDefines::WellPathComponentType::FISHBONES;
                }
                else if ( std::find( appCompletionTypes.begin(),
                                     appCompletionTypes.end(),
                                     RiaDefines::WellPathComponentType::PERFORATION_INTERVAL ) != appCompletionTypes.end() )
                {
                    appCompletionType = RiaDefines::WellPathComponentType::PERFORATION_INTERVAL;
                }

                if ( appCompletionType != RiaDefines::WellPathComponentType::WELL_PATH )
                {
                    completionTypeCellResult[globalCellIndex] = static_cast<int>( appCompletionType );
                }
            }
        }
    }
}
