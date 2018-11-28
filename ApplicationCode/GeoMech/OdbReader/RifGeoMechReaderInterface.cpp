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

#include "RifGeoMechReaderInterface.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifGeoMechReaderInterface::RifGeoMechReaderInterface()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifGeoMechReaderInterface::~RifGeoMechReaderInterface()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifGeoMechReaderInterface::setTimeStepFilter(const std::vector<size_t>& fileTimeStepIndices)
{
    m_fileTimeStepIndices.reserve(fileTimeStepIndices.size());
    for (size_t stepIndex : fileTimeStepIndices)
    {
        m_fileTimeStepIndices.push_back(static_cast<int>(stepIndex));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifGeoMechReaderInterface::isTimeStepIncludedByFilter(int timeStepIndex) const
{
    CVF_ASSERT(timeStepIndex >= 0);
    if (m_fileTimeStepIndices.empty()) return true;

    for (auto i : m_fileTimeStepIndices)
    {
        if (i == static_cast<size_t>(timeStepIndex))
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RifGeoMechReaderInterface::timeStepIndexOnFile(int timeStepIndex) const
{
    if (m_fileTimeStepIndices.empty())
    {
        return timeStepIndex;
    }
    CVF_ASSERT(timeStepIndex >= 0);
    CVF_ASSERT(static_cast<size_t>(timeStepIndex) < m_fileTimeStepIndices.size());

    if (static_cast<size_t>(timeStepIndex) < m_fileTimeStepIndices.size())
    {
        return static_cast<int>(m_fileTimeStepIndices[timeStepIndex]);
    }

    return timeStepIndex;
}