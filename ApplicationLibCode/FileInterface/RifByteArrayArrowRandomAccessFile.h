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

#pragma once

#undef signals
#include <arrow/csv/api.h>
#include <arrow/io/api.h>
#include <arrow/scalar.h>
#include <parquet/arrow/reader.h>
#define signals Q_SIGNALS

#include <QByteArray>

#include <memory>

//==================================================================================================
///
//==================================================================================================
class RifByteArrayArrowRandomAccessFile : public arrow::io::RandomAccessFile
{
public:
    RifByteArrayArrowRandomAccessFile( const QByteArray& data );

    arrow::Result<int64_t> ReadAt( int64_t position, int64_t nbytes, void* out ) override;

    arrow::Result<std::shared_ptr<arrow::Buffer>> ReadAt( int64_t position, int64_t nbytes ) override;

    arrow::Result<int64_t> Read( int64_t nbytes, void* out ) override;

    arrow::Result<std::shared_ptr<arrow::Buffer>> Read( int64_t nbytes ) override;

    arrow::Result<int64_t> GetSize() override;

    arrow::Result<int64_t> Tell() const override;

    arrow::Status Seek( int64_t position ) override;

    arrow::Status Close() override;

    bool closed() const override;

private:
    const QByteArray& m_data;
    bool              m_closed;
    int64_t           m_position;
};
