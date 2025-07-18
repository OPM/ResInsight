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
class RimcRimWellPathGeometryDef_appendNewWellTarget : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcRimWellPathGeometryDef_appendNewWellTarget( caf::PdmObjectHandle* self );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;
    QString                                       classKeywordReturnedType() const override;

private:
    caf::PdmField<cvf::Vec3d> m_coordinate;
    caf::PdmField<bool>       m_isAbsolute;

    caf::PdmField<bool>   m_useFixedAzimuth;
    caf::PdmField<double> m_fixedAzimuthValue;
    caf::PdmField<bool>   m_useFixedInclination;
    caf::PdmField<double> m_fixedInclinationValue;
};
