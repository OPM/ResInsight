/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RifByteArrayArrowRandomAccessFile.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifByteArrayArrowRandomAccessFile::RifByteArrayArrowRandomAccessFile( const QByteArray& data )
    : m_data( data )
    , m_position( 0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
arrow::Result<int64_t> RifByteArrayArrowRandomAccessFile::ReadAt( int64_t position, int64_t nbytes, void* out )
{
    if ( nbytes > 0 )
    {
        memcpy( out, m_data.data() + position, static_cast<size_t>( nbytes ) );
        m_position += nbytes;
    }
    return nbytes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
arrow::Result<std::shared_ptr<arrow::Buffer>> RifByteArrayArrowRandomAccessFile::ReadAt( int64_t position, int64_t nbytes )
{
    return std::make_shared<arrow::Buffer>( (const uint8_t*)m_data.data() + position, nbytes );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
arrow::Result<int64_t> RifByteArrayArrowRandomAccessFile::Read( int64_t nbytes, void* out )
{
    return ReadAt( m_position, nbytes, out );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
arrow::Result<std::shared_ptr<arrow::Buffer>> RifByteArrayArrowRandomAccessFile::Read( int64_t nbytes )
{
    return ReadAt( m_position, nbytes );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
arrow::Result<int64_t> RifByteArrayArrowRandomAccessFile::GetSize()
{
    return m_data.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
arrow::Result<int64_t> RifByteArrayArrowRandomAccessFile::Tell() const
{
    return m_position;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
arrow::Status RifByteArrayArrowRandomAccessFile::Seek( int64_t position )
{
    m_position = position;
    return arrow::Status::OK();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
arrow::Status RifByteArrayArrowRandomAccessFile::Close()
{
    m_closed = true;
    return arrow::Status::OK();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifByteArrayArrowRandomAccessFile::closed() const
{
    return m_closed;
}
