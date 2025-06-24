/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RimSeismicAlphaMapper.h"

#include <algorithm>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicAlphaMapper::RimSeismicAlphaMapper()
    : m_maxValue( 0.0 )
    , m_minValue( 0.0 )
    , m_dataRange( 0.0 )
    , m_scaleFactor( 0.0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSeismicAlphaMapper::~RimSeismicAlphaMapper()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSeismicAlphaMapper::setDataRangeAndAlphas( double minVal, double maxVal, std::vector<double> alphas )
{
    m_minValue    = minVal;
    m_maxValue    = maxVal;
    m_dataRange   = maxVal - minVal;
    m_alphavalues = alphas;

    if ( m_dataRange != 0.0 )
        m_scaleFactor = 1.0 * alphas.size() / m_dataRange;
    else
        m_scaleFactor = 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ubyte RimSeismicAlphaMapper::alphaValue( double dataValue ) const
{
    if ( m_alphavalues.empty() ) return 255;

    int index = (int)( m_scaleFactor * ( dataValue - m_minValue ) );
    index     = std::clamp( index, 0, (int)( m_alphavalues.size() - 1 ) );

    return ( cvf::ubyte )( m_alphavalues[index] * 255 );
}
