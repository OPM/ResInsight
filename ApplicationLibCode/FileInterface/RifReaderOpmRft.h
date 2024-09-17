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
    RifReaderOpmRft( const QString& fileName, const QString& dataDeckFileName );

    std::set<RifEclipseRftAddress> eclipseRftAddresses() override;
    void                           values( const RifEclipseRftAddress& rftAddress, std::vector<double>* values ) override;

    std::set<QDateTime>                                   availableTimeSteps( const QString& wellName ) override;
    std::set<QDateTime>                                   availableTimeSteps( const QString&                                     wellName,
                                                                              const RifEclipseRftAddress::RftWellLogChannelType& wellLogChannelName ) override;
    std::set<QDateTime>                                   availableTimeSteps( const QString&                                               wellName,
                                                                              const std::set<RifEclipseRftAddress::RftWellLogChannelType>& relevantChannels ) override;
    std::set<RifEclipseRftAddress::RftWellLogChannelType> availableWellLogChannels( const QString& wellName ) override;
    std::set<QString>                                     wellNames() override;

    std::vector<caf::VecIjk> cellIndices( const QString& wellName, const QDateTime& timeStep ) override;

    std::map<int, int> branchIdsAndOneBasedIndices( const QString& wellName, const QDateTime& timeStep, RiaDefines::RftBranchType branchType );

    RifRftSegment segmentForWell( const QString& wellName, const QDateTime& timeStep );

private:
    // Segment data
    // RftDate must be synced with definition in Opm::EclIO::ERft::RftDate
    using RftDate       = std::tuple<int, int, int>;
    using RftSegmentKey = std::pair<std::string, RftDate>;

    void openFiles( const QString& fileName, const QString& dataDeckFileName );
    void buildMetaData();
    void buildSegmentData();
    void segmentDataDebugLog() const;
    bool isOpen() const;
    void importWellNames();
    void buildSegmentBranchTypes( const RftSegmentKey& segmentKey );
    void identifyTubingCandidateBranches( RifRftSegment&             segmentRef,
                                          const std::string&         wellName,
                                          const std::vector<double>& seglenstValues,
                                          const std::vector<double>& seglenenValues );
    void identifyAnnulusBranches( RifRftSegment& segmentRef, const std::vector<double>& seglenstValues );
    void reassignBranchIndices( RifRftSegment& segmentRef );
    void identifyDeviceBranches( RifRftSegment& segmentRef, const std::vector<double>& seglenstValues );

    std::vector<int> importWellData( const std::string& wellName, const std::string& propertyName, const RftDate& date ) const;

    void                             readWseglink( const std::string& filePath );
    std::vector<std::pair<int, int>> annulusLinksForWell( const std::string& wellName ) const;
    std::vector<int>                 annulusSegmentsForWell( const std::string& wellName ) const;

    static RifEclipseRftAddress::RftWellLogChannelType identifyChannelType( const std::string& resultName );
    static std::string                                 resultNameFromChannelType( RifEclipseRftAddress::RftWellLogChannelType channelType );

    std::vector<float> resultAsFloat( const std::string& resultName, const std::string& wellName, int year, int month, int day ) const;

    bool openFiles();

    using SegmentConnectionStartEnd = std::tuple<float, float, bool>;
    std::vector<SegmentConnectionStartEnd> segmentConnectionStartEndMeasuredDepth( const RifEclipseRftAddress& rftAddress );

    std::vector<float> segmentConnectionValues( const RifEclipseRftAddress& rftAddress,
                                                const RifRftSegment&        rftSegment,
                                                const std::vector<float>&   nativeValues );

private:
    std::unique_ptr<Opm::EclIO::ERft> m_opm_rft;

    // RFT and PLT addresses
    std::set<RifEclipseRftAddress> m_addresses;
    std::set<QString>              m_wellNames;

    std::map<RftSegmentKey, RifRftSegment> m_rftWellDateSegments;
    std::set<QDateTime>                    m_rftSegmentTimeSteps;

    std::map<std::string, size_t> m_segmentResultItemCount;
    std::map<std::string, size_t> m_connectionResultItemCount;

    std::map<std::string, std::vector<std::pair<int, int>>> m_wseglink;

    QString m_fileName;
    QString m_dataDeckFileName;
    bool    m_detectedErrorWhenOpeningRftFile;
};
