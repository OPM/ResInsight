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
    class Viewer;
};

class QMouseEvent;

class RicPointTangentManipulatorPartMgr;

//==================================================================================================
//
//
//==================================================================================================
class RicPointTangentManipulator : public QObject
{
    Q_OBJECT

public:
    explicit RicPointTangentManipulator(caf::Viewer* viewer);
    ~RicPointTangentManipulator() override;

    void setOrigin(const cvf::Vec3d& origin);
    void setTangent(const cvf::Vec3d& tangent);
    void setHandleSize(double handleSize);

    void appendPartsToModel(cvf::ModelBasicList* model);

signals:
    void        notifySelected();
    void        notifyDragFinished();
    void        notifyUpdate(const cvf::Vec3d& origin, const cvf::Vec3d& tangent);

protected:
    bool        eventFilter(QObject *obj, QEvent *event) override;

private:
    QPointer<caf::Viewer>           m_viewer;

    cvf::ref<RicPointTangentManipulatorPartMgr> m_partManager;
};

