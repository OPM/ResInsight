/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -    Equinor ASA
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

#include <QString>

#include "RimGenericParameter.h"

//==================================================================================================
///
///
//==================================================================================================
class RimStringParameter : public RimGenericParameter
{
    CAF_PDM_HEADER_INIT;

public:
    RimStringParameter();
    ~RimStringParameter() override;

    void     setValue( QString value ) override;
    QVariant variantValue() const override;
    QString  stringValue() const override;
    QString  jsonValue() const override;

    RimGenericParameter* duplicate() const override;

private:
    caf::PdmField<QString> m_value;
};
