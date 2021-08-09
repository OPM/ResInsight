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

//==================================================================================================
///
///
//==================================================================================================
class RimGenericParameter : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimGenericParameter();
    ~RimGenericParameter() override;

    void setName( QString name );
    void setLabel( QString labelText );
    void setDescription( QString description );
    void setAdvanced( bool isAdvanced = false );

    virtual void     setValue( QString value ) = 0;
    virtual QVariant variantValue() const      = 0;
    virtual QString  stringValue() const       = 0;
    virtual QString  jsonValue() const;

    QString name() const;
    QString label() const;
    QString description() const;
    bool    isAdvanced() const;
    bool    isValid() const;

    virtual RimGenericParameter* duplicate() const = 0;

protected:
    void setValid( bool valid );

private:
    caf::PdmField<QString> m_name;
    caf::PdmField<QString> m_label;
    caf::PdmField<QString> m_description;
    caf::PdmField<bool>    m_advanced;
    caf::PdmField<bool>    m_valid;
};
