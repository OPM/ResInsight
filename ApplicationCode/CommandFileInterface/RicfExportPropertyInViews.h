/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Equinor ASA
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

#include "RiaDefines.h"

#include "RicfCommandObject.h"

#include "cafPdmField.h"

#include <QString>

#include <vector>

//==================================================================================================
//
//
//
//==================================================================================================
class RicfExportPropertyInViews : public RicfCommandObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicfExportPropertyInViews();

    void execute() override;

private:
    caf::PdmField<int>                  m_caseId;
    caf::PdmField<std::vector<QString>> m_viewNames;
    caf::PdmField<double>               m_undefinedValue;
};
