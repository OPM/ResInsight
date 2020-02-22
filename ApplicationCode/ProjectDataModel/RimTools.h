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

#include "RimWellPath.h"

#include <QList>
#include <QString>

#include <vector>

class QDateTime;

namespace caf
{
class PdmOptionItemInfo;
}

class RimWellPathCollection;

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

    static void                   wellPathOptionItems( QList<caf::PdmOptionItemInfo>* options );
    static void                   wellPathWithFormationsOptionItems( QList<caf::PdmOptionItemInfo>* options );
    static void                   wellPathWithFormations( std::vector<RimWellPath*>* wellPaths );
    static void                   caseOptionItems( QList<caf::PdmOptionItemInfo>* options );
    static void                   eclipseCaseOptionItems( QList<caf::PdmOptionItemInfo>* options );
    static void                   geoMechCaseOptionItems( QList<caf::PdmOptionItemInfo>* options );
    static RimWellPathCollection* wellPathCollection();
};
