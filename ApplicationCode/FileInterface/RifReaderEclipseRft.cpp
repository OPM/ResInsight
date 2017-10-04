/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include "RifReaderEclipseRft.h"

#include "RifEclipseRftAddress.h"

#include "ert/ecl/ecl_rft_file.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderEclipseRft::RifReaderEclipseRft(const std::string& fileName):
    m_fileName(fileName), m_ecl_rft_file(nullptr)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifReaderEclipseRft::~RifReaderEclipseRft()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseRft::open()
{
    if (m_fileName.empty()) return;

    m_ecl_rft_file = ecl_rft_file_alloc_case(m_fileName.data());

    if (m_ecl_rft_file == NULL) return;

    int fileSize = ecl_rft_file_get_size(m_ecl_rft_file);

    for (int i = 0; i < fileSize; i++)
    {
        ecl_rft_node_type* node = ecl_rft_file_iget_node(m_ecl_rft_file, i);
        
        std::string wellName = ecl_rft_node_get_well_name(node);
        time_t timeStep = ecl_rft_node_get_date(node);
        
        std::string wellLogChannelName = "PRESSURE";
        RifEclipseRftAddress addressPressure(wellName, timeStep, wellLogChannelName);
        m_eclipseRftAddresses.push_back(addressPressure);
        m_rftAddressToLibeclNodeIdx[addressPressure] = i;

        if (ecl_rft_node_is_RFT(node))
        {
            wellLogChannelName = "SWAT";
            RifEclipseRftAddress addressSwat(wellName, timeStep, wellLogChannelName);
            m_eclipseRftAddresses.push_back(addressSwat);
            m_rftAddressToLibeclNodeIdx[addressSwat] = i;

            wellLogChannelName = "SOIL";
            RifEclipseRftAddress addressSoil(wellName, timeStep, wellLogChannelName);
            m_eclipseRftAddresses.push_back(addressSoil);
            m_rftAddressToLibeclNodeIdx[addressSoil] = i;

            wellLogChannelName = "SGAS";
            RifEclipseRftAddress addressSgas(wellName, timeStep, wellLogChannelName);
            m_eclipseRftAddresses.push_back(addressSgas);
            m_rftAddressToLibeclNodeIdx[addressSgas] = i;
        }
        else if (ecl_rft_node_is_PLT(node))
        {
            wellLogChannelName = "WRAT";
            RifEclipseRftAddress addressWrat(wellName, timeStep, wellLogChannelName);
            m_eclipseRftAddresses.push_back(addressWrat);
            m_rftAddressToLibeclNodeIdx[addressWrat] = i;

            wellLogChannelName = "GRAT";
            RifEclipseRftAddress addressGrat(wellName, timeStep, wellLogChannelName);
            m_eclipseRftAddresses.push_back(addressGrat);
            m_rftAddressToLibeclNodeIdx[addressGrat] = i;

            wellLogChannelName = "ORAT";
            RifEclipseRftAddress addressOrat(wellName, timeStep, wellLogChannelName);
            m_eclipseRftAddresses.push_back(addressOrat);
            m_rftAddressToLibeclNodeIdx[addressOrat] = i;
        }
    }

    return;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<RifEclipseRftAddress>& RifReaderEclipseRft::eclipseRftAddresses()
{
    if (!m_ecl_rft_file)
    {
        open();
    }

    return m_eclipseRftAddresses;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseRft::values(const RifEclipseRftAddress& rftAddress, std::vector<double>* values)
{
    if (!m_ecl_rft_file)
    {
        open();
    }

    int index = indexFromAddress(rftAddress);
    if (index < 0) return;

    ecl_rft_node_type* node = ecl_rft_file_iget_node(m_ecl_rft_file, index);
    
    std::string wellLogChannelName = rftAddress.wellLogChannelName();

    if (wellLogChannelName == "PRESSURE")
    {
        for (int i = 0; i < ecl_rft_node_get_size(node); i++)
        {
            values->push_back(ecl_rft_node_iget_pressure(node, i));
        }
    }
    else if (wellLogChannelName == "SWAT")
    {
        for (int i = 0; i < ecl_rft_node_get_size(node); i++)
        {
            values->push_back(ecl_rft_node_iget_swat(node, i));
        }
    }
    else if (wellLogChannelName == "SOIL")
    {
        for (int i = 0; i < ecl_rft_node_get_size(node); i++)
        {
            values->push_back(ecl_rft_node_iget_soil(node, i));
        }
    }
    else if (wellLogChannelName == "SGAS")
    {
        for (int i = 0; i < ecl_rft_node_get_size(node); i++)
        {
            values->push_back(ecl_rft_node_iget_sgas(node, i));
        }
    }
    else if (wellLogChannelName == "WRAT")
    {
        for (int i = 0; i < ecl_rft_node_get_size(node); i++)
        {
            values->push_back(ecl_rft_node_iget_wrat(node, i));
        }
    }
    else if (wellLogChannelName == "GRAT")
    {
        for (int i = 0; i < ecl_rft_node_get_size(node); i++)
        {
            values->push_back(ecl_rft_node_iget_grat(node, i));
        }
    }
    else if (wellLogChannelName == "ORAT")
    {
        for (int i = 0; i < ecl_rft_node_get_size(node); i++)
        {
            values->push_back(ecl_rft_node_iget_orat(node, i));
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RifReaderEclipseRft::indexFromAddress(const RifEclipseRftAddress& rftAddress) const
{
    auto it = m_rftAddressToLibeclNodeIdx.find(rftAddress);

    if (it != m_rftAddressToLibeclNodeIdx.end())
    {
        return it->second;
    }

    return -1;
}

