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

#include "RimCellFilter.h"

class RimEclipseView;
class RimCellPropertyFilterCollection;
class RimEclipseResultDefinition;

class RigGridBase;
class RigCaseCellResultsData;


//==================================================================================================
///  
///  
//==================================================================================================
class RimEclipsePropertyFilter : public RimCellFilter
{
    CAF_PDM_HEADER_INIT;
 
public:
    RimEclipsePropertyFilter();
    virtual ~RimEclipsePropertyFilter();

    caf::PdmField<RimEclipseResultDefinition*>     resultDefinition;

    caf::PdmField<double>                   lowerBound;
    caf::PdmField<double>                   upperBound;

    void                                    setParentContainer(RimCellPropertyFilterCollection* parentContainer);
    RimCellPropertyFilterCollection*        parentContainer();
    void                                    setToDefaultValues();
    void                                    computeResultValueRange();

    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
protected:
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly );
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) ;
    virtual void                            defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute);

private:
    RimCellPropertyFilterCollection*        m_parentContainer;
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

