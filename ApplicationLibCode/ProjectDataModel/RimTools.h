/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "cvfBoundingBox.h"

#include <QList>
#include <QString>

#include <vector>

class QDateTime;

namespace caf
{
class PdmOptionItemInfo;
}

class RimGeoMechCase;
class RimEclipseCase;
class RimWellPathCollection;
class RimCase;
class RimWellPath;
class RimSurfaceCollection;
class RimFaultInViewCollection;
class RimPolygonCollection;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RimTools
{
public:
    static QString getCacheRootDirectoryPathFromProject();

    static QString relocateFile( const QString&        originalFileName,
                                 const QString&        currentProjectPath,
                                 const QString&        previousProjectPath,
                                 bool*                 foundFile,
                                 std::vector<QString>* searchedPaths );

    static void wellPathOptionItemsSubset( const std::vector<RimWellPath*>& wellPathsToExclude, QList<caf::PdmOptionItemInfo>* options );
    static void wellPathOptionItems( QList<caf::PdmOptionItemInfo>* options );
    static void wellPathWithFormationsOptionItems( QList<caf::PdmOptionItemInfo>* options );
    static void wellPathWithFormations( std::vector<RimWellPath*>* wellPaths );
    static void caseOptionItems( QList<caf::PdmOptionItemInfo>* options );
    static void eclipseCaseOptionItems( QList<caf::PdmOptionItemInfo>* options );
    static void eclipseGridOptionItems( QList<caf::PdmOptionItemInfo>* options, RimEclipseCase* eCase );
    static void geoMechCaseOptionItems( QList<caf::PdmOptionItemInfo>* options );
    static void geoMechPartOptionItems( QList<caf::PdmOptionItemInfo>* options, RimGeoMechCase* gCase );
    static void geoMechElementSetOptionItems( QList<caf::PdmOptionItemInfo>* options, RimGeoMechCase* gCase, int partId );
    static void colorLegendOptionItems( QList<caf::PdmOptionItemInfo>* options );
    static void seismicDataOptionItems( QList<caf::PdmOptionItemInfo>* options, cvf::BoundingBox worldBBox, bool basicDataOnly = false );
    static void seismicDataOptionItems( QList<caf::PdmOptionItemInfo>* options );
    static void polygonOptionItems( QList<caf::PdmOptionItemInfo>* options );

    static void faultOptionItems( QList<caf::PdmOptionItemInfo>* options, RimFaultInViewCollection* coll );

    static RimWellPathCollection* wellPathCollection();
    static RimWellPath*           firstWellPath();

    static RimSurfaceCollection* surfaceCollection();
    static RimPolygonCollection* polygonCollection();

    static void timeStepsForCase( RimCase* gridCase, QList<caf::PdmOptionItemInfo>* options );

    static void optionItemsForSpecifiedWellPaths( const std::vector<RimWellPath*>& wellPaths, QList<caf::PdmOptionItemInfo>* options );
};
