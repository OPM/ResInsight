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

#include "RimPropertyFilter.h"

#include "cafPdmChildField.h"

class RimEclipsePropertyFilterCollection;
class RimEclipseResultDefinition;

//==================================================================================================
///  
///  
//==================================================================================================
class RimEclipsePropertyFilter : public RimPropertyFilter
{
    CAF_PDM_HEADER_INIT;
 
public:
    RimEclipsePropertyFilter();
    virtual ~RimEclipsePropertyFilter();

    caf::PdmChildField<RimEclipseResultDefinition*> resultDefinition;

    void                                    rangeValues(double* lower, double* upper) const;
    bool                                    isCategorySelectionActive() const;

    void                                    setToDefaultValues();
    void                                    updateFilterName();
    void                                    computeResultValueRange();
    void                                    updateFromCurrentTimeStep();

    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void                            initAfterRead();

protected:
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);
    virtual void                            defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName);

    virtual void                            defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute);

private:
    friend class RimEclipsePropertyFilterCollection;
    friend class RicEclipsePropertyFilterFeatureImpl;

    void                                    updateActiveState();
    void                                    updateReadOnlyStateOfAllFields();
    void                                    updateRangeLabel();
    bool                                    isPropertyFilterControlled();
    void                                    setCategoriesFromTracerNames(const std::vector<QString>& tracerNames);

    RimEclipsePropertyFilterCollection*     parentContainer();

private:
    caf::PdmField<QString>                  m_rangeLabelText;
    caf::PdmField<double>                   m_lowerBound;
    caf::PdmField<double>                   m_upperBound;

    caf::PdmField<bool>                     m_useCategorySelection;

    double                                  m_minimumResultValue;
    double                                  m_maximumResultValue;

public:
    // Obsolete stuff 
    enum EvaluationRegionType
    {
        RANGE_FILTER_REGION,
        GLOBAL_REGION
    };
private:
    caf::PdmField< caf::AppEnum< EvaluationRegionType > > obsoleteField_evaluationRegion;
};

