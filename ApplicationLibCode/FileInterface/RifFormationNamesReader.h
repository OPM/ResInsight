/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "RigFormationNames.h"

#include <expected>

#include <QString>

namespace cvf
{
class Color3f;
}

//==================================================================================================
///
//==================================================================================================
class RifFormationNamesReader
{
public:
    [[nodiscard]] static std::expected<RigFormationNames, QString> readFormationNamesFile( const QString& fileName );

private:
    static std::expected<RigFormationNames, QString> readLyrFormationNameFile( const QString& fileName );
    static std::expected<RigFormationNames, QString> readFmuFormationNameFile( const QString& fileName );

    static bool convertStringToColor( const QString& word, cvf::Color3f* color );
};
