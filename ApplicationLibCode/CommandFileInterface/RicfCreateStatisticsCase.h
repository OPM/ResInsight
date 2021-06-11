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

class RicfCreateStatisticsCaseResult : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicfCreateStatisticsCaseResult( int caseId = -1 );

public:
    caf::PdmField<int> caseId;
};

//==================================================================================================
//
//
//
//==================================================================================================
class RicfCreateStatisticsCase : public RicfCommandObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicfCreateStatisticsCase();

    caf::PdmScriptResponse execute() override;

private:
    caf::PdmField<int> m_caseGroupId;
};
