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
        : m_resultCatType( RiaDefines::UNDEFINED )
        , m_timeLapseBaseFrameIdx( NO_TIME_LAPSE )
        , m_differenceCaseId( NO_CASE_DIFF )
    {
    }

    explicit RigEclipseResultAddress( const QString& resultName )
        : m_resultCatType( RiaDefines::UNDEFINED )
        , m_resultName( resultName )
        , m_timeLapseBaseFrameIdx( NO_TIME_LAPSE )
        , m_differenceCaseId( NO_CASE_DIFF )
    {
    }

    explicit RigEclipseResultAddress( RiaDefines::ResultCatType type,
                                      const QString&            resultName,
                                      int                       timeLapseBaseTimeStep = NO_TIME_LAPSE,
                                      int                       differenceCaseId      = NO_CASE_DIFF )
        : m_resultCatType( type )
        , m_resultName( resultName )
        , m_timeLapseBaseFrameIdx( timeLapseBaseTimeStep )
        , m_differenceCaseId( differenceCaseId )
    {
    }

    bool isValid() const
    {
        if ( m_resultName.isEmpty() || m_resultName == RiaDefines::undefinedResultName() )
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    static constexpr int allTimeLapsesValue() { return ALL_TIME_LAPSES; }
    static constexpr int noTimeLapseValue() { return NO_TIME_LAPSE; }
    static constexpr int noCaseDiffValue() { return NO_CASE_DIFF; }

    bool isTimeLapse() const { return m_timeLapseBaseFrameIdx > NO_TIME_LAPSE; }
    bool representsAllTimeLapses() const { return m_timeLapseBaseFrameIdx == ALL_TIME_LAPSES; }

    bool hasDifferenceCase() const { return m_differenceCaseId > NO_CASE_DIFF; }

    bool operator<( const RigEclipseResultAddress& other ) const
    {
        if ( m_differenceCaseId != other.m_differenceCaseId )
        {
            return ( m_differenceCaseId < other.m_differenceCaseId );
        }

        if ( m_timeLapseBaseFrameIdx != other.m_timeLapseBaseFrameIdx )
        {
            return ( m_timeLapseBaseFrameIdx < other.m_timeLapseBaseFrameIdx );
        }

        if ( m_resultCatType != other.m_resultCatType )
        {
            return ( m_resultCatType < other.m_resultCatType );
        }

        return ( m_resultName < other.m_resultName );
    }

    bool operator==( const RigEclipseResultAddress& other ) const
    {
        if ( m_resultCatType != other.m_resultCatType || m_resultName != other.m_resultName ||
             m_timeLapseBaseFrameIdx != other.m_timeLapseBaseFrameIdx || m_differenceCaseId != other.m_differenceCaseId )
        {
            return false;
        }

        return true;
    }

    RiaDefines::ResultCatType m_resultCatType;
    QString                   m_resultName;

    int m_timeLapseBaseFrameIdx;
    int m_differenceCaseId;

private:
    static const int ALL_TIME_LAPSES = -2;
    static const int NO_TIME_LAPSE   = -1;
    static const int NO_CASE_DIFF    = -1;
};
