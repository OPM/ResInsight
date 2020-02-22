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
#include "RimMswCompletionParameters.h"

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

    const RimMswCompletionParameters* mswParameters() const;
    void                              addFracture( RimWellPathFracture* fracture );
    void                              deleteFractures();
    void                              setUnitSystemSpecificDefaults();

    std::vector<RimWellPathFracture*> allFractures() const;
    std::vector<RimWellPathFracture*> activeFractures() const;

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void initAfterRead() override;

private:
    caf::PdmChildArrayField<RimWellPathFracture*>   m_fractures;
    caf::PdmChildField<RimMswCompletionParameters*> m_mswParameters;

    caf::PdmField<int>    m_refMDType_OBSOLETE;
    caf::PdmField<double> m_refMD_OBSOLETE;
};
