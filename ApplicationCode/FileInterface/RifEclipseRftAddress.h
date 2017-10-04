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

//==================================================================================================
///  
///  
//==================================================================================================
class RifEclipseRftAddress
{
public:
    RifEclipseRftAddress(std::string wellName, time_t timeStep, std::string wellLogChannelName) :
        m_wellName(wellName), m_timeStep(timeStep), m_wellLogChannelName(wellLogChannelName) {}

    const std::string&  wellName() const            { return m_wellName; }
    time_t              timeStep() const            { return m_timeStep; }
    const std::string&  wellLogChannelName() const  { return m_wellLogChannelName; }

private:
    std::string m_wellName;
    time_t m_timeStep;
    std::string m_wellLogChannelName;
};

bool operator==(const RifEclipseRftAddress& first, const RifEclipseRftAddress& second);

bool operator<(const RifEclipseRftAddress& first, const RifEclipseRftAddress& second);