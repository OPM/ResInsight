/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RicTransmissibilityCalculator.h"

#include <optional>

#include "RiaEclipseUnitTools.h"
#include "RiaFractureDefines.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"
#include "RigPerforationTransmissibilityEquations.h"
#include "RigResultAccessorFactory.h"
#include "RigTransmissibilityEquations.h"

#include "RimEclipseCase.h"
#include "RimNonDarcyPerforationParameters.h"
#include "RimWellPath.h"

namespace internal
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::PorosityModelType porosityModel( RimEclipseCase* eclipseCase )
{
    if ( eclipseCase && eclipseCase->eclipseCaseData() && eclipseCase->eclipseCaseData()->mainGrid() &&
         eclipseCase->eclipseCaseData()->mainGrid()->isDualPorosity() )
    {
        return RiaDefines::PorosityModelType::FRACTURE_MODEL;
    }
    return RiaDefines::PorosityModelType::MATRIX_MODEL;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigResultAccessor> loadResultAccessor( RimEclipseCase* eclipseCase, const QString& resultName )
{
    if ( !eclipseCase || !eclipseCase->eclipseCaseData() ) return nullptr;

    auto porModel = porosityModel( eclipseCase );

    auto* results = eclipseCase->results( porModel );
    if ( !results ) return nullptr;

    results->ensureKnownResultLoaded( RigEclipseResultAddress( resultName ) );
    return RigResultAccessorFactory::createFromResultAddress( eclipseCase->eclipseCaseData(), 0, porModel, 0, RigEclipseResultAddress( resultName ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
struct CellProperties
{
    double dx, dy, dz;
    double permx, permy, permz;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<CellProperties> loadCellProperties( RimEclipseCase* eclipseCase, size_t globalCellIndex )
{
    auto dxAcc    = loadResultAccessor( eclipseCase, "DX" );
    auto dyAcc    = loadResultAccessor( eclipseCase, "DY" );
    auto dzAcc    = loadResultAccessor( eclipseCase, "DZ" );
    auto permxAcc = loadResultAccessor( eclipseCase, "PERMX" );
    auto permyAcc = loadResultAccessor( eclipseCase, "PERMY" );
    auto permzAcc = loadResultAccessor( eclipseCase, "PERMZ" );

    if ( dxAcc.isNull() || dyAcc.isNull() || dzAcc.isNull() || permxAcc.isNull() || permyAcc.isNull() || permzAcc.isNull() )
    {
        return std::nullopt;
    }

    CellProperties props;
    props.dx    = dxAcc->cellScalarGlobIdx( globalCellIndex );
    props.dy    = dyAcc->cellScalarGlobIdx( globalCellIndex );
    props.dz    = dzAcc->cellScalarGlobIdx( globalCellIndex );
    props.permx = permxAcc->cellScalarGlobIdx( globalCellIndex );
    props.permy = permyAcc->cellScalarGlobIdx( globalCellIndex );
    props.permz = permzAcc->cellScalarGlobIdx( globalCellIndex );
    return props;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double loadNtg( RimEclipseCase* eclipseCase, size_t globalCellIndex )
{
    auto ntgAcc = loadResultAccessor( eclipseCase, "NTG" );
    if ( ntgAcc.notNull() ) return ntgAcc->cellScalarGlobIdx( globalCellIndex );
    return 1.0;
}

} // namespace internal

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCompletionData::CellDirection RicTransmissibilityCalculator::calculateCellMainDirection( RimEclipseCase*   eclipseCase,
                                                                                            size_t            globalCellIndex,
                                                                                            const cvf::Vec3d& lengthsInCell )
{
    if ( !eclipseCase || !eclipseCase->eclipseCaseData() ) return RigCompletionData::CellDirection::DIR_K;

    auto props = internal::loadCellProperties( eclipseCase, globalCellIndex );
    if ( !props ) return RigCompletionData::CellDirection::DIR_K;

    double xLengthFraction = fabs( lengthsInCell.x() / props->dx );
    double yLengthFraction = fabs( lengthsInCell.y() / props->dy );
    double zLengthFraction = fabs( lengthsInCell.z() / props->dz );

    if ( xLengthFraction > yLengthFraction && xLengthFraction > zLengthFraction )
    {
        return RigCompletionData::CellDirection::DIR_I;
    }
    else if ( yLengthFraction > xLengthFraction && yLengthFraction > zLengthFraction )
    {
        return RigCompletionData::CellDirection::DIR_J;
    }
    else
    {
        return RigCompletionData::CellDirection::DIR_K;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TransmissibilityData RicTransmissibilityCalculator::calculateTransmissibilityData( RimEclipseCase*    eclipseCase,
                                                                                   const RimWellPath* wellPath,
                                                                                   const cvf::Vec3d&  internalCellLengths,
                                                                                   double             skinFactor,
                                                                                   double             wellRadius,
                                                                                   size_t             globalCellIndex,
                                                                                   bool               useLateralNTG,
                                                                                   size_t             volumeScaleConstant,
                                                                                   RigCompletionData::CellDirection directionForVolumeScaling )
{
    if ( !eclipseCase || !eclipseCase->eclipseCaseData() || !wellPath ) return TransmissibilityData();

    auto props = internal::loadCellProperties( eclipseCase, globalCellIndex );
    if ( !props )
    {
        return TransmissibilityData();
    }

    double ntg    = internal::loadNtg( eclipseCase, globalCellIndex );
    double latNtg = useLateralNTG ? ntg : 1.0;

    const double totalKh = RigTransmissibilityEquations::totalKh( props->permx, props->permy, props->permz, internalCellLengths, latNtg, ntg );
    const double effectiveK =
        RigTransmissibilityEquations::effectiveK( props->permx, props->permy, props->permz, internalCellLengths, latNtg, ntg );
    const double effectiveH = RigTransmissibilityEquations::effectiveH( internalCellLengths, latNtg, ntg );

    double darcy = RiaEclipseUnitTools::darcysConstant( wellPath->unitSystem() );

    double dx = props->dx;
    double dy = props->dy;
    double dz = props->dz;
    if ( volumeScaleConstant != 1 )
    {
        if ( directionForVolumeScaling == RigCompletionData::CellDirection::DIR_I ) dx = dx / volumeScaleConstant;
        if ( directionForVolumeScaling == RigCompletionData::CellDirection::DIR_J ) dy = dy / volumeScaleConstant;
        if ( directionForVolumeScaling == RigCompletionData::CellDirection::DIR_K ) dz = dz / volumeScaleConstant;
    }

    const double transx = RigTransmissibilityEquations::wellBoreTransmissibilityComponent( internalCellLengths.x() * latNtg,
                                                                                           props->permy,
                                                                                           props->permz,
                                                                                           dy,
                                                                                           dz,
                                                                                           wellRadius,
                                                                                           skinFactor,
                                                                                           darcy );
    const double transy = RigTransmissibilityEquations::wellBoreTransmissibilityComponent( internalCellLengths.y() * latNtg,
                                                                                           props->permx,
                                                                                           props->permz,
                                                                                           dx,
                                                                                           dz,
                                                                                           wellRadius,
                                                                                           skinFactor,
                                                                                           darcy );
    const double transz = RigTransmissibilityEquations::wellBoreTransmissibilityComponent( internalCellLengths.z() * ntg,
                                                                                           props->permy,
                                                                                           props->permx,
                                                                                           dy,
                                                                                           dx,
                                                                                           wellRadius,
                                                                                           skinFactor,
                                                                                           darcy );

    const double totalConnectionFactor = RigTransmissibilityEquations::totalConnectionFactor( transx, transy, transz );

    TransmissibilityData trData;
    trData.setData( effectiveH, effectiveK, totalConnectionFactor, totalKh );
    return trData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicTransmissibilityCalculator::calculateDFactor( RimEclipseCase*                         eclipseCase,
                                                        double                                  effectiveH,
                                                        size_t                                  globalCellIndex,
                                                        const RimNonDarcyPerforationParameters* nonDarcyParameters,
                                                        const double                            effectivePermeability )
{
    using EQ = RigPerforationTransmissibilityEquations;

    if ( !eclipseCase || !eclipseCase->eclipseCaseData() )
    {
        return std::numeric_limits<double>::infinity();
    }

    double porosity = 0.0;
    {
        auto poroAcc = internal::loadResultAccessor( eclipseCase, "PORO" );
        if ( poroAcc.notNull() )
        {
            porosity = poroAcc->cellScalar( globalCellIndex );
        }
    }

    const double betaFactor = EQ::betaFactor( nonDarcyParameters->inertialCoefficientBeta0(),
                                              effectivePermeability,
                                              nonDarcyParameters->permeabilityScalingFactor(),
                                              porosity,
                                              nonDarcyParameters->porosityScalingFactor() );

    const double alpha = RiaDefines::nonDarcyFlowAlpha( eclipseCase->eclipseCaseData()->unitsType() );

    return EQ::dFactor( alpha,
                        betaFactor,
                        effectivePermeability,
                        effectiveH,
                        nonDarcyParameters->wellRadius(),
                        nonDarcyParameters->relativeGasDensity(),
                        nonDarcyParameters->gasViscosity() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicTransmissibilityCalculator::calculateTransmissibilityAsEclipseDoes( RimEclipseCase*                  eclipseCase,
                                                                              double                           skinFactor,
                                                                              double                           wellRadius,
                                                                              size_t                           globalCellIndex,
                                                                              RigCompletionData::CellDirection direction )
{
    if ( !eclipseCase || !eclipseCase->eclipseCaseData() ) return cvf::UNDEFINED_DOUBLE;

    auto props = internal::loadCellProperties( eclipseCase, globalCellIndex );
    if ( !props ) return cvf::UNDEFINED_DOUBLE;

    double ntg   = internal::loadNtg( eclipseCase, globalCellIndex );
    double darcy = RiaEclipseUnitTools::darcysConstant( eclipseCase->eclipseCaseData()->unitsType() );

    double trans = cvf::UNDEFINED_DOUBLE;
    if ( direction == RigCompletionData::CellDirection::DIR_I )
    {
        trans = RigTransmissibilityEquations::wellBoreTransmissibilityComponent( props->dx,
                                                                                 props->permy,
                                                                                 props->permz,
                                                                                 props->dy,
                                                                                 props->dz,
                                                                                 wellRadius,
                                                                                 skinFactor,
                                                                                 darcy );
    }
    else if ( direction == RigCompletionData::CellDirection::DIR_J )
    {
        trans = RigTransmissibilityEquations::wellBoreTransmissibilityComponent( props->dy,
                                                                                 props->permx,
                                                                                 props->permz,
                                                                                 props->dx,
                                                                                 props->dz,
                                                                                 wellRadius,
                                                                                 skinFactor,
                                                                                 darcy );
    }
    else if ( direction == RigCompletionData::CellDirection::DIR_K )
    {
        trans = RigTransmissibilityEquations::wellBoreTransmissibilityComponent( props->dz * ntg,
                                                                                 props->permy,
                                                                                 props->permx,
                                                                                 props->dy,
                                                                                 props->dx,
                                                                                 wellRadius,
                                                                                 skinFactor,
                                                                                 darcy );
    }

    return trans;
}
