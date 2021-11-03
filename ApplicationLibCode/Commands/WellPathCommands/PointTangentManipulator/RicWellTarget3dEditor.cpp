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

#include "RicWellTarget3dEditor.h"

#include "RicPointTangentManipulator.h"

#include "RigWellPath.h"

#include "Rim2dIntersectionView.h"
#include "Rim3dView.h"
#include "RimCase.h"
#include "RimModeledWellPath.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathGeometryDefTools.h"
#include "RimWellPathTarget.h"
#include "RimWellPathTieIn.h"

#include "RiuViewer.h"

#include "cafDisplayCoordTransform.h"
#include "cafPdmUiCommandSystemProxy.h"
#include "cafSelectionManager.h"

#include "cvfModelBasicList.h"
#include "cvfPart.h"

CAF_PDM_UI_3D_OBJECT_EDITOR_SOURCE_INIT( RicWellTarget3dEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicWellTarget3dEditor::RicWellTarget3dEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicWellTarget3dEditor::~RicWellTarget3dEditor()
{
    auto* ownerRiuViewer = dynamic_cast<RiuViewer*>( ownerViewer() );

    if ( m_cvfModel.notNull() && ownerRiuViewer )
    {
        // Could result in some circularities ....
        ownerRiuViewer->removeStaticModel( m_cvfModel.p() );
    }

    removeAllFieldEditors();

    delete m_manipulator;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellTarget3dEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    auto*      target         = dynamic_cast<RimWellPathTarget*>( this->pdmObject() );
    auto*      ownerRiuViewer = dynamic_cast<RiuViewer*>( ownerViewer() );
    Rim3dView* view           = mainOrComparisonView();

    // TODO: The location of the well target must be updated before displayed in the 2D intersection view. Currently
    // disabled.
    if ( dynamic_cast<Rim2dIntersectionView*>( view ) ) return;

    if ( !target || !target->isEnabled() || !view )
    {
        if ( m_cvfModel.notNull() ) m_cvfModel->removeAllParts();

        return;
    }

    RimWellPathGeometryDef* geomDef;
    target->firstAncestorOrThisOfTypeAsserted( geomDef );

    for ( auto field : target->fieldsFor3dManipulator() )
    {
        field->uiCapability()->addFieldEditor( this );
    }

    if ( m_manipulator.isNull() )
    {
        m_manipulator = new RicPointTangentManipulator( ownerRiuViewer );

        QObject::connect( m_manipulator,
                          SIGNAL( notifyUpdate( const cvf::Vec3d&, const cvf::Vec3d& ) ),
                          this,
                          SLOT( slotUpdated( const cvf::Vec3d&, const cvf::Vec3d& ) ) );

        QObject::connect( m_manipulator, SIGNAL( notifySelected() ), this, SLOT( slotSelectedIn3D() ) );
        QObject::connect( m_manipulator, SIGNAL( notifyDragFinished() ), this, SLOT( slotDragFinished() ) );

        m_cvfModel = new cvf::ModelBasicList;
        ownerRiuViewer->addStaticModelOnce( m_cvfModel.p(), isInComparisonView() );
    }

    cvf::ref<caf::DisplayCoordTransform> dispXf     = view->displayCoordTransform();
    double                               handleSize = 0.7 * view->ownerCase()->characteristicCellSize();

    m_manipulator->setOrigin( dispXf->transformToDisplayCoord( target->targetPointXYZ() + geomDef->anchorPointXyz() ) );
    m_manipulator->setTangent( target->tangent() );
    m_manipulator->setHandleSize( handleSize );

    {
        RimWellPath* wellPath = nullptr;
        target->firstAncestorOrThisOfType( wellPath );

        if ( wellPath && !wellPath->isTopLevelWellPath() && geomDef->firstActiveTarget() == target )
        {
            if ( auto parentWellPath = wellPath->wellPathTieIn()->parentWell() )
            {
                auto geo    = parentWellPath->wellPathGeometry();
                auto points = geo->wellPathPoints();

                for ( auto& p : points )
                {
                    p = dispXf->transformToDisplayCoord( p );
                }

                // For the first target of a lateral, use the coordinates from the parent well as snap-to locations for
                // the 3D manipulator sphere
                m_manipulator->setPolyline( points );
            }
        }
    }

    m_cvfModel->removeAllParts();
    m_manipulator->appendPartsToModel( m_cvfModel.p() );

    m_cvfModel->updateBoundingBoxesRecursive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellTarget3dEditor::cleanupBeforeSettingPdmObject()
{
    removeAllFieldEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellTarget3dEditor::slotUpdated( const cvf::Vec3d& origin, const cvf::Vec3d& tangent )
{
    auto*      manipulatedTarget = dynamic_cast<RimWellPathTarget*>( this->pdmObject() );
    Rim3dView* view              = mainOrComparisonView();

    if ( !manipulatedTarget || !view )
    {
        return;
    }

    RimWellPathGeometryDef* geomDef;
    manipulatedTarget->firstAncestorOrThisOfTypeAsserted( geomDef );
    if ( !geomDef ) return;

    RimModeledWellPath* modeledWellPath = nullptr;
    geomDef->firstAncestorOfType( modeledWellPath );

    cvf::Vec3d domainCoordXYZ; // domain coordinate of the new location
    cvf::Vec3d deltaManipulatorMovement; // delta change relative current location of target
    cvf::Vec3d relativePositionXYZ; // position of well target relative to anchor point
    {
        cvf::ref<caf::DisplayCoordTransform> dispXf = view->displayCoordTransform();
        domainCoordXYZ                              = dispXf->transformToDomainCoord( origin );

        relativePositionXYZ      = domainCoordXYZ - geomDef->anchorPointXyz();
        deltaManipulatorMovement = manipulatedTarget->targetPointXYZ() - relativePositionXYZ;
    }

    if ( geomDef->activeWellTargets().front() == manipulatedTarget )
    {
        // The first well target of a lateral is the tie-in well target

        if ( modeledWellPath && modeledWellPath->wellPathTieIn() && modeledWellPath->wellPathTieIn()->parentWell() )
        {
            auto parentWell  = modeledWellPath->wellPathTieIn()->parentWell();
            auto wellPathGeo = parentWell->wellPathGeometry();
            auto closestMD   = wellPathGeo->closestMeasuredDepth( domainCoordXYZ );

            modeledWellPath->wellPathTieIn()->setTieInMeasuredDepth( closestMD );
            modeledWellPath->wellPathTieIn()->updateChildWellGeometry();
        }

        bool modifyAllTargetsOnAllWells = ( ( QApplication::keyboardModifiers() & Qt::ControlModifier ) &&
                                            ( QApplication::keyboardModifiers() & Qt::SHIFT ) );

        if ( modifyAllTargetsOnAllWells )
        {
            for ( auto wellLateral : modeledWellPath->wellPathLaterals() )
            {
                if ( auto modeledLateral = dynamic_cast<RimModeledWellPath*>( wellLateral ) )
                {
                    auto activeTargets = modeledLateral->geometryDefinition()->activeWellTargets();
                    for ( auto t : activeTargets )
                    {
                        if ( t == activeTargets.front() ) continue;
                        if ( t == manipulatedTarget ) continue;

                        // Does not work very well
                        // Must update the tie-in MD also
                        updateTargetWithDeltaChange( t, deltaManipulatorMovement );
                    }
                }
            }
        }

        if ( QApplication::keyboardModifiers() & Qt::ControlModifier )
        {
            for ( auto target : geomDef->activeWellTargets() )
            {
                if ( target == geomDef->activeWellTargets().front() ) continue;

                updateTargetWithDeltaChange( target, deltaManipulatorMovement );
            }
        }

        cvf::Vec3d relativePositionXYD = relativePositionXYZ;
        relativePositionXYD.z()        = -relativePositionXYD.z();

        manipulatedTarget->updateFrom3DManipulator( relativePositionXYD );

        return;
    }

    if ( modeledWellPath && modeledWellPath->isTopLevelWellPath() )
    {
        // Modification of top level well path

        bool modifyReferencePoint = ( ( QApplication::keyboardModifiers() & Qt::ControlModifier ) &&
                                      ( QApplication::keyboardModifiers() & Qt::SHIFT ) );
        if ( modifyReferencePoint )
        {
            // Find all linked wells and update reference point with delta change
            std::vector<RimWellPathGeometryDef*> linkedWellPathGeoDefs;
            if ( geomDef->isReferencePointUpdatesLinked() )
            {
                linkedWellPathGeoDefs = RimWellPathGeometryDefTools::linkedDefinitions();
            }
            else
            {
                linkedWellPathGeoDefs.push_back( geomDef );
            }

            RimWellPathGeometryDefTools::updateLinkedGeometryDefinitions( linkedWellPathGeoDefs, deltaManipulatorMovement );

            return;
        }

        bool modifyAllTargetOnWell = ( QApplication::keyboardModifiers() & Qt::ControlModifier );
        if ( modifyAllTargetOnWell )
        {
            for ( auto t : geomDef->activeWellTargets() )
            {
                if ( t == manipulatedTarget ) continue;

                updateTargetWithDeltaChange( t, deltaManipulatorMovement );
            }
        }
    }
    else if ( modeledWellPath && !modeledWellPath->isTopLevelWellPath() )
    {
        bool modifyAllTargetsOnAllWells = ( ( QApplication::keyboardModifiers() & Qt::ControlModifier ) &&
                                            ( QApplication::keyboardModifiers() & Qt::SHIFT ) );
        if ( modifyAllTargetsOnAllWells )
        {
            // Update all well targets on all connected laterals

            for ( auto wellLateral : modeledWellPath->wellPathLaterals() )
            {
                if ( auto modeledLateral = dynamic_cast<RimModeledWellPath*>( wellLateral ) )
                {
                    auto activeTargets = modeledLateral->geometryDefinition()->activeWellTargets();
                    for ( auto t : activeTargets )
                    {
                        if ( t == activeTargets.front() ) continue;
                        if ( t == manipulatedTarget ) continue;

                        updateTargetWithDeltaChange( t, deltaManipulatorMovement );
                    }
                }
            }
        }

        bool modifyAllTargets = ( QApplication::keyboardModifiers() & Qt::ControlModifier );
        if ( modifyAllTargets )
        {
            // Update all well targets on current well path

            for ( auto t : geomDef->activeWellTargets() )
            {
                if ( t == geomDef->activeWellTargets().front() ) continue;
                if ( t == manipulatedTarget ) continue;

                updateTargetWithDeltaChange( t, deltaManipulatorMovement );
            }
        }
    }

    // Modify a single well target
    {
        cvf::Vec3d relativePositionXYD = relativePositionXYZ;
        relativePositionXYD.z()        = -relativePositionXYD.z();

        manipulatedTarget->updateFrom3DManipulator( relativePositionXYD );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellTarget3dEditor::slotSelectedIn3D()
{
    auto* target = dynamic_cast<RimWellPathTarget*>( this->pdmObject() );
    if ( !target )
    {
        return;
    }

    caf::SelectionManager::instance()->setSelectedItemAtLevel( target, caf::SelectionManager::FIRST_LEVEL );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellTarget3dEditor::slotDragFinished()
{
    auto* target = dynamic_cast<RimWellPathTarget*>( this->pdmObject() );
    if ( !target )
    {
        return;
    }

    target->onMoved();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellTarget3dEditor::removeAllFieldEditors()
{
    if ( auto* oldTarget = dynamic_cast<RimWellPathTarget*>( this->pdmObject() ) )
    {
        for ( auto field : oldTarget->fieldsFor3dManipulator() )
        {
            field->uiCapability()->removeFieldEditor( this );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellTarget3dEditor::updateTargetWithDeltaChange( RimWellPathTarget* target, const cvf::Vec3d& delta )
{
    auto coordXYZ = target->targetPointXYZ() - delta;
    target->setPointXYZ( coordXYZ );
    target->updateConnectedEditors();
}
