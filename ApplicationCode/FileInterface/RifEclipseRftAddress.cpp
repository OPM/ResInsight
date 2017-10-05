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

#include "RifEclipseRftAddress.h"


const QString RifEclipseRftAddress::DEPTH = "Depth";
const QString RifEclipseRftAddress::PRESSURE = "Pressure";
const QString RifEclipseRftAddress::SWAT = "Water Saturation";
const QString RifEclipseRftAddress::SOIL = "Oil Saturation";
const QString RifEclipseRftAddress::SGAS = "Gas Saturation";
const QString RifEclipseRftAddress::WRAT = "Water Flow";
const QString RifEclipseRftAddress::ORAT = "Oil Flow";
const QString RifEclipseRftAddress::GRAT = "Gas Flow";

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifEclipseRftAddress::RifEclipseRftAddress(QString wellName, QDateTime timeStep, QString wellLogChannelName) :
    m_wellName(wellName), m_wellLogChannelName(wellLogChannelName)
{
    timeStep.setTimeSpec(Qt::TimeSpec::UTC);

    m_timeStep.setTimeSpec(Qt::TimeSpec::UTC);
    m_timeStep = timeStep;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QString> RifEclipseRftAddress::allWellLogChannelNames()
{
    std::vector<QString> channelNames;
    channelNames.push_back(RifEclipseRftAddress::DEPTH);
    channelNames.push_back(RifEclipseRftAddress::PRESSURE);
    channelNames.push_back(RifEclipseRftAddress::SWAT);
    channelNames.push_back(RifEclipseRftAddress::SOIL);
    channelNames.push_back(RifEclipseRftAddress::SGAS);
    channelNames.push_back(RifEclipseRftAddress::WRAT);
    channelNames.push_back(RifEclipseRftAddress::ORAT);
    channelNames.push_back(RifEclipseRftAddress::GRAT);

    return channelNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool operator==(const RifEclipseRftAddress& first, const RifEclipseRftAddress& second)
{
    if (first.wellName() != second.wellName()) return false;
    if (first.timeStep() != second.timeStep()) return false;
    if (first.wellLogChannelName() != second.wellLogChannelName()) return false;
    
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool operator<(const RifEclipseRftAddress& first, const RifEclipseRftAddress& second)
{
    if (first.wellName() != second.wellName()) return (first.wellName() < second.wellName());
    if (first.timeStep() != second.timeStep()) return (first.timeStep() < second.timeStep());
    if (first.wellLogChannelName() != second.wellLogChannelName()) return (first.wellLogChannelName() < second.wellLogChannelName());

    return false;
}

