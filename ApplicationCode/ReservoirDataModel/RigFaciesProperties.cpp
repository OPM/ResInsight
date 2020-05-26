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

#include "RigFaciesProperties.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFaciesProperties::RigFaciesProperties( const QString& fieldName, const QString& formationName, const QString& faciesName )
    : m_fieldName( fieldName )
    , m_formationName( formationName )
    , m_faciesName( faciesName )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RigFaciesProperties::fieldName() const
{
    return m_fieldName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RigFaciesProperties::formationName() const
{
    return m_formationName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RigFaciesProperties::faciesName() const
{
    return m_faciesName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigFaciesProperties::porosity() const
{
    return m_porosity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigFaciesProperties::youngsModulus() const
{
    return m_youngsModulus;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigFaciesProperties::poissonsRatio() const
{
    return m_poissonsRatio;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigFaciesProperties::K_Ic() const
{
    return m_K_Ic;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigFaciesProperties::proppantEmbedment() const
{
    return m_proppantEmbedment;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaciesProperties::appendValues( double porosity,
                                        double youngsModulus,
                                        double poissonsRatio,
                                        double K_Ic,
                                        double proppantEmbedment )
{
    m_porosity.push_back( porosity );
    m_youngsModulus.push_back( youngsModulus );
    m_poissonsRatio.push_back( poissonsRatio );
    m_K_Ic.push_back( K_Ic );
    m_proppantEmbedment.push_back( proppantEmbedment );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigFaciesProperties::getYoungsModulus( double porosity ) const
{
    return -1.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigFaciesProperties::getPoissonsRatio( double porosity ) const
{
    return -1.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigFaciesProperties::getK_Ic( double porosity ) const
{
    return -1.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigFaciesProperties::getProppantEmbedment( double porosity ) const
{
    return -1.0;
}
