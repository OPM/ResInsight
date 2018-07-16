/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
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
class RimCurveNameConfigHolderInterface
{
public:
    virtual QString createAutoName() const = 0;
    virtual void    updateHolder() {}
};

//==================================================================================================
///
///
//==================================================================================================
class RimCurveNameConfig : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimCurveNameConfig(const RimCurveNameConfigHolderInterface* configHolder = nullptr);
    virtual ~RimCurveNameConfig();
    bool                             isUsingAutoName() const;
    caf::PdmFieldHandle*             nameField();
    QString                          name() const;
    virtual caf::PdmUiGroup*         createUiGroup(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);

protected:
    virtual void                     fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    QString                          autoName() const;
    void                             setCustomName(const QString& name);
    virtual void                     updateAllSettings();
protected:
    caf::PdmField<bool>              m_isUsingAutoName;
    caf::PdmField<QString>           m_customName;
    caf::PdmProxyValueField<QString> m_autoName;

    const RimCurveNameConfigHolderInterface* m_configHolder;
};

//==================================================================================================
///
///
//==================================================================================================
class RimWellLogExtractionCurveNameConfig : public RimCurveNameConfig
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellLogExtractionCurveNameConfig(const RimCurveNameConfigHolderInterface* configHolder = nullptr);
    virtual caf::PdmUiGroup* createUiGroup(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

    bool                     addCaseName() const;
    bool                     addProperty() const;
    bool                     addWellName() const;
    bool                     addTimeStep() const;
    bool                     addDate() const;

protected:
    virtual void             updateAllSettings();

private:
    caf::PdmField<bool>              m_addCaseName;
    caf::PdmField<bool>              m_addProperty;
    caf::PdmField<bool>              m_addWellName;
    caf::PdmField<bool>              m_addTimestep;
    caf::PdmField<bool>              m_addDate;
};

//==================================================================================================
///
///
//==================================================================================================
class RimWellLogFileCurveNameConfig : public RimCurveNameConfig
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellLogFileCurveNameConfig(const RimCurveNameConfigHolderInterface* configHolder = nullptr);
};

//==================================================================================================
///
///
//==================================================================================================
class RimWellLogRftCurveNameConfig : public RimCurveNameConfig
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellLogRftCurveNameConfig(const RimCurveNameConfigHolderInterface* configHolder = nullptr);
};
