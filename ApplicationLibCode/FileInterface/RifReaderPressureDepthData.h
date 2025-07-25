/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-  Equinor ASA
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
#include "RifReaderRftInterface.h"

#include "RigPressureDepthData.h"

#include "cvfObject.h"

#include <map>
#include <string>
#include <vector>

#include <QDateTime>
#include <QDir>

//==================================================================================================
//
//
//==================================================================================================
class RifReaderPressureDepthData : public RifReaderRftInterface
{
public:
    RifReaderPressureDepthData( const QString& filePath );
    ~RifReaderPressureDepthData() override = default;

    std::vector<QString> labels( const RifEclipseRftAddress& rftAddress );

    std::set<RifEclipseRftAddress> eclipseRftAddresses() override;
    void                           values( const RifEclipseRftAddress& rftAddress, std::vector<double>* values ) override;

    std::set<QDateTime> availableTimeSteps( const QString&                                               wellName,
                                            const std::set<RifEclipseRftAddress::RftWellLogChannelType>& relevantChannels ) override;
    std::set<QDateTime> availableTimeSteps( const QString& wellName ) override;
    std::set<QDateTime> availableTimeSteps( const QString&                                     wellName,
                                            const RifEclipseRftAddress::RftWellLogChannelType& wellLogChannelName ) override;

    std::set<RifEclipseRftAddress::RftWellLogChannelType> availableWellLogChannels( const QString& wellName ) override;
    std::set<QString>                                     wellNames() override;

    void load();

private:
    QString                           m_filePath;
    std::vector<RigPressureDepthData> m_pressureDepthDataItems;
};
