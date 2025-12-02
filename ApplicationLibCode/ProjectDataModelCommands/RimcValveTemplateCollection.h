/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025- Equinor ASA
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
#include "RimValveTemplateCollection.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectMethod.h"

#include <QString>

//==================================================================================================
///
//==================================================================================================
class RimcValveTemplateCollection_add_template : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    enum class ValveTemplateType
    {
        ICD,
        ICV,
        AICD,
        UNDEFINED
    };

public:
    RimcValveTemplateCollection_add_template( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<caf::AppEnum<ValveTemplateType>> m_completionType;
    caf::PdmField<double>                          m_orificeDiameter;
    caf::PdmField<double>                          m_flowCoefficient;
    caf::PdmField<QString>                         m_userLabel;
};