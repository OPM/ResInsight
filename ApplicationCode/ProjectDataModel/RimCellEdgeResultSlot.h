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
#include "RimLegendConfig.h"
#include "cafAppEnum.h"
#include "RimDefines.h"

class RigCaseCellResultsData;

namespace caf
{
template <typename T, size_t vectorSize>
class fvector
{
public:
    T v[vectorSize];
    T& operator[] (size_t idx) { return v[idx]; }
const T& operator[] (size_t idx) const { return v[idx]; }

};
}

//==================================================================================================
///  
///  
//==================================================================================================
class RimCellEdgeResultSlot :  public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimCellEdgeResultSlot();
    virtual ~RimCellEdgeResultSlot();

    enum EdgeFaceType
    {
        X, NEG_X, 
        Y, NEG_Y, 
        Z, NEG_Z
    };

    typedef  caf::AppEnum<RimCellEdgeResultSlot::EdgeFaceType> EdgeFaceEnum;

    void                                  setReservoirView(RimReservoirView* ownerReservoirView);

    caf::PdmField<QString>                resultVariable;
    caf::PdmField<bool>                   useXVariable;
    caf::PdmField<bool>                   useYVariable;
    caf::PdmField<bool>                   useZVariable;

    caf::PdmField<RimLegendConfig*>       legendConfig;
    double                                ignoredScalarValue() { return m_ignoredResultScalar; }
    void                                  gridScalarIndices(size_t resultIndices[6]);
    void                                  gridScalarResultNames(QStringList* resultNames);
    void                                  loadResult();
    bool                                  hasResult() const; 

    void                                  minMaxCellEdgeValues(double& min, double& max);
protected:

    virtual void                          fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly );
    QStringList                           findResultVariableNames();

private:
    void                                  resetResultIndices();
    void                                  updateIgnoredScalarValue();
protected:
    caf::fvector<std::pair<QString, size_t>, 6 >     m_resultNameToIndexPairs;
    caf::PdmPointer<RimReservoirView>                m_reservoirView;
    double                                           m_ignoredResultScalar;
};

