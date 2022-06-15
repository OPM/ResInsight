/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 -     Equinor ASA
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

#include <memory>

class RigThermalFractureDefinition;

class RifThermalFractureReader
{
public:
    static std::pair<std::shared_ptr<RigThermalFractureDefinition>, QString> readFractureCsvFile( const QString& fileName );

private:
    static bool isHeaderLine( const QString& line );
    static bool isCenterNodeLine( const QString& line );
    static bool isInternalNodeLine( const QString& line );

    static std::pair<QString, QString> parseNameAndUnit( const QString& value );
};
