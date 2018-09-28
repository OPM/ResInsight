/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 -    equinor
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

class RimWellPathTarget : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimWellPathTarget();
    ~RimWellPathTarget();

    bool isEnabled() const;

    void setAsPointTargetXYD(const cvf::Vec3d& point);
    void setAsPointXYZAndTangentTarget(const cvf::Vec3d& point, double azimuth, double inclination);
    void setDerivedTangent(double azimuth, double inclination);

    RiaLineArcWellPathCalculator::WellTarget wellTargetData();

    enum TargetTypeEnum { POINT_AND_TANGENT, POINT };
    TargetTypeEnum targetType() const;
    cvf::Vec3d     targetPointXYZ() const;
    double         azimuth() const;
    double         inclination() const;
    cvf::Vec3d     tangent() const;
    double         radius1() const;
    double         radius2() const;
    void           flagRadius1AsIncorrect(bool isIncorrect, double actualRadius);
    void           flagRadius2AsIncorrect(bool isIncorrect, double actualRadius);

private:
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    friend class RicWellTarget3dEditor;
    void                                         enableFullUpdate(bool enable);
    bool                                         m_isFullUpdateEnabled;
    caf::PdmField<bool>                          m_isEnabled;
    caf::PdmField<caf::AppEnum<TargetTypeEnum> > m_targetType;
    caf::PdmField<cvf::Vec3d>                    m_targetPoint;
    caf::PdmField<double>                        m_azimuth;
    caf::PdmField<double>                        m_inclination;
    caf::PdmField<double>                        m_dogleg1;
    caf::PdmField<double>                        m_dogleg2;

};

