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
#include "RimEllipseFractureTemplate.h"

//==================================================================================================
///
///
//==================================================================================================
class RimSimWellFracture : public RimFracture
{
    CAF_PDM_HEADER_INIT;

public:
    RimSimWellFracture(void);
    ~RimSimWellFracture(void) override;

    void setClosestWellCoord(cvf::Vec3d& position, size_t branchIndex);

    void recomputeWellCenterlineCoordinates();
    void updateFracturePositionFromLocation();
    void updateAzimuthBasedOnWellAzimuthAngle() override;

    double wellAzimuthAtFracturePosition() const override;
    double wellDipAtFracturePosition();
    double fractureMD() const override
    {
        return m_location;
    }

    int branchIndex() const
    {
        return m_branchIndex();
    }

    void loadDataAndUpdate() override;

    std::vector<cvf::Vec3d> perforationLengthCenterLineCoords() const override;

protected:
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void defineEditorAttribute(const caf::PdmFieldHandle* field,
                               QString                    uiConfigName,
                               caf::PdmUiEditorAttribute* attribute) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                        bool*                      useOptionsOnly) override;

private:
    RigMainGrid* ownerCaseMainGrid() const;
    void         computeSimWellBranchesIfRequired();
    void         computeSimWellBranchCenterLines();
    QString      createOneBasedIJKText() const;

private:
    caf::PdmField<float>                      m_location;
    caf::PdmField<int>                        m_branchIndex;
    std::vector<RigSimulationWellCoordsAndMD> m_branchCenterLines;

    caf::PdmProxyValueField<QString> m_displayIJK;
};
