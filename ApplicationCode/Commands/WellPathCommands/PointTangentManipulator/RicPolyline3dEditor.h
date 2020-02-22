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
};

class RicPolylineTarget3dEditor;

//==================================================================================================
///
//==================================================================================================
class RicPolyline3dEditorAttribute : public caf::PdmUiEditorAttribute
{
public:
    RicPolyline3dEditorAttribute()
        : enablePicking( false )
    {
    }

public:
    bool                                   enablePicking;
    std::shared_ptr<caf::PickEventHandler> pickEventHandler;
};

//==================================================================================================
///
//==================================================================================================
class RicPolyline3dEditor : public caf::PdmUi3dObjectEditorHandle
{
    CAF_PDM_UI_3D_OBJECT_EDITOR_HEADER_INIT;
    Q_OBJECT
public:
    RicPolyline3dEditor();
    ~RicPolyline3dEditor() override;

protected:
    void configureAndUpdateUi( const QString& uiConfigName ) override;

private:
    std::vector<RicPolylineTarget3dEditor*> m_targetEditors;
    RicPolyline3dEditorAttribute            m_attribute;
};
