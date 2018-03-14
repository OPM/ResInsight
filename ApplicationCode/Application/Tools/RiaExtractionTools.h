/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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

class RigEclipseWellLogExtractor;
class RigGeoMechWellLogExtractor;
class RimEclipseCase;
class RimGeoMechCase;
class RimWellLogPlotCollection;
class RimWellPath;
class RigWellPath;

class QString;

//==================================================================================================
//
//==================================================================================================
namespace RiaExtractionTools
{
RigEclipseWellLogExtractor* wellLogExtractorEclipseCase(RimWellPath* wellPath, RimEclipseCase* eclipseCase);
RigGeoMechWellLogExtractor* wellLogExtractorGeoMechCase(RimWellPath* wellPath, RimGeoMechCase* geomCase);

/*
RigEclipseWellLogExtractor* findOrCreateSimWellExtractor(const QString&        simWellName,
                                                         const QString&        caseUserDescription,
                                                         const RigWellPath*    wellPathGeom,
                                                         const RimEclipseCase* eclipseCase);
*/

RimWellLogPlotCollection* wellLogPlotCollection();

}; // namespace RiaExtractionTools
