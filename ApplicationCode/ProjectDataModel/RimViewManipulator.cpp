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

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimEclipseView.h"

#include "RiuViewer.h"

#include "cvfBoundingBox.h"
#include "cvfCamera.h"
#include "cvfMatrix4.h"
#include "cvfScene.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewManipulator::applySourceViewCameraOnDestinationViews( RimGridView*               sourceView,
                                                                  std::vector<RimGridView*>& destinationViews )
{
    bool       setPointOfInterest = false;
    cvf::Vec3d sourceCamUp;
    cvf::Vec3d sourceCamEye;
    cvf::Vec3d sourceCamViewRefPoint;
    cvf::Vec3d sourcePointOfInterest;
    sourceView->viewer()->mainCamera()->toLookAt( &sourceCamEye, &sourceCamViewRefPoint, &sourceCamUp );

    cvf::Vec3d sourceCamGlobalEye          = sourceCamEye;
    cvf::Vec3d sourceCamGlobalViewRefPoint = sourceCamViewRefPoint;
    if ( sourceView->viewer()->getNavigationPolicy() != nullptr )
    {
        setPointOfInterest    = true;
        sourcePointOfInterest = sourceView->viewer()->pointOfInterest();
    }

    // Source bounding box in global coordinates including scaleZ
    cvf::BoundingBox sourceSceneBB = sourceView->viewer()->currentScene()->boundingBox();
    {
        cvf::Vec3d offset          = cvf::Vec3d::ZERO;
        RimCase*   sourceOwnerCase = sourceView->ownerCase();
        if ( sourceOwnerCase )
        {
            offset = sourceOwnerCase->displayModelOffset();
            offset.z() *= sourceView->scaleZ();
        }

        sourceCamGlobalEye += offset;
        sourceCamGlobalViewRefPoint += offset;
        if ( setPointOfInterest ) sourcePointOfInterest += offset;

        cvf::Mat4d trans;
        trans.setTranslation( offset );
        sourceSceneBB.transform( trans );
    }

    for ( RimGridView* destinationView : destinationViews )
    {
        if ( !destinationView ) continue;

        destinationView->isPerspectiveView = sourceView->isPerspectiveView;

        RiuViewer* destinationViewer = destinationView->viewer();
        if ( destinationViewer )
        {
            destinationViewer->enableParallelProjection( !sourceView->isPerspectiveView );

            // Destination bounding box in global coordinates including scaleZ
            cvf::BoundingBox destSceneBB                = destinationViewer->currentScene()->boundingBox();
            cvf::Vec3d       destinationCamEye          = sourceCamGlobalEye;
            cvf::Vec3d       destinationCamViewRefPoint = sourceCamGlobalViewRefPoint;
            cvf::Vec3d       offset                     = cvf::Vec3d::ZERO;

            RimCase* destinationOwnerCase = destinationView->ownerCase();
            if ( destinationOwnerCase )
            {
                offset = destinationOwnerCase->displayModelOffset();
                offset.z() *= destinationView->scaleZ();
            }

            destinationCamEye -= offset;
            destinationCamViewRefPoint -= offset;

            cvf::Mat4d trans;
            trans.setTranslation( offset );
            destSceneBB.transform( trans );

            if ( isBoundingBoxesOverlappingOrClose( sourceSceneBB, destSceneBB ) )
            {
                destinationViewer->mainCamera()->setFromLookAt( destinationCamEye,
                                                                destinationCamViewRefPoint,
                                                                sourceCamUp );
            }
            else
            {
                // Fallback using values from source camera
                destinationViewer->mainCamera()->setFromLookAt( sourceCamEye, sourceCamViewRefPoint, sourceCamUp );
            }

            if ( setPointOfInterest )
            {
                cvf::Vec3d pointOfInterest = sourcePointOfInterest;
                pointOfInterest -= offset;
                destinationViewer->updateParallelProjectionHeightFromMoveZoom( pointOfInterest );
                destinationViewer->updateParallelProjectionCameraPosFromPointOfInterestMove( pointOfInterest );
            }

            destinationViewer->update();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimViewManipulator::calculateEquivalentCamPosOffset( Rim3dView* sourceView, Rim3dView* destView )
{
    cvf::Vec3d soffset         = cvf::Vec3d::ZERO;
    RimCase*   sourceOwnerCase = sourceView->ownerCase();
    if ( sourceOwnerCase )
    {
        soffset = sourceOwnerCase->displayModelOffset();
        soffset.z() *= sourceView->scaleZ();
    }

    cvf::Vec3d doffset = cvf::Vec3d::ZERO;

    RimCase* destinationOwnerCase = destView->ownerCase();
    if ( destinationOwnerCase )
    {
        doffset = destinationOwnerCase->displayModelOffset();
        doffset.z() *= sourceView->scaleZ();
    }

    return soffset - doffset;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewManipulator::isBoundingBoxesOverlappingOrClose( const cvf::BoundingBox& sourceBB,
                                                            const cvf::BoundingBox& destBB )
{
    if ( !sourceBB.isValid() || !destBB.isValid() ) return false;

    if ( sourceBB.intersects( destBB ) ) return true;

    double largestExtent = sourceBB.extent().length();
    if ( destBB.extent().length() > largestExtent )
    {
        largestExtent = destBB.extent().length();
    }

    double centerDist = ( sourceBB.center() - destBB.center() ).length();
    if ( centerDist < largestExtent * 5 )
    {
        return true;
    }

    return false;
}
