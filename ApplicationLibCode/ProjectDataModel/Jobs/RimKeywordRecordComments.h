/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include <QString>

#include <map>

class RigMswTableData;

//==================================================================================================
/// Comments placed above individual keyword records in deck serialization.
///
/// A comment is a (possibly multi-line) text emitted as "--" lines directly above a record. A comment
/// starting with a newline is preceded by a blank line. A comment line can also be one of the markers:
///   separator()        rendered as a line of dashes as wide as the record row
///   sectionTitle( t )  rendered as "-- t ----" padded with dashes to the record row width
///   columnHeader()     repeats the column header line (aligned output only)
/// Markers can be combined with plain text lines in one comment, separated by newlines.
//==================================================================================================
namespace RimKeywordRecordComments
{
QString separator();
QString sectionTitle( const QString& title );
QString columnHeader();

// Recognize a sectionTitle() marker line, and get its title text
bool    isSectionTitle( const QString& line );
QString sectionTitleText( const QString& line );

// Comments (record index -> comment) for the records of the keyword from RimKeywordFactory::welsegsKeyword(),
// telling which part of the well the segments belong to: a "Branch N: WELL.WELLPATH" heading above each well
// path branch and a title above each group of completion segments.
std::map<size_t, QString> welsegsRecordComments( const RigMswTableData& mswData );
} // namespace RimKeywordRecordComments
