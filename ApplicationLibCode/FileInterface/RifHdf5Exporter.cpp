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

#include "RifHdf5Exporter.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifHdf5Exporter::RifHdf5Exporter( const std::string& fileName )
    : m_fileName( fileName )
{
    try
    {
        // Create new or overwrite existing
        m_hdfFile = new H5::H5File( m_fileName, H5F_ACC_TRUNC );
    }
    catch ( ... )
    {
        delete m_hdfFile;
        m_hdfFile = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifHdf5Exporter::~RifHdf5Exporter()
{
    if ( m_hdfFile )
    {
        delete m_hdfFile;
        m_hdfFile = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifHdf5Exporter::exportSummaryVector( H5::Group&                summaryRootGroup,
                                           const std::string&        vectorName,
                                           const std::string&        vectorSubNodeName,
                                           const std::string&        datasetName,
                                           const std::vector<float>& values )
{
    auto summaryGroup = findOrCreateGroup( &summaryRootGroup, vectorName );

    auto subNodeGroup = summaryGroup.createGroup( vectorSubNodeName );

    return writeDataset( subNodeGroup, datasetName, values );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifHdf5Exporter::writeDataset( const H5::Group& group, const std::string& datasetName, const std::vector<float>& values )
{
    try
    {
        hsize_t dimsf[1];
        dimsf[0] = values.size();
        H5::DataSpace dataspace( 1, dimsf );

        H5::DataType datatype( H5::PredType::NATIVE_FLOAT );
        H5::DataSet  dataset = group.createDataSet( datasetName, datatype, dataspace );
        dataset.write( values.data(), H5::PredType::NATIVE_FLOAT );

        return true;
    }
    catch ( ... )
    {
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifHdf5Exporter::writeDataset( const H5::Group& group, const std::string& datasetName, const std::vector<int>& values )
{
    try
    {
        hsize_t dimsf[1];
        dimsf[0] = values.size();
        H5::DataSpace dataspace( 1, dimsf );

        H5::DataType datatype( H5::PredType::NATIVE_INT );
        H5::DataSet  dataset = group.createDataSet( datasetName, datatype, dataspace );
        dataset.write( values.data(), H5::PredType::NATIVE_INT );

        return true;
    }
    catch ( ... )
    {
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifHdf5Exporter::writeDataset( const std::string& groupName, const std::string& datasetName, const std::vector<int>& values )
{
    if ( !m_hdfFile ) return false;

    {
        auto generalGroup = findOrCreateGroup( nullptr, groupName );

        return writeDataset( generalGroup, datasetName, values );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
H5::Group RifHdf5Exporter::findOrCreateGroup( H5::Group* parentGroup, const std::string& groupName )
{
    if ( parentGroup )
    {
        try
        {
            auto group = parentGroup->openGroup( groupName );

            return group;
        }
        catch ( ... )
        {
            return parentGroup->createGroup( groupName );
        }
    }
    else
    {
        try
        {
            auto group = m_hdfFile->openGroup( groupName );
            return group;
        }
        catch ( ... )
        {
            return m_hdfFile->createGroup( groupName );
        }
    }

    return {};
}
