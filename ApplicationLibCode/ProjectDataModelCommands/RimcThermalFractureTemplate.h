/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmPtrField.h"

#include <QString>

class RimThermalFractureTemplate;

//==================================================================================================
///
//==================================================================================================
class RimcThermalFractureTemplate_exportToFile : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcThermalFractureTemplate_exportToFile( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<QString> m_filePath;
    caf::PdmField<int>     m_timeStep;
};

//==================================================================================================
///
//==================================================================================================
class RimcThermalFractureTemplate_timeSteps : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcThermalFractureTemplate_timeSteps( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    std::unique_ptr<PdmObjectHandle>              defaultResult() const override;
};
