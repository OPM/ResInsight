/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 -     Equinor ASA
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

#include <QDateTime>
#include <QString>

#include <memory>
#include <optional>
#include <vector>

class RigPressureDepthData;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RifPressureDepthTextFileReader
{
public:
    static std::pair<std::vector<RigPressureDepthData>, QString> readFile( const QString& fileName );

private:
    static bool isHeaderLine( const QString& line );
    static bool isCommentLine( const QString& line );
    static bool isDateLine( const QString& line );
    static bool isPropertiesLine( const QString& line );
    static bool isUnitsLine( const QString& line );

    static std::optional<std::pair<double, double>> parseDataLine( const QString& line );
    static std::optional<QDateTime>                 parseDateLine( const QString& line );
};
