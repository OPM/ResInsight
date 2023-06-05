/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RiaDefines.h"

#include "cafPdmChildField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

class RimEclipseResultDefinition;
class RimEclipseCase;

//==================================================================================================
///
///
//==================================================================================================
class RimResultSelectionUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimResultSelectionUi();

    void setEclipseResultAddress( RimEclipseCase* eclipseCase, caf::AppEnum<RiaDefines::ResultCatType> resultType, const QString& resultName );

    RimEclipseCase*           eclipseCase() const;
    RiaDefines::ResultCatType resultType() const;
    QString                   resultVariable() const;

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    caf::PdmPtrField<RimEclipseCase*>               m_eclipseCase;
    caf::PdmChildField<RimEclipseResultDefinition*> m_eclipseResult;
};
