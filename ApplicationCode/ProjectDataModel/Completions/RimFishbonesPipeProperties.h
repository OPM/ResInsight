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

#include "RiaEclipseUnitTools.h"

#include "cvfBase.h"
#include "cvfVector3.h"
#include "cvfColor3.h"

#include "cafPdmObject.h"
#include "cafPdmField.h"

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
    ~RimFishbonesPipeProperties() override;

    double                              skinFactor() const { return m_skinFactor(); }
    double                              holeDiameter(RiaEclipseUnitTools::UnitSystem unitSystem) const;

    void                                setUnitSystemSpecificDefaults();

protected:
    void                        defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    caf::PdmField<double>               m_skinFactor;
    caf::PdmField<double>               m_lateralHoleDiameter;
};
