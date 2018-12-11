/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     equinor ASA
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

#include "RimAnnotationCollectionBase.h"
#include "RimPolylineTarget.h"
#include "Rim3dView.h"
#include "RimCase.h"
#include "RimUserDefinedPolylinesAnnotation.h"

#include "RiuViewer.h"

#include "cafDisplayCoordTransform.h"
#include "cafPdmUiCommandSystemProxy.h"
#include "cafSelectionManager.h"

#include "cvfPart.h"
#include "cvfModelBasicList.h"

CAF_PDM_UI_3D_OBJECT_EDITOR_SOURCE_INIT(RicPolylineTarget3dEditor);

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
    RiuViewer* ownerRiuViewer = dynamic_cast<RiuViewer*>(ownerViewer());

    if (m_cvfModel.notNull() && ownerRiuViewer) 
    {

        // Could result in some circularities ....
        ownerRiuViewer->removeStaticModel(m_cvfModel.p());
    }

    RimPolylineTarget* oldTarget = dynamic_cast<RimPolylineTarget*>(this->pdmObject());
    if (oldTarget)
    {
        oldTarget->targetPointUiCapability()->removeFieldEditor(this);
    }

    delete m_manipulator;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPolylineTarget3dEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    RimPolylineTarget* target = dynamic_cast<RimPolylineTarget*>(this->pdmObject());
    RiuViewer* ownerRiuViewer = dynamic_cast<RiuViewer*>(ownerViewer());

    if ( !target || !target->isEnabled())
    {
        m_cvfModel->removeAllParts();
        return;
    }

    RimUserDefinedPolylinesAnnotation* polylineDef;
    target->firstAncestorOrThisOfTypeAsserted(polylineDef);

    target->targetPointUiCapability()->addFieldEditor(this);

    if (m_manipulator.isNull())
    {
        m_manipulator = new RicPointTangentManipulator(ownerRiuViewer);
        QObject::connect(m_manipulator,
                         SIGNAL( notifyUpdate(const cvf::Vec3d& , const cvf::Vec3d& ) ),
                         this,
                         SLOT( slotUpdated(const cvf::Vec3d& , const cvf::Vec3d& ) ) );
        QObject::connect(m_manipulator,
                         SIGNAL( notifySelected() ),
                         this,
                         SLOT( slotSelectedIn3D() ) );
        QObject::connect(m_manipulator,
                         SIGNAL( notifyDragFinished() ),
                         this,
                         SLOT( slotDragFinished() ) );
        m_cvfModel = new cvf::ModelBasicList;
        ownerRiuViewer->addStaticModelOnce(m_cvfModel.p());
    }

    cvf::ref<caf::DisplayCoordTransform> dispXf;
    double handleSize = 1.0;
    {
        dispXf = ownerRiuViewer->ownerReservoirView()->displayCoordTransform();
        Rim3dView* view = dynamic_cast<Rim3dView*>(ownerRiuViewer->ownerReservoirView());
        handleSize = 0.7 * view->ownerCase()->characteristicCellSize();
    }

    m_manipulator->setOrigin(dispXf->transformToDisplayCoord( target->targetPointXYZ()));
    //m_manipulator->setTangent(target->tangent());
    m_manipulator->setHandleSize(handleSize);
    m_cvfModel->removeAllParts();
    m_manipulator->appendPartsToModel(m_cvfModel.p());

    m_cvfModel->updateBoundingBoxesRecursive();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPolylineTarget3dEditor::cleanupBeforeSettingPdmObject()
{
    RimPolylineTarget* oldTarget = dynamic_cast<RimPolylineTarget*>(this->pdmObject());
    if (oldTarget)
    {
        oldTarget->targetPointUiCapability()->removeFieldEditor(this);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPolylineTarget3dEditor::slotUpdated(const cvf::Vec3d& origin, const cvf::Vec3d& tangent)
{
    RimPolylineTarget* target = dynamic_cast<RimPolylineTarget*>(this->pdmObject());

    if ( !target)
    {
        return;
    }

    cvf::ref<caf::DisplayCoordTransform> dispXf;
    {
        RiuViewer* viewer = dynamic_cast<RiuViewer*>(ownerViewer());
        dispXf = viewer->ownerReservoirView()->displayCoordTransform();
    }

    RimUserDefinedPolylinesAnnotation* polylineDef;
    target->firstAncestorOrThisOfTypeAsserted(polylineDef);

    cvf::Vec3d domainOrigin = dispXf->transformToDomainCoord( origin);
    domainOrigin.z() = -domainOrigin.z();
    QVariant originVariant = caf::PdmValueFieldSpecialization < cvf::Vec3d >::convert(domainOrigin);

    target->enableFullUpdate(false);
    caf::PdmUiCommandSystemProxy::instance()->setUiValueToField(target->targetPointUiCapability(), originVariant);
    target->enableFullUpdate(true);
}

void RicPolylineTarget3dEditor::slotSelectedIn3D()
{
    RimPolylineTarget* target = dynamic_cast<RimPolylineTarget*>(this->pdmObject());
    if ( !target)
    {
        return;
    }

    caf::SelectionManager::instance()->setSelectedItemAtLevel(target, caf::SelectionManager::FIRST_LEVEL);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPolylineTarget3dEditor::slotDragFinished()
{
    RimPolylineTarget* target = dynamic_cast<RimPolylineTarget*>(this->pdmObject());
    if ( !target)
    {
        return;
    }

    RimAnnotationCollectionBase* annColl;
    target->firstAncestorOrThisOfTypeAsserted(annColl);
    annColl->scheduleRedrawOfRelevantViews();
}


