
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
    explicit RicBoxManipulatorEventHandler(caf::Viewer* viewer);
    ~RicBoxManipulatorEventHandler();

    void setOrigin(const cvf::Vec3d& origin);
    void setSize(const cvf::Vec3d& size);

    void appendPartsToModel(cvf::ModelBasicList* model);

signals:
    void        notifyRedraw();
    void        notifyUpdate(const cvf::Vec3d& origin, const cvf::Vec3d& size);

protected:
    bool        eventFilter(QObject *obj, QEvent *event);

private:
    QPointer<caf::Viewer>           m_viewer;

    cvf::ref<caf::BoxManipulatorPartManager> m_partManager;
};

