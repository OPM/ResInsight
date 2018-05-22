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
    virtual ~RimSimWellFracture(void);

    void                                            setClosestWellCoord(cvf::Vec3d& position, size_t branchIndex);

    virtual void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

    void                                            recomputeWellCenterlineCoordinates();
    void                                            updateFracturePositionFromLocation();
    void                                            updateAzimuthBasedOnWellAzimuthAngle() override;

    double                                          wellAzimuthAtFracturePosition() const override;
    double                                          wellDipAtFracturePosition();
    double                                          fractureMD() const override { return m_location; }

    int                                             branchIndex() const { return m_branchIndex(); }

    virtual void                                    loadDataAndUpdate() override;

    virtual std::vector<cvf::Vec3d>                 perforationLengthCenterLineCoords() const override;

protected:
    virtual void                                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                                    defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;
    virtual QList<caf::PdmOptionItemInfo>           calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;

    RigMainGrid*                                    ownerCaseMainGrid() const;

private:
    void                                            computeSimWellBranchesIfRequired();
    void                                            computeSimWellBranchCenterLines();

private:
    caf::PdmField<float>                            m_location;
    caf::PdmField<int>                              m_branchIndex;
    std::vector<RigSimulationWellCoordsAndMD>       m_branchCenterLines;

    caf::PdmProxyValueField<QString>                m_displayIJK;
    QString                                         createOneBasedIJKText() const;

};
