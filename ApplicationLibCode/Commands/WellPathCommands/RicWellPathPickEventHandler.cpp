/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicWellPathPickEventHandler.h"

#include "RiaApplication.h"

#include "Rim2dIntersectionView.h"
#include "Rim3dView.h"
#include "RimPerforationInterval.h"
#include "RimWellMeasurement.h"
#include "RimWellMeasurementCollection.h"
#include "RimWellMeasurementInView.h"
#include "RimWellMeasurementInViewCollection.h"
#include "RimWellPath.h"
#include "RimWellPathAttribute.h"
#include "RimWellPathAttributeCollection.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathValve.h"

#include "RiuMainWindow.h"

#include "RivExtrudedCurveIntersectionPartMgr.h"
#include "RivObjectSourceInfo.h"
#include "RivWellPathSourceInfo.h"

#include "cafDisplayCoordTransform.h"
#include "cafSelectionManager.h"
#include "cvfPart.h"
#include "cvfVector3.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicWellPathPickEventHandler* RicWellPathPickEventHandler::instance()
{
    static RicWellPathPickEventHandler* singleton = new RicWellPathPickEventHandler;
    return singleton;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWellPathPickEventHandler::handle3dPickEvent( const Ric3dPickEvent& eventObject )
{
    if ( eventObject.m_pickItemInfos.empty() ) return false;

    const caf::PdmObject* objectToSelect = nullptr;

    cvf::uint                    wellPathTriangleIndex = cvf::UNDEFINED_UINT;
    const RivWellPathSourceInfo* wellPathSourceInfo    = nullptr;

    if ( !eventObject.m_pickItemInfos.empty() )
    {
        const auto&      firstPickedItem = eventObject.m_pickItemInfos.front();
        const cvf::Part* firstPickedPart = firstPickedItem.pickedPart();

        if ( firstPickedPart )
        {
            const RivObjectSourceInfo* sourceInfo =
                dynamic_cast<const RivObjectSourceInfo*>( firstPickedPart->sourceInfo() );
            if ( sourceInfo )
            {
                if ( dynamic_cast<RimPerforationInterval*>( sourceInfo->object() ) )
                {
                    objectToSelect = sourceInfo->object();

                    if ( eventObject.m_pickItemInfos.size() > 1 )
                    {
                        const auto&      secondPickedItem = eventObject.m_pickItemInfos[1];
                        const cvf::Part* secondPickedPart = secondPickedItem.pickedPart();
                        if ( secondPickedPart )
                        {
                            auto wellPathSourceCandidate =
                                dynamic_cast<const RivWellPathSourceInfo*>( secondPickedPart->sourceInfo() );
                            if ( wellPathSourceCandidate )
                            {
                                RimWellPath* perforationWellPath = nullptr;
                                objectToSelect->firstAncestorOrThisOfType( perforationWellPath );
                                if ( perforationWellPath == wellPathSourceCandidate->wellPath() )
                                {
                                    wellPathSourceInfo =
                                        dynamic_cast<const RivWellPathSourceInfo*>( secondPickedPart->sourceInfo() );
                                    wellPathTriangleIndex = secondPickedItem.faceIdx();
                                }
                            }
                        }
                    }
                }
                else if ( dynamic_cast<RimWellPathValve*>( sourceInfo->object() ) )
                {
                    objectToSelect        = sourceInfo->object();
                    RimWellPath* wellPath = nullptr;
                    objectToSelect->firstAncestorOrThisOfType( wellPath );

                    RimWellPathValve* valve = static_cast<RimWellPathValve*>( sourceInfo->object() );

                    QString valveText = QString( "Well Path: %1\nValve: %2\nTemplate: %3" )
                                            .arg( wellPath->name() )
                                            .arg( valve->name() )
                                            .arg( valve->valveTemplate()->name() );

                    RiuMainWindow::instance()->setResultInfo( valveText );
                    RiuMainWindow::instance()->selectAsCurrentItem( objectToSelect );
                }
                else if ( dynamic_cast<RimWellPathAttribute*>( sourceInfo->object() ) )
                {
                    RimWellPath* wellPath = nullptr;

                    RimWellPathAttribute* attribute = static_cast<RimWellPathAttribute*>( sourceInfo->object() );
                    RimWellPathAttributeCollection* collection = nullptr;
                    attribute->firstAncestorOrThisOfTypeAsserted( collection );
                    collection->firstAncestorOrThisOfTypeAsserted( wellPath );

                    QString attrText = QString( "Well Path: %1\nCasing Design Attribute: %2" )
                                           .arg( wellPath->name() )
                                           .arg( attribute->componentLabel() );

                    RiuMainWindow::instance()->setResultInfo( attrText );
                    RiuMainWindow::instance()->selectAsCurrentItem( collection );
                }
                else if ( dynamic_cast<RimWellMeasurement*>( sourceInfo->object() ) )
                {
                    RimWellMeasurement* measurement = dynamic_cast<RimWellMeasurement*>( sourceInfo->object() );
                    RimWellMeasurementCollection* collection = nullptr;
                    measurement->firstAncestorOrThisOfTypeAsserted( collection );

                    QString measurementText = QString( "Well path name: %1\n" ).arg( measurement->wellName() );
                    measurementText += QString( "Measured Depth: %1\n" ).arg( measurement->MD() );
                    measurementText += QString( "Value: %1\n" ).arg( measurement->value() );
                    measurementText += QString( "Date: %1\n" ).arg( measurement->date().toString() );

                    if ( !measurement->kind().isEmpty() )
                    {
                        measurementText += QString( "Kind: %1\n" ).arg( measurement->kind() );
                    }

                    if ( measurement->quality() > 0 )
                    {
                        measurementText += QString( "Quality: %1\n" ).arg( measurement->quality() );
                    }

                    if ( !measurement->remark().isEmpty() )
                    {
                        measurementText += QString( "Remark: %1\n" ).arg( measurement->remark() );
                    }

                    RiuMainWindow::instance()->setResultInfo( measurementText );

                    Rim3dView* rimView = RiaApplication::instance()->activeReservoirView();
                    if ( rimView )
                    {
                        // Find the RimWellMeasurementInView which matches the selection
                        std::vector<RimWellMeasurementInViewCollection*> wellMeasurementInViewCollections;
                        rimView->descendantsIncludingThisOfType( wellMeasurementInViewCollections );
                        if ( !wellMeasurementInViewCollections.empty() )
                        {
                            RimWellMeasurementInView* wellMeasurementInView =
                                wellMeasurementInViewCollections[0]->getWellMeasurementInView( measurement );
                            if ( wellMeasurementInView )
                            {
                                RiuMainWindow::instance()->selectAsCurrentItem( wellMeasurementInView );
                            }
                        }
                    }
                }
                else if ( auto geoDef = dynamic_cast<RimWellPathGeometryDef*>( sourceInfo->object() ) )
                {
                    RiuMainWindow::instance()->selectAsCurrentItem( geoDef );
                }
            }

            if ( dynamic_cast<const RivWellPathSourceInfo*>( firstPickedPart->sourceInfo() ) )
            {
                wellPathSourceInfo    = dynamic_cast<const RivWellPathSourceInfo*>( firstPickedPart->sourceInfo() );
                wellPathTriangleIndex = firstPickedItem.faceIdx();
            }
        }
    }

    if ( wellPathSourceInfo )
    {
        Rim3dView* rimView = RiaApplication::instance()->activeReservoirView();
        if ( !rimView ) return false;

        cvf::ref<caf::DisplayCoordTransform> transForm = rimView->displayCoordTransform();
        cvf::Vec3d                           pickedPositionInUTM =
            transForm->transformToDomainCoord( eventObject.m_pickItemInfos.front().globalPickedPoint() );

        if ( auto intersectionView = dynamic_cast<Rim2dIntersectionView*>( rimView ) )
        {
            if ( intersectionView->flatIntersectionPartMgr() )
            {
                pickedPositionInUTM = intersectionView->transformToUtm( pickedPositionInUTM );
            }
        }

        double measuredDepth = wellPathSourceInfo->measuredDepth( wellPathTriangleIndex, pickedPositionInUTM );

        // NOTE: This computation was used to find the location for a fracture when clicking on a well path
        // It turned out that the computation was a bit inaccurate
        // Consider to use code in RigSimulationWellCoordsAndMD instead
        cvf::Vec3d trueVerticalDepth =
            wellPathSourceInfo->closestPointOnCenterLine( wellPathTriangleIndex, pickedPositionInUTM );

        QString wellPathText;
        wellPathText += QString( "Well path name : %1\n" ).arg( wellPathSourceInfo->wellPath()->name() );
        wellPathText += QString( "Measured depth : %1\n" ).arg( measuredDepth );

        QString formattedText = QString( "Intersection point : [E: %1, N: %2, Depth: %3]" )
                                    .arg( trueVerticalDepth.x(), 5, 'f', 2 )
                                    .arg( trueVerticalDepth.y(), 5, 'f', 2 )
                                    .arg( -trueVerticalDepth.z(), 5, 'f', 2 );
        wellPathText += formattedText;

        RiuMainWindow::instance()->setResultInfo( wellPathText );

        if ( objectToSelect )
        {
            RiuMainWindow::instance()->selectAsCurrentItem( objectToSelect );
        }
        else
        {
            RiuMainWindow::instance()->selectAsCurrentItem( wellPathSourceInfo->wellPath() );
        }

        return true;
    }

    return false;
}
