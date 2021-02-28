/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

//==================================================================================================
//
//
//==================================================================================================
class RifOpmCommonEclipseSummary : public RifSummaryReaderInterface
{
public:
    RifOpmCommonEclipseSummary();
    ~RifOpmCommonEclipseSummary();

    void useLodsmaryFiles( bool enable );

    bool open( const QString& headerFileName, bool includeRestartFiles );

    const std::vector<time_t>& timeSteps( const RifEclipseSummaryAddress& resultAddress ) const override;
    bool        values( const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values ) const override;
    std::string unitName( const RifEclipseSummaryAddress& resultAddress ) const override;
    RiaDefines::EclipseUnitSystem unitSystem() const override;

private:
    void buildMetaData();

    static RifEclipseSummaryAddress createAddressFromSummaryNode( const Opm::EclIO::SummaryNode& summaryNode,
                                                                  Opm::EclIO::ESmry*             summaryFile );

private:
    std::unique_ptr<Opm::EclIO::ESmry>         m_eSmry;
    std::vector<std::string>                   m_eSmryKeywords;
    std::map<RifEclipseSummaryAddress, size_t> m_adrToSummaryNodeIndex;
    std::vector<time_t>                        m_timeSteps;

    bool m_useLodsmryFiles;
};
