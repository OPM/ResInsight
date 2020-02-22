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
    enum RftWellLogChannelType
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
        PRESSURE_ERROR
    };

public:
    RifEclipseRftAddress( QString wellName, QDateTime timeStep, RftWellLogChannelType wellLogChannel );

    const QString&               wellName() const { return m_wellName; }
    QDateTime                    timeStep() const { return m_timeStep; }
    const RftWellLogChannelType& wellLogChannel() const { return m_wellLogChannel; }

    static std::set<RftWellLogChannelType> rftPlotChannelTypes()
    {
        return {RifEclipseRftAddress::PRESSURE,
                RifEclipseRftAddress::PRESSURE_ERROR,
                RifEclipseRftAddress::PRESSURE_MEAN,
                RifEclipseRftAddress::PRESSURE_P10,
                RifEclipseRftAddress::PRESSURE_P50,
                RifEclipseRftAddress::PRESSURE_P90};
    }

    static std::set<RftWellLogChannelType> pltPlotChannelTypes()
    {
        return {RifEclipseRftAddress::ORAT, RifEclipseRftAddress::WRAT, RifEclipseRftAddress::GRAT};
    }

private:
    QString               m_wellName;
    QDateTime             m_timeStep;
    RftWellLogChannelType m_wellLogChannel;
};

bool operator==( const RifEclipseRftAddress& first, const RifEclipseRftAddress& second );

bool operator<( const RifEclipseRftAddress& first, const RifEclipseRftAddress& second );
