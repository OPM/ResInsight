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

#include "RiaDefines.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class RimEllipseFractureTemplate;
class RimFractureTemplate;

//==================================================================================================
///
///
//==================================================================================================
class RimFractureTemplateCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFractureTemplateCollection();
    ~RimFractureTemplateCollection() override;

    RimFractureTemplate*              fractureTemplate( int id ) const;
    std::vector<RimFractureTemplate*> fractureTemplates() const;
    RimEllipseFractureTemplate*       addDefaultEllipseTemplate();
    void                              addFractureTemplate( RimFractureTemplate* templ );

    caf::AppEnum<RiaDefines::EclipseUnitSystem> defaultUnitSystemType() const;
    void                                        setDefaultUnitSystemBasedOnLoadedCases();

    RimFractureTemplate* firstFractureOfUnit( RiaDefines::EclipseUnitSystem unitSet ) const;

    std::vector<std::pair<QString, QString>> resultNamesAndUnits() const;
    void                                     computeMinMax( const QString& uiResultName,
                                                            const QString& unit,
                                                            double*        minValue,
                                                            double*        maxValue,
                                                            double*        posClosestToZero,
                                                            double*        negClosestToZero ) const;

    void createAndAssignTemplateCopyForNonMatchingUnit();
    void loadAndUpdateData();

    void onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                         std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

protected:
    void initAfterRead() override;

private:
    int nextFractureTemplateId();

    caf::PdmChildArrayField<RimFractureTemplate*>              m_fractureDefinitions;
    caf::PdmField<caf::AppEnum<RiaDefines::EclipseUnitSystem>> m_defaultUnitsForFracTemplates;
    caf::PdmField<int> m_nextValidFractureTemplateId; // Unique fracture template ID within a project, used to identify
                                                      // a fracture template
};
