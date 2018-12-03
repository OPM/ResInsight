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

#include "RicWellTarget3dEditor.h"

#include "RicPointTangentManipulator.h"

#include "RimWellPathTarget.h"
#include "Rim3dView.h"
#include "RimCase.h"
#include "RimModeledWellPath.h"
#include "RimWellPathGeometryDef.h"

#include "RiuViewer.h"

#include "cafDisplayCoordTransform.h"
#include "cafPdmUiCommandSystemProxy.h"
#include "cafSelectionManager.h"

#include "cvfPart.h"
#include "cvfModelBasicList.h"

CAF_PDM_UI_3D_OBJECT_EDITOR_SOURCE_INIT(RicWellTarget3dEditor);

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
    RiuViewer* ownerRiuViewer = dynamic_cast<RiuViewer*>(ownerViewer());

    if (m_cvfModel.notNull() && ownerRiuViewer) 
    {

        // Could result in some circularities ....
        ownerRiuViewer->removeStaticModel(m_cvfModel.p());
    }

    RimWellPathTarget* oldTarget = dynamic_cast<RimWellPathTarget*>(this->pdmObject());
    if (oldTarget)
    {
        oldTarget->m_targetType.uiCapability()->removeFieldEditor(this);
        oldTarget->m_targetPoint.uiCapability()->removeFieldEditor(this);
        oldTarget->m_azimuth.uiCapability()->removeFieldEditor(this);
        oldTarget->m_inclination.uiCapability()->removeFieldEditor(this);
    }

    delete m_manipulator;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellTarget3dEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    RimWellPathTarget* target = dynamic_cast<RimWellPathTarget*>(this->pdmObject());
    RiuViewer* ownerRiuViewer = dynamic_cast<RiuViewer*>(ownerViewer());

    if ( !target || !target->isEnabled())
    {
        m_cvfModel->removeAllParts();
        return;
    }

    RimWellPathGeometryDef* geomDef;
    target->firstAncestorOrThisOfTypeAsserted(geomDef);

    target->m_targetType.uiCapability()->addFieldEditor(this);
    target->m_targetPoint.uiCapability()->addFieldEditor(this);
    target->m_azimuth.uiCapability()->addFieldEditor(this);
    target->m_inclination.uiCapability()->addFieldEditor(this);

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

    m_manipulator->setOrigin(dispXf->transformToDisplayCoord( target->targetPointXYZ() + geomDef->referencePointXyz()));
    m_manipulator->setTangent(target->tangent());
    m_manipulator->setHandleSize(handleSize);
    m_cvfModel->removeAllParts();
    m_manipulator->appendPartsToModel(m_cvfModel.p());

    m_cvfModel->updateBoundingBoxesRecursive();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellTarget3dEditor::cleanupBeforeSettingPdmObject()
{
    RimWellPathTarget* oldTarget = dynamic_cast<RimWellPathTarget*>(this->pdmObject());
    if (oldTarget)
    {
        oldTarget->m_targetType.uiCapability()->removeFieldEditor(this);
        oldTarget->m_targetPoint.uiCapability()->removeFieldEditor(this);
        oldTarget->m_azimuth.uiCapability()->removeFieldEditor(this);
        oldTarget->m_inclination.uiCapability()->removeFieldEditor(this);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellTarget3dEditor::slotUpdated(const cvf::Vec3d& origin, const cvf::Vec3d& tangent)
{
    RimWellPathTarget* target = dynamic_cast<RimWellPathTarget*>(this->pdmObject());

    if ( !target)
    {
        return;
    }

    cvf::ref<caf::DisplayCoordTransform> dispXf;
    {
        RiuViewer* viewer = dynamic_cast<RiuViewer*>(ownerViewer());
        dispXf = viewer->ownerReservoirView()->displayCoordTransform();
    }

    RimWellPathGeometryDef* geomDef;
    target->firstAncestorOrThisOfTypeAsserted(geomDef);

    cvf::Vec3d domainOrigin = dispXf->transformToDomainCoord( origin)  - geomDef->referencePointXyz();
    domainOrigin.z() = -domainOrigin.z();
    QVariant originVariant = caf::PdmValueFieldSpecialization < cvf::Vec3d >::convert(domainOrigin);

    target->enableFullUpdate(false);
    caf::PdmUiCommandSystemProxy::instance()->setUiValueToField(target->m_targetPoint.uiCapability(), originVariant);
    target->enableFullUpdate(true);
}

void RicWellTarget3dEditor::slotSelectedIn3D()
{
    RimWellPathTarget* target = dynamic_cast<RimWellPathTarget*>(this->pdmObject());
    if ( !target)
    {
        return;
    }

    caf::SelectionManager::instance()->setSelectedItemAtLevel(target, caf::SelectionManager::FIRST_LEVEL);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellTarget3dEditor::slotDragFinished()
{
    RimWellPathTarget* target = dynamic_cast<RimWellPathTarget*>(this->pdmObject());
    if ( !target)
    {
        return;
    }

    RimModeledWellPath* wellpath;
    target->firstAncestorOrThisOfTypeAsserted(wellpath);
    wellpath->scheduleUpdateOfDependentVisualization();
}


