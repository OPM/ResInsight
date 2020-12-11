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

#include "RiaGuiApplication.h"
#include "RiaOffshoreSphericalCoords.h"

#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigHexIntersectionTools.h"
#include "RigMainGrid.h"
#include "RigWellPath.h"

#include "Rim3dView.h"
#include "RimEclipseView.h"
#include "RimGeoMechView.h"
#include "RimModeledWellPath.h"
#include "RimWellPath.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathLateralGeometryDef.h"
#include "RimWellPathTarget.h"

#include "RiuViewerCommands.h"

#include "RivFemPartGeometryGenerator.h"
#include "RivFemPickSourceInfo.h"
#include "RivSourceInfo.h"
#include "RivWellPathSourceInfo.h"

#include "cafDisplayCoordTransform.h"
#include "cafSelectionManager.h"

#include "cvfStructGridGeometryGenerator.h"

#include <QDebug>

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicCreateWellTargetsPickEventHandler::RicCreateWellTargetsPickEventHandler(
    gsl::not_null<RimWellPathGeometryDefInterface*> wellGeometryDef )
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
    RiaGuiApplication::instance()->setOverrideCursor( Qt::CrossCursor );
    Ric3dViewPickEventHandler::registerAsPickEventHandler();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateWellTargetsPickEventHandler::notifyUnregistered()
{
    RiaGuiApplication::instance()->restoreOverrideCursor();
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
        auto firstPickItem = eventObject.m_pickItemInfos.front();

        auto intersectionPointInDomain =
            rimView->displayCoordTransform()->transformToDomainCoord( firstPickItem.globalPickedPoint() );

        double azimuth     = std::numeric_limits<double>::infinity();
        double inclination = std::numeric_limits<double>::infinity();

        auto wellPathSourceInfo = dynamic_cast<const RivWellPathSourceInfo*>( firstPickItem.sourceInfo() );

        if ( isValidWellPathSourceObject( wellPathSourceInfo ) )
        {
            calculateWellPathGeometryAtPickPoint( firstPickItem,
                                                  wellPathSourceInfo,
                                                  intersectionPointInDomain,
                                                  &targetPointInDomain,
                                                  &azimuth,
                                                  &inclination );
        }
        else if ( isGridSourceObject( firstPickItem.sourceInfo() ) )
        {
            targetPointInDomain = calculateGridPickPoint( rimView, firstPickItem, intersectionPointInDomain );
        }
        else
        {
            targetPointInDomain = intersectionPointInDomain;
        }

        if ( auto wellPathGeometryDef = dynamic_cast<RimWellPathGeometryDef*>( m_geometryToAddTargetsTo.p() );
             wellPathGeometryDef )
        {
            addNewTargetToModeledWellPath( firstPickItem,
                                           wellPathGeometryDef,
                                           intersectionPointInDomain,
                                           targetPointInDomain,
                                           azimuth,
                                           inclination );
        }
        else if ( auto wellPathLateralGeometryDef =
                      dynamic_cast<RimWellPathLateralGeometryDef*>( m_geometryToAddTargetsTo.p() );
                  wellPathLateralGeometryDef )
        {
            addNewTargetToModeledWellPathLateral( firstPickItem,
                                                  wellPathLateralGeometryDef,
                                                  intersectionPointInDomain,
                                                  targetPointInDomain,
                                                  azimuth,
                                                  inclination );
        }

        return true; // Todo: See if we really should eat the event instead
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateWellTargetsPickEventHandler::calculateAzimuthAndInclinationAtMd( double measuredDepth,
                                                                               gsl::not_null<const RigWellPath*> wellPathGeometry,
                                                                               double* azimuth,
                                                                               double* inclination ) const
{
    int  mdIndex = -1;
    auto mdList  = wellPathGeometry->measuredDepths();

    for ( int i = 0; i < (int)mdList.size(); i++ )
    {
        if ( mdList[i] > measuredDepth )
        {
            mdIndex = i - 1;
            break;
        }
    }

    auto ptList = wellPathGeometry->wellPathPoints();
    if ( mdIndex > 0 && mdIndex < (int)ptList.size() - 2 )
    {
        auto v1 = cvf::Vec3d( ptList[mdIndex - 1] );
        auto v2 = cvf::Vec3d( ptList[mdIndex] );
        auto v3 = cvf::Vec3d( ptList[mdIndex + 1] );
        auto v4 = cvf::Vec3d( ptList[mdIndex + 2] );

        auto v21 = v2 - v1;
        auto v32 = v3 - v2;
        auto v43 = v4 - v3;

        v21.normalize();
        v32.normalize();
        v43.normalize();

        auto v13mean = ( v21 + v32 ) / 2;
        auto v24mean = ( v32 + v43 ) / 2;

        double weight = ( measuredDepth - mdList[mdIndex] ) / ( mdList[mdIndex + 1] - mdList[mdIndex] );
        auto   vTan   = v13mean * weight + v24mean * ( 1 - weight );

        RiaOffshoreSphericalCoords coords( vTan );
        *azimuth     = coords.azi();
        *inclination = coords.inc();
        return true;
    }

    *azimuth     = 0.0;
    *inclination = 0.0;
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateWellTargetsPickEventHandler::calculateWellPathGeometryAtPickPoint(
    const RiuPickItemInfo&                      pickItem,
    gsl::not_null<const RivWellPathSourceInfo*> wellPathSourceInfo,
    const cvf::Vec3d&                           intersectionPointInDomain,
    gsl::not_null<cvf::Vec3d*>                  targetPointInDomain,
    gsl::not_null<double*>                      azimuth,
    gsl::not_null<double*>                      inclination ) const
{
    *targetPointInDomain = wellPathSourceInfo->closestPointOnCenterLine( pickItem.faceIdx(), intersectionPointInDomain );

    bool doSetAzimuthAndInclination = false;

    auto wellPathGeometry = wellPathSourceInfo->wellPath()->wellPathGeometry();
    if ( wellPathGeometry )
    {
        double md = wellPathSourceInfo->measuredDepth( pickItem.faceIdx(), intersectionPointInDomain );

        doSetAzimuthAndInclination = calculateAzimuthAndInclinationAtMd( md, wellPathGeometry, azimuth, inclination );
        double rkbDiff             = wellPathGeometry->rkbDiff();
        auto   wellPathGeometryDef = dynamic_cast<RimWellPathGeometryDef*>( m_geometryToAddTargetsTo.p() );
        if ( wellPathGeometryDef && wellPathGeometryDef->airGap() == 0.0 &&
             rkbDiff != std::numeric_limits<double>::infinity() )
        {
            wellPathGeometryDef->setAirGap( rkbDiff );
        }
    }
    return doSetAzimuthAndInclination;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RicCreateWellTargetsPickEventHandler::calculateGridPickPoint( gsl::not_null<const Rim3dView*> rimView,
                                                                         const RiuPickItemInfo&          pickItem,
                                                                         const cvf::Vec3d& intersectionPointInDomain ) const
{
    auto targetPointInDomain = intersectionPointInDomain;

    cvf::Vec3d domainRayOrigin = rimView->displayCoordTransform()->transformToDomainCoord( pickItem.globalRayOrigin() );
    cvf::Vec3d domainRayEnd    = targetPointInDomain + ( targetPointInDomain - domainRayOrigin );

    cvf::Vec3d hexElementIntersection = findHexElementIntersection( rimView, pickItem, domainRayOrigin, domainRayEnd );
    CVF_TIGHT_ASSERT( !hexElementIntersection.isUndefined() );
    if ( !hexElementIntersection.isUndefined() )
    {
        targetPointInDomain = hexElementIntersection;
    }
    return targetPointInDomain;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateWellTargetsPickEventHandler::addNewTargetToModeledWellPath( const RiuPickItemInfo& pickItem,
                                                                          gsl::not_null<RimWellPathGeometryDef*> wellPathGeometryDef,
                                                                          const cvf::Vec3d& intersectionPointInDomain,
                                                                          const cvf::Vec3d& targetPointInDomain,
                                                                          double            azimuth,
                                                                          double            inclination )
{
    if ( !m_geometryToAddTargetsTo->firstActiveTarget() )
    {
        wellPathGeometryDef->setReferencePointXyz( targetPointInDomain );

        auto wellPathSourceInfo = dynamic_cast<const RivWellPathSourceInfo*>( pickItem.sourceInfo() );
        if ( wellPathSourceInfo )
        {
            double mdAtFirstTarget = wellPathSourceInfo->measuredDepth( pickItem.faceIdx(), intersectionPointInDomain );

            RimModeledWellPath* modeledWellPath = dynamic_cast<RimModeledWellPath*>( wellPathSourceInfo->wellPath() );
            if ( modeledWellPath )
            {
                mdAtFirstTarget += modeledWellPath->geometryDefinition()->mdAtFirstTarget();
            }

            wellPathGeometryDef->setMdAtFirstTarget( mdAtFirstTarget );
        }
    }

    cvf::Vec3d referencePoint      = wellPathGeometryDef->referencePointXyz();
    cvf::Vec3d relativeTargetPoint = targetPointInDomain - referencePoint;

    RimWellPathTarget* newTarget = new RimWellPathTarget;

    bool doSetAzimuthAndInclination = azimuth != std::numeric_limits<double>::infinity() &&
                                      inclination != std::numeric_limits<double>::infinity();
    if ( doSetAzimuthAndInclination )
    {
        newTarget->setAsPointXYZAndTangentTarget( cvf::Vec3d( relativeTargetPoint.x(),
                                                              relativeTargetPoint.y(),
                                                              relativeTargetPoint.z() ),
                                                  azimuth,
                                                  inclination );
    }
    else
    {
        newTarget->setAsPointTargetXYD(
            cvf::Vec3d( relativeTargetPoint.x(), relativeTargetPoint.y(), -relativeTargetPoint.z() ) );
    }

    m_geometryToAddTargetsTo->insertTarget( nullptr, newTarget );
    m_geometryToAddTargetsTo->updateConnectedEditors();
    m_geometryToAddTargetsTo->updateWellPathVisualization( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateWellTargetsPickEventHandler::addNewTargetToModeledWellPathLateral(
    const RiuPickItemInfo&                        pickItem,
    gsl::not_null<RimWellPathLateralGeometryDef*> wellPathLateralGeometryDef,
    const cvf::Vec3d&                             intersectionPointInDomain,
    const cvf::Vec3d&                             targetPointInDomain,
    double                                        azimuth,
    double                                        inclination )
{
    auto wellPathSourceInfo = dynamic_cast<const RivWellPathSourceInfo*>( pickItem.sourceInfo() );
    if ( wellPathSourceInfo )
    {
        double mdAtConnection = wellPathSourceInfo->measuredDepth( pickItem.faceIdx(), intersectionPointInDomain );

        wellPathLateralGeometryDef->setParentGeometry( wellPathSourceInfo->wellPath()->wellPathGeometry() );
        wellPathLateralGeometryDef->setMdAtConnection( mdAtConnection );
    }
    cvf::Vec3d referencePoint      = wellPathLateralGeometryDef->referencePointXyz();
    cvf::Vec3d relativeTargetPoint = targetPointInDomain - referencePoint;

    RimWellPathTarget* newTarget = new RimWellPathTarget;

    bool doSetAzimuthAndInclination = azimuth != std::numeric_limits<double>::infinity() &&
                                      inclination != std::numeric_limits<double>::infinity();
    if ( doSetAzimuthAndInclination )
    {
        newTarget->setAsPointXYZAndTangentTarget( cvf::Vec3d( relativeTargetPoint.x(),
                                                              relativeTargetPoint.y(),
                                                              relativeTargetPoint.z() ),
                                                  azimuth,
                                                  inclination );
    }
    else
    {
        newTarget->setAsPointTargetXYD(
            cvf::Vec3d( relativeTargetPoint.x(), relativeTargetPoint.y(), -relativeTargetPoint.z() ) );
    }

    m_geometryToAddTargetsTo->insertTarget( nullptr, newTarget );
    m_geometryToAddTargetsTo->updateConnectedEditors();
    m_geometryToAddTargetsTo->updateWellPathVisualization( false );
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
bool RicCreateWellTargetsPickEventHandler::isValidWellPathSourceObject( const RivWellPathSourceInfo* wellPathSourceInfo )
{
    return wellPathSourceInfo && wellPathSourceInfo->wellPath() && wellPathSourceInfo->wellPath()->wellPathGeometry();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RicCreateWellTargetsPickEventHandler::findHexElementIntersection( gsl::not_null<const Rim3dView*> view,
                                                                             const RiuPickItemInfo&          pickItem,
                                                                             const cvf::Vec3d& domainRayOrigin,
                                                                             const cvf::Vec3d& domainRayEnd )
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

            const RimEclipseView* eclipseView = dynamic_cast<const RimEclipseView*>( view.get() );
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

            const RimGeoMechView* geoMechView = dynamic_cast<const RimGeoMechView*>( view.get() );
            if ( geoMechView && geoMechView->femParts() )
            {
                const RigFemPart* femPart = geoMechView->femParts()->part( femPartIndex );
                RigElementType    elType  = femPart->elementType( elementIndex );

                if ( elType == HEX8 || elType == HEX8P )
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
        RigHexIntersectionTools::lineHexCellIntersection( domainRayOrigin,
                                                          domainRayEnd,
                                                          cornerVertices.data(),
                                                          cellIndex,
                                                          &intersectionInfo );
        if ( !intersectionInfo.empty() )
        {
            // Sort intersection on distance to ray origin
            CVF_ASSERT( intersectionInfo.size() > 1 );
            std::sort( intersectionInfo.begin(),
                       intersectionInfo.end(),
                       [&domainRayOrigin]( const HexIntersectionInfo& lhs, const HexIntersectionInfo& rhs ) {
                           return ( lhs.m_intersectionPoint - domainRayOrigin ).lengthSquared() <
                                  ( rhs.m_intersectionPoint - domainRayOrigin ).lengthSquared();
                       } );
            const double eps             = 1.0e-2;
            cvf::Vec3d   intersectionRay = intersectionInfo.back().m_intersectionPoint -
                                         intersectionInfo.front().m_intersectionPoint;
            cvf::Vec3d newPoint = intersectionInfo.front().m_intersectionPoint + intersectionRay * eps;
            CVF_ASSERT( RigHexIntersectionTools::isPointInCell( newPoint, cornerVertices.data() ) );
            return newPoint;
        }
    }
    return cvf::Vec3d::UNDEFINED;
}
