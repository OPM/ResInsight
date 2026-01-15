/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 - Equinor ASA
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
class RimResultNameAlias : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimResultNameAlias();
    ~RimResultNameAlias() override;

    void setResultNameAndAlias( const QString& resultName, const QString& aliasName );

    QString resultName() const { return m_resultName; }
    QString aliasName() const { return m_aliasName; }

protected:
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    caf::PdmField<QString> m_resultName;
    caf::PdmField<QString> m_aliasName;
};
