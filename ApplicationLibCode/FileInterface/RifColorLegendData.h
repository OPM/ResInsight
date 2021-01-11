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
#include "cvfObject.h"

class RigFormationNames;
class RimColorLegend;
class QString;

namespace cvf
{
class Color3f;
}

//==================================================================================================
///
//==================================================================================================
class RifColorLegendData
{
public:
    static std::pair<cvf::ref<RigFormationNames>, caf::PdmPointer<RimColorLegend>>
        readFormationNamesFile( const QString& fileName, QString* errorMessage );

private:
    static std::pair<cvf::ref<RigFormationNames>, caf::PdmPointer<RimColorLegend>>
        readLyrFormationNameFile( const QString& fileName, QString* errorMessage );
    static std::pair<cvf::ref<RigFormationNames>, caf::PdmPointer<RimColorLegend>>
        readFmuFormationNameFile( const QString& fileName, QString* errorMessage );

    static bool convertStringToColor( const QString& word, cvf::Color3f* color );
};
