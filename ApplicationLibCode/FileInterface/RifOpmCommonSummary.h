/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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

#include "RiaDefines.h"

#include "RifEclipseSummaryAddress.h"
#include "RifSummaryReaderInterface.h"

#include <QString>
#include <QStringList>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace Opm
{
namespace EclIO
{
    class ESmry;
    struct SummaryNode;
} // namespace EclIO
} // namespace Opm

class RiaThreadSafeLogger;

class RifOpmCommonSummaryTools
{
public:
    static RifEclipseSummaryAddress createAddressFromSummaryNode( const Opm::EclIO::SummaryNode& summaryNode,
                                                                  const Opm::EclIO::ESmry*       summaryFile );

    static std::pair<std::set<RifEclipseSummaryAddress>, std::map<RifEclipseSummaryAddress, size_t>>
        buildMetaData( const Opm::EclIO::ESmry* summaryFile );
};

//==================================================================================================
//
//
//==================================================================================================
class RifOpmCommonEclipseSummary : public RifSummaryReaderInterface
{
public:
    RifOpmCommonEclipseSummary();
    ~RifOpmCommonEclipseSummary() override;

    void useLodsmaryFiles( bool enable );
    void createLodsmaryFiles( bool enable );

    static void   resetLodCount();
    static size_t numberOfLodFilesCreated();

    bool open( const QString& headerFileName, bool includeRestartFiles, RiaThreadSafeLogger* threadSafeLogger );

    const std::vector<time_t>& timeSteps( const RifEclipseSummaryAddress& resultAddress ) const override;
    bool        values( const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values ) const override;
    std::string unitName( const RifEclipseSummaryAddress& resultAddress ) const override;
    RiaDefines::EclipseUnitSystem unitSystem() const override;

private:
    void buildMetaData();
    bool openESmryFile( const QString& headerFileName, bool includeRestartFiles, RiaThreadSafeLogger* threadSafeLogger );

    static void increaseLodFileCount();

private:
    std::unique_ptr<Opm::EclIO::ESmry>         m_eSmry;
    std::vector<std::string>                   m_eSmryKeywords;
    std::map<RifEclipseSummaryAddress, size_t> m_adrToSummaryNodeIndex;
    std::vector<time_t>                        m_timeSteps;

    static size_t sm_createdLodFileCount;

    bool m_useLodsmryFiles;
    bool m_createLodsmryFiles;
};
