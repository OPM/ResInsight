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

#include "cvfVector3.h"

#include <QString>

//==================================================================================================
///
//==================================================================================================
class RimcModeledWellPath_appendLateral : public caf::PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcModeledWellPath_appendLateral( caf::PdmObjectHandle* self );

    caf::PdmObjectHandle*            execute() override;
    bool                             resultIsPersistent() const override;
    std::unique_ptr<PdmObjectHandle> defaultResult() const override;

private:
    caf::PdmField<double>  m_tieInDepth;
    caf::PdmField<QString> m_lateralName;
};
