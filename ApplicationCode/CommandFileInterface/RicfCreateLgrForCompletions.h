/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "ExportCommands/RicLgrSplitType.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"

class RimWellPath;

//==================================================================================================
//
//
//
//==================================================================================================
class RicfCreateLgrForCompletions : public RicfCommandObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicfCreateLgrForCompletions();

    RicfCommandResponse execute() override;

private:
    caf::PdmField<int>                  m_caseId;
    caf::PdmField<int>                  m_timeStep;
    caf::PdmField<std::vector<QString>> m_wellPathNames;
    caf::PdmField<int>                  m_refinementI;
    caf::PdmField<int>                  m_refinementJ;
    caf::PdmField<int>                  m_refinementK;
    caf::PdmField<Lgr::SplitTypeEnum>   m_splitType;
};
