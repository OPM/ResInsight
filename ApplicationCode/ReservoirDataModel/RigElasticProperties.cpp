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
const std::vector<double>& RigElasticProperties::youngsModulus() const
{
    return m_youngsModulus;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigElasticProperties::poissonsRatio() const
{
    return m_poissonsRatio;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigElasticProperties::K_Ic() const
{
    return m_K_Ic;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigElasticProperties::proppantEmbedment() const
{
    return m_proppantEmbedment;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigElasticProperties::biotCoefficient() const
{
    return m_biotCoefficient;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigElasticProperties::k0() const
{
    return m_k0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigElasticProperties::fluidLossCoefficient() const
{
    return m_fluidLossCoefficient;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigElasticProperties::spurtLoss() const
{
    return m_spurtLoss;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigElasticProperties::immobileFluidSaturation() const
{
    return m_immobileFluidSaturation;
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
double RigElasticProperties::getYoungsModulus( double porosity ) const
{
    return RiaInterpolationTools::linear( m_porosity, m_youngsModulus, porosity );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigElasticProperties::getPoissonsRatio( double porosity ) const
{
    return RiaInterpolationTools::linear( m_porosity, m_poissonsRatio, porosity );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigElasticProperties::getK_Ic( double porosity ) const
{
    return RiaInterpolationTools::linear( m_porosity, m_K_Ic, porosity );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigElasticProperties::getProppantEmbedment( double porosity ) const
{
    return RiaInterpolationTools::linear( m_porosity, m_proppantEmbedment, porosity );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigElasticProperties::getBiotCoefficient( double porosity ) const
{
    return RiaInterpolationTools::linear( m_porosity, m_biotCoefficient, porosity );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigElasticProperties::getK0( double porosity ) const
{
    return RiaInterpolationTools::linear( m_porosity, m_k0, porosity );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigElasticProperties::getFluidLossCoefficient( double porosity ) const
{
    return RiaInterpolationTools::linear( m_porosity, m_fluidLossCoefficient, porosity );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigElasticProperties::getSpurtLoss( double porosity ) const
{
    return RiaInterpolationTools::linear( m_porosity, m_spurtLoss, porosity );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigElasticProperties::getImmobileFluidSaturation( double porosity ) const
{
    return RiaInterpolationTools::linear( m_porosity, m_immobileFluidSaturation, porosity );
}
