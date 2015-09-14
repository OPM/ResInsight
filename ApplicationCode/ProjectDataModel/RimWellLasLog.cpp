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

#include "RimWellLasLog.h"

#include <QString>


CAF_PDM_SOURCE_INIT(RimWellLasLog, "WellLasLog");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLasLog::RimWellLasLog()
{
    CAF_PDM_InitObject("Well LAS File Log", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_name, "Name", "",  "", "", "");
    m_name.uiCapability()->setUiHidden(true);
    m_name.xmlCapability()->setIOWritable(false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLasLog::setName(const QString& name)
{
    m_name = name;
}