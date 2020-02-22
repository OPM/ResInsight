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
    , m_wellLogChannel( wellLogChannelName )
{
    m_timeStep = timeStep;
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
