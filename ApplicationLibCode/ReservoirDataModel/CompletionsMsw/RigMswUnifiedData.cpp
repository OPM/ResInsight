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

#include "RigMswUnifiedData.h"

#include <algorithm>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMswUnifiedData::addWellData( RigMswTableData wellData )
{
    // Check if well name already exists
    const std::string& wellName = wellData.wellName();
    auto               it       = std::find_if( m_wellDataList.begin(),
                            m_wellDataList.end(),
                            [&wellName]( const RigMswTableData& data ) { return data.wellName() == wellName; } );

    if ( it != m_wellDataList.end() )
    {
        // Replace existing data
        *it = std::move( wellData );
    }
    else
    {
        // Add new data
        m_wellDataList.push_back( std::move( wellData ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigMswUnifiedData::clear()
{
    m_wellDataList.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<CompsegsRow> RigMswUnifiedData::getAllCompsegsRows( bool lgrOnly ) const
{
    std::vector<CompsegsRow> allRows;

    for ( const auto& wellData : m_wellDataList )
    {
        std::vector<CompsegsRow> wellRows;

        if ( lgrOnly )
        {
            wellRows = wellData.lgrCompsegsData();
        }
        else
        {
            wellRows = wellData.mainGridCompsegsData();
        }

        allRows.insert( allRows.end(), wellRows.begin(), wellRows.end() );
    }

    return allRows;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<WsegvalvRow> RigMswUnifiedData::getAllWsegvalvRows() const
{
    std::vector<WsegvalvRow> allRows;

    for ( const auto& wellData : m_wellDataList )
    {
        const auto& wellRows = wellData.wsegvalvData();
        allRows.insert( allRows.end(), wellRows.begin(), wellRows.end() );
    }

    return allRows;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<WsegaicdRow> RigMswUnifiedData::getAllWsegaicdRows() const
{
    std::vector<WsegaicdRow> allRows;

    for ( const auto& wellData : m_wellDataList )
    {
        const auto& wellRows = wellData.wsegaicdData();
        allRows.insert( allRows.end(), wellRows.begin(), wellRows.end() );
    }

    return allRows;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigMswUnifiedData::hasAnyLgrData() const
{
    return std::any_of( m_wellDataList.begin(), m_wellDataList.end(), []( const RigMswTableData& wellData ) { return wellData.hasLgrData(); } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RigMswUnifiedData::wellNames() const
{
    std::vector<std::string> names;
    names.reserve( m_wellDataList.size() );

    std::transform( m_wellDataList.begin(),
                    m_wellDataList.end(),
                    std::back_inserter( names ),
                    []( const RigMswTableData& wellData ) { return wellData.wellName(); } );

    return names;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigMswUnifiedData::isValid() const
{
    return validationErrors().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RigMswUnifiedData::validationErrors() const
{
    std::vector<QString> errors;

    if ( m_wellDataList.empty() )
    {
        errors.push_back( "No well data available" );
        return errors;
    }

    // Check each well's data validity
    for ( const auto& wellData : m_wellDataList )
    {
        if ( !wellData.isValid() )
        {
            auto wellErrors = wellData.validationErrors();
            for ( const auto& error : wellErrors )
            {
                errors.push_back( QString( "%1: %2" ).arg( QString::fromStdString( wellData.wellName() ), error ) );
            }
        }
    }

    // Check for duplicate well names
    std::vector<std::string> names = wellNames();
    std::sort( names.begin(), names.end() );
    auto it = std::adjacent_find( names.begin(), names.end() );
    if ( it != names.end() )
    {
        errors.push_back( QString( "Duplicate well name: %1" ).arg( QString::fromStdString( *it ) ) );
    }

    return errors;
}
