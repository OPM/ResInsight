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

#include "RimFaultReactivationDataAccessorStress.h"
#include "RimFaultReactivationEnums.h"

#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include "cvfObject.h"

#include <vector>

class RigGriddedPart3d;
class RimModeledWellPath;
class RigWellPath;
class RimEclipseCase;
class RigEclipseCaseData;
class RigResultAccessor;
class RigEclipseWellLogExtractor;

//==================================================================================================
///
///
//==================================================================================================
class RimFaultReactivationDataAccessorStressEclipse : public RimFaultReactivationDataAccessorStress
{
public:
    RimFaultReactivationDataAccessorStressEclipse( RimEclipseCase*                                            geoMechCase,
                                                   RimFaultReactivation::Property                             property,
                                                   double                                                     gradient,
                                                   double                                                     seabedDepth,
                                                   double                                                     waterDensity,
                                                   double                                                     lateralStressComponent,
                                                   const std::map<RimFaultReactivation::ElementSets, double>& densities );
    ~RimFaultReactivationDataAccessorStressEclipse() override;

private:
    void updateResultAccessor() override;

    bool isDataAvailable() const override;

    double extractStressValue( StressType stressType, const cvf::Vec3d& position, RimFaultReactivation::GridPart gridPart ) const override;

    std::pair<double, cvf::Vec3d>
        calculatePorBar( const cvf::Vec3d& position, double gradient, RimFaultReactivation::GridPart gridPart ) const override;

    bool isPositionValid( const cvf::Vec3d&              position,
                          const cvf::Vec3d&              topPosition,
                          const cvf::Vec3d&              bottomPosition,
                          RimFaultReactivation::GridPart gridPart ) const override;

    double lateralStressComponentX( const cvf::Vec3d& position, RimFaultReactivation::GridPart gridPart ) const override;
    double lateralStressComponentY( const cvf::Vec3d& position, RimFaultReactivation::GridPart gridPart ) const override;

    static std::vector<double> integrateVerticalStress( const RigWellPath&                                         wellPath,
                                                        const std::vector<cvf::Vec3d>&                             intersections,
                                                        const RigFaultReactivationModel&                           model,
                                                        RimFaultReactivation::GridPart                             gridPart,
                                                        double                                                     seabedDepth,
                                                        double                                                     waterDensity,
                                                        const std::map<RimFaultReactivation::ElementSets, double>& densities );

    static void addOverburdenAndUnderburdenPoints( std::vector<cvf::Vec3d>& intersections, const std::vector<cvf::Vec3d>& wellPathPoints );

    static std::pair<bool, RimFaultReactivation::ElementSets>
        findElementSetForPoint( const RigGriddedPart3d&                                                       part,
                                const cvf::Vec3d&                                                             point,
                                const std::map<RimFaultReactivation::ElementSets, std::vector<unsigned int>>& elementSets );

    RimEclipseCase*             m_eclipseCase;
    RigEclipseCaseData*         m_caseData;
    const RigMainGrid*          m_mainGrid;
    cvf::ref<RigResultAccessor> m_resultAccessor;
    double                      m_waterDensity;
    double                      m_lateralStressComponent;

    std::map<RimFaultReactivation::GridPart, cvf::ref<RigWellPath>>                m_wellPaths;
    std::map<RimFaultReactivation::GridPart, cvf::ref<RigEclipseWellLogExtractor>> m_extractors;
    std::map<RimFaultReactivation::GridPart, std::vector<double>>                  m_stressValues;
    std::map<RimFaultReactivation::ElementSets, double>                            m_densities;
};
