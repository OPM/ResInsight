/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Equinor ASA
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

#include "RicMswCompletions.h"

#include "RicMswSubSegment.h"

#include "RimWellPathValve.h"
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswCompletion::RicMswCompletion(const QString&                    label,
                                   size_t                            index /* = cvf::UNDEFINED_SIZE_T */,
                                   int                               branchNumber /*= 0*/)
    : m_label(label)
    , m_index(index)
    , m_branchNumber(branchNumber)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RicMswCompletion::label() const
{
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RicMswCompletion::index() const
{
    return m_index;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicMswCompletion::branchNumber() const
{
    return m_branchNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswCompletion::setBranchNumber(int branchNumber)
{
    m_branchNumber = branchNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswCompletion::addSubSegment(std::shared_ptr<RicMswSubSegment> subSegment)
{
    m_subSegments.push_back(subSegment);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<RicMswSubSegment>>& RicMswCompletion::subSegments()
{
    return m_subSegments;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::shared_ptr<RicMswSubSegment>>& RicMswCompletion::subSegments() const
{
    return m_subSegments;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswCompletion::setLabel(const QString& label)
{
    m_label = label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswFracture::RicMswFracture(const QString& label,
                               size_t         index /*= cvf::UNDEFINED_SIZE_T*/,
                               int            branchNumber /*= cvf::UNDEFINED_INT*/)
    : RicMswCompletion(label, index, branchNumber)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCompletionData::CompletionType RicMswFracture::completionType() const
{
    return RigCompletionData::FRACTURE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswPerforation::RicMswPerforation(const QString& label,
                                     size_t         index /*= cvf::UNDEFINED_SIZE_T*/,
                                     int            branchNumber /*= cvf::UNDEFINED_INT*/)
    : RicMswCompletion(label, index, branchNumber)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCompletionData::CompletionType RicMswPerforation::completionType() const
{
    return RigCompletionData::PERFORATION;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswValve::RicMswValve(const QString& label,
                         size_t         index /*= cvf::UNDEFINED_SIZE_T*/,
                         int            branchNumber /*= cvf::UNDEFINED_INT*/)
    : RicMswCompletion(label, index, branchNumber)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswICD::RicMswICD(const QString& label, size_t index /*= cvf::UNDEFINED_SIZE_T*/, int branchNumber /*= cvf::UNDEFINED_INT*/)
    : RicMswValve(label, index, branchNumber)
    , m_flowCoefficient(0.0)
    , m_area(0.0)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswICD::flowCoefficient() const
{
    return m_flowCoefficient;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswICD::area() const
{
    return m_area;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswICD::setFlowCoefficient(double icdFlowCoefficient)
{
    m_flowCoefficient = icdFlowCoefficient;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswICD::setArea(double icdArea)
{
    m_area = icdArea;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswFishbonesICD::RicMswFishbonesICD(const QString& label,
                                       size_t         index /*= cvf::UNDEFINED_SIZE_T*/,
                                       int            branchNumber /*= cvf::UNDEFINED_INT*/)
    : RicMswICD(label, index, branchNumber)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCompletionData::CompletionType RicMswFishbonesICD::completionType() const
{
    return RigCompletionData::FISHBONES_ICD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswPerforationICD::RicMswPerforationICD(const QString& label,
                                           size_t         index /*= cvf::UNDEFINED_SIZE_T*/,
                                           int            branchNumber /*= cvf::UNDEFINED_INT*/)
    : RicMswICD(label, index, branchNumber)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCompletionData::CompletionType RicMswPerforationICD::completionType() const
{
    return RigCompletionData::PERFORATION_ICD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswPerforationAICD::RicMswPerforationAICD(const QString& label,
                                             size_t         index /*= cvf::UNDEFINED_SIZE_T*/,
                                             int            branchNumber /*= cvf::UNDEFINED_INT*/)
    : RicMswValve(label, index, branchNumber), m_valid(false), m_deviceOpen(false)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCompletionData::CompletionType RicMswPerforationAICD::completionType() const
{
    return RigCompletionData::PERFORATION_AICD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicMswPerforationAICD::isValid() const
{
    return m_valid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswPerforationAICD::setIsValid(bool valid)
{
    m_valid = valid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicMswPerforationAICD::isOpen() const
{
    return m_deviceOpen;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswPerforationAICD::setIsOpen(bool deviceOpen)
{
    m_deviceOpen = deviceOpen;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswPerforationAICD::length() const
{
    return m_length;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswPerforationAICD::setLength(double length)
{
    m_length = length;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::array<double, AICD_NUM_PARAMS>& RicMswPerforationAICD::values() const
{
    return m_parameters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<double, AICD_NUM_PARAMS>& RicMswPerforationAICD::values()
{
    return m_parameters;
}
