/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

class RimGeoMechCase;

//==================================================================================================
///
///
//==================================================================================================
class RimGeoMechModels : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimGeoMechModels();
    ~RimGeoMechModels() override;

    RimGeoMechCase* copyCase( RimGeoMechCase* thecase, const QString& newInputFileName );
    void            removeCase( RimGeoMechCase* thecase );
    void            addCase( RimGeoMechCase* thecase );

    std::vector<RimGeoMechCase*> cases() const;

private:
    caf::PdmChildArrayField<RimGeoMechCase*> m_cases;
};
