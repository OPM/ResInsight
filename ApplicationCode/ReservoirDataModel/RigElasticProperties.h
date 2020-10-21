/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RiaFractureModelDefines.h"

#include <QString>

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigElasticProperties
{
public:
    RigElasticProperties( const QString& fieldName, const QString& formationName, const QString& faciesName );
    const QString& fieldName() const;
    const QString& formationName() const;
    const QString& faciesName() const;

    void   appendValues( double porosity,
                         double youngsModulus,
                         double poissonsRatio,
                         double m_K_Ic,
                         double proppantEmbedment,
                         double biotCoefficient,
                         double k0,
                         double fluidLossCoefficient,
                         double spurtLoss,
                         double immobileFluidSaturation );

    size_t numValues() const;
    double getValue( RiaDefines::CurveProperty property, size_t index, double scale = 1.0 ) const;
    double getValueForPorosity( RiaDefines::CurveProperty property, double porosity, double scale = 1.0 ) const;

    const std::vector<double>& porosity() const;

private:
    const std::vector<double>& getVector( RiaDefines::CurveProperty property ) const;

    QString m_fieldName;
    QString m_formationName;
    QString m_faciesName;

    std::vector<double> m_porosity;
    std::vector<double> m_youngsModulus;
    std::vector<double> m_poissonsRatio;
    std::vector<double> m_K_Ic;
    std::vector<double> m_proppantEmbedment;
    std::vector<double> m_biotCoefficient;
    std::vector<double> m_k0;
    std::vector<double> m_fluidLossCoefficient;
    std::vector<double> m_spurtLoss;
    std::vector<double> m_immobileFluidSaturation;
};
