/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

class RimEclipseCaseCollection;
class RimGeoMechModels;
class RimWellPathCollection;
class RimFractureTemplateCollection;
class RimSummaryCaseCollection;
class RimFormationNamesCollection;

//==================================================================================================
///  
///  
//==================================================================================================
class RimOilField : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;

public:
    RimOilField(void);
    virtual ~RimOilField(void);

    caf::PdmChildField<RimEclipseCaseCollection*>           analysisModels;
    caf::PdmChildField<RimGeoMechModels*>                   geoMechModels;
    caf::PdmChildField<RimWellPathCollection*>              wellPathCollection;
    caf::PdmChildField<RimFractureTemplateCollection*>    fractureDefinitionCollection;
    caf::PdmChildField<RimSummaryCaseCollection*>           summaryCaseCollection;
    caf::PdmChildField<RimFormationNamesCollection*>        formationNamesCollection;

};
