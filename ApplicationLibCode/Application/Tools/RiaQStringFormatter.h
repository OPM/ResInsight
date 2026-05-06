/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include <format>
#include <string_view>

#include <QByteArray>
#include <QString>

// std::formatter specialization for QString. Include this header in any
// translation unit that calls std::format with QString arguments.
//
// QString is encoded as UTF-8 before being formatted as a string_view; the
// QByteArray holding the encoded bytes lives for the duration of the format()
// call, which writes synchronously to the output iterator.
template <>
struct std::formatter<QString> : std::formatter<std::string_view>
{
    auto format( const QString& value, std::format_context& ctx ) const
    {
        const QByteArray bytes = value.toUtf8();
        return std::formatter<std::string_view>::format( std::string_view{ bytes.constData(), static_cast<size_t>( bytes.size() ) }, ctx );
    }
};
