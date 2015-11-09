/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafPdmChildArrayField.h"

#include "RimWellsEntry.h"



class RimOilFieldEntry : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimOilFieldEntry();
    ~RimOilFieldEntry();


    caf::PdmField<QString>          name;
    caf::PdmField<QString>          edmId;
    caf::PdmField<bool>             selected;
    caf::PdmField<QString>          wellsFilePath;  // Location of the response file from request "/wells"

    caf::PdmChildArrayField<RimWellPathEntry*> wells;

    RimWellPathEntry* find(const QString& name, RimWellPathEntry::WellTypeEnum wellPathType);


    virtual caf::PdmFieldHandle*    userDescriptionField();
    virtual caf::PdmFieldHandle*    objectToggleField();
    virtual void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void                    initAfterRead();

//private:
    void updateEnabledState();

};


