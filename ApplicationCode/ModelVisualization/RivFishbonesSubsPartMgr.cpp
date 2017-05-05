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

#include "RivFishbonesSubsPartMgr.h"

#include "RigWellPath.h"

#include "RimFishbonesMultipleSubs.h"
#include "RimWellPath.h"

#include "RivObjectSourceInfo.h"
#include "RivPipeGeometryGenerator.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfTransform.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivFishbonesSubsPartMgr::RivFishbonesSubsPartMgr(RimFishbonesMultipleSubs* subs)
    : m_rimFishbonesSubs(subs)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivFishbonesSubsPartMgr::~RivFishbonesSubsPartMgr()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFishbonesSubsPartMgr::appendGeometryPartsToModel(cvf::ModelBasicList* model, caf::DisplayCoordTransform* displayCoordTransform, double characteristicCellSize)
{
    clearGeometryCache();

    if (!m_rimFishbonesSubs->isChecked()) return;

    if (m_parts.size() == 0)
    {
        buildParts(displayCoordTransform, characteristicCellSize);
    }

    for (auto part : m_parts)
    {
        model->addPart(part.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFishbonesSubsPartMgr::clearGeometryCache()
{
    m_parts.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFishbonesSubsPartMgr::buildParts(caf::DisplayCoordTransform* displayCoordTransform, double characteristicCellSize)
{
    std::vector<double> locationOfSubs = m_rimFishbonesSubs->locationOfSubs();

    RimWellPath* wellPath = nullptr;
    m_rimFishbonesSubs->firstAncestorOrThisOfTypeAsserted(wellPath);

    RigWellPath* rigWellPath = wellPath->wellPathGeometry();

    RivPipeGeometryGenerator geoGenerator;
    geoGenerator.setRadius(m_rimFishbonesSubs->tubingRadius());

    for (size_t instanceIndex = 0; instanceIndex < locationOfSubs.size(); instanceIndex++)
    {
        double measuredDepth = locationOfSubs[instanceIndex];

        cvf::Vec3d position = rigWellPath->interpolatedPointAlongWellPath(measuredDepth);

        cvf::Vec3d p1 = cvf::Vec3d::UNDEFINED;
        cvf::Vec3d p2 = cvf::Vec3d::UNDEFINED;
        rigWellPath->twoClosestPoints(position, &p1, &p2);

        if (!p1.isUndefined() && !p2.isUndefined())
        {
            std::vector<double> lateralLengths = m_rimFishbonesSubs->lateralLengths();
            cvf::Mat4d buildAngleRotationMatrix;

            for (size_t i = 0; i < lateralLengths.size(); i++)
            {
                cvf::Vec3d lateralDirection;
                {
                    cvf::Vec3d alongWellPath = (p2 - p1).getNormalized();

                    cvf::Vec3d lateralInitialDirection = cvf::Vec3d::Z_AXIS;

                    if (RivFishbonesSubsPartMgr::closestMainAxis(alongWellPath) == cvf::Vec3d::Z_AXIS)
                    {
                        // Use x-axis if well path is heading close to z-axis
                        lateralInitialDirection = cvf::Vec3d::X_AXIS;
                    }

                    {
                        double intialRotationAngle = m_rimFishbonesSubs->rotationAngle(instanceIndex);
                        double lateralOffsetDegrees = 360.0 / lateralLengths.size();

                        double lateralOffsetRadians = cvf::Math::toRadians(intialRotationAngle + lateralOffsetDegrees * i);

                        cvf::Mat4d lateralOffsetMatrix = cvf::Mat4d::fromRotation(alongWellPath, lateralOffsetRadians);

                        lateralInitialDirection = lateralInitialDirection.getTransformedVector(lateralOffsetMatrix);
                    }

                    cvf::Vec3d rotationAxis;
                    rotationAxis.cross(alongWellPath, lateralInitialDirection);

                    double exitAngleRadians = cvf::Math::toRadians(m_rimFishbonesSubs->exitAngle());
                    cvf::Mat4d lateralRotationMatrix = cvf::Mat4d::fromRotation(rotationAxis, exitAngleRadians);

                    lateralDirection = alongWellPath.getTransformedVector(lateralRotationMatrix);

                    double buildAngleRadians = cvf::Math::toRadians(m_rimFishbonesSubs->buildAngle());
                    buildAngleRotationMatrix = cvf::Mat4d::fromRotation(rotationAxis, buildAngleRadians);
                }

                std::vector<cvf::Vec3d> lateralDomainCoords = 
                    RivFishbonesSubsPartMgr::computeLateralCoords(position, lateralLengths[i], lateralDirection, buildAngleRotationMatrix);

                std::vector<cvf::Vec3d> displayCoords;
                for (auto domainCoord : lateralDomainCoords)
                {
                    displayCoords.push_back(displayCoordTransform->transformToDisplayCoord(domainCoord));
                }

                cylinderWithCenterLineParts(&m_parts, displayCoords, wellPath->wellPathColor(), wellPath->combinedScaleFactor() * characteristicCellSize * 0.5);

                cvf::ref<RivObjectSourceInfo> objectSourceInfo = new RivObjectSourceInfo(m_rimFishbonesSubs);

                for (auto p : m_parts)
                {
                    p->setSourceInfo(objectSourceInfo.p());
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RivFishbonesSubsPartMgr::computeLateralCoords(const cvf::Vec3d& startCoord, double lateralLength, const cvf::Vec3d& lateralStartDirection, const cvf::Mat4d& buildAngleRotationMatrix)
{
    std::vector<cvf::Vec3d> coords;

    cvf::Vec3d lateralDirection(lateralStartDirection);

    // Compute coordinates along the lateral by modifying the lateral direction by the build angle for 
    // every unit vector along the lateral
    cvf::Vec3d accumulatedPosition = startCoord;
    double accumulatedLength = 0.0;
    while (accumulatedLength < lateralLength)
    {
        coords.push_back(accumulatedPosition);

        double delta = 1.0;

        if (lateralLength - accumulatedLength < 1.0)
        {
            delta = lateralLength - accumulatedLength;
        }

        accumulatedPosition += delta * lateralDirection;

        // Modify the lateral direction by the build angle for each unit vector
        lateralDirection = lateralDirection.getTransformedVector(buildAngleRotationMatrix);

        accumulatedLength += delta;
    }

    // Add the last accumulated position if it is not present
    if (coords.back() != accumulatedPosition)
    {
        coords.push_back(accumulatedPosition);
    }

    return coords;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivFishbonesSubsPartMgr::cylinderWithCenterLineParts(cvf::Collection<cvf::Part>* destinationParts, const std::vector<cvf::Vec3d>& centerCoords, const cvf::Color3f& color, double radius)
{
    cvf::ref<RivPipeGeometryGenerator> geoGenerator = new RivPipeGeometryGenerator;
    geoGenerator->setRadius(radius);
    geoGenerator->setCrossSectionVertexCount(12);

    cvf::ref<cvf::Vec3dArray> cvfCoords = new cvf::Vec3dArray(centerCoords);
    geoGenerator->setPipeCenterCoords(cvfCoords.p());

    cvf::ref<cvf::DrawableGeo> surfaceGeo = geoGenerator->createPipeSurface();
    if (surfaceGeo.notNull())
    {
        cvf::Part* part = new cvf::Part;
        part->setDrawable(surfaceGeo.p());

        caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(color), caf::PO_1);
        cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

        part->setEffect(eff.p());

        destinationParts->push_back(part);
    }

    cvf::ref<cvf::DrawableGeo> centerLineGeo = geoGenerator->createCenterLine();
    if (centerLineGeo.notNull())
    {
        cvf::Part* part = new cvf::Part;
        part->setDrawable(centerLineGeo.p());

        caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(color), caf::PO_1);
        cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

        part->setEffect(eff.p());

        destinationParts->push_back(part);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RivFishbonesSubsPartMgr::closestMainAxis(const cvf::Vec3d& vec)
{
    size_t maxComponent = 0;
    double maxValue = cvf::Math::abs(vec.x());
    if (cvf::Math::abs(vec.y()) > maxValue)
    {
        maxComponent = 1;
        maxValue = cvf::Math::abs(vec.y());
    }

    if (cvf::Math::abs(vec.z()) > maxValue)
    {
        maxComponent = 2;
        maxValue = cvf::Math::abs(vec.z());
    }

    if (maxComponent == 0)
    {
        return cvf::Vec3d::X_AXIS;
    }
    else if (maxComponent == 1)
    {
        return cvf::Vec3d::Y_AXIS;
    }
    else
    {
        return cvf::Vec3d::Z_AXIS;
    }
}

