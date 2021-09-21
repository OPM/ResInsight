/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RigSurface.h"

#include "RigSurfaceStatisticsCalculator.h"

#include "RimSurface.h"
#include "cafAppEnum.h"

class RimEnsembleStatisticsSurface : public RimSurface
{
    CAF_PDM_HEADER_INIT;

public:
    RimEnsembleStatisticsSurface();
    ~RimEnsembleStatisticsSurface() override;

    bool        onLoadData() override;
    RimSurface* createCopy() override;

    void setStatisticsType( RigSurfaceStatisticsCalculator::StatisticsType statisticsType );
    RigSurfaceStatisticsCalculator::StatisticsType statisticsType() const;

    QString fullName() const override;

protected:
    bool updateSurfaceData() override;

    std::vector<cvf::Vec3d> extractStatisticalDepthForVertices( const RigSurface* surface ) const;

    void clearCachedNativeData() override;

private:
    caf::PdmField<caf::AppEnum<RigSurfaceStatisticsCalculator::StatisticsType>> m_statisticsType;
};
