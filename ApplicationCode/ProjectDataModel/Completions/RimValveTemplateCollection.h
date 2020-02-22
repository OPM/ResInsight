/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class RimValveTemplate;

//==================================================================================================
///
///
//==================================================================================================
class RimValveTemplateCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimValveTemplateCollection();
    ~RimValveTemplateCollection() override;

    std::vector<RimValveTemplate*> valveTemplates() const;
    void                           addValveTemplate( RimValveTemplate* valveTemplate );
    void                           removeAndDeleteValveTemplate( RimValveTemplate* valveTemplate );
    void                           addDefaultValveTemplates();

    RiaEclipseUnitTools::UnitSystemType defaultUnitSystemType() const;
    void                                setDefaultUnitSystemBasedOnLoadedCases();

private:
    caf::PdmChildArrayField<RimValveTemplate*>         m_valveDefinitions;
    caf::PdmField<RiaEclipseUnitTools::UnitSystemType> m_defaultUnitsForValveTemplates;
};
