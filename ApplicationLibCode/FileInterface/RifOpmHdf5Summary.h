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

class RifHdf5SummaryReader;
class RiaThreadSafeLogger;

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
class RifOpmHdf5Summary : public RifSummaryReaderInterface
{
public:
    RifOpmHdf5Summary();
    ~RifOpmHdf5Summary() override;

    bool open( const QString& headerFileName, bool includeRestartFiles, RiaThreadSafeLogger* threadSafeLogger );

    std::vector<time_t>                  timeSteps( const RifEclipseSummaryAddress& resultAddress ) const override;
    std::pair<bool, std::vector<double>> values( const RifEclipseSummaryAddress& resultAddress ) const override;
    std::string                          unitName( const RifEclipseSummaryAddress& resultAddress ) const override;
    RiaDefines::EclipseUnitSystem        unitSystem() const override;

private:
    void buildMetaData() override;
    bool openESmryFile( const QString& headerFileName, bool includeRestartFiles, RiaThreadSafeLogger* threadSafeLogger );

private:
    std::unique_ptr<Opm::EclIO::ESmry>              m_eSmry;
    std::map<RifEclipseSummaryAddress, std::string> m_summaryAddressToKeywordMap;
    std::map<RifEclipseSummaryAddress, size_t>      m_adrToSmspecIndices;
    std::vector<time_t>                             m_timeSteps;

    std::unique_ptr<RifHdf5SummaryReader> m_hdf5Reader;
};
