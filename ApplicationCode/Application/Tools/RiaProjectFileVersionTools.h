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

#pragma once

#include <QString>

//==================================================================================================
//
//==================================================================================================
struct RiaProjectFileVersionData
{
    int m_majorVersion;
    int m_minorVersion;
    int m_patch;
    int m_developmentId;

    RiaProjectFileVersionData() : m_majorVersion(0), m_minorVersion(0), m_patch(0), m_developmentId(0) {}

    RiaProjectFileVersionData(int majorVersion, int minorVersion, int patch, int developmentId)
        : m_majorVersion(majorVersion), m_minorVersion(minorVersion), m_patch(patch), m_developmentId(developmentId)

    {
    }
};

//==================================================================================================
//
//==================================================================================================
class RiaProjectFileVersionTools
{
public:
    static bool isProjectFileVersionNewerThan(const QString&                   projectFileVersion,
                                              const RiaProjectFileVersionData& fileVersionData);

private:
    static bool isProjectFileVersionNewerThan(const QString& projectFileVersion, int majorVersion, int minorVersion, int patch,
                                              int developmentId);

    static QStringList knownProjectVersionStrings();
};
