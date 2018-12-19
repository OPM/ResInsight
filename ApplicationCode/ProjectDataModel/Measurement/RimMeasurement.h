/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "cvfBase.h"
#include "cvfVector3.h"
#include "cafPdmObject.h"

//==================================================================================================
///
///
//==================================================================================================
class RimMeasurement : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

    using Vec3d = cvf::Vec3d;

public:
    RimMeasurement();
    ~RimMeasurement() override;

    void                setMeasurementMode(bool measurementMode);
    bool                isInMeasurementMode() const;

    void                addPointInDomain(const Vec3d& pointInDomain);
    std::vector<Vec3d>  pointsInDomain() const;

private:
    void                updateView() const;

    bool                m_isInMeasurementMode;

    std::vector<Vec3d>  m_pointsInDomain;
};

