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
    virtual void    updateHolder() {}
};

//==================================================================================================
///
///
//==================================================================================================
class RimNameConfig : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimNameConfig(const RimNameConfigHolderInterface* configHolder = nullptr);
    virtual ~RimNameConfig();
    QString                          customName() const;
    void                             setCustomName(const QString& name);    
    virtual void                     enableAllAutoNameTags(bool enable) {}

    caf::PdmFieldHandle*             nameField();
    QString                          name() const;
    virtual caf::PdmUiGroup*         createUiGroup(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);
    
protected:
    virtual void                     fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    QString                          autoName() const;
    virtual void                     updateAllSettings();
    virtual void                     initAfterRead() override;

protected:
    caf::PdmField<bool>              m_isUsingAutoName_OBSOLETE;
    caf::PdmField<QString>           m_customName;
    caf::PdmProxyValueField<QString> m_autoName;

    const RimNameConfigHolderInterface* m_configHolder;
};


