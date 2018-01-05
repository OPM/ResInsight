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

#include "RimPropertyFilter.h"

#include "cafPdmChildField.h"


class RimGeoMechResultDefinition;
class RimGeoMechPropertyFilterCollection;

//==================================================================================================
///  
///  
//==================================================================================================
class RimGeoMechPropertyFilter : public RimPropertyFilter
{
    CAF_PDM_HEADER_INIT;

public:
    RimGeoMechPropertyFilter();
    virtual ~RimGeoMechPropertyFilter();

    caf::PdmChildField<RimGeoMechResultDefinition*>  resultDefinition;

    caf::PdmField<double>                       lowerBound;
    caf::PdmField<double>                       upperBound;

    void                                        setParentContainer(RimGeoMechPropertyFilterCollection* parentContainer);
    RimGeoMechPropertyFilterCollection*         parentContainer();
    void                                        setToDefaultValues();
    void                                        updateFilterName();
    void                                        computeResultValueRange();
    
    void                                        updateActiveState();
    bool                                        isActiveAndHasResult();
    
protected:
    virtual void                                fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void                                defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) ;
    virtual void                                defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName);
    virtual void                                defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute);

private:
    void                                        updateReadOnlyStateOfAllFields();
    bool                                        isPropertyFilterControlled();

private:
    RimGeoMechPropertyFilterCollection*         m_parentContainer;
    double                                      m_minimumResultValue; 
    double                                      m_maximumResultValue;
};

