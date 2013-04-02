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
#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "RimDefines.h"
#include "cafPdmPointer.h"


class RimReservoirView;
class RigCaseCellResultsData;

//==================================================================================================
///  
///  
//==================================================================================================
class RimResultDefinition : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimResultDefinition();
    virtual ~RimResultDefinition();

    virtual void setReservoirView(RimReservoirView* ownerReservoirView);
    RimReservoirView* reservoirView();

    caf::PdmField< caf::AppEnum< RimDefines::ResultCatType > >      resultType;
    caf::PdmField< caf::AppEnum< RimDefines::PorosityModelType > >  porosityModel;
    caf::PdmField<QString>                                          resultVariable;

    void    loadResult();
    size_t  gridScalarIndex() const;
    bool    hasStaticResult() const;
    bool    hasDynamicResult() const;
    bool    hasResult() const;


    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly );
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);

protected:

    mutable size_t m_gridScalarResultIndex;

    caf::PdmPointer<RimReservoirView> m_reservoirView;
};

