/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 -    equinor ASA
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

#include "RimWellPath.h"

#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafAppEnum.h"
#include "cafPdmPtrField.h"

class RimWellPathTarget;
class RimWellPath;
class RimWellPathGeometryDef;

class RimModeledWellPath: public RimWellPath
{
    CAF_PDM_HEADER_INIT; 
public:

    RimModeledWellPath();
    ~RimModeledWellPath();

    void createWellPathGeometry();
    void updateWellPathVisualization();

private:
   

    caf::PdmChildField<RimWellPathGeometryDef*> m_geometryDefinition;

    virtual void defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName) override;

};

