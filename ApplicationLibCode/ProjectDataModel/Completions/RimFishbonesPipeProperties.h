/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RiaDefines.h"

#include "cvfColor3.h"
#include "cvfVector3.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <algorithm>
#include <memory>

//==================================================================================================
///
///
//==================================================================================================
class RimFishbonesPipeProperties : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFishbonesPipeProperties();

    double skinFactor() const;
    void   setSkinFactor( const double& skinFactor );

    void   setHoleDiameter( const double& diameter );
    double holeDiameter() const;
    double holeDiameter( RiaDefines::EclipseUnitSystem unitSystem ) const;

    void setUnitSystemSpecificDefaults();

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    caf::PdmField<double> m_skinFactor;
    caf::PdmField<double> m_lateralHoleDiameter;
};
