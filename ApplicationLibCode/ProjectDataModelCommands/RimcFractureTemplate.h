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

#include "cafPdmField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmObjectMethod.h"

#include <QString>

class RimFractureTemplate;

//==================================================================================================
///
//==================================================================================================
class RimcFractureTemplate_setScaleFactors : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimcFractureTemplate_setScaleFactors( caf::PdmObjectHandle* self );

    void setScaleFactors( double halfLength, double height, double dFactor, double conductivity );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<double> m_halfLength;
    caf::PdmField<double> m_height;
    caf::PdmField<double> m_dFactor;
    caf::PdmField<double> m_conductivity;
};

//==================================================================================================
/// Set the K layer range the fractures created from this template are contained within.
//==================================================================================================
class RimFractureTemplate_setContainment : public caf::PdmVoidObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    RimFractureTemplate_setContainment( caf::PdmObjectHandle* self );

    void setLayers( int topLayer, int baseLayer );

    std::expected<caf::PdmObjectHandle*, QString> execute() override;

private:
    caf::PdmField<int> m_topLayer;
    caf::PdmField<int> m_baseLayer;
};
