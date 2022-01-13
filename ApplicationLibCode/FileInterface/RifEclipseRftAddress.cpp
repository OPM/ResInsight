/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include "RifEclipseRftAddress.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseRftAddress::RifEclipseRftAddress( QString wellName, QDateTime timeStep, RftWellLogChannelType wellLogChannelName )
    : m_wellName( wellName )
    , m_timeStep( timeStep )
    , m_wellLogChannel( wellLogChannelName )
    , m_segmentId( -1 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseRftAddress::setSegmentId( int id )
{
    m_segmentId = id;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifEclipseRftAddress::segmentId() const
{
    return m_segmentId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseRftAddress::setResultName( const QString& resultName )
{
    m_resultName = resultName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEclipseRftAddress::resultName() const
{
    return m_resultName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RifEclipseRftAddress::wellName() const
{
    return m_wellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDateTime RifEclipseRftAddress::timeStep() const
{
    return m_timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RifEclipseRftAddress::RifEclipseRftAddress::RftWellLogChannelType& RifEclipseRftAddress::wellLogChannel() const
{
    return m_wellLogChannel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseRftAddress::RftWellLogChannelType> RifEclipseRftAddress::rftPlotChannelTypes()
{
    return { RifEclipseRftAddress::RftWellLogChannelType::PRESSURE,
             RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_ERROR,
             RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_MEAN,
             RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_P10,
             RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_P50,
             RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_P90 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseRftAddress::RifEclipseRftAddress::RftWellLogChannelType> RifEclipseRftAddress::pltPlotChannelTypes()
{
    return { RifEclipseRftAddress::RftWellLogChannelType::ORAT,
             RifEclipseRftAddress::RftWellLogChannelType::WRAT,
             RifEclipseRftAddress::RftWellLogChannelType::GRAT };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool operator==( const RifEclipseRftAddress& first, const RifEclipseRftAddress& second )
{
    if ( first.wellName() != second.wellName() ) return false;
    if ( first.timeStep() != second.timeStep() ) return false;
    if ( first.wellLogChannel() != second.wellLogChannel() ) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool operator<( const RifEclipseRftAddress& first, const RifEclipseRftAddress& second )
{
    if ( first.wellName() != second.wellName() ) return ( first.wellName() < second.wellName() );
    if ( first.timeStep() != second.timeStep() ) return ( first.timeStep() < second.timeStep() );
    if ( first.wellLogChannel() != second.wellLogChannel() )
        return ( first.wellLogChannel() < second.wellLogChannel() );

    return false;
}
