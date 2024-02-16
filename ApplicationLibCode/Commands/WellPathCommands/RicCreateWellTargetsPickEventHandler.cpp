/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RicCreateWellTargetsPickEventHandler.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigHexIntersectionTools.h"
#include "RigMainGrid.h"
#include "RigWellPath.h"
#include "RigWellPathGeometryTools.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimEclipseView.h"
#include "RimGeoMechView.h"
#include "RimModeledWellPath.h"
#include "RimWellPath.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathTarget.h"

#include "RiuViewerCommands.h"

#include "RivFemPartGeometryGenerator.h"
#include "RivFemPickSourceInfo.h"
#include "RivSourceInfo.h"
#include "RivWellPathSourceInfo.h"

#include "cafDisplayCoordTransform.h"
#include "cafSelectionManager.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicCreateWellTargetsPickEventHandler::RicCreateWellTargetsPickEventHandler( gsl::not_null<RimWellPathGeometryDef*> wellGeometryDef )
    : m_geometryToAddTargetsTo( wellGeometryDef )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicCreateWellTargetsPickEventHandler::~RicCreateWellTargetsPickEventHandler()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateWellTargetsPickEventHandler::registerAsPickEventHandler()
{
    QApplication::setOverrideCursor( Qt::CrossCursor );
    Ric3dViewPickEventHandler::registerAsPickEventHandler();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateWellTargetsPickEventHandler::notifyUnregistered()
{
    QApplication::restoreOverrideCursor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateWellTargetsPickEventHandler::handle3dPickEvent( const Ric3dPickEvent& eventObject )
{
    if ( m_geometryToAddTargetsTo )
    {
        Rim3dView* rimView             = eventObject.m_view;
        cvf::Vec3d targetPointInDomain = cvf::Vec3d::ZERO;

        // If clicked on an other well path, snap target point to well path center line
        auto firstPickItem      = eventObject.m_pickItemInfos.front();
        auto wellPathSourceInfo = dynamic_cast<const RivWellPathSourceInfo*>( firstPickItem.sourceInfo() );

        auto   intersectionPointInDomain  = rimView->displayCoordTransform()->transformToDomainCoord( firstPickItem.globalPickedPoint() );
        bool   doSetAzimuthAndInclination = false;
        double azimuth                    = 0.0;
        double inclination                = 0.0;

        if ( wellPathSourceInfo && wellPathSourceInfo->wellPath() && wellPathSourceInfo->wellPath()->wellPathGeometry() )
        {
            auto wellPathGeometry = wellPathSourceInfo->wellPath()->wellPathGeometry();

            targetPointInDomain = wellPathSourceInfo->closestPointOnCenterLine( firstPickItem.faceIdx(), intersectionPointInDomain );
            double md           = wellPathSourceInfo->measuredDepth( firstPickItem.faceIdx(), intersectionPointInDomain );

            {
                const auto [az, inc]       = RigWellPathGeometryTools::calculateAzimuthAndInclinationAtMd( md, wellPathGeometry );
                azimuth                    = az;
                inclination                = inc;
                doSetAzimuthAndInclination = true;
            }

            double rkbDiff = wellPathGeometry->rkbDiff();
            if ( m_geometryToAddTargetsTo->airGap() == 0.0 && rkbDiff != std::numeric_limits<double>::infinity() )
            {
                m_geometryToAddTargetsTo->setAirGap( rkbDiff );
            }
        }
        else if ( isGridSourceObject( firstPickItem.sourceInfo() ) )
        {
            targetPointInDomain        = intersectionPointInDomain;
            doSetAzimuthAndInclination = false;

            cvf::Vec3d domainRayOrigin = rimView->displayCoordTransform()->transformToDomainCoord( firstPickItem.globalRayOrigin() );

            auto         rayVector        = ( targetPointInDomain - domainRayOrigin );
            const double minimumRayLength = rimView->ownerCase()->characteristicCellSize() * 2;
            if ( rayVector.length() < minimumRayLength )
            {
                rayVector = rayVector.getNormalized() * minimumRayLength;
            }
            cvf::Vec3d domainRayEnd = targetPointInDomain + rayVector;

            cvf::Vec3d hexElementIntersection = findHexElementIntersection( rimView, firstPickItem, domainRayOrigin, domainRayEnd );
            CVF_TIGHT_ASSERT( !hexElementIntersection.isUndefined() );
            if ( !hexElementIntersection.isUndefined() )
            {
                targetPointInDomain = hexElementIntersection;
            }
        }
        else
        {
            targetPointInDomain        = intersectionPointInDomain;
            doSetAzimuthAndInclination = false;
        }

        if ( !m_geometryToAddTargetsTo->firstActiveTarget() )
        {
            m_geometryToAddTargetsTo->setReferencePointXyz( targetPointInDomain );

            if ( wellPathSourceInfo )
            {
                double mdAtFirstTarget = wellPathSourceInfo->measuredDepth( firstPickItem.faceIdx(), intersectionPointInDomain );

                RimModeledWellPath* modeledWellPath = dynamic_cast<RimModeledWellPath*>( wellPathSourceInfo->wellPath() );
                if ( modeledWellPath )
                {
                    mdAtFirstTarget += modeledWellPath->geometryDefinition()->mdAtFirstTarget();
                }

                m_geometryToAddTargetsTo->setMdAtFirstTarget( mdAtFirstTarget );
            }
        }

        cvf::Vec3d referencePoint     = m_geometryToAddTargetsTo->anchorPointXyz();
        cvf::Vec3d relativeTagetPoint = targetPointInDomain - referencePoint;

        RimWellPathTarget* newTarget = new RimWellPathTarget;

        if ( doSetAzimuthAndInclination )
        {
            newTarget->setAsPointXYZAndTangentTarget( cvf::Vec3d( relativeTagetPoint.x(), relativeTagetPoint.y(), relativeTagetPoint.z() ),
                                                      azimuth,
                                                      inclination );
        }
        else
        {
            newTarget->setAsPointTargetXYD( cvf::Vec3d( relativeTagetPoint.x(), relativeTagetPoint.y(), -relativeTagetPoint.z() ) );
        }

        m_geometryToAddTargetsTo->insertTarget( nullptr, newTarget );

        m_geometryToAddTargetsTo->updateConnectedEditors();
        m_geometryToAddTargetsTo->updateWellPathVisualization( true );

        return true; // Todo: See if we really should eat the event instead
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateWellTargetsPickEventHandler::isGridSourceObject( const cvf::Object* object )
{
    auto sourceInfo    = dynamic_cast<const RivSourceInfo*>( object );
    auto femSourceInfo = dynamic_cast<const RivFemPickSourceInfo*>( object );
    return sourceInfo || femSourceInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RicCreateWellTargetsPickEventHandler::findHexElementIntersection( gsl::not_null<Rim3dView*> view,
                                                                             const RiuPickItemInfo&    pickItem,
                                                                             const cvf::Vec3d&         domainRayOrigin,
                                                                             const cvf::Vec3d&         domainRayEnd )
{
    auto sourceInfo    = dynamic_cast<const RivSourceInfo*>( pickItem.sourceInfo() );
    auto femSourceInfo = dynamic_cast<const RivFemPickSourceInfo*>( pickItem.sourceInfo() );

    size_t                    cellIndex = cvf::UNDEFINED_SIZE_T;
    std::array<cvf::Vec3d, 8> cornerVertices;
    if ( sourceInfo )
    {
        size_t gridIndex = sourceInfo->gridIndex();
        if ( sourceInfo->hasCellFaceMapping() )
        {
            cellIndex = sourceInfo->m_cellFaceFromTriangleMapper->cellIndex( pickItem.faceIdx() );

            RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( view.get() );
            if ( eclipseView && eclipseView->mainGrid() )
            {
                RigGridBase* hitGrid = eclipseView->mainGrid()->gridByIndex( gridIndex );
                hitGrid->cellCornerVertices( cellIndex, cornerVertices.data() );
            }
        }
    }
    else if ( femSourceInfo )
    {
        size_t femPartIndex = femSourceInfo->femPartIndex();
        if ( femSourceInfo->triangleToElmMapper() )
        {
            size_t elementIndex = femSourceInfo->triangleToElmMapper()->elementIndex( pickItem.faceIdx() );

            RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>( view.get() );
            if ( geoMechView && geoMechView->femParts() )
            {
                RigFemPart*    femPart = geoMechView->femParts()->part( femPartIndex );
                RigElementType elType  = femPart->elementType( elementIndex );

                if ( RigFemTypes::is8NodeElement( elType ) )
                {
                    cellIndex                     = elementIndex;
                    const RigFemPartGrid* femGrid = femPart->getOrCreateStructGrid();
                    femGrid->cellCornerVertices( cellIndex, cornerVertices.data() );
                }
            }
        }
    }

    if ( cellIndex )
    {
        std::vector<HexIntersectionInfo> intersectionInfo;
        RigHexIntersectionTools::lineHexCellIntersection( domainRayOrigin, domainRayEnd, cornerVertices.data(), cellIndex, &intersectionInfo );

        if ( intersectionInfo.empty() ) return cvf::Vec3d::UNDEFINED;

        if ( intersectionInfo.size() == 1 )
        {
            return intersectionInfo.front().m_intersectionPoint;
        }

        // Sort intersection on distance to ray origin
        std::sort( intersectionInfo.begin(),
                   intersectionInfo.end(),
                   [&domainRayOrigin]( const HexIntersectionInfo& lhs, const HexIntersectionInfo& rhs ) {
                       return ( lhs.m_intersectionPoint - domainRayOrigin ).lengthSquared() <
                              ( rhs.m_intersectionPoint - domainRayOrigin ).lengthSquared();
                   } );
        const double eps             = 1.0e-2;
        cvf::Vec3d   intersectionRay = intersectionInfo.back().m_intersectionPoint - intersectionInfo.front().m_intersectionPoint;
        cvf::Vec3d   newPoint        = intersectionInfo.front().m_intersectionPoint + intersectionRay * eps;
        CVF_ASSERT( RigHexIntersectionTools::isPointInCell( newPoint, cornerVertices.data() ) );
        return newPoint;
    }

    return cvf::Vec3d::UNDEFINED;
}
