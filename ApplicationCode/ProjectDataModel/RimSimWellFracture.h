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

#include "RimFracture.h"

#include "RigSimulationWellCoordsAndMD.h"


//==================================================================================================
///  
///  
//==================================================================================================
class RimSimWellFracture : public RimFracture
{
     CAF_PDM_HEADER_INIT;

public:
    RimSimWellFracture(void);
    virtual ~RimSimWellFracture(void);

    caf::PdmField<float>                            measuredDepth;

    void                                            setIJK(size_t i, size_t j, size_t k);

    virtual void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

    void updateFractureAnchorPosition();

protected:
    virtual void                                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                                    defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;
    virtual QList<caf::PdmOptionItemInfo>           calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;

private:
    cvf::Vec3d                                      findCellCenterPosition(size_t i, size_t j, size_t k) const;
    void                                            updateBranchGeometry();

private:
    caf::PdmField<int>                              m_branchIndex;
    std::vector<RigSimulationWellCoordsAndMD>       m_branchCenterLines;
};
