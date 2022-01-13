/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include <set>
#include <string>
#include <vector>

#include <QDateTime>

//==================================================================================================
///
///
//==================================================================================================
class RifEclipseRftAddress
{
public:
    enum class RftWellLogChannelType
    {
        NONE,
        TVD,
        PRESSURE,
        SWAT,
        SOIL,
        SGAS,
        WRAT,
        ORAT,
        GRAT,
        MD,
        PRESSURE_P10,
        PRESSURE_P50,
        PRESSURE_P90,
        PRESSURE_MEAN,
        PRESSURE_ERROR,
        SEGMENT_VALUES
    };

public:
    RifEclipseRftAddress( QString wellName, QDateTime timeStep, RftWellLogChannelType wellLogChannel );

    void    setResultName( const QString& resultName );
    QString resultName() const;

    void setSegmentId( int id );
    int  segmentId() const;

    const QString&               wellName() const;
    QDateTime                    timeStep() const;
    const RftWellLogChannelType& wellLogChannel() const;

    static std::set<RftWellLogChannelType> rftPlotChannelTypes();
    static std::set<RftWellLogChannelType> pltPlotChannelTypes();

private:
    QString               m_wellName;
    QString               m_resultName;
    QDateTime             m_timeStep;
    RftWellLogChannelType m_wellLogChannel;

    int m_segmentId;
};

bool operator==( const RifEclipseRftAddress& first, const RifEclipseRftAddress& second );

bool operator<( const RifEclipseRftAddress& first, const RifEclipseRftAddress& second );
