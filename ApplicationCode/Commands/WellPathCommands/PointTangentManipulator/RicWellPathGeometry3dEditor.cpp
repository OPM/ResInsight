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

#include "RicWellPathGeometry3dEditor.h"

#include "RicWellPathGeometry3dEditor.h"
#include "RicWellTarget3dEditor.h"

#include "RimWellPathTarget.h"
#include "RimWellPathGeometryDef.h"


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
        targetEditor->setViewer(ownerViewer());
        targetEditor->setPdmObject(target);
        m_targetEditors.push_back(targetEditor); 
        targetEditor->updateUi();
    }
}


