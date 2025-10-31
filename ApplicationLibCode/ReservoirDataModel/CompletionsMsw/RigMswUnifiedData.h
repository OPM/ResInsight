/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "CompletionsMsw/RigMswTableData.h"

#include <map>
#include <vector>

//==================================================================================================
/// Container class for unified MSW table data from multiple wells
//==================================================================================================
class RigMswUnifiedDataWIP
{
public:
    RigMswUnifiedDataWIP() = default;

    // Data modification
    void addWellData( RigMswTableData wellData );
    void clear();

    // Data access
    const std::vector<RigMswTableData>& wellDataList() const { return m_wellDataList; }
    const RigMswTableData*              getWellData( const std::string& wellName ) const;

    // Aggregated data access - combines data from all wells
    std::vector<WelsegsHeader> getAllWelsegsHeaders() const;
    std::vector<WelsegsRow>    getAllWelsegsRows() const;
    std::vector<CompsegsRow>   getAllCompsegsRows( bool lgrOnly = false ) const;
    std::vector<WsegvalvRow>   getAllWsegvalvRows() const;
    std::vector<WsegaicdRow>   getAllWsegaicdRows() const;

    // Metadata and analysis
    bool                     isEmpty() const { return m_wellDataList.empty(); }
    bool                     hasAnyLgrData() const;
    size_t                   wellCount() const { return m_wellDataList.size(); }
    std::vector<std::string> wellNames() const;

    // Data validation
    bool                 isValid() const;
    std::vector<QString> validationErrors() const;

    // Iterator support for range-based loops
    auto begin() const { return m_wellDataList.begin(); }
    auto end() const { return m_wellDataList.end(); }
    auto begin() { return m_wellDataList.begin(); }
    auto end() { return m_wellDataList.end(); }

private:
    std::vector<RigMswTableData>  m_wellDataList;
    std::map<std::string, size_t> m_wellNameToIndex;

    void updateIndex();
};
