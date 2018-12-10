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

#include "RimWellLogFileChannel.h"

#include "RiaFieldHandleTools.h"

#include <QString>


CAF_PDM_SOURCE_INIT(RimWellLogFileChannel, "WellLogFileChannel");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogFileChannel::RimWellLogFileChannel()
{
    CAF_PDM_InitObject("Well Log File Channel", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_name, "Name", "",  "", "", "");
    RiaFieldhandleTools::disableWriteAndSetFieldHidden(&m_name);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogFileChannel::setName(const QString& name)
{
    m_name = name;
}