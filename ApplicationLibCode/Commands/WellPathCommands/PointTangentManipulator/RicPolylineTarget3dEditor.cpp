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

#include "RicPolylineTarget3dEditor.h"

#include "RicPointTangentManipulator.h"

#include "Rim3dView.h"
#include "RimPolylinePickerInterface.h"
#include "RimPolylineTarget.h"

#include "RiuViewer.h"

#include "cafDisplayCoordTransform.h"
#include "cafPdmUiCommandSystemProxy.h"
#include "cafSelectionManager.h"

#include "cvfModelBasicList.h"
#include "cvfPart.h"

CAF_PDM_UI_3D_OBJECT_EDITOR_SOURCE_INIT( RicPolylineTarget3dEditor );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicPolylineTarget3dEditor::RicPolylineTarget3dEditor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicPolylineTarget3dEditor::~RicPolylineTarget3dEditor()
{
    RiuViewer* ownerRiuViewer = dynamic_cast<RiuViewer*>( ownerViewer() );

    if ( m_cvfModel.notNull() && ownerRiuViewer )
    {
        // Could result in some circularities ....
        ownerRiuViewer->removeStaticModel( m_cvfModel.p() );
    }

    auto* oldTarget = dynamic_cast<RimPolylineTarget*>( pdmObject() );
    if ( oldTarget )
    {
        oldTarget->targetPointUiCapability()->removeFieldEditor( this );
    }

    delete m_manipulator;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPolylineTarget3dEditor::configureAndUpdateUi( const QString& uiConfigName )
{
    auto*      target         = dynamic_cast<RimPolylineTarget*>( pdmObject() );
    RiuViewer* ownerRiuViewer = dynamic_cast<RiuViewer*>( ownerViewer() );
    Rim3dView* view           = mainOrComparisonView();

    if ( !target || !view )
    {
        if ( m_cvfModel.notNull() ) m_cvfModel->removeAllParts();

        return;
    }

    target->targetPointUiCapability()->addFieldEditor( this );

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

    cvf::ref<caf::DisplayCoordTransform> dispXf = view->displayCoordTransform();

    double scalingFactor = 0.7;
    if ( auto pickerInterface = target->firstAncestorOrThisOfType<RimPolylinePickerInterface>() )
    {
        scalingFactor *= pickerInterface->handleScalingFactor();
    }

    const double handleSize = scalingFactor * view->characteristicCellSize();

    m_manipulator->setOrigin( dispXf->transformToDisplayCoord( target->targetPointXYZ() ) );
    m_manipulator->setHandleSize( handleSize );
    m_cvfModel->removeAllParts();
    m_manipulator->appendPartsToModel( m_cvfModel.p() );

    m_cvfModel->updateBoundingBoxesRecursive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPolylineTarget3dEditor::cleanupBeforeSettingPdmObject()
{
    auto* oldTarget = dynamic_cast<RimPolylineTarget*>( pdmObject() );
    if ( oldTarget )
    {
        oldTarget->targetPointUiCapability()->removeFieldEditor( this );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPolylineTarget3dEditor::slotUpdated( const cvf::Vec3d& origin, const cvf::Vec3d& tangent )
{
    auto*      target = dynamic_cast<RimPolylineTarget*>( pdmObject() );
    Rim3dView* view   = mainOrComparisonView();

    if ( !target || !view )
    {
        return;
    }

    cvf::ref<caf::DisplayCoordTransform> dispXf = view->displayCoordTransform();

    cvf::Vec3d domainOrigin = dispXf->transformToDomainCoord( origin );
    domainOrigin.z()        = -domainOrigin.z();
    QVariant originVariant  = caf::PdmValueFieldSpecialization<cvf::Vec3d>::convert( domainOrigin );

    caf::PdmUiCommandSystemProxy::instance()->setUiValueToField( target->targetPointUiCapability(), originVariant );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPolylineTarget3dEditor::slotSelectedIn3D()
{
    auto* target = dynamic_cast<RimPolylineTarget*>( pdmObject() );
    if ( !target )
    {
        return;
    }

    caf::SelectionManager::instance()->setSelectedItemAtLevel( target, caf::SelectionManager::FIRST_LEVEL );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPolylineTarget3dEditor::slotDragFinished()
{
    auto* target = dynamic_cast<RimPolylineTarget*>( pdmObject() );
    if ( target )
    {
        target->triggerVisualizationUpdate();
    }
}
