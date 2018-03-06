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

#include "RiaEclipseUnitTools.h"

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

    RimFractureTemplate*                        fractureTemplate(int id) const;
    std::vector<RimFractureTemplate*>           fractureTemplates() const;
    void                                        addFractureTemplate(RimFractureTemplate* templ);
    RiaEclipseUnitTools::UnitSystemType         defaultUnitSystemType() const;

    RimFractureTemplate* firstFractureOfUnit(RiaEclipseUnitTools::UnitSystem unitSet) const;

    std::vector<std::pair<QString, QString> >   resultNamesAndUnits() const;
    void                                        computeMinMax(const QString& uiResultName, const QString& unit, double* minValue, double* maxValue, double* posClosestToZero, double* negClosestToZero) const;

    void                                        createAndAssignTemplateCopyForNonMatchingUnit();
    void                                        loadAndUpdateData();

    void                                        updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath);
protected:
    virtual void                                initAfterRead() override;

private:
    int                                         nextFractureTemplateId();

    caf::PdmChildArrayField<RimFractureTemplate*>           m_fractureDefinitions;
    caf::PdmField< RiaEclipseUnitTools::UnitSystemType >    m_defaultUnitsForFracTemplates;
    caf::PdmField<int>                                      m_nextValidFractureTemplateId;          // Unique fracture template ID within a project, used to identify a fracture template

};
