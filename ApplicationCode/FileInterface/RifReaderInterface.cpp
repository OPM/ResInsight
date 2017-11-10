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

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RifReaderSettings.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderInterface::isFaultImportEnabled()
{
    return readerSettings()->importFaults;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderInterface::isImportOfCompleteMswDataEnabled()
{
    return readerSettings()->importAdvancedMswData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderInterface::isNNCsEnabled()
{
    return readerSettings()->importNNCs;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QString RifReaderInterface::faultIncludeFileAbsolutePathPrefix()
{
    return readerSettings()->faultIncludeFileAbsolutePathPrefix;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderInterface::setTimeStepFilter(const std::vector<size_t>& fileTimeStepIndices)
{
    m_fileTimeStepIndices = fileTimeStepIndices;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<RiaDefines::PhaseType> RifReaderInterface::availablePhases() const
{
    return std::set<RiaDefines::PhaseType>();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifReaderInterface::isTimeStepIncludedByFilter(size_t timeStepIndex) const
{
    if (m_fileTimeStepIndices.empty()) return true;

    for (auto i : m_fileTimeStepIndices)
    {
        if (i == timeStepIndex)
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RifReaderInterface::timeStepIndexOnFile(size_t timeStepIndex) const
{
    if (timeStepIndex < m_fileTimeStepIndices.size())
    {
        return m_fileTimeStepIndices[timeStepIndex];
    }

    return timeStepIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RifReaderSettings* RifReaderInterface::readerSettings() const
{
    RiaPreferences* prefs = RiaApplication::instance()->preferences();

    CVF_ASSERT(prefs->readerSettings());

    return prefs->readerSettings();
}

