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
#include "RimCase.h"

class RifReaderInterface;
class RigMainGrid;

//==================================================================================================
//
// 
//
//==================================================================================================
class RimResultCase : public RimCase
{
    CAF_PDM_HEADER_INIT;

public:
    RimResultCase();
    RimResultCase(const QString& caseName, const QString& caseFileName, const QString& caseDirectory);
    virtual ~RimResultCase();


    // Fields:                        
    caf::PdmField<QString>      caseFileName;
    caf::PdmField<QString>      caseDirectory;

    virtual bool                openEclipseGridFile();
    bool                        openAndReadActiveCellData(RigCaseData* mainEclipseCase);
    void                        readGridDimensions(std::vector< std::vector<int> >& gridDimensions);

    //virtual caf::PdmFieldHandle*    userDescriptionField()  { return &caseName;}

    virtual QString locationOnDisc() const;

private:
    cvf::ref<RifReaderInterface> createMockModel(QString modelName);

    QString createAbsoluteFilenameFromCase(const QString& caseName);

};
