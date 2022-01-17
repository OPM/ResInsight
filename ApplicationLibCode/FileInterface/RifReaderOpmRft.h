/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022- Equinor ASA
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

#include "RifReaderEclipseRft.h"
#include "RifRftSegment.h"

#include "cvfObject.h"

#include <memory>

namespace Opm
{
namespace EclIO
{
    class ERft;
} // namespace EclIO
} // namespace Opm

class RifReaderOpmRft : public RifReaderRftInterface, public cvf::Object
{
public:
    RifReaderOpmRft( const QString& fileName );

    std::set<RifEclipseRftAddress> eclipseRftAddresses() override;
    void values( const RifEclipseRftAddress& rftAddress, std::vector<double>* values ) override;

    std::set<QDateTime> availableTimeSteps( const QString& wellName ) override;
    std::set<QDateTime> availableTimeSteps( const QString&                                     wellName,
                                            const RifEclipseRftAddress::RftWellLogChannelType& wellLogChannelName ) override;
    std::set<QDateTime>
                                                          availableTimeSteps( const QString&                                               wellName,
                                                                              const std::set<RifEclipseRftAddress::RftWellLogChannelType>& relevantChannels ) override;
    std::set<RifEclipseRftAddress::RftWellLogChannelType> availableWellLogChannels( const QString& wellName ) override;
    std::set<QString>                                     wellNames() override;

    void cellIndices( const RifEclipseRftAddress& rftAddress, std::vector<caf::VecIjk>* indices ) override;

private:
    void buildMetaData();
    void buildSegmentData();
    void segmentDataDebugLog() const;
    bool isOpen() const;

    static RifEclipseRftAddress::RftWellLogChannelType identifyChannelType( const std::string& resultName );
    static std::string resultNameFromChannelType( RifEclipseRftAddress::RftWellLogChannelType channelType );

private:
    std::unique_ptr<Opm::EclIO::ERft> m_opm_rft;

    // RFT and PLT addresses
    std::set<RifEclipseRftAddress> m_addresses;
    std::set<QString>              m_wellNames;

    // Segment data
    // RftDate must be synced with definition in Opm::EclIO::ERft::RftDate
    using RftDate       = std::tuple<int, int, int>;
    using RftSegmentKey = std::pair<std::string, RftDate>;
    std::map<RftSegmentKey, RifRftSegment> m_rftWellDateSegments;
};
