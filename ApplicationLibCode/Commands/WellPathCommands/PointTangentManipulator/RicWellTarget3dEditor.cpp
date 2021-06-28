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

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimModeledWellPath.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathTarget.h"

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
    RiuViewer* ownerRiuViewer = dynamic_cast<RiuViewer*>( ownerViewer() );

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
    RimWellPathTarget* target         = dynamic_cast<RimWellPathTarget*>( this->pdmObject() );
    RiuViewer*         ownerRiuViewer = dynamic_cast<RiuViewer*>( ownerViewer() );
    Rim3dView*         view           = mainOrComparisonView();

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
    RimWellPathTarget* target = dynamic_cast<RimWellPathTarget*>( this->pdmObject() );
    Rim3dView*         view   = mainOrComparisonView();

    if ( !target || !view )
    {
        return;
    }

    RimWellPathGeometryDef* geomDef;
    target->firstAncestorOrThisOfTypeAsserted( geomDef );
    if ( !geomDef ) return;

    if ( geomDef->useReferencePointFromTopLevelWell() )
    {
        RimModeledWellPath* modeledWellPath = nullptr;
        geomDef->firstAncestorOfType( modeledWellPath );
        if ( modeledWellPath )
        {
            auto topLevelWellPath = dynamic_cast<RimModeledWellPath*>( modeledWellPath->topLevelWellPath() );
            if ( topLevelWellPath )
            {
                // Manipulate the reference point of top level well path
                geomDef = topLevelWellPath->geometryDefinition();
            }
        }
    }

    cvf::ref<caf::DisplayCoordTransform> dispXf = view->displayCoordTransform();

    auto domainCoordXYZ = dispXf->transformToDomainCoord( origin );

    // If CTRL is pressed, modify the reference point instead of the well path target
    bool modifyReferencePoint = ( QApplication::keyboardModifiers() & Qt::ControlModifier );
    if ( modifyReferencePoint )
    {
        auto relativePositionXYZ = domainCoordXYZ - geomDef->anchorPointXyz();
        auto delta               = target->targetPointXYZ() - relativePositionXYZ;

        auto currentRefPointXyz = geomDef->anchorPointXyz();
        auto newRefPointXyz     = currentRefPointXyz - delta;
        geomDef->setReferencePointXyz( newRefPointXyz );
        geomDef->changed.send( false );
        geomDef->updateWellPathVisualization( true );
        for ( auto wt : geomDef->activeWellTargets() )
        {
            wt->updateConnectedEditors();
        }
    }
    else
    {
        cvf::Vec3d relativePositionXYD = domainCoordXYZ - geomDef->anchorPointXyz();
        relativePositionXYD.z()        = -relativePositionXYD.z();

        target->updateFrom3DManipulator( relativePositionXYD );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellTarget3dEditor::slotSelectedIn3D()
{
    RimWellPathTarget* target = dynamic_cast<RimWellPathTarget*>( this->pdmObject() );
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
    RimWellPathTarget* target = dynamic_cast<RimWellPathTarget*>( this->pdmObject() );
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
    if ( RimWellPathTarget* oldTarget = dynamic_cast<RimWellPathTarget*>( this->pdmObject() ) )
    {
        for ( auto field : oldTarget->fieldsFor3dManipulator() )
        {
            field->uiCapability()->removeFieldEditor( this );
        }
    }
}
