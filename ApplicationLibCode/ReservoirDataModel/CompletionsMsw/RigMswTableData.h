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

#include "RiaDefines.h"
#include "RigMswTableRows.h"

#include <vector>

//==================================================================================================
/// Container class for MSW table data for a single well
//==================================================================================================
class RigMswTableData
{
public:
    RigMswTableData( const std::string& wellName, RiaDefines::EclipseUnitSystem unitSystem );

    // Data access
    const WelsegsHeader&            welsegsHeader() const { return m_welsegsHeader; }
    const std::vector<WelsegsRow>&  welsegsData() const { return m_welsegsData; }
    const std::vector<CompsegsRow>& compsegsData() const { return m_compsegsData; }
    const std::vector<WsegvalvRow>& wsegvalvData() const { return m_wsegvalvData; }
    const std::vector<WsegaicdRow>& wsegaicdData() const { return m_wsegaicdData; }
    const std::vector<WsegsicdRow>& wsegsicdData() const { return m_wsegsicdData; }

    // Data modification
    void setWelsegsHeader( const WelsegsHeader& header );
    void addWelsegsRow( const WelsegsRow& row );
    void addCompsegsRow( const CompsegsRow& row );
    void addWsegvalvRow( const WsegvalvRow& row );
    void addWsegaicdRow( const WsegaicdRow& row );
    void addWsegsicdRow( const WsegsicdRow& row );

    // Metadata
    std::string                   wellName() const { return m_wellName; }
    RiaDefines::EclipseUnitSystem unitSystem() const { return m_unitSystem; }

    // Data analysis
    bool hasLgrData() const;
    bool isEmpty() const;
    bool hasWelsegsData() const { return !m_welsegsData.empty(); }
    bool hasCompsegsData() const { return !m_compsegsData.empty(); }
    bool hasWsegvalvData() const { return !m_wsegvalvData.empty(); }
    bool hasWsegaicdData() const { return !m_wsegaicdData.empty(); }
    bool hasWsegsicdData() const { return !m_wsegsicdData.empty(); }

    // Filtering for LGR data
    std::vector<CompsegsRow> mainGridCompsegsData() const;
    std::vector<CompsegsRow> lgrCompsegsData() const;

    // Data validation
    bool                 isValid() const;
    std::vector<QString> validationErrors() const;

private:
    std::string                   m_wellName;
    RiaDefines::EclipseUnitSystem m_unitSystem;

    WelsegsHeader            m_welsegsHeader;
    std::vector<WelsegsRow>  m_welsegsData;
    std::vector<CompsegsRow> m_compsegsData;
    std::vector<WsegvalvRow> m_wsegvalvData;
    std::vector<WsegaicdRow> m_wsegaicdData;
    std::vector<WsegsicdRow> m_wsegsicdData;
};
