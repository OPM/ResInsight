/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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

#include "cafPdmField.h"
#include "cafPdmObject.h"

class RigMainGrid;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RimFractureContainment : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimFractureContainment();
    ~RimFractureContainment();

    bool isEnabled() const;
    bool isEclipseCellOpenForFlow(const RigMainGrid*      mainGrid,
                                  size_t                  globalCellIndex,
                                  const std::set<size_t>& reservoirCellIndicesOpenForFlow) const;

    void setTopKLayer(int topKLayer);
    int  topKLayer() const;

    void setBaseKLayer(int baseKLayer);
    int  baseKLayer() const;

    double minimumFaultThrow() const; // Negative value means do not test for fault throw

private:
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

private:
    caf::PdmField<bool> m_useContainment;
    caf::PdmField<int>  m_topKLayer;
    caf::PdmField<int>  m_baseKLayer;

    caf::PdmField<bool>  m_truncateAtFaults;
    caf::PdmField<float> m_minimumFaultThrow;
};
