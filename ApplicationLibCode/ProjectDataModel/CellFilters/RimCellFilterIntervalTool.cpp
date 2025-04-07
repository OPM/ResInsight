/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 Equinor ASA
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

#include "RimCellFilterIntervalTool.h"

#include "RiaTextStringTools.h"

#include <QStringList>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellFilterInterval::RimCellFilterInterval( size_t minIncludeVal, size_t maxIncludeVal, size_t step )
    : m_minIncludeVal( minIncludeVal )
    , m_maxIncludeVal( maxIncludeVal )
    , m_step( step )
{
    m_valid = maxIncludeVal >= minIncludeVal;
    m_valid = m_valid && minIncludeVal > 0;
    m_valid = m_valid && step > 0;
}

RimCellFilterInterval::RimCellFilterInterval( size_t includeVal )
    : m_minIncludeVal( includeVal )
    , m_maxIncludeVal( includeVal )
    , m_step( 1 )
{
    m_valid = includeVal > 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellFilterInterval::~RimCellFilterInterval()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCellFilterInterval::isIncluded( size_t val ) const
{
    if ( ( val < m_minIncludeVal ) || ( val > m_maxIncludeVal ) ) return false;

    size_t tmp = val - m_minIncludeVal;

    return m_valid && ( tmp % m_step == 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellFilterIntervalTool::RimCellFilterIntervalTool( bool includeAllByDefault )
    : m_includeAllByDefault( includeAllByDefault )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellFilterIntervalTool::~RimCellFilterIntervalTool()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCellFilterIntervalTool::isNumberIncluded( size_t number ) const
{
    if ( m_intervals.empty() ) return m_includeAllByDefault;

    number = number + 1;

    for ( const auto& interval : m_intervals )
    {
        if ( interval.isIncluded( number ) ) return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimCellFilterIntervalTool::numberFromPart( std::string strVal ) const
{
    QString qStrVal = QString::fromStdString( strVal );
    return qStrVal.toUInt();
}

//--------------------------------------------------------------------------------------------------
///
// Define a range with the comma separated format A,B,C-D, etc.,  i.e. 1,4,5-8
// Only numbers > 0 are supported.
// For a range with increment > 1, use i.e. 4-8:2
//
// Related code in RiaStdStringTools::valuesFromRangeSelection( const std::string& s )
//
//--------------------------------------------------------------------------------------------------
void RimCellFilterIntervalTool::setInterval( bool enabled, std::string intervalText )
{
    m_intervals.clear();

    if ( !enabled ) return;

    QString qIntervalText = QString::fromStdString( intervalText );

    QStringList parts = RiaTextStringTools::splitSkipEmptyParts( qIntervalText, "," );

    for ( auto& part : parts )
    {
        QStringList rangeStep = RiaTextStringTools::splitSkipEmptyParts( part, ":" );
        QStringList minmax    = RiaTextStringTools::splitSkipEmptyParts( rangeStep[0], "-" );
        size_t      step      = 1;
        if ( rangeStep.size() == 2 )
        {
            step = numberFromPart( rangeStep[1].toStdString() );
        }

        switch ( minmax.size() )
        {
            case 1:
                m_intervals.push_back( RimCellFilterInterval( numberFromPart( minmax[0].toStdString() ) ) );
                break;
            case 2:
                m_intervals.push_back(
                    RimCellFilterInterval( numberFromPart( minmax[0].toStdString() ), numberFromPart( minmax[1].toStdString() ), step ) );
                break;

            default:
                break;
        }
    }
}
