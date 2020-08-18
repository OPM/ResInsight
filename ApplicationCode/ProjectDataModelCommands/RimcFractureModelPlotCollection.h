/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include "RimFractureModelPlotCollection.h"

#include "cafPdmField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"

class RimFractureModelPlotCollection;
class RimWellPath;
class RimFractureModel;
class RimEclipseCase;

//==================================================================================================
///
//==================================================================================================
class RimcFractureModelPlotCollection_newFractureModelPlot : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcFractureModelPlotCollection_newFractureModelPlot( caf::PdmObjectHandle* self );

    caf::PdmObjectHandle*            execute();
    bool                             resultIsPersistent() const override;
    std::unique_ptr<PdmObjectHandle> defaultResult() const override;

private:
    caf::PdmPtrField<RimEclipseCase*>   m_eclipseCase;
    caf::PdmPtrField<RimFractureModel*> m_fractureModel;
    caf::PdmField<int>                  m_timeStep;
};
