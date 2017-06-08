/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimUnitSystem.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmChildArrayField.h"

class RimFractureTemplate;

//==================================================================================================
///  
///  
//==================================================================================================
class RimFractureTemplateCollection : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;

public:
    RimFractureTemplateCollection(void);
    virtual ~RimFractureTemplateCollection(void);
    
    caf::PdmChildArrayField<RimFractureTemplate*>               fractureDefinitions;
    caf::PdmField< RimUnitSystem::UnitSystemType >     defaultUnitsForFracTemplates;

    std::vector<std::pair<QString, QString> >   stimPlanResultNamesAndUnits() const;
    std::vector<QString>                        stimPlanResultNames() const;
    void                                        computeMinMax(const QString& resultName, const QString& unit, double* minValue, double* maxValue) const;

    void                                        deleteFractureDefinitions();
    void                                        loadAndUpdateData();

};
