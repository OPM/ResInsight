/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-  Equinor ASA
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

#include "cafAppEnum.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafVecIjk.h"

#include "RimEclipseCase.h"

#include <optional>

//==================================================================================================
///
///
//==================================================================================================
class RimWellTargetCandidatesGenerator : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellTargetCandidatesGenerator();
    ~RimWellTargetCandidatesGenerator() override;

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void generateCandidates();

    std::optional<caf::VecIjk> findStartCell( RimEclipseCase* eclipseCase );

private:
    caf::PdmField<double> m_volume;
    caf::PdmField<double> m_pressure;
    caf::PdmField<double> m_permeability;
    caf::PdmField<double> m_transmissibility;

    caf::PdmField<bool> m_generateButton;
};
