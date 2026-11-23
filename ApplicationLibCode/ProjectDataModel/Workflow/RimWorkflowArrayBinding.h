/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimWorkflowFieldBinding.h"

#include <vector>

//==================================================================================================
/// A list config field. Lists of integers, numbers and strings are edited as lists, other lists
/// and lists the list editor cannot hold exactly as JSON text. The value is stored as JSON text.
//==================================================================================================
class RimWorkflowArrayBinding : public RimWorkflowFieldBinding
{
    CAF_PDM_HEADER_INIT;

public:
    RimWorkflowArrayBinding();

    void       applySchema( const QJsonObject& fieldSchema ) override;
    QJsonValue toJsonValue() const override;
    QString    displayValue() const override;

    caf::PdmFieldHandle* valueField() override;

protected:
    void initAfterRead() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    void       setValues( const QJsonArray& values );
    QJsonArray listValues() const;

    static bool canEditAsList( const QJsonArray& values, const QString& itemType );

private:
    caf::PdmField<QString>              m_itemType;
    caf::PdmField<QString>              m_value;
    caf::PdmField<std::vector<int>>     m_intValues;
    caf::PdmField<std::vector<double>>  m_doubleValues;
    caf::PdmField<std::vector<QString>> m_stringValues;
};
