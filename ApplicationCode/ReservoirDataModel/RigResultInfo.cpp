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

#include "RigResultInfo.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigResultInfo::RigResultInfo(RiaDefines::ResultCatType resultType, bool needsToBeStored, bool mustBeCalculated,
                       QString resultName, size_t gridScalarResultIndex)
    : m_resultType(resultType),
    m_needsToBeStored(needsToBeStored),
    m_mustBeCalculated(mustBeCalculated),
    m_resultName(resultName),
    m_gridScalarResultIndex(gridScalarResultIndex)
{
}
