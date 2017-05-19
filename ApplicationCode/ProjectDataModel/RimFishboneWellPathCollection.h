/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimCheckableNamedObject.h"
#include "RimFishboneWellPath.h"

#include "cafPdmObject.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"

//==================================================================================================
//
// 
//
//==================================================================================================
class RimFishboneWellPathCollection : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFishboneWellPathCollection();
    ~RimFishboneWellPathCollection();

    void appendCompletion(RimFishboneWellPath* completion);
    void importCompletionsFromFile(const QList<QString> filePaths);

    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);

    caf::PdmChildArrayField<RimFishboneWellPath*>    m_completions;
};
