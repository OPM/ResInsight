/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiaProjectFileVersionTools.h"

#include <QStringList>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaProjectFileVersionTools::isProjectFileVersionNewerThan(const QString&                   projectFileVersion,
                                                               const RiaProjectFileVersionData& fileVersionData)
{
    return isProjectFileVersionNewerThan(projectFileVersion, fileVersionData.m_majorVersion, fileVersionData.m_minorVersion,
                                         fileVersionData.m_patch, fileVersionData.m_developmentId);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaProjectFileVersionTools::isProjectFileVersionNewerThan(const QString& projectFileVersion, int majorVersion,
                                                               int minorVersion, int patchNumber, int developmentId)
{
    int projectMajorVersion  = 0;
    int projectMinorVersion  = 0;
    int projectPatchNumber   = 0;
    int projectDevelopmentId = 0;

    // Split string and interpret sub strings
    {
        QStringList subStrings = projectFileVersion.split(".");

        if (subStrings.size() > 0)
        {
            projectMajorVersion = subStrings[0].toInt();
        }

        if (subStrings.size() > 1)
        {
            projectMinorVersion = subStrings[1].toInt();
        }

        if (subStrings.size() > 2)
        {
            projectPatchNumber = subStrings[2].toInt();
        }
    }

    if (projectMajorVersion != majorVersion)
    {
        return (projectMajorVersion > majorVersion);
    }

    if (projectMinorVersion != minorVersion)
    {
        return (projectMinorVersion > minorVersion);
    }

    if (projectPatchNumber != patchNumber)
    {
        return (projectPatchNumber > patchNumber);
    }

    if (projectDevelopmentId != developmentId)
    {
        return (projectDevelopmentId > developmentId);
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RiaProjectFileVersionTools::knownProjectVersionStrings()
{
    QStringList versionStrings;

    versionStrings << "2017.05.2-dev.15";
    versionStrings << "2017.05.2-dev.14";
    versionStrings << "2017.05.2-dev.13";
    versionStrings << "2017.05.2-dev.12";

    return versionStrings;
}
