/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RicWellPathExportMswTableData.h"

#include "RiaDefines.h"

#include "MswExport/RicWellPathExportMswGeometryPath.h"
#include "RicExportCompletionDataSettingsUi.h"

#include "CompletionsMsw/RigMswTableData.h"
#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigGridBase.h"
#include "RigMainGrid.h"
#include "Well/RigWellLogExtractor.h"
#include "Well/RigWellPath.h"
#include "Well/RigWellPathIntersectionTools.h"

#include "RimEclipseCase.h"
#include "RimMswCompletionParameters.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCompletions.h"
#include "RimWellPathTieIn.h"

#include <algorithm>
#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<RigMswTableData, std::string>
    RicWellPathExportMswTableData::extractSingleWellMswData( RimEclipseCase*                 eclipseCase,
                                                             RimWellPath*                    wellPath,
                                                             bool                            exportCompletionsAfterMainBoreSegments,
                                                             CompletionType                  completionType,
                                                             const std::optional<QDateTime>& exportDate )
{
    if ( !wellPath || !wellPath->wellPathGeometry() )
    {
        return std::unexpected( "Well path has no geometry; the well path file may be missing or failed to load." );
    }

    return extractSingleWellMsw( eclipseCase, wellPath, exportCompletionsAfterMainBoreSegments, completionType, exportDate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<RigMswTableData, std::string> RicWellPathExportMswTableData::extractSingleWellMsw( RimEclipseCase* eclipseCase,
                                                                                                 RimWellPath*    wellPath,
                                                                                                 bool exportCompletionsAfterMainBoreSegments,
                                                                                                 CompletionType completionType,
                                                                                                 const std::optional<QDateTime>& exportDate )
{
    if ( !eclipseCase || !wellPath || eclipseCase->eclipseCaseData() == nullptr )
        return std::unexpected( "Invalid eclipse case or well path provided" );

    auto mswParameters = wellPath->mswCompletionParameters();
    if ( !mswParameters ) return std::unexpected( "Missing MSW completion parameters" );

    const std::vector<std::pair<double, double>> customSegmentIntervals = mswParameters->getSegmentIntervals();
    auto                                         wellExportData = RicWellPathExportMswGeometryPath::buildMswWellExportData( eclipseCase,
                                                                                    wellPath,
                                                                                    mswParameters->maxSegmentLength(),
                                                                                    customSegmentIntervals,
                                                                                    completionType,
                                                                                    exportDate );

    auto unitSystem = eclipseCase->eclipseCaseData()->unitsType();
    return RicWellPathExportMswGeometryPath::collectTableData( wellExportData, unitSystem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicWellPathExportMswTableData::CompletionType
    RicWellPathExportMswTableData::convertFromExportSettings( const RicExportCompletionDataSettingsUi& settings )
{
    CompletionType result = CompletionType::NONE;

    if ( settings.includePerforations() )
    {
        result |= CompletionType::PERFORATIONS;
    }

    if ( settings.includeFishbones() )
    {
        result |= CompletionType::FISHBONES;
    }

    if ( settings.includeFractures() )
    {
        result |= CompletionType::FRACTURES;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<WellPathCellIntersectionInfo> RicWellPathExportMswTableData::generateCellSegments( const RimEclipseCase* eclipseCase,
                                                                                               const RimWellPath*    wellPath )
{
    auto wellPathGeometry = wellPath->wellPathGeometry();
    if ( !wellPathGeometry ) return {};

    const std::vector<cvf::Vec3d>& coords = wellPathGeometry->uniqueWellPathPoints();
    const std::vector<double>&     mds    = wellPathGeometry->uniqueMeasuredDepths();
    if ( coords.empty() || mds.empty() ) return {};

    const RigMainGrid* mainGrid = eclipseCase->mainGrid();

    std::vector<WellPathCellIntersectionInfo> allIntersections =
        RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( eclipseCase->eclipseCaseData(), wellPath->name(), coords, mds );
    if ( allIntersections.empty() ) return {};

    std::vector<WellPathCellIntersectionInfo> continuousIntersections =
        RigWellPathIntersectionTools::buildContinuousIntersections( allIntersections, mainGrid );

    return continuousIntersections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicWellPathExportMswTableData::computeIntitialMeasuredDepth( const RimEclipseCase*                            eclipseCase,
                                                                    const RimWellPath*                               wellPath,
                                                                    const RimMswCompletionParameters*                mswParameters,
                                                                    const std::vector<WellPathCellIntersectionInfo>& allIntersections )
{
    if ( allIntersections.empty() ) return 0.0;

    const RigActiveCellInfo* activeCellInfo = eclipseCase->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

    double candidateMeasuredDepth = 0.0;
    if ( mswParameters->referenceMDType() == RimMswCompletionParameters::ReferenceMDType::MANUAL_REFERENCE_MD )
    {
        candidateMeasuredDepth = mswParameters->manualReferenceMD();
    }
    else
    {
        for ( const WellPathCellIntersectionInfo& intersection : allIntersections )
        {
            if ( activeCellInfo->isActive( ReservoirCellIndex( intersection.globCellIndex ) ) )
            {
                candidateMeasuredDepth = intersection.startMD;
                break;
            }
        }

        double startOfFirstCompletion = std::numeric_limits<double>::infinity();
        {
            std::vector<const RimWellPathComponentInterface*> allCompletions = wellPath->completions()->allCompletions();

            for ( const RimWellPathComponentInterface* completion : allCompletions )
            {
                if ( completion->isEnabled() && completion->startMD() < startOfFirstCompletion )
                {
                    startOfFirstCompletion = completion->startMD();
                }
            }
        }

        // Initial MD is the lowest MD based on grid intersection and start of fracture completions
        // https://github.com/OPM/ResInsight/issues/6071
        candidateMeasuredDepth = std::min( candidateMeasuredDepth, startOfFirstCompletion );
    }

    return candidateMeasuredDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<WellPathCellIntersectionInfo>
    RicWellPathExportMswTableData::filterIntersections( const std::vector<WellPathCellIntersectionInfo>& intersections,
                                                        double                                           initialMD,
                                                        gsl::not_null<const RigWellPath*>                wellPathGeometry,
                                                        gsl::not_null<const RimEclipseCase*>             eclipseCase )
{
    std::vector<WellPathCellIntersectionInfo> filteredIntersections;

    if ( !intersections.empty() && intersections[0].startMD > initialMD )
    {
        WellPathCellIntersectionInfo firstIntersection = intersections[0];

        // Add a segment from user defined MD to start of grid
        cvf::Vec3d intersectionPoint = wellPathGeometry->interpolatedPointAlongWellPath( initialMD );

        WellPathCellIntersectionInfo extraIntersection;

        extraIntersection.globCellIndex         = std::numeric_limits<size_t>::max();
        extraIntersection.startPoint            = intersectionPoint;
        extraIntersection.endPoint              = firstIntersection.startPoint;
        extraIntersection.startMD               = initialMD;
        extraIntersection.endMD                 = firstIntersection.startMD;
        extraIntersection.intersectedCellFaceIn = cvf::StructGridInterface::NO_FACE;

        if ( firstIntersection.intersectedCellFaceIn != cvf::StructGridInterface::NO_FACE )

        {
            extraIntersection.intersectedCellFaceOut = cvf::StructGridInterface::oppositeFace( firstIntersection.intersectedCellFaceIn );
        }
        else if ( firstIntersection.intersectedCellFaceOut != cvf::StructGridInterface::NO_FACE )
        {
            extraIntersection.intersectedCellFaceOut = firstIntersection.intersectedCellFaceOut;
        }

        extraIntersection.intersectionLengthsInCellCS = cvf::Vec3d::ZERO;

        filteredIntersections.push_back( extraIntersection );
    }

    const double epsilon = 1.0e-3;

    for ( const WellPathCellIntersectionInfo& intersection : intersections )
    {
        if ( ( intersection.endMD - initialMD ) < epsilon )
        {
            // Skip all intersections before initial measured depth
            continue;
        }

        if ( ( intersection.startMD - initialMD ) > epsilon )
        {
            filteredIntersections.push_back( intersection );
        }
        else
        {
            // InitialMD is inside intersection, split based on intersection point

            cvf::Vec3d intersectionPoint = wellPathGeometry->interpolatedPointAlongWellPath( initialMD );

            WellPathCellIntersectionInfo extraIntersection;

            extraIntersection.globCellIndex          = intersection.globCellIndex;
            extraIntersection.startPoint             = intersectionPoint;
            extraIntersection.endPoint               = intersection.endPoint;
            extraIntersection.startMD                = initialMD;
            extraIntersection.endMD                  = intersection.endMD;
            extraIntersection.intersectedCellFaceIn  = cvf::StructGridInterface::NO_FACE;
            extraIntersection.intersectedCellFaceOut = intersection.intersectedCellFaceOut;

            const RigMainGrid* grid = eclipseCase->mainGrid();

            if ( intersection.globCellIndex < grid->cellCount() )
            {
                extraIntersection.intersectionLengthsInCellCS =
                    RigWellPathIntersectionTools::calculateLengthInCell( grid, intersection.globCellIndex, intersectionPoint, intersection.endPoint );
            }
            else
            {
                extraIntersection.intersectionLengthsInCellCS = cvf::Vec3d::ZERO;
            }

            filteredIntersections.push_back( extraIntersection );
        }
    }

    return filteredIntersections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicWellPathExportMswTableData::wellPathsWithTieIn( const RimWellPath* wellPath )
{
    std::vector<RimWellPath*> connectedWellPaths;
    {
        auto wellPaths = RimProject::current()->allWellPaths();
        for ( auto well : wellPaths )
        {
            if ( well && well->isEnabled() && well->wellPathTieIn() && well->wellPathTieIn()->parentWell() == wellPath )
            {
                connectedWellPaths.push_back( well );
            }
        }
    }

    return connectedWellPaths;
}
