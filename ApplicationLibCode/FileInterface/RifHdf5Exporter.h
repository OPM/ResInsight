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

#include "H5Cpp.h"

#include <string>
#include <vector>

//==================================================================================================
//
//
//==================================================================================================
class RifHdf5Exporter
{
public:
    explicit RifHdf5Exporter( const std::string& fileName );
    ~RifHdf5Exporter();

    H5::Group findOrCreateGroup( H5::Group* parentGroup, const std::string& groupName );

    bool exportSummaryVector( H5::Group&                summaryRootGroup,
                              const std::string&        vectorName,
                              const std::string&        vectorSubNodeName,
                              const std::string&        datasetName,
                              const std::vector<float>& values );

    bool writeDataset( const std::string& groupName, const std::string& datasetName, const std::vector<int>& values );

private:
    bool writeDataset( const H5::Group& group, const std::string& datasetName, const std::vector<int>& values );
    bool writeDataset( const H5::Group& group, const std::string& datasetName, const std::vector<float>& values );

private:
    std::string m_fileName;
    H5::H5File* m_hdfFile;
};
