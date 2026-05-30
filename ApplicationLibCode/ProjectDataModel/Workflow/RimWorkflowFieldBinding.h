/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

class QJsonObject;

class RimWorkflowFieldBinding : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWorkflowFieldBinding();

    QString fieldName() const;
    void    setFieldName( const QString& name );
    void    setDescription( const QString& description );
    void    setRequired( bool required );

    virtual void    applySchema( const QJsonObject& fieldSchema );
    virtual QString toYamlValue() const = 0;

protected:
    caf::PdmField<QString> m_fieldName;
    caf::PdmField<QString> m_description;
    caf::PdmField<bool>    m_required;
};
