/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiaRftPltCurveDefinition.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaRftPltCurveDefinition::RiaRftPltCurveDefinition( const RifDataSourceForRftPlt& address, const QString& wellName, const QDateTime& timeStep )
    : m_curveAddress( address )
    , m_wellName( wellName )
    , m_timeStep( timeStep )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RifDataSourceForRftPlt& RiaRftPltCurveDefinition::address() const
{
    return m_curveAddress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RiaRftPltCurveDefinition::wellName() const
{
    return m_wellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QDateTime& RiaRftPltCurveDefinition::timeStep() const
{
    return m_timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
auto RiaRftPltCurveDefinition::operator<=>( const RiaRftPltCurveDefinition& other ) const -> std::strong_ordering
{
    RimSummaryCaseCollection* thisEnsemble  = m_curveAddress.ensemble();
    RimSummaryCaseCollection* otherEnsemble = other.m_curveAddress.ensemble();

    if ( ( thisEnsemble && !otherEnsemble ) || ( !thisEnsemble && otherEnsemble ) )
    {
        // If one is an ensemble and the other is not, the ensemble should be first to make sure the ensemble curves are created and plotted
        // before the single summary curves

        return m_curveAddress.ensemble() <=> other.m_curveAddress.ensemble();
    }

    if ( ( m_curveAddress <=> other.m_curveAddress ) == std::strong_ordering::equal )
    {
        if ( m_wellName == other.m_wellName )
        {
            return m_timeStep.toMSecsSinceEpoch() <=> other.m_timeStep.toMSecsSinceEpoch();
        }
        return m_wellName.toStdString() <=> other.m_wellName.toStdString();
    }
    return m_curveAddress <=> other.m_curveAddress;
}
