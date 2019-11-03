/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RifEclipseRftAddress.h"

#include <QDateTime>
#include <QString>

#include <set>
#include <vector>

class RifReaderRftInterface
{
public:
    std::set<RifEclipseRftAddress>         eclipseRftAddresses( const QString& wellName, const QDateTime& timeStep );
    virtual std::set<RifEclipseRftAddress> eclipseRftAddresses()                               = 0;
    virtual void values( const RifEclipseRftAddress& rftAddress, std::vector<double>* values ) = 0;

    virtual std::set<QDateTime>
                                availableTimeSteps( const QString&                                               wellName,
                                                    const std::set<RifEclipseRftAddress::RftWellLogChannelType>& relevantChannels ) = 0;
    virtual std::set<QDateTime> availableTimeSteps( const QString& wellName ) = 0;
    virtual std::set<QDateTime>
                                                                  availableTimeSteps( const QString&                                     wellName,
                                                                                      const RifEclipseRftAddress::RftWellLogChannelType& wellLogChannelName ) = 0;
    virtual std::set<RifEclipseRftAddress::RftWellLogChannelType> availableWellLogChannels( const QString& wellName ) = 0;
    virtual std::set<QString>                                     wellNames() = 0;
};
