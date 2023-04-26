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
#include "RiaResultNames.h"

#include <QString>

class RigEclipseResultAddress
{
public:
    RigEclipseResultAddress()
        : m_divideByCellFaceArea( false )
        , m_differenceCaseId( NO_CASE_DIFF )
        , m_timeLapseBaseFrameIdx( NO_TIME_LAPSE )
        , m_resultCatType( RiaDefines::ResultCatType::UNDEFINED )
        , m_resultDataType( RiaDefines::ResultDataType::DOUBLE )
    {
    }

    explicit RigEclipseResultAddress( const QString& resultName )
        : m_divideByCellFaceArea( false )
        , m_differenceCaseId( NO_CASE_DIFF )
        , m_timeLapseBaseFrameIdx( NO_TIME_LAPSE )
        , m_resultCatType( RiaDefines::ResultCatType::UNDEFINED )
        , m_resultDataType( RiaDefines::ResultDataType::DOUBLE )
        , m_resultName( resultName.toStdString() )
    {
    }

    explicit RigEclipseResultAddress( RiaDefines::ResultCatType type,
                                      const QString&            resultName,
                                      int                       timeLapseBaseTimeStep = NO_TIME_LAPSE,
                                      int                       differenceCaseId      = NO_CASE_DIFF,
                                      bool                      divideByCellFaceArea  = false )
        : m_divideByCellFaceArea( false )
        , m_differenceCaseId( differenceCaseId )
        , m_timeLapseBaseFrameIdx( timeLapseBaseTimeStep )
        , m_resultCatType( type )
        , m_resultDataType( RiaDefines::ResultDataType::DOUBLE )
        , m_resultName( resultName.toStdString() )
    {
        enableDivideByCellFaceArea( divideByCellFaceArea );
    }

    bool isValid() const
    {
        return !( m_resultName.empty() || QString::fromStdString( m_resultName ) == RiaResultNames::undefinedResultName() );
    }

    // Delta Time Step
    bool                 isDeltaTimeStepActive() const { return m_timeLapseBaseFrameIdx > NO_TIME_LAPSE; }
    void                 setDeltaTimeStepIndex( int timeStepIndex ) { m_timeLapseBaseFrameIdx = timeStepIndex; }
    int                  deltaTimeStepIndex() const { return m_timeLapseBaseFrameIdx; }
    bool                 representsAllTimeLapses() const { return m_timeLapseBaseFrameIdx == ALL_TIME_LAPSES; }
    static constexpr int allTimeLapsesValue() { return ALL_TIME_LAPSES; }
    static constexpr int noTimeLapseValue() { return NO_TIME_LAPSE; }

    // Delta Grid Case
    bool                 isDeltaCaseActive() const { return m_differenceCaseId > NO_CASE_DIFF; }
    void                 setDeltaCaseId( int caseId ) { m_differenceCaseId = caseId; }
    int                  deltaCaseId() const { return m_differenceCaseId; }
    static constexpr int noCaseDiffValue() { return NO_CASE_DIFF; }

    // Divide by Cell Face Area
    void enableDivideByCellFaceArea( bool enable ) { m_divideByCellFaceArea = enable; }

    void                       setDataType( RiaDefines::ResultDataType dataType ) { m_resultDataType = dataType; }
    RiaDefines::ResultDataType dataType() const { return m_resultDataType; }

    bool isDivideByCellFaceAreaActive() const { return m_divideByCellFaceArea; }

    auto operator<=>( const RigEclipseResultAddress& ) const = default;

    const QString resultName() const { return QString::fromStdString( m_resultName ); }
    void          setResultName( const QString& name ) { m_resultName = name.toStdString(); }

    RiaDefines::ResultCatType resultCatType() const { return m_resultCatType; }
    void                      setResultCatType( RiaDefines::ResultCatType catType ) { m_resultCatType = catType; }

private:
    // Note: The order of the members is important for the sorting of the addresses
    bool                       m_divideByCellFaceArea;
    int                        m_differenceCaseId;
    int                        m_timeLapseBaseFrameIdx;
    RiaDefines::ResultCatType  m_resultCatType;
    RiaDefines::ResultDataType m_resultDataType;
    std::string                m_resultName;

    static const int ALL_TIME_LAPSES = -2;
    static const int NO_TIME_LAPSE   = -1;
    static const int NO_CASE_DIFF    = -1;
};
