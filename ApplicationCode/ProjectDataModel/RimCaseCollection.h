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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmChildArrayField.h"

class RimEclipseCase;
class RimIdenticalGridCaseGroup;

//==================================================================================================
//
// 
//
//==================================================================================================
class RimCaseCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimCaseCollection();
    ~RimCaseCollection() override;
    caf::PdmChildArrayField<RimEclipseCase*> reservoirs;

    RimIdenticalGridCaseGroup*  parentCaseGroup();
    RimEclipseCase*                    findByDescription(const QString& caseDescription) const;

private:

};
