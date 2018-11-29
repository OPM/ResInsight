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

#include "RicTextAnnotation3dEditor.h"

#include "../WellPathCommands/PointTangentManipulator/RicPointTangentManipulator.h"

#include "RimTextAnnotation.h"
#include "Rim3dView.h"
#include "RimCase.h"

#include "RiuViewer.h"

#include "cafDisplayCoordTransform.h"
#include "cafPdmUiCommandSystemProxy.h"
#include "cafSelectionManager.h"

#include "cvfPart.h"
#include "cvfModelBasicList.h"

CAF_PDM_UI_3D_OBJECT_EDITOR_SOURCE_INIT(RicTextAnnotation3dEditor);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicTextAnnotation3dEditor::RicTextAnnotation3dEditor()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicTextAnnotation3dEditor::~RicTextAnnotation3dEditor()
{
    RiuViewer* ownerRiuViewer = dynamic_cast<RiuViewer*>(ownerViewer());

    if (m_cvfModel.notNull() && ownerRiuViewer) 
    {

        // Could result in some circularities ....
        ownerRiuViewer->removeStaticModel(m_cvfModel.p());
    }

    auto textAnnot = dynamic_cast<RimTextAnnotation*>(this->pdmObject());
    if (textAnnot)
    {
        textAnnot->m_anchorPointXyd.uiCapability()->removeFieldEditor(this);
        textAnnot->m_labelPointXyd.uiCapability()->removeFieldEditor(this);
    }

    delete m_labelManipulator;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicTextAnnotation3dEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    RimTextAnnotation* textAnnot = dynamic_cast<RimTextAnnotation*>(this->pdmObject());
    RiuViewer* ownerRiuViewer = dynamic_cast<RiuViewer*>(ownerViewer());

    if ( !textAnnot || !textAnnot->isActive())
    {
        m_cvfModel->removeAllParts();
        return;
    }

    textAnnot->m_anchorPointXyd.uiCapability()->addFieldEditor(this);
    textAnnot->m_labelPointXyd.uiCapability()->addFieldEditor(this);


    if (m_labelManipulator.isNull())
    {
        m_labelManipulator = new RicPointTangentManipulator(ownerRiuViewer);
        m_anchorManipulator = new RicPointTangentManipulator(ownerRiuViewer);
        QObject::connect(m_labelManipulator,
                         SIGNAL( notifyUpdate(const cvf::Vec3d& , const cvf::Vec3d& ) ),
                         this,
                         SLOT( slotLabelUpdated(const cvf::Vec3d& , const cvf::Vec3d& ) ) );
        QObject::connect(m_anchorManipulator,
                         SIGNAL( notifyUpdate(const cvf::Vec3d& , const cvf::Vec3d& ) ),
                         this,
                         SLOT( slotAnchorUpdated(const cvf::Vec3d& , const cvf::Vec3d& ) ) );

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
    cvf::Vec3d labelPos(textAnnot->m_labelPointXyd());
    labelPos.z() *= -1.0;
    m_labelManipulator->setOrigin(dispXf->transformToDisplayCoord( labelPos ));
    m_labelManipulator->setHandleSize(handleSize);

    cvf::Vec3d anchorPos(textAnnot->m_anchorPointXyd());
    anchorPos.z() *= -1.0;
    m_anchorManipulator->setOrigin(dispXf->transformToDisplayCoord( anchorPos ));
    m_anchorManipulator->setHandleSize(handleSize);


    m_cvfModel->removeAllParts();
    m_labelManipulator->appendPartsToModel(m_cvfModel.p());
    m_anchorManipulator->appendPartsToModel(m_cvfModel.p());

    m_cvfModel->updateBoundingBoxesRecursive();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicTextAnnotation3dEditor::cleanupBeforeSettingPdmObject()
{
    RimTextAnnotation* textAnnot = dynamic_cast<RimTextAnnotation*>(this->pdmObject());
    if (textAnnot)
    {
        textAnnot->m_anchorPointXyd.uiCapability()->removeFieldEditor(this);
        textAnnot->m_labelPointXyd.uiCapability()->removeFieldEditor(this);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicTextAnnotation3dEditor::slotLabelUpdated(const cvf::Vec3d& origin, const cvf::Vec3d& tangent)
{
    RimTextAnnotation* textAnnot = dynamic_cast<RimTextAnnotation*>(this->pdmObject());

    if ( !textAnnot)
    {
        return;
    }
    updatePoint(textAnnot->m_labelPointXyd.uiCapability(), origin);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicTextAnnotation3dEditor::slotAnchorUpdated(const cvf::Vec3d& origin, const cvf::Vec3d& dummy)
{
    RimTextAnnotation* textAnnot = dynamic_cast<RimTextAnnotation*>(this->pdmObject());

    if ( !textAnnot)
    {
        return;
    }
    updatePoint(textAnnot->m_anchorPointXyd.uiCapability(), origin);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicTextAnnotation3dEditor::updatePoint(caf::PdmUiFieldHandle* uiField, const cvf::Vec3d& newPos)
{
    cvf::ref<caf::DisplayCoordTransform> dispXf;
    {
        RiuViewer* viewer = dynamic_cast<RiuViewer*>(ownerViewer());
        dispXf = viewer->ownerReservoirView()->displayCoordTransform();
    }

    cvf::Vec3d domainPos = dispXf->transformToDomainCoord( newPos);
    domainPos.z() = -domainPos.z();
    QVariant originVariant = caf::PdmValueFieldSpecialization < cvf::Vec3d >::convert(domainPos);

    caf::PdmUiCommandSystemProxy::instance()->setUiValueToField(uiField, originVariant); 
}

