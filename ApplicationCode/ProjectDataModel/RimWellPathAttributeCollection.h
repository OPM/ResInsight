/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 -    Equinor ASA
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

#include "RimWellPathAttribute.h"

#include "cafAppEnum.h"
#include "cvfBase.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class RimWellPathAttributeCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimWellPathAttributeCollection();
    ~RimWellPathAttributeCollection();

    std::vector<RimWellPathAttribute*> attributes() const;
    void                               insertAttribute(RimWellPathAttribute* insertBefore, RimWellPathAttribute* attribute);
    void                               deleteAttribute(RimWellPathAttribute* attributeToDelete);

protected:
    virtual void defineCustomContextMenu(const caf::PdmFieldHandle* fieldNeedingMenu, QMenu* menu, QWidget* fieldEditorWidget) override;
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
private:
    caf::PdmChildArrayField<RimWellPathAttribute*> m_attributes;
};