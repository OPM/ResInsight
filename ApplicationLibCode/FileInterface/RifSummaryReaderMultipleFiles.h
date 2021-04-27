////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RifSummaryReaderInterface.h"

#include <memory>
#include <string>
#include <vector>

//==================================================================================================
//
//
//==================================================================================================
class RifSummaryReaderMultipleFiles : public RifSummaryReaderInterface
{
public:
    RifSummaryReaderMultipleFiles( const std::vector<std::string>& smspecFilesOrderedByNewestFirst );

    bool createReadersAndImportMetaData();

    const std::vector<time_t>& timeSteps( const RifEclipseSummaryAddress& resultAddress ) const override;
    bool        values( const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values ) const override;
    std::string unitName( const RifEclipseSummaryAddress& resultAddress ) const override;
    RiaDefines::EclipseUnitSystem unitSystem() const override;

private:
    std::vector<std::string>                                m_fileNames;
    std::vector<std::unique_ptr<RifSummaryReaderInterface>> m_summaryReaders;

    std::vector<time_t> m_aggregatedTimeSteps;
};
