/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "Ric3dObjectEditorHandle.h"

#include "cvfObject.h"
#include "cvfVector3.h"

#include <QPointer>

class RicPointTangentManipulator;
class RimWellPathTarget;

namespace cvf
{
class ModelBasicList;
}

class QString;

class RicWellTarget3dEditor : public Ric3dObjectEditorHandle
{
    CAF_PDM_UI_3D_OBJECT_EDITOR_HEADER_INIT;
    Q_OBJECT
public:
    RicWellTarget3dEditor();
    ~RicWellTarget3dEditor() override;

protected:
    void configureAndUpdateUi( const QString& uiConfigName ) override;
    void cleanupBeforeSettingPdmObject() override;

private slots:
    void slotUpdated( const cvf::Vec3d& origin, const cvf::Vec3d& tangent );
    void slotSelectedIn3D();
    void slotDragFinished();

private:
    void removeAllFieldEditors();

    static void updateTargetWithDeltaChange( RimWellPathTarget* target, const cvf::Vec3d& delta );

private:
    QPointer<RicPointTangentManipulator> m_manipulator;
    cvf::ref<cvf::ModelBasicList>        m_cvfModel;
};
