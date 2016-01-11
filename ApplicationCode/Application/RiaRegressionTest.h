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
#include "cafAppEnum.h"


class RiaRegressionTest : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RiaRegressionTest(void);
    virtual ~RiaRegressionTest(void);

public:
    caf::PdmField<QString>  applicationWorkingFolder;
    caf::PdmField<QString>  regressionTestFolder;
    caf::PdmField<bool>     showInteractiveDiffImages;

protected:
    virtual void defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute);


};
