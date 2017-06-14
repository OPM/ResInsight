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

