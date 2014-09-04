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

#include "RimDefines.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"


class RimReservoirView;
class RigCaseCellResultsData;
class RimReservoirCellResultsStorage;
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

    virtual void                    setReservoirView(RimReservoirView* ownerReservoirView);
    RimReservoirView*               reservoirView();

    RimDefines::ResultCatType       resultType() const { return m_resultType(); }
    void                            setResultType(RimDefines::ResultCatType val);
    RimDefines::PorosityModelType   porosityModel() const { return m_porosityModel(); }
    void                            setPorosityModel(RimDefines::PorosityModelType val);
    QString                         resultVariable() const { return m_resultVariable(); }
    virtual void                    setResultVariable(const QString& val);

    void                            loadResult();
    size_t                          scalarResultIndex() const;
    bool                            hasStaticResult() const;
    bool                            hasDynamicResult() const;
    bool                            hasResult() const;
    bool                            isTernarySaturationSelected() const;

    RimReservoirCellResultsStorage* currentGridCellResults() const;


    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly );
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void initAfterRead();

protected:
    caf::PdmField< caf::AppEnum< RimDefines::ResultCatType > >      m_resultType;
    caf::PdmField< caf::AppEnum< RimDefines::PorosityModelType > >  m_porosityModel;
    caf::PdmField<QString>                                          m_resultVariable;

    friend class RimCellPropertyFilter;
    friend class RimFaultResultSlot;
    // User interface only fields, to support "filtering"-like behaviour etc.
    caf::PdmField< caf::AppEnum< RimDefines::ResultCatType > >      m_resultTypeUiField;
    caf::PdmField< caf::AppEnum< RimDefines::PorosityModelType > >  m_porosityModelUiField;
    caf::PdmField<QString>                                          m_resultVariableUiField;


    //mutable size_t                                                  m_gridScalarResultIndex;

    caf::PdmPointer<RimReservoirView>                               m_reservoirView;

protected:
    void updateFieldVisibility();

private:
    QStringList getResultVariableListForCurrentUIFieldSettings();
};

