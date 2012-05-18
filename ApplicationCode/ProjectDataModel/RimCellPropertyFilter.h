/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafPdmDocument.h"
#include "cafAppEnum.h"

#include "RimDefines.h"
#include "RimCellFilter.h"
#include "cvfStructGridGeometryGenerator.h"

class RimReservoirView;
class RimCellPropertyFilterCollection;
class RimResultDefinition;

class RigGridBase;
class RigReservoirCellResults;
namespace cvf
{
    //enum CellRangeFilter::CellStateType;
}

//==================================================================================================
///  
///  
//==================================================================================================
class RimCellPropertyFilter : public RimCellFilter
{
    CAF_PDM_HEADER_INIT;

    enum EvaluationRegionType
    {
        RANGE_FILTER_REGION,
        GLOBAL_REGION
    };

public:
    RimCellPropertyFilter();
    virtual ~RimCellPropertyFilter();

    void setParentContainer(RimCellPropertyFilterCollection* parentContainer);
    RimCellPropertyFilterCollection* parentContainer();
    void setDefaultValues();

    caf::PdmField<RimResultDefinition*> resultDefinition;

    caf::PdmField<double>   lowerBound;
    caf::PdmField<double>   upperBound;
    caf::PdmField< caf::AppEnum< EvaluationRegionType > > evaluationRegion;

    bool isCellRejected(const RigGridBase* grid, size_t timeStepIndex, size_t cellIndex) const;
    cvf::CellRangeFilter::CellStateType  cellFilterState(const RigGridBase* grid, size_t timeStepIndex, size_t cellIndex) const;

    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);

    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly );

private:
    RimCellPropertyFilterCollection* m_parentContainer;
};



