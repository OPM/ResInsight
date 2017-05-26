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


RigWellPathStimplanIntersector::RigWellPathStimplanIntersector(const RigWellPath* wellpathGeom, const RimFracture * rimFracture)
{
    std::vector<cvf::Vec3d> wellPathPoints                   = wellpathGeom->m_wellPathPoints;
    cvf::Mat4f fractureXf                                    = rimFracture->transformMatrix();
    double wellRadius                                        = rimFracture->wellRadius();
    std::vector<cvf::Vec3f> fracturePolygonf ; 
    std::vector<std::vector<cvf::Vec3d> > stpCellPolygons;
    {
        auto stimPlanFractureTemplate = dynamic_cast<RimStimPlanFractureTemplate*> (rimFracture->attachedFractureDefinition());

        CVF_ASSERT(stimPlanFractureTemplate);

        fracturePolygonf = stimPlanFractureTemplate->fracturePolygon(rimFracture->fractureUnit());
        {
            const std::vector<RigStimPlanFracTemplateCell>& stpCells = stimPlanFractureTemplate->getStimPlanCells();
            for ( const auto& stpCell: stpCells ) stpCellPolygons.push_back(stpCell.getPolygon());
        }
    }

    calculate(fractureXf, fracturePolygonf, wellPathPoints, wellRadius, stpCellPolygons, m_stimPlanCellIdxToIntersectionInfoMap);

}

//--------------------------------------------------------------------------------------------------
///  Todo: Use only the perforated parts of the well path
//--------------------------------------------------------------------------------------------------

void RigWellPathStimplanIntersector::calculate(const cvf::Mat4f &fractureXf, 
                                               const std::vector<cvf::Vec3f>& fracturePolygonf, 
                                               const std::vector<cvf::Vec3d>& wellPathPointsOrg, 
                                               double wellRadius, 
                                               const std::vector<std::vector<cvf::Vec3d> >& stpCellPolygons, 
                                               std::map<size_t, WellCellIntersection>& m_stimPlanCellIdxToIntersectionInfoMap)
{
    cvf::Mat4d toFractureXf = cvf::Mat4d(fractureXf.getInverted());

    std::vector<cvf::Vec3d> fracturePolygon;
    for ( auto fpv: fracturePolygonf ) fracturePolygon.push_back(cvf::Vec3d(fpv));

    // Convert well path to fracture template system

    std::vector<cvf::Vec3d> fractureRelativeWellPathPoints;
    for ( auto & wellPPoint : wellPathPointsOrg )  fractureRelativeWellPathPoints.push_back(wellPPoint.getTransformedPoint( toFractureXf));

    // Clip well path to fracture domain 

    std::vector<std::vector<cvf::Vec3d> > wellPathPartsWithinFracture =
        RigCellGeometryTools::clipPolylineByPolygon(fractureRelativeWellPathPoints, fracturePolygon, RigCellGeometryTools::INTERPOLATE_LINE_Z);

    // Remove the part of the well path that is more than well radius away from the fracture plane

    std::vector< std::vector< cvf::Vec3d > > intersectingWellPathParts;

    for ( const auto& part : wellPathPartsWithinFracture )
    {
        std::vector< cvf::Vec3d > currentIntersectingWpPart;
        for ( size_t vxIdx = 0; vxIdx < part.size() -1; ++vxIdx )
        {
            double thisAbsZ = fabs(part[vxIdx].z());
            double nextAbsZ = fabs(part[vxIdx + 1].z());
            double thisZ = part[vxIdx].z();
            double nextZ = part[vxIdx + 1].z();

            if ( thisAbsZ >= wellRadius && nextAbsZ >= wellRadius )
            {
                if (    (thisZ >= 0 && nextZ >= 0)
                     || (thisZ <= 0 && nextZ <= 0 )  ) 
                {
                    continue; // Outside
                }
                else // In and out
                {
                    {
                        double wellRadiusDistFromPlane = thisZ > 0 ? wellRadius: -wellRadius;

                        double fraction = (wellRadiusDistFromPlane - thisZ)/ (nextZ - thisZ);

                        cvf::Vec3d intersectPoint =  part[vxIdx] +  fraction * (part[vxIdx+1] - part[vxIdx]);
                        currentIntersectingWpPart.push_back(intersectPoint);
                    }
                    {
                        double wellRadiusDistFromPlane = nextZ > 0 ? wellRadius: -wellRadius;

                        double fraction = (wellRadiusDistFromPlane - thisZ)/ (nextZ - thisZ);

                        cvf::Vec3d intersectPoint =  part[vxIdx] +  fraction * (part[vxIdx+1] - part[vxIdx]);
                        currentIntersectingWpPart.push_back(intersectPoint);

                        intersectingWellPathParts.push_back(currentIntersectingWpPart);
                        currentIntersectingWpPart.clear();
                    }
                    continue;
                }
            }
            if ( thisAbsZ < wellRadius && nextAbsZ < wellRadius ) // Inside
            {
                currentIntersectingWpPart.push_back(part[vxIdx]);
                continue;
            }

            if ( thisAbsZ < wellRadius && nextAbsZ >= wellRadius ) // Going out
            {
                currentIntersectingWpPart.push_back(part[vxIdx]);

                double wellRadiusDistFromPlane = nextZ > 0 ? wellRadius: -wellRadius;

                double fraction = (wellRadiusDistFromPlane - thisZ)/ (nextZ - thisZ);
                
                cvf::Vec3d intersectPoint =  part[vxIdx] +  fraction * (part[vxIdx+1] - part[vxIdx]);
                currentIntersectingWpPart.push_back(intersectPoint);

                intersectingWellPathParts.push_back(currentIntersectingWpPart);
                currentIntersectingWpPart.clear();
                continue;
            }

            if ( thisAbsZ >= wellRadius && nextAbsZ < wellRadius )  // Going in
            {
                double wellRadiusDistFromPlane = thisZ > 0 ? wellRadius: -wellRadius;

                double fraction = (wellRadiusDistFromPlane - thisZ)/ (nextZ - thisZ);

                cvf::Vec3d intersectPoint =  part[vxIdx] +  fraction * (part[vxIdx+1] - part[vxIdx]);
                currentIntersectingWpPart.push_back(intersectPoint);
                continue;
            }

        }

        // Add last point if it is within the radius

        if (part.size() > 1 && fabs(part.back().z()) < wellRadius)  
        {
            currentIntersectingWpPart.push_back(part.back());
        }

        if ( currentIntersectingWpPart.size() )
        {
            intersectingWellPathParts.push_back(currentIntersectingWpPart);
        }
    }

    // Find the StimPlan cells touched by the intersecting well path parts 

    for ( size_t cIdx = 0; cIdx < stpCellPolygons.size(); ++ cIdx )
    {
        const std::vector<cvf::Vec3d>& cellPolygon = stpCellPolygons[cIdx];
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
