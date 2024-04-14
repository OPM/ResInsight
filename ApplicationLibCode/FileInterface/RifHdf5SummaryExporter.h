/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include <string>
#include <vector>

class RifSummaryReaderInterface;
class RifHdf5Exporter;

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
class RifHdf5SummaryExporter
{
public:
    static bool ensureHdf5FileIsCreatedMultithreaded( const std::vector<std::string>& smspecFileNames,
                                                      const std::vector<std::string>& h5FileNames,
                                                      bool                            createHdfIfNotPresent,
                                                      int                             threadCount );

    static bool ensureHdf5FileIsCreated( const std::string& smspecFileName,
                                         const std::string& h5FileName,
                                         bool               createHdfIfNotPresent,
                                         size_t&            hdfFilesCreatedCount );

private:
    static bool writeGeneralSection( RifHdf5Exporter& exporter, Opm::EclIO::ESmry& sourceSummaryData );
    static bool writeSummaryVectors( RifHdf5Exporter& exporter, Opm::EclIO::ESmry& sourceSummaryData );
};
