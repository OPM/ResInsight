/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Statoil ASA
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

#include "RigEclipseToThermalCellTransmissibilityCalculator.h"

#include "RimEclipseCase.h"
#include "RimFracture.h"
#include "RimThermalFractureTemplate.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseToThermalCellTransmissibilityCalculator::RigEclipseToThermalCellTransmissibilityCalculator(
    const RimEclipseCase*                              caseToApply,
    cvf::Mat4d                                         fractureTransform,
    double                                             skinFactor,
    double                                             cDarcy,
    const RigFractureCell&                             stimPlanCell,
    const RimFracture*                                 fracture,
    RimThermalFractureTemplate::FilterCakePressureDrop filterCakePressureDrop,
    double                                             injectivityFactor,
    double                                             filterCakeMobility,
    double                                             viscosity,
    double                                             relativePermeability )
    : RigEclipseToStimPlanCellTransmissibilityCalculator( caseToApply, fractureTransform, skinFactor, cDarcy, stimPlanCell, fracture )
    , m_filterCakePressureDrop( filterCakePressureDrop )
    , m_injectivityFactor( injectivityFactor )
    , m_filterCakeMobility( filterCakeMobility )
    , m_viscosity( viscosity )
    , m_relativePermeability( relativePermeability )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigEclipseToThermalCellTransmissibilityCalculator::calculateTransmissibility( const cvf::Vec3d& transmissibilityVector,
                                                                                     double            fractureArea )
{
    double fractureMatrixTransimissibility = transmissibilityVector.length();

    if ( m_filterCakePressureDrop == RimThermalFractureTemplate::FilterCakePressureDrop::RELATIVE )
    {
        return m_injectivityFactor * fractureMatrixTransimissibility;
    }
    else if ( m_filterCakePressureDrop == RimThermalFractureTemplate::FilterCakePressureDrop::ABSOLUTE )
    {
        double filterCakeTransmissibility = ( m_viscosity / m_relativePermeability ) * fractureArea * m_filterCakeMobility;

        // Harmonic mean
        return ( fractureMatrixTransimissibility * filterCakeTransmissibility ) /
               ( fractureMatrixTransimissibility + filterCakeTransmissibility );
    }
    else
    {
        CAF_ASSERT( m_filterCakePressureDrop == RimThermalFractureTemplate::FilterCakePressureDrop::NONE );
        return fractureMatrixTransimissibility;
    }
}
