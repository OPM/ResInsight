#include "RigWellPathStimplanIntersector.h"
#include "RigWellPath.h"
#include "RimFracture.h"
#include "RigCellGeometryTools.h"
#include "RimFractureTemplate.h"
#include "cvfBase.h"
#include "cvfMatrix4.h"
#include "RigStimPlanFracTemplateCell.h"
#include "RimStimPlanFractureTemplate.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///  Todo: Use only the perforated parts of the well path
//--------------------------------------------------------------------------------------------------
RigWellPathStimplanIntersector::RigWellPathStimplanIntersector(const RigWellPath* wellpathGeom, const RimFracture * rimFracture)
{
    auto stimPlanFractureTemplate = dynamic_cast<RimStimPlanFractureTemplate*> (rimFracture->attachedFractureDefinition());

    CVF_ASSERT(stimPlanFractureTemplate);

    std::vector<cvf::Vec3d> wellPathPoints = wellpathGeom->m_wellPathPoints;
    cvf::Mat4d toFractureXf                = cvf::Mat4d (rimFracture->transformMatrix().getInverted());
    double wellRadius                      = rimFracture->wellRadius();
    std::vector<cvf::Vec3d> fracturePolygon;
    {
        std::vector<cvf::Vec3f> fracturePolygonf = stimPlanFractureTemplate->fracturePolygon(rimFracture->fractureUnit());
        for ( auto fpv: fracturePolygonf ) fracturePolygon.push_back(cvf::Vec3d(fpv));
    }

    // Convert well path to fracture template system
    for ( auto & wellPPoint : wellPathPoints )  wellPPoint.transformPoint(toFractureXf);

    // Clip well path to fracture domain 

    std::vector<std::vector<cvf::Vec3d> > wellPathPartsWithinFracture =
        RigCellGeometryTools::clipPolylineByPolygon(wellPathPoints, fracturePolygon, RigCellGeometryTools::INTERPOLATE_LINE_Z);

    // Remove the part of the well path that is more than well radius away from the fracture plane

    std::vector< std::vector< cvf::Vec3d > > intersectingWellPathParts;

    for ( const auto& part : wellPathPartsWithinFracture )
    {
        std::vector< cvf::Vec3d > currentIntersectingWpPart;
        for ( size_t vxIdx = 0; vxIdx < part.size() -1; ++vxIdx )
        {
            double thisZ = fabs(wellPathPoints[vxIdx].z());
            double nextZ = fabs(wellPathPoints[vxIdx + 1].z());

            if ( thisZ >= wellRadius && nextZ >= wellRadius ) continue;

            if ( thisZ < wellRadius && nextZ < wellRadius )
            {
                currentIntersectingWpPart.push_back(wellPathPoints[vxIdx]);
                continue;
            }

            if ( thisZ < wellRadius && nextZ >= wellRadius )
            {
                currentIntersectingWpPart.push_back(wellPathPoints[vxIdx]);
                double fraction = (wellRadius - thisZ)/ (nextZ - thisZ);
                cvf::Vec3d intersectPoint =  wellPathPoints[vxIdx] +  fraction * (wellPathPoints[vxIdx+1] - wellPathPoints[vxIdx]);
                currentIntersectingWpPart.push_back(intersectPoint);

                intersectingWellPathParts.push_back(currentIntersectingWpPart);
                currentIntersectingWpPart.clear();
                continue;
            }

            if ( thisZ >= wellRadius && nextZ < wellRadius )
            {
                double fraction = (wellRadius - thisZ)/ (nextZ - thisZ);
                cvf::Vec3d intersectPoint =  wellPathPoints[vxIdx] +  fraction * (wellPathPoints[vxIdx+1] - wellPathPoints[vxIdx]);
                currentIntersectingWpPart.push_back(intersectPoint);
                continue;
            }
        }

        if ( currentIntersectingWpPart.size() )
        {
            intersectingWellPathParts.push_back(currentIntersectingWpPart);
        }
    }

    // Find the StimPlan cells touched by the intersecting well path parts 

    const std::vector<RigStimPlanFracTemplateCell>& stpCells = stimPlanFractureTemplate->getStimPlanCells();


    for ( size_t cIdx = 0; cIdx < stpCells.size(); ++ cIdx )
    {
        std::vector<cvf::Vec3d> cellPolygon = stpCells[cIdx].getPolygon();
        for ( const auto& wellpathPart :intersectingWellPathParts )
        {
            std::vector<std::vector<cvf::Vec3d> > wellPathPartsInPolygon =
                RigCellGeometryTools::clipPolylineByPolygon(wellpathPart,
                                                            cellPolygon,
                                                            RigCellGeometryTools::USE_HUGEVAL);
            for ( const auto& wellPathPartInCell: wellPathPartsInPolygon )
            {
                if ( wellPathPartInCell.size() )
                {
                    int endpointCount = 0;
                    if ( wellPathPartInCell.front().z() != HUGE_VAL ) ++endpointCount;
                    if ( wellPathPartInCell.back().z() != HUGE_VAL ) ++endpointCount;

                    cvf::Vec3d intersectionLength = (wellPathPartInCell.back() - wellPathPartInCell.front());
                    double xLengthInCell = fabs(intersectionLength.x());
                    double yLengthInCell = fabs(intersectionLength.y());

                    m_stimPlanCellIdxToIntersectionInfoMap[cIdx].endpointCount += endpointCount;
                    m_stimPlanCellIdxToIntersectionInfoMap[cIdx].hlength += xLengthInCell;
                    m_stimPlanCellIdxToIntersectionInfoMap[cIdx].vlength += yLengthInCell;
                }
            }
        }
    }
}
