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

#include "RifWellFormationReader.h"

#include <QFile>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<QString, std::vector<RifWellFormation> > RifWellFormationReader::readWellFormations(const QStringList& filePaths)
{
    std::map<QString, std::vector<RifWellFormation>> formations;

    foreach (QString filePath, filePaths)
    {
        readFileIntoMap(filePath, &formations);
    }

    return formations;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<QString, std::vector<RifWellFormation> > RifWellFormationReader::readWellFormations(const QString& filePath)
{
    std::map<QString, std::vector<RifWellFormation> > formations;

    readFileIntoMap(filePath, &formations);

    return formations;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifWellFormationReader::readFileIntoMap(const QString& filePath, std::map<QString, std::vector<RifWellFormation>>* formations)
{
    QFile data(filePath);

    if (!data.open(QFile::ReadOnly))
    {
        return;
    }
    QStringList header;

    while (header.size() < 3)
    {
        QString line = data.readLine().toLower();
        QString lineWithoutSpaces = line.remove(' ');
        
        header = lineWithoutSpaces.split(';', QString::KeepEmptyParts);
    }

    const QString wellNameText = "wellname";
    const QString surfaceNameText = "surfacename";
    const QString measuredDepthText = "md";

    int wellNameIndex = header.indexOf(wellNameText);
    int surfaceNameIndex = header.indexOf(surfaceNameText);
    int measuredDepthIndex = header.indexOf(measuredDepthText);

    if (wellNameIndex == -1 || surfaceNameIndex == -1 || measuredDepthIndex == -1)
    {
        return;
    }

    do {
        QString line = data.readLine();

        QStringList dataLine = line.split(';', QString::KeepEmptyParts);
        if (dataLine.size() != header.size()) continue;

        RifWellFormation formation(dataLine[wellNameIndex], dataLine[surfaceNameIndex], dataLine[measuredDepthIndex].toDouble());

        (*formations)[formation.wellName].push_back(formation);

    } while (!data.atEnd());
}
