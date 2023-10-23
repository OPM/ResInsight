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

#include "RimFaultReactivationTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimFaultReactivationTools::normalVector( QMap<QString, QVariant> settings )
{
    double x = settings.value( "faultNormal_X", 0.0 ).toDouble();
    double y = settings.value( "faultNormal_Y", 0.0 ).toDouble();
    double z = settings.value( "faultNormal_Z", 0.0 ).toDouble();

    return cvf::Vec3d( x, y, z );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimFaultReactivationTools::topFaultPosition( QMap<QString, QVariant> settings )
{
    double x = settings.value( "faultTop_X", 0.0 ).toDouble();
    double y = settings.value( "faultTop_Y", 0.0 ).toDouble();
    double z = settings.value( "faultTop_Z", 0.0 ).toDouble();

    return cvf::Vec3d( x, y, z );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimFaultReactivationTools::bottomFaultPosition( QMap<QString, QVariant> settings )
{
    double x = settings.value( "faultBottom_X", 0.0 ).toDouble();
    double y = settings.value( "faultBottom_Y", 0.0 ).toDouble();
    double z = settings.value( "faultBottom_Z", 0.0 ).toDouble();

    return cvf::Vec3d( x, y, z );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultReactivationTools::addSettingsToMap( QMap<QString, QVariant>& map,
                                                  cvf::Vec3d               normalVector,
                                                  cvf::Vec3d               topFaultPosition,
                                                  cvf::Vec3d               bottomFaultPosition )
{
    map["faultTop_X"] = topFaultPosition.x();
    map["faultTop_Y"] = topFaultPosition.y();
    map["faultTop_Z"] = topFaultPosition.z();

    map["faultBottom_X"] = bottomFaultPosition.x();
    map["faultBottom_Y"] = bottomFaultPosition.y();
    map["faultBottom_Z"] = bottomFaultPosition.z();

    map["faultNormal_X"] = normalVector.x();
    map["faultNormal_Y"] = normalVector.y();
    map["faultNormal_Z"] = normalVector.z();
}
