/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RimNamedObject.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmPtrField.h"

class RimCaseCollection;
class RimEclipseCase;
class RimEclipseView;

//==================================================================================================
//
//
//
//==================================================================================================
class RimEclipseCaseEnsemble : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimEclipseCaseEnsemble();
    ~RimEclipseCaseEnsemble() override;

    void addCase( RimEclipseCase* reservoir );
    void removeCase( RimEclipseCase* reservoir );
    bool contains( RimEclipseCase* reservoir ) const;

    std::vector<RimEclipseCase*> cases() const;

    void addView( RimEclipseView* view );

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    caf::PdmField<int>                       m_groupId;
    caf::PdmChildField<RimCaseCollection*>   m_caseCollection;
    caf::PdmChildArrayField<RimEclipseView*> m_views;
    caf::PdmPtrField<RimEclipseCase*>        m_selectedCase;
};
