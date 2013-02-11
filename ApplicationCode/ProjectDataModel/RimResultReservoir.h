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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "RimReservoir.h"

 class RifReaderInterface;

//==================================================================================================
//
// 
//
//==================================================================================================
class RimResultReservoir : public RimReservoir
{
    CAF_PDM_HEADER_INIT;

public:
    RimResultReservoir();
    virtual ~RimResultReservoir();


    // Fields:                        
    caf::PdmField<QString>      caseFileName;
    caf::PdmField<QString>      caseDirectory;

    virtual bool                openEclipseGridFile();

    //virtual caf::PdmFieldHandle*    userDescriptionField()  { return &caseName;}

    virtual QString locationOnDisc() const;

private:
    cvf::ref<RifReaderInterface> createMockModel(QString modelName);

    QString createAbsoluteFilenameFromCase(const QString& caseName);

};
