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

#include "RimStimPlanModelCollection.h"

#include "cafPdmField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"

#include <QString>

class RimStimPlanModelCollection;
class RimStimPlanModelTemplate;
class RimWellPath;
class RimEclipseCase;

//==================================================================================================
///
//==================================================================================================
class RimcStimPlanModelCollection_newStimPlanModel : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcStimPlanModelCollection_newStimPlanModel( caf::PdmObjectHandle* self );

    caf::PdmObjectHandle*            execute() override;
    bool                             resultIsPersistent() const override;
    std::unique_ptr<PdmObjectHandle> defaultResult() const override;

private:
    caf::PdmPtrField<RimEclipseCase*>           m_eclipseCase;
    caf::PdmField<int>                          m_timeStep;
    caf::PdmPtrField<RimWellPath*>              m_wellPath;
    caf::PdmField<double>                       m_md;
    caf::PdmPtrField<RimStimPlanModelTemplate*> m_stimPlanModelTemplate;
};
