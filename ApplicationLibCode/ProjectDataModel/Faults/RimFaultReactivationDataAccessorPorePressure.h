/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 -    Equinor ASA
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

#include "RimFaultReactivationDataAccessor.h"

#include "cvfObject.h"
#include "cvfVector3.h"

class RimEclipseCase;
class RigEclipseCaseData;
class RigMainGrid;
class RigResultAccessor;
class RigEclipseWellLogExtractor;
class RigWellPath;

//==================================================================================================
///
///
//==================================================================================================
class RimFaultReactivationDataAccessorPorePressure : public RimFaultReactivationDataAccessor
{
public:
    RimFaultReactivationDataAccessorPorePressure( RimEclipseCase* eclipseCase, double porePressureGradient, double seabedDepth );
    ~RimFaultReactivationDataAccessorPorePressure();

    bool isMatching( RimFaultReactivation::Property property ) const override;

    double valueAtPosition( const cvf::Vec3d&                position,
                            const RigFaultReactivationModel& model,
                            RimFaultReactivation::GridPart   gridPart,
                            double                           topDepth     = std::numeric_limits<double>::infinity(),
                            double                           bottomDepth  = std::numeric_limits<double>::infinity(),
                            size_t                           elementIndex = std::numeric_limits<size_t>::max() ) const override;

private:
    void updateResultAccessor() override;

    static double calculatePorePressure( double depth, double gradient );

    RimEclipseCase*             m_eclipseCase;
    RigEclipseCaseData*         m_caseData;
    const RigMainGrid*          m_mainGrid;
    double                      m_defaultPorePressureGradient;
    double                      m_seabedDepth;
    cvf::ref<RigResultAccessor> m_resultAccessor;

    std::map<RimFaultReactivation::GridPart, cvf::ref<RigWellPath>>                m_wellPaths;
    std::map<RimFaultReactivation::GridPart, cvf::ref<RigEclipseWellLogExtractor>> m_extractors;
};
