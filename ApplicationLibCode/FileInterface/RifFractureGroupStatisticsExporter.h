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

#include <vector>

class QString;
class QTextStream;

class RigSlice2D;

//==================================================================================================
//
//==================================================================================================
class RifFractureGroupStatisticsExporter
{
public:
    static bool writeAsStimPlanXml( const RigSlice2D&          statistics,
                                    const QString&             filePath,
                                    const std::vector<double>& gridXs,
                                    const std::vector<double>& gridYs,
                                    double                     time );

private:
    static void appendHeaderToStream( QTextStream& stream );
    static void appendGridDimensionsToStream( QTextStream&               stream,
                                              const std::vector<double>& gridXs,
                                              const std::vector<double>& gridYs );
    static void appendPropertiesToStream( QTextStream&               stream,
                                          const RigSlice2D&          statistics,
                                          const std::vector<double>& gridYs,
                                          double                     time );
    static void appendFooterToStream( QTextStream& stream );
};
