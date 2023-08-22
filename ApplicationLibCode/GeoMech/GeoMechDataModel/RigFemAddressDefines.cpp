/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RigFemAddressDefines.h"
#include "RigFemResultAddress.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::string RigFemAddressDefines::porBar()
{
    return "POR-Bar";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemResultAddress RigFemAddressDefines::getResultLookupAddress( const RigFemResultAddress& sourceAddress )
{
    if ( sourceAddress.resultPosType == RIG_NODAL && sourceAddress.fieldName == RigFemAddressDefines::porBar() )
    {
        RigFemResultAddress lookupAddressForPOR = sourceAddress;

        lookupAddressForPOR.resultPosType = RIG_ELEMENT_NODAL;

        return lookupAddressForPOR;
    }

    return sourceAddress;
}
