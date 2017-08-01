/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicfLoadCase.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

CAF_PDM_SOURCE_INIT(RicfLoadCase, "loadCase");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfLoadCase::RicfLoadCase()
{
    RICF_InitField(&m_path, "path", QString(), "Path to Case File",  "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfLoadCase::execute()
{
    bool ok = RiaApplication::instance()->openEclipseCaseFromFile(m_path);
    if (!ok)
    {
        RiaLogging::error(QString("loadCase: Unable to load case from %1").arg(m_path()));
    }
}
