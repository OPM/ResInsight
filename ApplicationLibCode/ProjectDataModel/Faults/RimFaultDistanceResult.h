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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrArrayField.h"

#include <QString>

#include <vector>

class RigFault;
class RimFaultInView;

//==================================================================================================
///
//==================================================================================================
class RimFaultDistanceResult : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFaultDistanceResult();

    QString resultName() const;
    void    setResultName( const QString& name );

    void                         setSelectedFaults( const std::vector<RimFaultInView*>& faults );
    std::vector<const RigFault*> selectedRigFaults() const;

    void compute();

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    caf::PdmFieldHandle*          userDescriptionField() override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    void removeGeneratedResult( const QString& name );

    caf::PdmPtrArrayField<RimFaultInView*> m_faults;
    caf::PdmField<QString>                 m_resultName;
};
