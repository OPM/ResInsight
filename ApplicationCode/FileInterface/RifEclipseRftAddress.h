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

#include <string>
#include <QDateTime>

//==================================================================================================
///  
///  
//==================================================================================================
class RifEclipseRftAddress
{
public:
    RifEclipseRftAddress(QString wellName, QDateTime timeStep, QString wellLogChannelName);
    RifEclipseRftAddress(std::string wellName, time_t timeStep, std::string wellLogChannelName);

    const QString&  wellName() const            { return m_wellName; }
    QDateTime           timeStep() const            { return m_timeStep; }
    const QString&  wellLogChannelName() const  { return m_wellLogChannelName; }

private:
    QString m_wellName;
    QDateTime   m_timeStep;
    QString m_wellLogChannelName;
};

bool operator==(const RifEclipseRftAddress& first, const RifEclipseRftAddress& second);

bool operator<(const RifEclipseRftAddress& first, const RifEclipseRftAddress& second);