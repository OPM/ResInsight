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

#include "RiaTimeTTools.h"

#include <QDateTime>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RiaTimeTTools::toDouble( time_t time )
{
    double milliSecSinceEpoch = time * 1000; // This is kind of hack, as the c++ standard does not state what
                                             // time_t is. "Almost always" secs since epoch according to
                                             // cppreference.com
    return milliSecSinceEpoch;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
time_t RiaTimeTTools::fromDouble( double time )
{
    time_t timet = static_cast<time_t>( time / 1000 );
    return timet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
time_t RiaTimeTTools::fromQDateTime( const QDateTime& dateTime )
{
    time_t timet = dateTime.toSecsSinceEpoch();
    return timet;
}
