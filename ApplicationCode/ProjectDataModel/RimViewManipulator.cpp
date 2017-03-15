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

#include "RimViewManipulator.h"

#include "RigMainGrid.h"

#include "RimEclipseView.h"
#include "RimView.h"

#include "RiuViewer.h"

#include "cvfBase.h"
#include "cvfBoundingBox.h"
#include "cvfCamera.h"
#include "cvfMatrix4.h"
#include "cvfScene.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewManipulator::applySourceViewCameraOnDestinationViews(RimView* sourceView, std::vector<RimView*>& destinationViews)
{
    cvf::Vec3d sourceCamUp;
    cvf::Vec3d sourceCamEye;
    cvf::Vec3d sourceCamViewRefPoint;
    sourceView->viewer()->mainCamera()->toLookAt(&sourceCamEye, &sourceCamViewRefPoint, &sourceCamUp);

    cvf::Vec3d sourceCamGlobalEye = sourceCamEye;
    cvf::Vec3d sourceCamGlobalViewRefPoint = sourceCamViewRefPoint;

    // Source bounding box in global coordinates including scaleZ
    cvf::BoundingBox sourceSceneBB = sourceView->viewer()->currentScene()->boundingBox();

    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(sourceView);
    if (eclipseView && eclipseView->mainGrid())
    {
        cvf::Vec3d offset = eclipseView->mainGrid()->displayModelOffset();
        offset.z() *= eclipseView->scaleZ();

        sourceCamGlobalEye += offset;
        sourceCamGlobalViewRefPoint += offset;

        cvf::Mat4d trans;
        trans.setTranslation(offset);
        sourceSceneBB.transform(trans);
    }

    for (RimView* destinationView : destinationViews)
    {
        if (!destinationView) continue;

        destinationView->isPerspectiveView = sourceView->isPerspectiveView;

        RiuViewer* destinationViewer = destinationView->viewer();
        if (destinationViewer)
        {
            destinationViewer->enableParallelProjection(!sourceView->isPerspectiveView);

            // Destination bounding box in global coordinates including scaleZ
            cvf::BoundingBox destSceneBB = destinationViewer->currentScene()->boundingBox();

            RimEclipseView* destEclipseView = dynamic_cast<RimEclipseView*>(destinationView);
            if (destEclipseView && destEclipseView->mainGrid())
            {
                cvf::Vec3d destOffset = destEclipseView->mainGrid()->displayModelOffset();
                destOffset.z() *= destEclipseView->scaleZ();

                cvf::Vec3d destinationCamEye = sourceCamGlobalEye - destOffset;
                cvf::Vec3d destinationCamViewRefPoint = sourceCamGlobalViewRefPoint - destOffset;

                cvf::Mat4d trans;
                trans.setTranslation(destOffset);
                destSceneBB.transform(trans);

                if (isBoundingBoxesOverlappingOrClose(sourceSceneBB, destSceneBB))
                {
                    destinationViewer->mainCamera()->setFromLookAt(destinationCamEye, destinationCamViewRefPoint, sourceCamUp);
                }
                else
                {
                    // Fallback using values from source camera
                    destinationViewer->mainCamera()->setFromLookAt(sourceCamEye, sourceCamViewRefPoint, sourceCamUp);
                }
            }
            else
            {
                if (isBoundingBoxesOverlappingOrClose(sourceSceneBB, destSceneBB))
                {
                    destinationViewer->mainCamera()->setFromLookAt(sourceCamGlobalEye, sourceCamGlobalViewRefPoint, sourceCamUp);
                }
                else
                {
                    // Fallback using values from source camera
                    destinationViewer->mainCamera()->setFromLookAt(sourceCamEye, sourceCamViewRefPoint, sourceCamUp);
                }
            }

            destinationViewer->updateParallelProjectionSettings(sourceView->viewer());

            destinationViewer->update();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimViewManipulator::isBoundingBoxesOverlappingOrClose(const cvf::BoundingBox& sourceBB, const cvf::BoundingBox& destBB)
{
    if (!sourceBB.isValid() || !destBB.isValid()) return false;

    if (sourceBB.intersects(destBB)) return true;

    double largestExtent = sourceBB.extent().length();
    if (destBB.extent().length() > largestExtent)
    {
        largestExtent = destBB.extent().length();
    }

    double centerDist = (sourceBB.center() - destBB.center()).length();
    if (centerDist < largestExtent * 5)
    {
        return true;
    }

    return false;
}
