/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 -    Equinor
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

#include "cvfBase.h"
#include "cvfVector3.h"
#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmCoreVec3d.h"
#include "RiaLineArcWellPathCalculator.h"

class RimPolylineTarget : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimPolylineTarget();
    ~RimPolylineTarget() override;

    bool isEnabled() const;

    void setAsPointTargetXYD(const cvf::Vec3d& point);
    void setAsPointXYZAndTangentTarget(const cvf::Vec3d& point, double azimuth, double inclination);

    cvf::Vec3d     targetPointXYZ() const;

private:
    QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    friend class RicPolylineTarget3dEditor;
    void                                         enableFullUpdate(bool enable);
    bool                                         m_isFullUpdateEnabled;
    caf::PdmField<bool>                          m_isEnabled;
    caf::PdmField<cvf::Vec3d>                    m_targetPoint;

};

