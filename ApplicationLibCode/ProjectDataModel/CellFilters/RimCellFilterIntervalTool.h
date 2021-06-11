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

#pragma once

#include <QString>
#include <list>

class RimCellFilterInterval
{
public:
    RimCellFilterInterval( size_t minIncludeVal, size_t maxIncludeVal );
    RimCellFilterInterval( size_t includeVal );
    ~RimCellFilterInterval();

    bool isIncluded( size_t val ) const;

private:
    size_t m_minIncludeVal;
    size_t m_maxIncludeVal;
    bool   m_valid;
};

class RimCellFilterIntervalTool
{
public:
    RimCellFilterIntervalTool( bool includeAllByDefault = true );
    ~RimCellFilterIntervalTool();

    void setInterval( bool enabled, QString intervalText );
    bool isNumberIncluded( size_t number ) const;

private:
    size_t numberFromPart( QString strVal ) const;

    bool    m_includeAllByDefault;
    QString m_intervalText;

    std::list<RimCellFilterInterval> m_intervals;
};
