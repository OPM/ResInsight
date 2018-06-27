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

class RimWellPathTarget : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimWellPathTarget();
    ~RimWellPathTarget();

    void setAsPointTarget(const cvf::Vec3d& point);
    void setAsPointAndTangentTarget(const cvf::Vec3d& point, double azimuth, double inclination);

    enum TargetTypeEnum { POINT_AND_TANGENT, POINT };
    TargetTypeEnum targetType();
    cvf::Vec3d     targetPoint();
    double         azimuth();
    double         inclination();

private:
    caf::PdmField<bool>                          m_isEnabled;
    caf::PdmField<caf::AppEnum<TargetTypeEnum> > m_targetType;
    caf::PdmField<cvf::Vec3d>                    m_targetPoint;
    caf::PdmField<double>                        m_azimuth;
    caf::PdmField<double>                        m_inclination;
};

