/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RigWellPathFormations.h"

#include "cvfBase.h"
#include "cvfObject.h"

#include <QDateTime>
#include <QString>

#include <map>
#include <vector>

//==================================================================================================
///
///
//==================================================================================================
class RifWellPathFormationsImporter
{
public:
    cvf::ref<RigWellPathFormations> readWellPathFormations(const QString& formationFilePath, const QString& wellName);

    std::map<QString, cvf::ref<RigWellPathFormations>> readWellPathFormationsFromPath(const QString& filePath);

private:
    void readAllWellPathFormations(const QString& filePath);

    std::map<QString /*filename*/, std::map<QString /*wellName*/, cvf::ref<RigWellPathFormations>>>
        m_fileNameToWellPathFormationMap;
};
