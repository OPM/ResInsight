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
class RimIntegerParameter : public RimGenericParameter
{
    CAF_PDM_HEADER_INIT;

public:
    RimIntegerParameter();
    ~RimIntegerParameter() override;

    void     setValue( int value );
    void     setValue( QString value ) override;
    QVariant variantValue() const override;
    QString  stringValue() const override;
    int      value() const;

    RimGenericParameter* duplicate() const override;

private:
    caf::PdmField<int> m_value;
};
