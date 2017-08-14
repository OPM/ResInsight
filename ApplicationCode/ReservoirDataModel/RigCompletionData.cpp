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

#include "cvfAssert.h"

#include <QString>

#include <cmath> // Needed for HUGE_VAL on Linux


//==================================================================================================
/// 
//==================================================================================================
RigCompletionData::RigCompletionData(const QString wellName, const IJKCellIndex& cellIndex)
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
      m_count(1),
      m_wpimult(HUGE_VAL),
      m_isMainBore(false),
      m_readyForExport(false)
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
RigCompletionData RigCompletionData::combine(const std::vector<RigCompletionData>& completions)
{
    CVF_ASSERT(!completions.empty());

    auto it = completions.cbegin();
    RigCompletionData result(*it);
    ++it;

    for (; it != completions.cend(); ++it)
    {
        if (it->completionType() != result.completionType())
        {
            RiaLogging::error(QString("Cannot combine completions of different types in same cell [%1, %2, %3]").arg(result.m_cellIndex.i).arg(result.m_cellIndex.j).arg(result.m_cellIndex.k));
            continue;
        }
        if (onlyOneIsDefaulted(result.m_transmissibility, it->m_transmissibility))
        {
            RiaLogging::error("Transmissibility defaulted in one but not both, will produce erroneous result");
        }
        else
        {
            result.m_transmissibility += it->m_transmissibility;
        }

        result.m_metadata.reserve(result.m_metadata.size() + it->m_metadata.size());
        result.m_metadata.insert(result.m_metadata.end(), it->m_metadata.begin(), it->m_metadata.end());

        result.m_count += it->m_count;
    }

    return result;
}

//==================================================================================================
/// 
//==================================================================================================
bool RigCompletionData::operator<(const RigCompletionData& other) const
{
    if (m_wellName != other.m_wellName) 
    { 
        return (m_wellName < other.m_wellName);
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
    m_completionType = FRACTURE;
    m_transmissibility = transmissibility;
    m_skinFactor = skinFactor;
}

//==================================================================================================
/// 
//==================================================================================================
void RigCompletionData::setTransAndWPImultBackgroundDataFromFishbone(double transmissibility, 
                                                                     double skinFactor,
                                                                     double diameter,
                                                                     CellDirection direction,
                                                                     bool isMainBore)
{
    m_completionType = FISHBONES;
    m_transmissibility = transmissibility;
    m_skinFactor = skinFactor;
    m_diameter = diameter;
    m_direction = direction;
    m_isMainBore = isMainBore;
}

//==================================================================================================
/// 
//==================================================================================================
void RigCompletionData::setTransAndWPImultBackgroundDataFromPerforation(double transmissibility,
                                                                        double skinFactor, 
                                                                        double diameter, 
                                                                        CellDirection direction)
{
    m_completionType = PERFORATION;
    m_transmissibility = transmissibility;
    m_skinFactor = skinFactor;
    m_diameter = diameter;
    m_direction = direction;
    m_isMainBore = true;
}

//==================================================================================================
/// 
//==================================================================================================
void RigCompletionData::setCombinedValuesExplicitTrans(double transmissibility, 
                                                       CompletionType completionType)
{
    m_completionType = completionType;
    m_transmissibility = transmissibility;
    m_readyForExport = true;
}

//==================================================================================================
/// 
//==================================================================================================
void RigCompletionData::setCombinedValuesImplicitTransWPImult(double wpimult, 
                                                              CellDirection celldirection, 
                                                              double skinFactor,
                                                              double wellDiameter,
                                                              CompletionType completionType)
{
    m_wpimult = wpimult;
    m_direction = celldirection;
    m_completionType = completionType;
    m_skinFactor = skinFactor;
    m_diameter = wellDiameter;
    m_readyForExport = true;
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
    target.m_isMainBore = from.m_isMainBore;
    target.m_readyForExport = from.m_readyForExport;
    target.m_count = from.m_count;
    target.m_wpimult = from.m_wpimult;
    target.m_completionType = from.m_completionType;
}
