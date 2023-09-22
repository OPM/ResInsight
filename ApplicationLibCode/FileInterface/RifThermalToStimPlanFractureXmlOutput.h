/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-    Equinor ASA
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

#include <vector>

class RimThermalFractureTemplate;
class RigFractureGrid;

class QString;
class QTextStream;

//==================================================================================================
//
//==================================================================================================
class RifThermalToStimPlanFractureXmlOutput
{
public:
    static bool writeToFile( RimThermalFractureTemplate* fractureTemplate, const QString& filepath );

private:
    static void appendHeaderToStream( QTextStream& stream );

    static void appendPropertiesToStream( QTextStream&                      stream,
                                          const RimThermalFractureTemplate& fractureTemplate,
                                          const RigFractureGrid&            fractureGrid );

    static void appendGridDefinitionToStream( QTextStream& stream, const RigFractureGrid& fractureGrid );

    static void appendDataVector( QTextStream& stream, const QString& name, const std::vector<double>& vals );

    static void appendFooterToStream( QTextStream& stream );
};
