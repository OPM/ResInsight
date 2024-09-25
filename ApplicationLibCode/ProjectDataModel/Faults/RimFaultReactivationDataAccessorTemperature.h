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

#include "cvfObject.h"
#include "cvfVector3.h"

#include "RimFaultReactivationDataAccessor.h"
#include "RimFaultReactivationEnums.h"

class RimEclipseCase;
class RigEclipseCaseData;
class RigMainGrid;
class RigResultAccessor;
class RigWellPath;
class RigEclipseWellLogExtractor;

//==================================================================================================
///
///
//==================================================================================================
class RimFaultReactivationDataAccessorTemperature : public RimFaultReactivationDataAccessor
{
public:
    RimFaultReactivationDataAccessorTemperature( RimEclipseCase* eclipseCase, double seabedTemperature, double seabedDepth, size_t firstTimeStep );
    ~RimFaultReactivationDataAccessorTemperature();

    bool isMatching( RimFaultReactivation::Property property ) const override;

    double valueAtPosition( const cvf::Vec3d&                position,
                            const RigFaultReactivationModel& model,
                            RimFaultReactivation::GridPart   gridPart,
                            double                           topDepth     = std::numeric_limits<double>::infinity(),
                            double                           bottomDepth  = std::numeric_limits<double>::infinity(),
                            size_t                           elementIndex = std::numeric_limits<size_t>::max() ) const override;

private:
    void   updateResultAccessor() override;
    double computeGradient() const;

    RimEclipseCase*     m_eclipseCase;
    RigEclipseCaseData* m_caseData;
    const RigMainGrid*  m_mainGrid;
    double              m_seabedTemperature;
    double              m_seabedDepth;
    double              m_gradient;
    size_t              m_firstTimeStep;

    cvf::ref<RigResultAccessor> m_resultAccessor0;
    cvf::ref<RigResultAccessor> m_resultAccessor;

    std::map<RimFaultReactivation::GridPart, cvf::ref<RigWellPath>>                m_wellPaths;
    std::map<RimFaultReactivation::GridPart, cvf::ref<RigEclipseWellLogExtractor>> m_extractors;
};
