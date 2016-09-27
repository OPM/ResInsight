
#include "RicBoxManipulatorEventHandler.h"

#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfDrawableGeo.h"
#include "cafEffectGenerator.h"


#include "cvfHitItemCollection.h"

#include "cafViewer.h"
#include "cafBoxManipulatorPartManager.h"

#include <QMouseEvent>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicBoxManipulatorEventHandler::RicBoxManipulatorEventHandler(caf::Viewer* viewer)
    : m_viewer(viewer),
    m_scaleZ(1.0),
    m_isGeometryCreated(false)
{
    m_partManager = new caf::BoxManipulatorPartManager;
    m_model = new cvf::ModelBasicList;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicBoxManipulatorEventHandler::~RicBoxManipulatorEventHandler()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicBoxManipulatorEventHandler::setScaleZ(double scaleZ)
{
    m_scaleZ = scaleZ;

    updatePartManagerCoords();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicBoxManipulatorEventHandler::setDisplayModelOffset(cvf::Vec3d offset)
{
    m_displayModelOffset = offset;

    updatePartManagerCoords();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicBoxManipulatorEventHandler::setOrigin(const cvf::Mat4d& origin)
{
    m_domainCoordOrigin = origin;

    updatePartManagerCoords();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicBoxManipulatorEventHandler::setSize(const cvf::Vec3d& size)
{
    m_domainCoordSize = size;

    updatePartManagerCoords();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Model* RicBoxManipulatorEventHandler::model()
{
    return m_model.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicBoxManipulatorEventHandler::eventFilter(QObject *obj, QEvent* inputEvent)
{
    if (!m_isGeometryCreated)
    {
        updateParts();

        emit geometryUpdated();

        inputEvent->setAccepted(true);

        m_isGeometryCreated = true;

        return true;
    }

    return false;

/*
    if (inputEvent->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(inputEvent);

        cvf::HitItemCollection hitItems;
        if (m_viewer->rayPick(mouseEvent->x(), mouseEvent->y(), &hitItems))
        {
            // TODO: Test if the first hit item is part of the manipulator
/ *
            if (hitItems.firstItem() && hitItems.firstItem()->part())
            {
                const cvf::Object* siConstObj = hitItems.firstItem()->part()->sourceInfo();
                cvf::Object* siObj = const_cast<cvf::Object*>(siConstObj);

                caf::BoxManipulatorSourceInfo* sourceInfo = dynamic_cast<caf::BoxManipulatorSourceInfo*>(siObj);
                if (sourceInfo)
                {

                }
            }
* /

            switch (inputEvent->type())
            {
            case QEvent::MouseButtonPress:
                mouseMoveEvent(static_cast<QMouseEvent*>(inputEvent));
                break;
            case QEvent::MouseButtonRelease:
                mouseReleaseEvent(static_cast<QMouseEvent*>(inputEvent));
                break;
            case QEvent::MouseMove:
                mouseMoveEvent(static_cast<QMouseEvent*>(inputEvent));
                break;
            }

            updateParts();

            emit geometryUpdated();

            inputEvent->setAccepted(true);
            return true;
        }
    }

    return false;
*/
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicBoxManipulatorEventHandler::mouseMoveEvent(QMouseEvent* event)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicBoxManipulatorEventHandler::mousePressEvent(QMouseEvent* event)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicBoxManipulatorEventHandler::mouseReleaseEvent(QMouseEvent* event)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicBoxManipulatorEventHandler::updateParts()
{
    m_model->removeAllParts();

    m_partManager->appendPartsToModel(m_model.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicBoxManipulatorEventHandler::updatePartManagerCoords()
{
    cvf::Mat4d displayCoordOrigin = m_domainCoordOrigin;
    cvf::Vec3d domainCoordTrans = m_domainCoordOrigin.translation();
    displayCoordOrigin.setTranslation(displayModelCoordFromDomainCoord(domainCoordTrans));

    m_partManager->setOrigin(displayCoordOrigin);

    cvf::Vec3d domainCoordSize = m_domainCoordSize;
    domainCoordSize.z() *= m_scaleZ;
    m_partManager->setSize(domainCoordSize);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RicBoxManipulatorEventHandler::displayModelCoordFromDomainCoord(const cvf::Vec3d& domainCoord) const
{
    cvf::Vec3d coord = domainCoord - m_displayModelOffset;
    coord.z() *= m_scaleZ;

    return coord;
}
