/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

//#include <memory>
#include <string>
#include <vector>

class QString;

// namespace H5
// {
// class H5File;
// }

//==================================================================================================
//
//
//==================================================================================================
class RifHdf5SummaryReader
{
public:
    explicit RifHdf5SummaryReader( const QString& fileName );
    ~RifHdf5SummaryReader();

    bool isValid() const;

    std::vector<std::string> vectorNames();
    std::vector<time_t>      timeSteps() const;

    std::vector<double> values( const std::string& vectorName, int summaryTypeId ) const;
    std::vector<double> values( const std::string& vectorName ) const;

private:
    time_t startDate() const;

private:
    H5::H5File* m_hdfFile;
};
