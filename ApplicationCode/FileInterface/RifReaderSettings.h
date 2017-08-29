/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "cafPdmField.h"
#include "cafPdmObject.h"


//==================================================================================================
///  
///  
//==================================================================================================
class RifReaderSettings : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

    friend class RiaPreferences;

public:
    RifReaderSettings();

    caf::PdmField<bool> importFaults;
    caf::PdmField<bool> importNNCs;
    caf::PdmField<bool> importAdvancedMswData;
    caf::PdmField<QString> faultIncludeFileAbsolutePathPrefix;
    caf::PdmField<bool> useResultIndexFile;

protected:
    virtual void defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute);

    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

};

