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

#include "RifWellPathFormationsImporter.h"
#include "RifWellPathFormationReader.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigWellPathFormations> RifWellPathFormationsImporter::readWellPathFormations(const QString& formationFilePath,
                                                                                      const QString& wellName)
{
    readAllWellPathFormations(formationFilePath);
    return m_fileNameToWellPathFormationMap[formationFilePath][wellName];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifWellPathFormationsImporter::readAllWellPathFormations(const QString& filePath)
{
    // If we have the file in the map, assume it is already read.
    if (m_fileNameToWellPathFormationMap.find(filePath) != m_fileNameToWellPathFormationMap.end())
    {
        return;
    }

    std::map<QString, cvf::ref<RigWellPathFormations>> wellPathToFormationMap =
        RifWellPathFormationReader::readWellFormationsToGeometry(filePath);

    m_fileNameToWellPathFormationMap[filePath] = wellPathToFormationMap;
}
