/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <vector>

class RimWellPathFracture;

//==================================================================================================
///
///
//==================================================================================================
class RimWellPathFractureCollection : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellPathFractureCollection( void );
    ~RimWellPathFractureCollection( void ) override;

    bool hasFractures() const;
    void addFracture( RimWellPathFracture* fracture );
    void deleteFractures();

    std::vector<RimWellPathFracture*> allFractures() const;
    std::vector<RimWellPathFracture*> activeFractures() const;

    void onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                         std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    caf::PdmChildArrayField<RimWellPathFracture*> m_fractures;
};
