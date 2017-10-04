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

#pragma once

#include <map>
#include <string>
#include <vector>

class RifEclipseRftAddress;

//==================================================================================================
//
//
//==================================================================================================
class RifReaderEclipseRft
{
public:
    RifReaderEclipseRft();
    ~RifReaderEclipseRft();

    bool open(const std::string& fileName);

    const std::vector<RifEclipseRftAddress>& eclipseRftAddresses() const;
    void values(const RifEclipseRftAddress& rftAddress, std::vector<double>* values) const;
    
    /*const std::vector<std::string>& wellNames() const;
    std::vector<time_t> timeSteps();*/
    //void values(const std::string& wellName, size_t timeStepIndex, std::vector<double>* values) const;

private:
    int indexFromAddress(const RifEclipseRftAddress& rftAddress) const;

private:

    // Taken from ecl_rft_file.h and ecl_rft_node.h
    typedef struct ecl_rft_file_struct ecl_rft_file_type;
    typedef struct ecl_rft_node_struct ecl_rft_node_type;

    ecl_rft_file_type*                  m_ecl_rft_file;
    std::vector<RifEclipseRftAddress>   m_eclipseRftAddresses;
    std::map<RifEclipseRftAddress, int> m_rftAddressToLibeclNodeIdx;

};

