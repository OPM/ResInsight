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
    versionStrings << "2017.05.2-dev.11";
    versionStrings << "2017.05.2-dev.10";
    versionStrings << "2017.05.2-dev.09";
    versionStrings << "2017.05.2-dev.08";
    versionStrings << "2017.05.2-dev.07";
    versionStrings << "2017.05.2-dev.06";
    versionStrings << "2017.05.2-dev.05";
    versionStrings << "2017.05.2-dev.04";
    versionStrings << "2017.05.2-dev.03";
    versionStrings << "2017.05.2-dev.02";
    versionStrings << "2017.05.2-fdev.02";
    versionStrings << "2017.05.2-dev.1";
    versionStrings << "2017.05.2-fdev.01";
    versionStrings << "2017.05.2";
    versionStrings << "2017.05.pre-proto.15";
    versionStrings << "2017.05.1-dev";
    versionStrings << "2017.05.1";
    versionStrings << "2017.05.0";

    versionStrings << "2016.11.flow.14";
    versionStrings << "2016.11.flow.12";
    versionStrings << "2016.11.flow.11";
    versionStrings << "2016.11.flow.9";
    versionStrings << "2016.11.flow.8";
    versionStrings << "2016.11.flow.7";
    versionStrings << "2016.11.flow.1";
    versionStrings << "2016.11.m.1";
    versionStrings << "2016.11.0";

    versionStrings << "1.6.10-dev";
    versionStrings << "1.6.9-dev";
    versionStrings << "1.6.8-dev";
    versionStrings << "1.6.7-gm-beta";
    versionStrings << "1.6.6-dev";
    versionStrings << "1.6.5-dev";
    versionStrings << "1.6.4-dev";
    versionStrings << "1.6.3-dev";
    versionStrings << "1.6.1-dev";
    versionStrings << "1.6.2-dev";
    versionStrings << "1.6.0-RC";

    versionStrings << "1.5.111-RC";
    versionStrings << "1.5.110-RC";
    versionStrings << "1.5.109-RC";
    versionStrings << "1.5.108-RC";
    versionStrings << "1.5.107-RC";
    versionStrings << "1.5.106-RC";
    versionStrings << "1.5.105-RC";
    versionStrings << "1.5.104-RC";
    versionStrings << "1.5.103-dev";
    versionStrings << "1.5.102-dev";
    versionStrings << "1.5.101-dev";
    versionStrings << "1.5.100-dev";
    versionStrings << "1.5.0";

    return versionStrings;
}
