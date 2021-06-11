/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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
#include "cafPdmProxyValueField.h"

//==================================================================================================
///
///
//==================================================================================================
class RimNameConfigHolderInterface
{
public:
    virtual QString createAutoName() const = 0;
    void            updateAutoName() { performAutoNameUpdate(); }

protected:
    virtual void performAutoNameUpdate() {}
};

//==================================================================================================
///
///
//==================================================================================================
class RimNameConfig : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimNameConfig( const QString& customName );
    ~RimNameConfig() override;

    QString customName() const;
    void    setCustomName( const QString& name );

    caf::PdmFieldHandle* nameField();
    QString              name() const;
    void                 uiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering );
    void                 enableAllAutoNameTags( bool enable );

protected:
    void         defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void         fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QString      autoName() const;
    virtual void updateAllSettings();

private:
    virtual void doEnableAllAutoNameTags( bool enable ) = 0;

private:
    caf::PdmField<QString>           m_customName;
    caf::PdmProxyValueField<QString> m_autoName;
};
