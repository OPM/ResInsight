/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
//  A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QString>
#include <optional>
#include <vector>

namespace Opm
{
class DeckKeyword;
} // namespace Opm

class RimWellEventControl;
class RimWellEvent;
class RimWellEventKeywordItem;

namespace RifEventKeywordFormatter
{
bool isRecordlessKeyword( const Opm::DeckKeyword& keyword );

// build*: produce a structured Opm::DeckKeyword (used by callers that merge records across wells).
std::optional<Opm::DeckKeyword> buildKeyword( const QString& keywordName, const std::vector<RimWellEventKeywordItem*>& items );
std::optional<Opm::DeckKeyword> buildWconprod( const RimWellEventControl* controlEvent, const QString& wellName );
std::optional<Opm::DeckKeyword> buildWconinje( const RimWellEventControl* controlEvent, const QString& wellName );
std::optional<Opm::DeckKeyword> buildWellEvent( const RimWellEvent* event, const QString& wellName );

// format*: serialize the build* result to text. Convenience for single-keyword stringification.
QString formatKeyword( const QString& keywordName, const std::vector<RimWellEventKeywordItem*>& items );
QString formatWconprod( const RimWellEventControl* controlEvent, const QString& wellName );
QString formatWconinje( const RimWellEventControl* controlEvent, const QString& wellName );
QString formatWellEvent( const RimWellEvent* event, const QString& wellName );
} // namespace RifEventKeywordFormatter
