/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "Well/RigWellPathFormations.h"

#include <expected>
#include <map>

#include <QString>

//==================================================================================================
///
//==================================================================================================
class RifWellPathFormationReader
{
public:
    using WellFormations = std::map<QString /*wellName*/, RigWellPathFormations>;

    static std::expected<WellFormations, QString> readWellFormations( const QString& filePath );
    static std::expected<WellFormations, QString> parseWellFormations( const QString& content, const QString& filePath );
};
