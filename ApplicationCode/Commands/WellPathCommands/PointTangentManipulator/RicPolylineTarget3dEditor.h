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

#include "cafPdmUi3dObjectEditorHandle.h"

class RicPointTangentManipulator;

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector3.h"

namespace cvf {
class ModelBasicList;
}

class QString;
#include <QPointer>

class RicPolylineTarget3dEditor : public caf::PdmUi3dObjectEditorHandle
{
    CAF_PDM_UI_3D_OBJECT_EDITOR_HEADER_INIT;
    Q_OBJECT
public:
    RicPolylineTarget3dEditor();
    ~RicPolylineTarget3dEditor() override;

protected:
    void configureAndUpdateUi(const QString& uiConfigName) override;
    void cleanupBeforeSettingPdmObject() override;

private slots:
    void slotUpdated(const cvf::Vec3d& origin, const cvf::Vec3d& tangent);
    void slotSelectedIn3D();
    void slotDragFinished();
private:
    QPointer<RicPointTangentManipulator> m_manipulator;
    cvf::ref<cvf::ModelBasicList> m_cvfModel;
};


