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

#include <QStringList>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCellFilterInterval::RimCellFilterInterval( size_t minIncludeVal, size_t maxIncludeVal )
    : m_minIncludeVal( minIncludeVal )
    , m_maxIncludeVal( maxIncludeVal )
{
    m_valid = maxIncludeVal >= minIncludeVal;
    m_valid = m_valid && minIncludeVal > 0;
}

RimCellFilterInterval::RimCellFilterInterval( size_t includeVal )
    : m_minIncludeVal( includeVal )
    , m_maxIncludeVal( includeVal )
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
    if ( ( val >= m_minIncludeVal ) && ( val <= m_maxIncludeVal ) ) return m_valid;
    return false;
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
    if ( m_intervals.size() == 0 ) return m_includeAllByDefault;

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
size_t RimCellFilterIntervalTool::numberFromPart( QString strVal ) const
{
    return strVal.toUInt();
}

//--------------------------------------------------------------------------------------------------
///
// Define a range with the comma separated format A,B,C-D, etc.,  i.e. 1,4,5-8
// Only positive numbers are supported.
//--------------------------------------------------------------------------------------------------
void RimCellFilterIntervalTool::setInterval( bool enabled, QString intervalText )
{
    m_intervals.clear();

    if ( !enabled ) return;

    QStringList parts = intervalText.split( ',', QString::SkipEmptyParts );

    for ( auto& part : parts )
    {
        QStringList minmax = part.split( '-', QString::SkipEmptyParts );
        switch ( minmax.size() )
        {
            case 1:
                m_intervals.push_back( RimCellFilterInterval( numberFromPart( minmax[0] ) ) );
                break;
            case 2:
                m_intervals.push_back( RimCellFilterInterval( numberFromPart( minmax[0] ), numberFromPart( minmax[1] ) ) );
                break;

            default:
                break;
        }
    }
}
