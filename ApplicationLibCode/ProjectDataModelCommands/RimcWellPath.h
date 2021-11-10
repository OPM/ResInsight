/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmPtrField.h"

#include <QString>

class RimStimPlanFractureTemplate;

//==================================================================================================
///
//==================================================================================================
class RimcWellPath_addFracture : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellPath_addFracture( caf::PdmObjectHandle* self );

    caf::PdmObjectHandle*            execute() override;
    bool                             resultIsPersistent() const override;
    std::unique_ptr<PdmObjectHandle> defaultResult() const override;

private:
    caf::PdmField<double>                          m_md;
    caf::PdmPtrField<RimStimPlanFractureTemplate*> m_stimPlanFractureTemplate;
};

//==================================================================================================
///
//==================================================================================================
class RimcWellPath_appendPerforationInterval : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcWellPath_appendPerforationInterval( caf::PdmObjectHandle* self );

    caf::PdmObjectHandle*            execute() override;
    bool                             resultIsPersistent() const override;
    std::unique_ptr<PdmObjectHandle> defaultResult() const override;

private:
    caf::PdmField<double> m_startMD;
    caf::PdmField<double> m_endMD;
    caf::PdmField<double> m_diameter;
    caf::PdmField<double> m_skinFactor;
};
