/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025- Equinor ASA
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
#include "cafPdmObjectMethod.h"

#include <QString>

#include <expected>

//==================================================================================================
///
//==================================================================================================
class RimcCornerPointCase_replaceCornerPointGridInternal : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcCornerPointCase_replaceCornerPointGridInternal( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<int>     m_nx;
    caf::PdmField<int>     m_ny;
    caf::PdmField<int>     m_nz;
    caf::PdmField<QString> m_coordKey;
    caf::PdmField<QString> m_zcornKey;
    caf::PdmField<QString> m_actnumKey;
};
