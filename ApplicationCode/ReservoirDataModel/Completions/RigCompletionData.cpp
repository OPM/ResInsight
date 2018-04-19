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
RigCompletionData::RigCompletionData(const QString wellName, const RigCompletionDataGridCell& cellIndex, double orderingValue)
    : m_wellName(wellName)
    , m_cellIndex(cellIndex)
    , m_saturation(HUGE_VAL)
    , m_transmissibility(HUGE_VAL)
    , m_diameter(HUGE_VAL)
    , m_kh(HUGE_VAL)
    , m_skinFactor(HUGE_VAL)
    , m_dFactor(HUGE_VAL)
    , m_direction(DIR_UNDEF)
    , m_connectionState(OPEN)
    , m_count(1)
    , m_wpimult(HUGE_VAL)
    , m_isMainBore(false)
    , m_completionType(CT_UNDEFINED)
    , m_firstOrderingValue(orderingValue)
    , m_secondOrderingValue(HUGE_VAL)
{
}

//==================================================================================================
///
//==================================================================================================
RigCompletionData::~RigCompletionData() {}

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
bool RigCompletionData::operator<(const RigCompletionData& other) const
{
    if (m_wellName != other.m_wellName)
    {
        return (m_wellName < other.m_wellName);
    }

    if (m_completionType != other.m_completionType)
    {
        return (m_completionType < other.m_completionType);
    }

    if (m_firstOrderingValue != other.m_firstOrderingValue)
    {
        return (m_firstOrderingValue < other.m_firstOrderingValue);
    }

    if (m_secondOrderingValue != other.m_secondOrderingValue)
    {
        return (m_secondOrderingValue < other.m_secondOrderingValue);
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
void RigCompletionData::setFromFracture(double transmissibility, double skinFactor, double diameter)
{
    m_completionType   = FRACTURE;
    m_transmissibility = transmissibility;
    m_skinFactor       = skinFactor;
    m_diameter         = diameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCompletionData::setSecondOrderingValue(double orderingValue)
{
    m_secondOrderingValue = orderingValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCompletionData::setDiameter(double diameter)
{
    m_diameter = diameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCompletionData::setTransmissibility(double transmissibility)
{
    m_transmissibility = transmissibility;
}

//==================================================================================================
///
//==================================================================================================
void RigCompletionData::setTransAndWPImultBackgroundDataFromFishbone(double        transmissibility,
                                                                     double        skinFactor,
                                                                     double        diameter,
                                                                     CellDirection direction,
                                                                     bool          isMainBore)
{
    m_completionType   = FISHBONES;
    m_transmissibility = transmissibility;
    m_skinFactor       = skinFactor;
    m_diameter         = diameter;
    m_direction        = direction;
    m_isMainBore       = isMainBore;
}

//==================================================================================================
///
//==================================================================================================
void RigCompletionData::setTransAndWPImultBackgroundDataFromPerforation(double        transmissibility,
                                                                        double        skinFactor,
                                                                        double        diameter,
                                                                        CellDirection direction)
{
    m_completionType   = PERFORATION;
    m_transmissibility = transmissibility;
    m_skinFactor       = skinFactor;
    m_diameter         = diameter;
    m_direction        = direction;
    m_isMainBore       = true;
}

//==================================================================================================
///
//==================================================================================================
void RigCompletionData::setCombinedValuesExplicitTrans(double         transmissibility,
                                                       double         skinFactor,
                                                       double         diameter,
                                                       CellDirection  celldirection,
                                                       CompletionType completionType)
{
    m_transmissibility = transmissibility;
    m_skinFactor       = skinFactor;
    m_diameter         = diameter;
    m_direction        = celldirection;
    m_completionType   = completionType;
}

//==================================================================================================
///
//==================================================================================================
void RigCompletionData::setCombinedValuesImplicitTransWPImult(double         wpimult,
                                                              double         skinFactor,
                                                              double         diameter,
                                                              CellDirection  celldirection,
                                                              CompletionType completionType)
{
    m_wpimult        = wpimult;
    m_direction      = celldirection;
    m_completionType = completionType;
    m_skinFactor     = skinFactor;
    m_diameter       = diameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCompletionData::isNonDarcyFlow() const
{
    if (!isDefaultValue(m_kh)) return true;
    if (!isDefaultValue(m_dFactor)) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCompletionData::setDFactor(double dFactor)
{
    m_dFactor = dFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCompletionData::setKh(double kh)
{
    m_kh = kh;
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RigCompletionMetaData>& RigCompletionData::metadata() const
{
    return m_metadata;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RigCompletionData::wellName() const
{
    return m_wellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigCompletionDataGridCell& RigCompletionData::completionDataGridCell() const
{
    return m_cellIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
WellConnectionState RigCompletionData::connectionState() const
{
    return m_connectionState;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigCompletionData::saturation() const
{
    return m_saturation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigCompletionData::transmissibility() const
{
    return m_transmissibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigCompletionData::diameter() const
{
    return m_diameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigCompletionData::kh() const
{
    return m_kh;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigCompletionData::skinFactor() const
{
    return m_skinFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigCompletionData::dFactor() const
{
    return m_dFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CellDirection RigCompletionData::direction() const
{
    return m_direction;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigCompletionData::count() const
{
    return m_count;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigCompletionData::wpimult() const
{
    return m_wpimult;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCompletionData::CompletionType RigCompletionData::completionType() const
{
    return m_completionType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCompletionData::isMainBore() const
{
    return m_isMainBore;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigCompletionData::firstOrderingValue() const
{
    return m_firstOrderingValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigCompletionData::secondOrderingValue() const
{
    return m_secondOrderingValue;
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
    target.m_metadata            = from.m_metadata;
    target.m_wellName            = from.m_wellName;
    target.m_cellIndex           = from.m_cellIndex;
    target.m_connectionState     = from.m_connectionState;
    target.m_saturation          = from.m_saturation;
    target.m_transmissibility    = from.m_transmissibility;
    target.m_diameter            = from.m_diameter;
    target.m_kh                  = from.m_kh;
    target.m_skinFactor          = from.m_skinFactor;
    target.m_dFactor             = from.m_dFactor;
    target.m_direction           = from.m_direction;
    target.m_isMainBore          = from.m_isMainBore;
    target.m_count               = from.m_count;
    target.m_wpimult             = from.m_wpimult;
    target.m_completionType      = from.m_completionType;
    target.m_firstOrderingValue  = from.m_firstOrderingValue;
    target.m_secondOrderingValue = from.m_secondOrderingValue;
}
