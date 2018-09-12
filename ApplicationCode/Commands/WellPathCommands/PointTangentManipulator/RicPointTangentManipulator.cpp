/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
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

#include "RicPointTangentManipulator.h"

#include "cafViewer.h"

#include "cvfCamera.h"
#include "cvfDrawableGeo.h"
#include "cvfHitItemCollection.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfRay.h"

#include <QDebug>
#include <QMouseEvent>
#include "RivPartPriority.h"
#include "cafPdmUiCommandSystemProxy.h"
#include "RimModeledWellPath.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicPointTangentManipulator::RicPointTangentManipulator(caf::Viewer* viewer)
    : m_viewer(viewer)
{
    m_partManager = new RicPointTangentManipulatorPartMgr;

    m_viewer->installEventFilter(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicPointTangentManipulator::~RicPointTangentManipulator()
{
    if (m_viewer) m_viewer->removeEventFilter(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulator::setOrigin(const cvf::Vec3d& origin)
{
    m_partManager->setOrigin(origin);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulator::setTangent(const cvf::Vec3d& tangent)
{
    m_partManager->setTangent(tangent);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulator::setHandleSize(double handleSize)
{
    m_partManager->setHandleSize(handleSize);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulator::appendPartsToModel(cvf::ModelBasicList* model)
{
    m_partManager->appendPartsToModel(model);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicPointTangentManipulator::eventFilter(QObject *obj, QEvent* inputEvent)
{
    if (inputEvent->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(inputEvent);

        if (mouseEvent->button() == Qt::LeftButton)
        {
            cvf::HitItemCollection hitItems;
            if (m_viewer->rayPick(mouseEvent->x(), mouseEvent->y(), &hitItems))
            {
                m_partManager->tryToActivateManipulator(hitItems.firstItem());

                if(m_partManager->isManipulatorActive())
                {
                    emit notifySelected();

                    return true;
                }
            }
        }
    }
    else if (inputEvent->type() == QEvent::MouseMove)
    {
        if (m_partManager->isManipulatorActive())
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(inputEvent);

            //qDebug() << "Inside mouse move";
            //qDebug() << mouseEvent->pos();

            int translatedMousePosX = mouseEvent->pos().x();
            int translatedMousePosY = m_viewer->height() - mouseEvent->pos().y();

            cvf::ref<cvf::Ray> ray = m_viewer->mainCamera()->rayFromWindowCoordinates(translatedMousePosX, translatedMousePosY);
            {
                m_partManager->updateManipulatorFromRay(ray.p());

                cvf::Vec3d origin;
                cvf::Vec3d tangent;
                m_partManager->originAndTangent(&origin, &tangent);

                emit notifyUpdate(origin, tangent);

                return true;
            }
        }
    }
    else if (inputEvent->type() == QEvent::MouseButtonRelease)
    {
        if (m_partManager->isManipulatorActive())
        {
            m_partManager->endManipulator();

            cvf::Vec3d origin;
            cvf::Vec3d tangent;
            m_partManager->originAndTangent(&origin, &tangent);

            emit notifyUpdate(origin, tangent);
            emit notifyDragFinished();

            return true;
        }
    }

    return false;
}



//==================================================================================================
/// 
///
//==================================================================================================



#include "RicPointTangentManipulator.h"

#include "cafBoxManipulatorGeometryGenerator.h"
#include "cafEffectGenerator.h"
#include "cafLine.h"
#include "cafSelectionManager.h"

#include "cvfBoxGenerator.h"
#include "cvfDrawableGeo.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfPrimitiveSetIndexedUShort.h"
#include "cvfRay.h"
#include "cvfPrimitiveSetDirect.h"
#include "cvfHitItem.h"
#include <QDebug>

#include "cvfGeometryBuilderTriangles.h"
#include "cvfGeometryUtils.h"
//



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicPointTangentManipulatorPartMgr::RicPointTangentManipulatorPartMgr() 
    : m_tangentOnStartManipulation(cvf::Vec3d::UNDEFINED),
    m_originOnStartManipulation(cvf::Vec3d::UNDEFINED),
    m_currentHandleIndex(cvf::UNDEFINED_SIZE_T),
    m_handleSize(1.0)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicPointTangentManipulatorPartMgr::~RicPointTangentManipulatorPartMgr()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::setOrigin(const cvf::Vec3d& origin)
{
    if (isManipulatorActive()) return;

    m_origin = origin;
    if (m_originOnStartManipulation.isUndefined()) m_originOnStartManipulation = origin;

    clearAllGeometryAndParts();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::setTangent(const cvf::Vec3d& tangent)
{
    if(isManipulatorActive()) return;

    m_tangent = tangent;
    if (m_tangentOnStartManipulation.isUndefined()) m_tangentOnStartManipulation = m_tangent;

    clearAllGeometryAndParts();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::setHandleSize(double handleSize)
{
    m_handleSize = handleSize;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::originAndTangent(cvf::Vec3d* origin, cvf::Vec3d* tangent)
{
    *origin = m_origin;
    *tangent = m_tangent;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicPointTangentManipulatorPartMgr::isManipulatorActive() const
{
    return m_currentHandleIndex != cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::appendPartsToModel(cvf::ModelBasicList* model)
{
    if (!m_handleParts.size())
    {
        recreateAllGeometryAndParts();
    }

    for (size_t i = 0; i < m_handleParts.size(); i++)
    {
        model->addPart(m_handleParts.at(i));
    }

    for (auto activeModePart: m_activeDragModeParts)
    {
        model->addPart(activeModePart.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::tryToActivateManipulator(const cvf::HitItem* hitItem)
{
    endManipulator();

    if (!hitItem) return;

    const cvf::Part* pickedPart = hitItem->part();
    const cvf::Vec3d intersectionPoint = hitItem->intersectionPoint();

    if (!pickedPart) return;

    for (size_t i = 0; i < m_handleParts.size(); i++)
    {
        if (pickedPart == m_handleParts.at(i))
        {
            m_initialPickPoint = intersectionPoint;
            m_tangentOnStartManipulation = m_tangent;
            m_originOnStartManipulation = m_origin;
            m_currentHandleIndex = i;
        }
    }

}


//--------------------------------------------------------------------------------------------------
/// Calculate new origin and tangent based on the new ray position
/// Clear geometry to trigger regeneration  
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::updateManipulatorFromRay(const cvf::Ray* newMouseRay)
{
    if (!isManipulatorActive()) return;

    if ( m_handleIds[m_currentHandleIndex] ==  HORIZONTAL_PLANE )
    {
        cvf::Plane plane;
        plane.setFromPointAndNormal(m_origin, cvf::Vec3d::Z_AXIS);
        cvf::Vec3d newIntersection;
        newMouseRay->planeIntersect(plane, &newIntersection);

        cvf::Vec3d newOrigin = m_originOnStartManipulation + (newIntersection - m_initialPickPoint);

        m_origin = newOrigin;
    }
    else if ( m_handleIds[m_currentHandleIndex] ==  VERTICAL_AXIS )
    {
        cvf::Plane plane;
        cvf::Vec3d planeNormal = (newMouseRay->direction() ^ cvf::Vec3d::Z_AXIS) ^ cvf::Vec3d::Z_AXIS;
        double length = planeNormal.length();

        if (length < 1e-5) return;

        planeNormal /= length;
        plane.setFromPointAndNormal(m_initialPickPoint, planeNormal );
        cvf::Vec3d newIntersection;
        newMouseRay->planeIntersect(plane, &newIntersection);

        cvf::Vec3d newOrigin = m_originOnStartManipulation;
        newOrigin.z() += (newIntersection.z() - m_initialPickPoint.z());

        m_origin = newOrigin;
    }
    //m_tangent = newTangent;

    clearAllGeometryAndParts();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::endManipulator()
{
    m_currentHandleIndex = cvf::UNDEFINED_SIZE_T;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::clearAllGeometryAndParts()
{
    m_handleIds.clear();
    m_handleParts.clear();
    m_activeDragModeParts.clear();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::recreateAllGeometryAndParts()
{
    createAllHandleParts();

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::createAllHandleParts()
{
    createHorizontalPlaneHandle();
    createVerticalAxisHandle();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void  RicPointTangentManipulatorPartMgr::createHorizontalPlaneHandle()
{
    using namespace cvf;
    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray(6);
    
    vertexArray->set(0,  {-1, -1, 0} );
    vertexArray->set(1,  { 1, -1, 0});
    vertexArray->set(2,  { 1,  1, 0});
    vertexArray->set(3,  {-1, -1, 0});
    vertexArray->set(4,  { 1,  1, 0});
    vertexArray->set(5,  {-1,  1, 0});

    Vec3f origin(m_origin);
    for (cvf::Vec3f& vx: *vertexArray)
    {
        vx *= 0.5*m_handleSize;
        vx += origin;
    }

    ref<DrawableGeo> geo = createTriangelDrawableGeo(vertexArray.p());

    HandleType handleId = HORIZONTAL_PLANE;
    cvf::Color4f color =  cvf::Color4f(1.0f, 0.0f, 1.0f, 0.5f);
    cvf::String partName("PointTangentManipulator Horizontal Plane Handle");

    addHandlePart(geo.p(), color,  handleId, partName);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void  RicPointTangentManipulatorPartMgr::createVerticalAxisHandle()
{
    using namespace cvf;
    
    cvf::ref< cvf::GeometryBuilderTriangles> geomBuilder = new cvf::GeometryBuilderTriangles;
    cvf::GeometryUtils::createBox({-0.3f, -0.3f, -1.0f}, { 0.3f,  0.3f, 1.0f}, geomBuilder.p());
    
    cvf::ref<cvf::Vec3fArray> vertexArray = geomBuilder->vertices();
    cvf::ref<cvf::UIntArray> indexArray = geomBuilder->triangles();

    Vec3f origin(m_origin);
    for (cvf::Vec3f& vx: *vertexArray)
    {
        vx *= 0.5*m_handleSize;
        vx += origin;
    }

    ref<DrawableGeo> geo = createIndexedTriangelDrawableGeo(vertexArray.p(), indexArray.p());

    HandleType handleId = VERTICAL_AXIS;
    cvf::Color4f color =  cvf::Color4f(0.0f, 0.2f, 0.8f, 0.5f);
    cvf::String partName("PointTangentManipulator Vertical Axis Handle");

    addHandlePart(geo.p(), color,  handleId, partName);
}

#if 0
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void  RicPointTangentManipulatorPartMgr::createAzimuthHandle()
{
    using namespace cvf;

    cvf::ref< cvf::GeometryBuilderTriangles> geomBuilder = new cvf::GeometryBuilderTriangles;
    cvf::GeometryUtils::createDisc(1.3, 1.1, 16, geomBuilder.p());

    cvf::ref<cvf::Vec3fArray> vertexArray = geomBuilder->vertices();
    cvf::ref<cvf::UIntArray> indexArray = geomBuilder->triangles();

    Vec3f origin(m_origin);
    for (cvf::Vec3f& vx: *vertexArray)
    {
        vx *= 0.5*m_handleSize;
        vx += origin;
    }

    ref<DrawableGeo> geo = createIndexedTriangelDrawableGeo(vertexArray.p(), indexArray.p());

    HandleType handleId = AZIMUTH;
    cvf::Color4f color =  cvf::Color4f(0.0f, 0.2f, 0.8f, 0.5f);
    cvf::String partName("PointTangentManipulator Azimuth Handle");

    addHandlePart(geo.p(), color,  handleId, partName);
}

#endif

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RicPointTangentManipulatorPartMgr::createIndexedTriangelDrawableGeo(cvf::Vec3fArray* triangleVertexArray, 
                                                                                               cvf::UIntArray* triangleIndices)
{
    using namespace cvf;
    ref<DrawableGeo> geo = new DrawableGeo;
    ref<PrimitiveSetIndexedUInt> primSet = new PrimitiveSetIndexedUInt(PT_TRIANGLES, triangleIndices);

    geo->setVertexArray(triangleVertexArray);
    geo->addPrimitiveSet(primSet.p());
    geo->computeNormals();

    return geo;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RicPointTangentManipulatorPartMgr::createTriangelDrawableGeo(cvf::Vec3fArray* triangleVertexArray)
{
    using namespace cvf;
    ref<DrawableGeo> geo = new DrawableGeo;

    geo->setVertexArray(triangleVertexArray);
    ref<cvf::PrimitiveSetDirect> primSet = new cvf::PrimitiveSetDirect(cvf::PT_TRIANGLES);
    primSet->setIndexCount(triangleVertexArray->size());

    geo->addPrimitiveSet(primSet.p());
    geo->computeNormals();

    return geo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::addHandlePart(cvf::DrawableGeo* geo, 
                                                      const cvf::Color4f& color, 
                                                      HandleType handleId, 
                                                      const cvf::String& partName)
{
    cvf::ref<cvf::Part> handlePart = createPart(geo, color, partName);

    m_handleParts.push_back(handlePart.p());
    m_handleIds.push_back(handleId);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicPointTangentManipulatorPartMgr::addActiveModePart(cvf::DrawableGeo* geo, 
                                                          const cvf::Color4f& color, 
                                                          HandleType handleId, 
                                                          const cvf::String& partName)
{
    cvf::ref<cvf::Part> handlePart = createPart(geo, color, partName);

    m_activeDragModeParts.push_back(handlePart.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RicPointTangentManipulatorPartMgr::createPart(cvf::DrawableGeo* geo,
                                                                  const cvf::Color4f& color,
                                                                  const cvf::String& partName)
{
    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setName(partName);
    part->setDrawable(geo);
    part->updateBoundingBox();

    caf::SurfaceEffectGenerator surfaceGen(color, caf::PO_1);
    cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();
    part->setEffect(eff.p());
    if (color.a() < 1.0) part->setPriority(RivPartPriority::Transparent);

    return part;
}

//==================================================================================================
/// 
///
///
//==================================================================================================
namespace caf 
{
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUi3dObjectEditorHandle::PdmUi3dObjectEditorHandle()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUi3dObjectEditorHandle::~PdmUi3dObjectEditorHandle()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUi3dObjectEditorHandle::setViewer(caf::Viewer* ownerViewer)
{
    // Not allowed to change viewer. Should be constructor argument, but makes factory stuff difficult.
    CAF_ASSERT(m_ownerViewer.isNull()); 
    m_ownerViewer = ownerViewer;
}
}

//==================================================================================================
/// 
///
///
//==================================================================================================

#include "cafSelectionManager.h"
#include "RimWellPathGeometryDef.h"

namespace caf 
{
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiSelectionVisualizer3d::PdmUiSelectionVisualizer3d(caf::Viewer* ownerViewer)
    : m_ownerViewer(ownerViewer)
{
    this->setParent(ownerViewer); // Makes this owned by the viewer.
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiSelectionVisualizer3d::~PdmUiSelectionVisualizer3d()
{
    for (auto editor: m_active3DEditors)
    {
        delete editor;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmUiSelectionVisualizer3d::onSelectionManagerSelectionChanged( const std::set<int>& changedSelectionLevels )
{
    if (!changedSelectionLevels.count(0)) return;

    for (auto editor: m_active3DEditors)
    {
        delete editor;
    }

    m_active3DEditors.clear();
     
    if (!m_ownerViewer) return;

    // Todo: How do we deduce the editor from the selected object ?
    // Alt 1: Register the rim object type name as key in the factory as well
    // Alt 2: Set the editor type name as PdmUiItem::setUiEditorTypeName
    // Alt 3: Use a specific config-name in alt 2.
    // Alt 4: Introduce a PdmUiItem::editorTypeName3d

    std::vector<RimWellPathGeometryDef*> wellPathGeomDefs;
    caf::SelectionManager::instance()->objectsByType(&wellPathGeomDefs);

    for (auto geomDef: wellPathGeomDefs)
    {
        auto editor = new RicWellPathGeometry3dEditor();        
        editor->setViewer(m_ownerViewer);
        editor->setPdmObject(geomDef);
        m_active3DEditors.push_back(editor);
        editor->updateUi();
    }
    m_ownerViewer->update();
}

} // caf

//==================================================================================================
/// 
///
///
//==================================================================================================

#include "RimWellPathTarget.h"

CAF_PDM_UI_3D_OBJECT_EDITOR_SOURCE_INIT(RicWellPathGeometry3dEditor);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicWellPathGeometry3dEditor::RicWellPathGeometry3dEditor()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicWellPathGeometry3dEditor::~RicWellPathGeometry3dEditor()
{
    for (auto targetEditor: m_targetEditors)
    {
        delete targetEditor;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathGeometry3dEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    RimWellPathGeometryDef* geomDef = dynamic_cast<RimWellPathGeometryDef*>(this->pdmObject());

    for (auto targetEditor: m_targetEditors)
    {
        delete targetEditor;
    }
    m_targetEditors.clear();

    if (!geomDef) return;


    std::vector<RimWellPathTarget*> targets = geomDef->activeWellTargets();

    for (auto target: targets)
    {
        auto targetEditor = new RicWellTarget3dEditor;
        targetEditor->setViewer(m_ownerViewer);
        targetEditor->setPdmObject(target);
        m_targetEditors.push_back(targetEditor); 
        targetEditor->updateUi();
    }
}

//==================================================================================================
/// 
///
///
//==================================================================================================


#include "RimWellPathTarget.h"
#include "RiuViewer.h"
#include "cafDisplayCoordTransform.h"
#include "Rim3dView.h"
#include "RimCase.h"

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
    if (m_cvfModel.notNull() && m_ownerViewer) 
    {
        // Could result in some circularities ....
        m_ownerViewer->removeStaticModel(m_cvfModel.p());
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

    if ( !target)
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
        m_manipulator = new RicPointTangentManipulator(m_ownerViewer);
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
        m_ownerViewer->addStaticModelOnce(m_cvfModel.p());
    }

    cvf::ref<caf::DisplayCoordTransform> dispXf;
    double handleSize = 1.0;
    {
        RiuViewer* viewer = dynamic_cast<RiuViewer*>(m_ownerViewer.data());
        dispXf = viewer->ownerReservoirView()->displayCoordTransform();
        Rim3dView* view = dynamic_cast<Rim3dView*>(viewer->ownerReservoirView());
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
        RiuViewer* viewer = dynamic_cast<RiuViewer*>(m_ownerViewer.data());
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
