/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-    Equinor ASA
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

#include "RiaDefines.h"

#include "RigStimPlanFractureDefinition.h"

#include <memory>
#include <vector>

class QString;
class QTextStream;

class RigSlice2D;

//==================================================================================================
//
//==================================================================================================
class RifEnsembleFractureStatisticsExporter
{
public:
    static bool writeAsStimPlanXml( const std::vector<std::shared_ptr<RigSlice2D>>& statistics,
                                    const std::vector<std::pair<QString, QString>>& properties,
                                    const QString&                                  filePath,
                                    const std::vector<double>&                      gridXs,
                                    const std::vector<double>&                      gridYs,
                                    double                                          time,
                                    RiaDefines::EclipseUnitSystem                   unitSystem,
                                    RigStimPlanFractureDefinition::Orientation      orientation );

private:
    static void appendHeaderToStream( QTextStream& stream );
    static void appendOrientationToStream( QTextStream& stream, RigStimPlanFractureDefinition::Orientation orientation );
    static void appendGridDimensionsToStream( QTextStream&                  stream,
                                              const std::vector<double>&    gridXs,
                                              const std::vector<double>&    gridYs,
                                              RiaDefines::EclipseUnitSystem unitSystem );
    static void appendPropertiesToStream( QTextStream&                                    stream,
                                          const std::vector<std::shared_ptr<RigSlice2D>>& statistics,
                                          const std::vector<std::pair<QString, QString>>& properties,
                                          const std::vector<double>&                      gridYs,
                                          double                                          time );
    static void appendFooterToStream( QTextStream& stream );

    static QString getStringForUnitSystem( RiaDefines::EclipseUnitSystem unitSystem );

    static QString getStringForOrientation( RigStimPlanFractureDefinition::Orientation orientation );
};
