
#pragma once

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfMatrix4.h"
#include "cvfVector3.h"

#include <QObject>
#include <QPointer>

namespace cvf {
    class Model;
    class ModelBasicList;
};


namespace caf {
    class BoxManipulatorPartManager;
    class Viewer;
};

class QMouseEvent;



//==================================================================================================
//
//
//==================================================================================================
class RicBoxManipulatorEventHandler : public QObject
{
    Q_OBJECT

public:
    RicBoxManipulatorEventHandler(caf::Viewer* viewer);
    ~RicBoxManipulatorEventHandler();

    void setScaleZ(double scaleZ);
    void setDisplayModelOffset(cvf::Vec3d offset);

    void setOrigin(const cvf::Mat4d& origin);
    void setSize(const cvf::Vec3d& size);

    cvf::Model* model();

signals:
    void        geometryUpdated();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void            mouseMoveEvent(QMouseEvent* event);
    void            mousePressEvent(QMouseEvent* event);
    void            mouseReleaseEvent(QMouseEvent* event);


private:
    void updateParts();

    void updatePartManagerCoords();

    cvf::Vec3d  displayModelCoordFromDomainCoord(const cvf::Vec3d& domainCoord) const;

private:
    cvf::ref<caf::BoxManipulatorPartManager> m_partManager;

    cvf::ref<cvf::ModelBasicList> m_model;
    QPointer<caf::Viewer> m_viewer;

    double              m_scaleZ;
    cvf::Vec3d          m_displayModelOffset;

    cvf::Mat4d          m_domainCoordOrigin;
    cvf::Vec3d          m_domainCoordSize;

    bool m_isGeometryCreated;

};

