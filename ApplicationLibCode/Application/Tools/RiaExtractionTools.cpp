/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RiaExtractionTools.h"

#include "RigWellPath.h"
#include "RimEclipseCase.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "RimWellLogPlotCollection.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseWellLogExtractor* RiaExtractionTools::findOrCreateWellLogExtractor( RimWellPath*    wellPath,
                                                                              RimEclipseCase* eclipseCase )
{
    if ( !( wellPath && eclipseCase ) ) return nullptr;

    auto wlPlotCollection = wellLogPlotCollection();
    if ( !wlPlotCollection ) return nullptr;

    return wlPlotCollection->findOrCreateExtractor( wellPath, eclipseCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGeoMechWellLogExtractor* RiaExtractionTools::findOrCreateWellLogExtractor( RimWellPath*    wellPath,
                                                                              RimGeoMechCase* geomCase )
{
    if ( !( wellPath && geomCase ) ) return nullptr;

    auto wlPlotCollection = wellLogPlotCollection();
    if ( !wlPlotCollection ) return nullptr;

    return wlPlotCollection->findOrCreateExtractor( wellPath, geomCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseWellLogExtractor* RiaExtractionTools::findOrCreateSimWellExtractor( const RimSimWellInView* simWell,
                                                                              const RigWellPath*      wellPathGeom )
{
    if ( !( simWell && wellPathGeom ) ) return nullptr;

    auto wlPlotCollection = wellLogPlotCollection();
    if ( !wlPlotCollection ) return nullptr;

    RimEclipseCase* eclipseCase = nullptr;
    simWell->firstAncestorOrThisOfType( eclipseCase );
    if ( !( eclipseCase && eclipseCase->eclipseCaseData() ) )
    {
        return nullptr;
    }

    QString caseUserDescription = eclipseCase->caseUserDescription();

    return wlPlotCollection->findOrCreateSimWellExtractor( simWell->name,
                                                           caseUserDescription,
                                                           wellPathGeom,
                                                           eclipseCase->eclipseCaseData() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogPlotCollection* RiaExtractionTools::wellLogPlotCollection()
{
    auto proj = RimProject::current();
    if ( !proj ) return nullptr;

    auto plotCollection = proj->mainPlotCollection();
    if ( !plotCollection ) return nullptr;

    auto wellLogPlotCollection = plotCollection->wellLogPlotCollection();
    return wellLogPlotCollection;
}
