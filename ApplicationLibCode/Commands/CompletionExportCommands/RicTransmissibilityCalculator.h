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

#pragma once

#include "RigCompletionData.h"

#include "cvfVector3.h"

class RimEclipseCase;
class RimNonDarcyPerforationParameters;
class RimWellPath;

//==================================================================================================
///
//==================================================================================================
class TransmissibilityData
{
public:
    TransmissibilityData()
        : m_isValid( false )
        , m_effectiveH( 0.0 )
        , m_effectiveK( 0.0 )
        , m_connectionFactor( 0.0 )
        , m_kh( 0.0 )
    {
    }

    bool isValid() const { return m_isValid; }

    void setData( double effectiveH, double effectiveK, double connectionFactor, double kh )
    {
        m_isValid = true;

        m_effectiveH       = effectiveH;
        m_effectiveK       = effectiveK;
        m_connectionFactor = connectionFactor;
        m_kh               = kh;
    }

    double effectiveH() const { return m_effectiveH; }

    double effectiveK() const { return m_effectiveK; }
    double connectionFactor() const { return m_connectionFactor; }
    double kh() const { return m_kh; }

private:
    bool   m_isValid;
    double m_effectiveH;
    double m_effectiveK;
    double m_connectionFactor;
    double m_kh;
};

//==================================================================================================
///
//==================================================================================================
class RicTransmissibilityCalculator
{
public:
    static RigCompletionData::CellDirection
        calculateCellMainDirection( RimEclipseCase* eclipseCase, size_t globalCellIndex, const cvf::Vec3d& lengthsInCell );

    static TransmissibilityData
        calculateTransmissibilityData( RimEclipseCase*    eclipseCase,
                                       const RimWellPath* wellPath,
                                       const cvf::Vec3d&  internalCellLengths,
                                       double             skinFactor,
                                       double             wellRadius,
                                       size_t             globalCellIndex,
                                       bool               useLateralNTG,
                                       size_t             volumeScaleConstant = 1,
                                       RigCompletionData::CellDirection directionForVolumeScaling = RigCompletionData::CellDirection::DIR_I );

    static double calculateDFactor( RimEclipseCase*                         eclipseCase,
                                    double                                  effectiveH,
                                    size_t                                  globalCellIndex,
                                    const RimNonDarcyPerforationParameters* nonDarcyParameters,
                                    const double                            effectivePermeability );

    static double calculateTransmissibilityAsEclipseDoes( RimEclipseCase*                  eclipseCase,
                                                          double                           skinFactor,
                                                          double                           wellRadius,
                                                          size_t                           globalCellIndex,
                                                          RigCompletionData::CellDirection direction );
};
