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
#include "cafPdmUi3dObjectEditorHandle.h"
#include "cafPdmUiFieldEditorHandle.h"
#include <memory>

namespace caf
{
class PickEventHandler;
}

class RicWellTarget3dEditor;

class RicWellPathGeometry3dEditorAttribute : public caf::PdmUiEditorAttribute
{
public:
    RicWellPathGeometry3dEditorAttribute()
        : enablePicking( false )
    {
    }
    bool                                   enablePicking;
    std::shared_ptr<caf::PickEventHandler> pickEventHandler;
};

class RicWellPathGeometry3dEditor : public caf::PdmUi3dObjectEditorHandle
{
    CAF_PDM_UI_3D_OBJECT_EDITOR_HEADER_INIT;
    Q_OBJECT
public:
    RicWellPathGeometry3dEditor();
    ~RicWellPathGeometry3dEditor() override;

protected:
    void configureAndUpdateUi( const QString& uiConfigName ) override;

private:
    std::vector<RicWellTarget3dEditor*>  m_targetEditors;
    RicWellPathGeometry3dEditorAttribute m_attribute;
};
