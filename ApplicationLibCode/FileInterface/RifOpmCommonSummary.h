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

#include "RifEnsembleImportConfig.h"
#include "RifSummaryReaderInterface.h"

#include <QString>
#include <QStringList>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace Opm
{
namespace EclIO
{
    class ESmry;
    class ExtESmry;
} // namespace EclIO
} // namespace Opm

class RiaThreadSafeLogger;
class RifEclipseSummaryAddress;

//==================================================================================================
//
//
//==================================================================================================
class RifOpmCommonEclipseSummary : public RifSummaryReaderInterface
{
public:
    RifOpmCommonEclipseSummary();
    ~RifOpmCommonEclipseSummary() override;

    void useEnhancedSummaryFiles( bool enable );
    void createEnhancedSummaryFiles( bool enable );

    void setEnsembleImportState( RifEnsembleImportConfig ensembleImportState );

    static void   resetEnhancedSummaryFileCount();
    static size_t numberOfEnhancedSummaryFileCreated();

    bool open( const QString& fileName, bool includeRestartFiles, RiaThreadSafeLogger* threadSafeLogger );

    std::vector<time_t>                  timeSteps( const RifEclipseSummaryAddress& resultAddress ) const override;
    std::pair<bool, std::vector<double>> values( const RifEclipseSummaryAddress& resultAddress ) const override;
    std::string                          unitName( const RifEclipseSummaryAddress& resultAddress ) const override;
    RiaDefines::EclipseUnitSystem        unitSystem() const override;

private:
    size_t keywordCount() const override;
    void   createAndSetAddresses() override;
    bool   openFileReader( const QString& fileName, bool includeRestartFiles, bool importEsmryFile, RiaThreadSafeLogger* threadSafeLogger );
    void   populateTimeSteps();
    std::string keywordForAddress( const RifEclipseSummaryAddress& address ) const;

    static bool writeEsmryFile( QString& smspecFileName, bool includeRestartFiles, RiaThreadSafeLogger* threadSafeLogger );
    static void increaseEsmryFileCount();

private:
    std::unique_ptr<Opm::EclIO::ESmry>    m_standardReader;
    std::unique_ptr<Opm::EclIO::ExtESmry> m_enhancedReader;

    std::map<RifEclipseSummaryAddress, std::string> m_summaryAddressToKeywordMap;
    std::vector<time_t>                             m_timeSteps;

    static size_t sm_createdEsmryFileCount;

    bool m_useEsmryFiles;
    bool m_createEsmryFiles;

    RifEnsembleImportConfig m_ensembleImportState;
};
