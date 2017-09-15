/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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
#include "RimSummaryCase.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cvfObject.h"

class RigObservedData;

//==================================================================================================
//
//==================================================================================================
class RimObservedData : public RimSummaryCase
{
    CAF_PDM_HEADER_INIT;
public:
    RimObservedData();

    void                   setSummaryHeaderFilename(const QString& fileName);
    virtual QString        summaryHeaderFilename() const  override;
    virtual QString        caseName() override;
    virtual void           updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath) override;
};
