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

#include "RigCompletionData.h"

#include "RiaLogging.h"

#include <QString>
#include <cmath> // Needed for HUGE_VAL on linux

//==================================================================================================
/// 
//==================================================================================================
RigCompletionData::RigCompletionData(const QString wellName, const IJKCellIndex cellIndex)
    : m_wellName(wellName),
      m_cellIndex(cellIndex),
      m_saturation(HUGE_VAL),
      m_transmissibility(HUGE_VAL),
      m_diameter(HUGE_VAL),
      m_kh(HUGE_VAL),
      m_skinFactor(HUGE_VAL),
      m_dFactor(HUGE_VAL),
      m_direction(DIR_UNDEF),
      m_connectionState(OPEN),
      m_count(1)
{
}

//==================================================================================================
/// 
//==================================================================================================
RigCompletionData::~RigCompletionData()
{
}

//==================================================================================================
/// 
//==================================================================================================
RigCompletionData::RigCompletionData(const RigCompletionData& other)
{
    copy(*this, other);
}

//==================================================================================================
/// 
//==================================================================================================
RigCompletionData RigCompletionData::combine(const RigCompletionData& other) const
{
    RigCompletionData result(*this);
    CVF_ASSERT(result.m_wellName == other.m_wellName);
    CVF_ASSERT(result.m_cellIndex == other.m_cellIndex);

    if (onlyOneIsDefaulted(result.m_transmissibility, other.m_transmissibility))
    {
        RiaLogging::error("Transmissibility defaulted in one but not both, will produce erroneous result");
    }
    else
    {
        result.m_transmissibility += other.m_transmissibility;
    }

    result.m_metadata.reserve(result.m_metadata.size() + other.m_metadata.size());
    result.m_metadata.insert(result.m_metadata.end(), other.m_metadata.begin(), other.m_metadata.end());

    result.m_count += other.m_count;

    return result;
}

//==================================================================================================
/// 
//==================================================================================================
bool RigCompletionData::operator<(const RigCompletionData& other) const
{
    if (m_wellName < other.m_wellName)
    {
        return true;
    }
    return m_cellIndex < other.m_cellIndex;
}

//==================================================================================================
/// 
//==================================================================================================
RigCompletionData& RigCompletionData::operator=(const RigCompletionData& other)
{
    if (this != &other)
    {
        copy(*this, other);
    }
    return *this;
}

//==================================================================================================
/// 
//==================================================================================================
void RigCompletionData::setFromFracture(double transmissibility, double skinFactor)
{
    m_transmissibility = transmissibility;
    m_skinFactor = skinFactor;
}

//==================================================================================================
/// 
//==================================================================================================
void RigCompletionData::setFromFishbone(double diameter, CellDirection direction)
{
    m_diameter = diameter;
    m_direction = direction;
}

//==================================================================================================
/// 
//==================================================================================================
void RigCompletionData::setFromPerforation(double diameter, CellDirection direction)
{
    m_diameter = diameter;
    m_direction = direction;
}

//==================================================================================================
/// 
//==================================================================================================
void RigCompletionData::addMetadata(const QString& name, const QString& comment)
{
    m_metadata.push_back(RigCompletionMetaData(name, comment));
}

//==================================================================================================
/// 
//==================================================================================================
bool RigCompletionData::isDefaultValue(double val)
{
    return val == HUGE_VAL;
}

//==================================================================================================
/// 
//==================================================================================================
bool RigCompletionData::onlyOneIsDefaulted(double first, double second)
{
    if (first == HUGE_VAL)
    {
        if (second == HUGE_VAL)
        {
            // Both have default values
            return false;
        }
        else
        {
            // First has default value, second does not
            return true;
        }
    }
    if (second == HUGE_VAL)
    {
        // Second has default value, first does not
        return true;
    }
    
    // Neither has default values
    return false;
}

//==================================================================================================
/// 
//==================================================================================================
void RigCompletionData::copy(RigCompletionData& target, const RigCompletionData& from)
{
    target.m_metadata = from.m_metadata;
    target.m_wellName = from.m_wellName;
    target.m_cellIndex = from.m_cellIndex;
    target.m_connectionState = from.m_connectionState;
    target.m_saturation = from.m_saturation;
    target.m_transmissibility = from.m_transmissibility;
    target.m_diameter = from.m_diameter;
    target.m_kh = from.m_kh;
    target.m_skinFactor = from.m_skinFactor;
    target.m_dFactor = from.m_dFactor;
    target.m_direction = from.m_direction;
    target.m_count = from.m_count;
}
