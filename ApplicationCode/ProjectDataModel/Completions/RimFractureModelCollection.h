/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor
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

#include "RimCheckableNamedObject.h"

#include "cafPdmChildArrayField.h"

#include <vector>

class RimFractureModel;

//==================================================================================================
///
///
//==================================================================================================
class RimFractureModelCollection : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFractureModelCollection( void );
    ~RimFractureModelCollection( void ) override;

    void addFractureModel( RimFractureModel* fracture );
    void deleteFractureModels();

    std::vector<RimFractureModel*> allFractureModels() const;
    std::vector<RimFractureModel*> activeFractureModels() const;

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    caf::PdmChildArrayField<RimFractureModel*> m_fractureModels;
};
