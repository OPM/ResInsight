/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RicfCommandObject.h"

#include "cafPdmField.h"

class RicfCreateGridCaseGroupResult : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicfCreateGridCaseGroupResult( int caseGroupId = -1, const QString& caseGroupName = "" );

public:
    caf::PdmField<int>     caseGroupId;
    caf::PdmField<QString> caseGroupName;
};

//==================================================================================================
//
//
//
//==================================================================================================
class RicfCreateGridCaseGroup : public RicfCommandObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicfCreateGridCaseGroup();

    caf::PdmScriptResponse execute() override;

private:
    caf::PdmField<std::vector<QString>> m_casePaths;
};
