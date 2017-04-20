/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RifReaderInterface.h"

#include "RifReaderSettings.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderInterface::setReaderSetting(RifReaderSettings* settings)
{
    m_settings = settings;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderInterface::isFaultImportEnabled()
{
    if (m_settings.notNull())
    {
        return m_settings->importFaults;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderInterface::isImportOfCompleteMswDataEnabled()
{
    if (m_settings.notNull())
    {
        return m_settings->importAdvancedMswData;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderInterface::isNNCsEnabled()
{
    if (m_settings.notNull())
    {
        return m_settings->importNNCs;
    }

    return false;
}

const QString RifReaderInterface::faultIncludeFileAbsolutePathPrefix()
{
    if (m_settings.notNull())
    {
        return m_settings->faultIncludeFileAbsolutePathPrefix;
    }

    return QString();
}
