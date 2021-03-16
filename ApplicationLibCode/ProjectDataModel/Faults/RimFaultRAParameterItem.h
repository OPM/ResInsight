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

#include <map>

#include <QString>

//==================================================================================================
///
///
//==================================================================================================
class RimFaultRAParameterItem : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFaultRAParameterItem();
    ~RimFaultRAParameterItem() override;

    void setValue( QString parametername, QString parametervalue );

    bool    isNumber() const;
    bool    isString() const;
    QString parameterName() const;
    QString stringValue() const;
    double  doubleValue() const;

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    caf::PdmField<QString> m_parameterName;
    caf::PdmField<QString> m_parameterValue;
};
