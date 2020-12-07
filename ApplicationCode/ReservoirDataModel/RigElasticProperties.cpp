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

#include "RigElasticProperties.h"

#include "RiaInterpolationTools.h"

#include "cafAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigElasticProperties::RigElasticProperties( const QString& fieldName, const QString& formationName, const QString& faciesName )
    : m_fieldName( fieldName )
    , m_formationName( formationName )
    , m_faciesName( faciesName )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RigElasticProperties::fieldName() const
{
    return m_fieldName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RigElasticProperties::formationName() const
{
    return m_formationName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RigElasticProperties::faciesName() const
{
    return m_faciesName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigElasticProperties::porosity() const
{
    return m_porosity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigElasticProperties::porosityMin() const
{
    if ( m_porosity.empty() ) return 0.0;
    return m_porosity[0];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigElasticProperties::porosityMax() const
{
    if ( m_porosity.empty() ) return 0.0;
    return m_porosity[m_porosity.size() - 1];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigElasticProperties::appendValues( double porosity,
                                         double youngsModulus,
                                         double poissonsRatio,
                                         double K_Ic,
                                         double proppantEmbedment,
                                         double biotCoefficient,
                                         double k0,
                                         double fluidLossCoefficient,
                                         double spurtLoss,
                                         double immobileFluidSaturation )
{
    m_porosity.push_back( porosity );
    m_youngsModulus.push_back( youngsModulus );
    m_poissonsRatio.push_back( poissonsRatio );
    m_K_Ic.push_back( K_Ic );
    m_proppantEmbedment.push_back( proppantEmbedment );
    m_biotCoefficient.push_back( biotCoefficient );
    m_k0.push_back( k0 );
    m_fluidLossCoefficient.push_back( fluidLossCoefficient );
    m_spurtLoss.push_back( spurtLoss );
    m_immobileFluidSaturation.push_back( immobileFluidSaturation );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigElasticProperties::numValues() const
{
    return m_porosity.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigElasticProperties::getValue( RiaDefines::CurveProperty property, size_t index, double scale ) const
{
    CAF_ASSERT( index < numValues() );
    const std::vector<double>& unscaledValues = getVector( property );
    return unscaledValues[index] * scale;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigElasticProperties::getVector( RiaDefines::CurveProperty property ) const
{
    if ( property == RiaDefines::CurveProperty::YOUNGS_MODULUS ) return m_youngsModulus;
    if ( property == RiaDefines::CurveProperty::POISSONS_RATIO ) return m_poissonsRatio;
    if ( property == RiaDefines::CurveProperty::K_IC ) return m_K_Ic;
    if ( property == RiaDefines::CurveProperty::PROPPANT_EMBEDMENT ) return m_proppantEmbedment;
    if ( property == RiaDefines::CurveProperty::BIOT_COEFFICIENT ) return m_biotCoefficient;
    if ( property == RiaDefines::CurveProperty::K0 ) return m_k0;
    if ( property == RiaDefines::CurveProperty::FLUID_LOSS_COEFFICIENT ) return m_fluidLossCoefficient;
    if ( property == RiaDefines::CurveProperty::SPURT_LOSS ) return m_spurtLoss;

    // Default: if we get this far only one option left
    CAF_ASSERT( property == RiaDefines::CurveProperty::IMMOBILE_FLUID_SATURATION );
    return m_immobileFluidSaturation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigElasticProperties::getValueForPorosity( RiaDefines::CurveProperty property, double porosity, double scale ) const
{
    const std::vector<double>& unscaledValues = getVector( property );
    std::vector<double>        scaledValues;
    for ( double unscaled : unscaledValues )
    {
        scaledValues.push_back( unscaled * scale );
    }

    return RiaInterpolationTools::linear( m_porosity, scaledValues, porosity );
}
