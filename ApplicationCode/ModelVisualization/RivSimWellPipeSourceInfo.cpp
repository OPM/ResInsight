/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RivSimWellPipeSourceInfo.h"

#include "RimEclipseView.h"
#include "RimEclipseWellCollection.h"
#include "RimSimWellInView.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivSimWellPipeSourceInfo::RivSimWellPipeSourceInfo(RimSimWellInView* eclipseWell, size_t branchIndex)
    : m_eclipseWell(eclipseWell),
    m_branchIndex(branchIndex)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSimWellInView* RivSimWellPipeSourceInfo::well() const
{
    return m_eclipseWell.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RivSimWellPipeSourceInfo::branchIndex() const
{
    return m_branchIndex;
}

