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

#include "cvfObject.h"

#include "opm/io/eclipse/EclIOdata.hpp"

#include <memory>
#include <unordered_set>

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
    class RftSegmentData
    {
    public:
        RftSegmentData( int segnxt, int brno, int brnst, int brnen, int segNo )
            : m_segNext( segnxt )
            , m_segbrno( brno )
            , m_brnst( brnst )
            , m_brnen( brnen )
            , m_segmentNo( segNo )
        {
        }

        int segNext() const { return m_segNext; }
        int segBrno() const { return m_segbrno; }
        int segBrnst() const { return m_brnst; }
        int segBrnen() const { return m_brnen; }
        int segNo() const { return m_segmentNo; }

    private:
        int m_segNext;
        int m_segbrno;
        int m_brnst;
        int m_brnen;
        int m_segmentNo;
    };

    using EclEntry = std::tuple<std::string, Opm::EclIO::eclArrType, int64_t>;

    class RftSegment
    {
    public:
        void setSegmentData( std::vector<RftSegmentData> segmentData ) { m_topology = segmentData; }
        std::vector<RftSegmentData> topology() const { return m_topology; }

        void addResultNameAndSize( const EclEntry& resultNameAndSize )
        {
            m_resultNameAndSize.push_back( resultNameAndSize );
        }
        std::vector<EclEntry> resultNameAndSize() const { return m_resultNameAndSize; }

        std::vector<int> branchIds() const
        {
            std::unordered_set<int> s;
            for ( const auto& segData : m_topology )
            {
                s.insert( segData.segBrno() );
            }

            std::vector<int> v;
            v.assign( s.begin(), s.end() );
            sort( v.begin(), v.end() );

            return v;
        }

    private:
        std::vector<RftSegmentData> m_topology;
        std::vector<EclEntry>       m_resultNameAndSize;
    };

    using RftDate       = std::tuple<int, int, int>;
    using RftSegmentKey = std::pair<std::string, RftDate>;

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
    std::map<RftSegmentKey, RftSegment> m_rftWellDateSegments2;
};
