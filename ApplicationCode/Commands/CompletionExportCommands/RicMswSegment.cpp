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

#include "RicMswSegment.h"

#include "RicMswExportInfo.h"

#include <cafPdmBase.h>
#include <cafPdmObject.h>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicMswSegment::RicMswSegment(const QString& label,
                             double         startMD,
                             double         endMD,
                             double         startTVD,
                             double         endTVD,
                             size_t         subIndex,
                             int            segmentNumber /*= -1*/)
    : m_label(label)
    , m_startMD(startMD)
    , m_endMD(endMD)
    , m_startTVD(startTVD)
    , m_endTVD(endTVD)
    , m_effectiveDiameter(0.15)
    , m_holeDiameter(RicMswExportInfo::defaultDoubleValue())
    , m_openHoleRoughnessFactor(5.0e-5)
    , m_skinFactor(RicMswExportInfo::defaultDoubleValue())
    , m_subIndex(subIndex)
    , m_segmentNumber(segmentNumber)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicMswSegment::label() const
{
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSegment::startMD() const
{
    return m_startMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSegment::endMD() const
{
    return m_endMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSegment::deltaMD() const
{
    return m_endMD - m_startMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSegment::startTVD() const
{
    return m_startTVD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSegment::endTVD() const
{
    return m_endTVD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSegment::deltaTVD() const
{
    return m_endTVD - m_startTVD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSegment::effectiveDiameter() const
{
    return m_effectiveDiameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSegment::holeDiameter() const
{
    return m_holeDiameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSegment::openHoleRoughnessFactor() const
{
    return m_openHoleRoughnessFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicMswSegment::skinFactor() const
{
    return m_skinFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RicMswSegment::subIndex() const
{
    return m_subIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RicMswSegment::segmentNumber() const
{
    return m_segmentNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<std::shared_ptr<RicMswCompletion>>& RicMswSegment::completions() const
{
    return m_completions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<RicMswCompletion>>& RicMswSegment::completions()
{
    return m_completions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswSegment::setLabel(const QString& label)
{
    m_label = label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswSegment::setEffectiveDiameter(double effectiveDiameter)
{
    m_effectiveDiameter = effectiveDiameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswSegment::setHoleDiameter(double holeDiameter)
{
    m_holeDiameter = holeDiameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswSegment::setOpenHoleRoughnessFactor(double roughnessFactor)
{
    m_openHoleRoughnessFactor = roughnessFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswSegment::setSkinFactor(double skinFactor)
{
    m_skinFactor = skinFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswSegment::setSegmentNumber(int segmentNumber)
{
    m_segmentNumber = segmentNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswSegment::addCompletion(std::shared_ptr<RicMswCompletion> completion)
{
    m_completions.push_back(completion);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicMswSegment::setSourcePdmObject(const caf::PdmObject* object)
{
    m_sourcePdmObject = const_cast<caf::PdmObject*>(object);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const caf::PdmObject* RicMswSegment::sourcePdmObject() const
{
    return m_sourcePdmObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicMswSegment::operator<(const RicMswSegment& rhs) const
{
    return startMD() < rhs.startMD();
}


