/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
#pragma once

#include "RiaDefines.h"
#include <QString>

class RigEclipseResultAddress
{
public:
    RigEclipseResultAddress()
        : m_resultCatType(RiaDefines::UNDEFINED)
        , m_timeLapseBaseFrameIdx(NO_TIME_LAPSE)
    {}
 
    explicit RigEclipseResultAddress(const QString& resultName)
        : m_resultCatType(RiaDefines::UNDEFINED)
        , m_resultName(resultName)
        , m_timeLapseBaseFrameIdx(NO_TIME_LAPSE)
    {}

    explicit RigEclipseResultAddress(RiaDefines::ResultCatType type, const QString& resultName, int timeLapseBaseTimeStep = NO_TIME_LAPSE)
        : m_resultCatType(type)
        , m_resultName(resultName)
        , m_timeLapseBaseFrameIdx(timeLapseBaseTimeStep)
    {}

    bool isValid() const
    {
        if (m_resultName.isEmpty() || m_resultName == RiaDefines::undefinedResultName())
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    static const int ALL_TIME_LAPSES = -2;
    static const int NO_TIME_LAPSE = -1;

    bool isTimeLapse() const { return m_timeLapseBaseFrameIdx > NO_TIME_LAPSE;}
    bool representsAllTimeLapses() const { return m_timeLapseBaseFrameIdx == ALL_TIME_LAPSES;}

    bool operator< (const RigEclipseResultAddress& other ) const
    {
        if (m_timeLapseBaseFrameIdx != other.m_timeLapseBaseFrameIdx)
        {
            return (m_timeLapseBaseFrameIdx < other.m_timeLapseBaseFrameIdx);
        }

        if (m_resultCatType != other.m_resultCatType)
        {
            return (m_resultCatType <  other.m_resultCatType);
        }

        return (m_resultName <  other.m_resultName);
    }

    bool operator==(const RigEclipseResultAddress& other ) const
    {
        if (   m_resultCatType != other.m_resultCatType
            || m_resultName    != other.m_resultName
            || m_timeLapseBaseFrameIdx != other.m_timeLapseBaseFrameIdx)
        {
            return false;
        }

        return true;  
    }

    RiaDefines::ResultCatType m_resultCatType;
    QString                   m_resultName;
    int                       m_timeLapseBaseFrameIdx;

};


