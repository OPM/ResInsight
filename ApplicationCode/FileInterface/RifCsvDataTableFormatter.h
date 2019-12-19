/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-    Equinor ASA
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

#include "RifTextDataTableFormatter.h"

//==================================================================================================
//
// CSV text formatter using the same pattern as RifEclipseDataTableFormatter so it will be easy to switch formatters
//
//==================================================================================================
class RifCsvDataTableFormatter
{
public:
    RifCsvDataTableFormatter( QTextStream& out, const QString fieldSeparator = "," );

    RifCsvDataTableFormatter& header( const std::vector<RifTextDataTableColumn>& tableHeader );
    RifCsvDataTableFormatter& add( const QString& str );
    RifCsvDataTableFormatter& add( double num );
    RifCsvDataTableFormatter& add( int num );
    RifCsvDataTableFormatter& add( size_t num );
    void                      rowCompleted();
    void                      tableCompleted();

private:
    void outputBuffer();

private:
    QTextStream&                        m_out;
    std::vector<RifTextDataTableColumn> m_columnHeaders;
    std::vector<RifTextDataTableLine>   m_buffer;
    std::vector<QString>                m_lineBuffer;
    QString                             m_fieldSeparator;
};
