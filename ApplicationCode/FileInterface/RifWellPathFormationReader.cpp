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

#include "RifWellPathFormationReader.h"

#include "RiaLogging.h"
#include "RiuMainWindow.h"

#include <QFile>
#include <QMessageBox>
#include <QStringList>

#include <algorithm>
#include <cctype>
#include <string>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, cvf::ref<RigWellPathFormations>>
    RifWellPathFormationReader::readWellFormationsToGeometry(const QString& filePath)
{
    std::map<QString, cvf::ref<RigWellPathFormations>> result;

    std::map<QString, std::vector<std::pair<double, QString>>> formations;

    readFileIntoMap(filePath, &formations);
    if (formations.empty())
    {
        QMessageBox::warning(RiuMainWindow::instance(), "Import failure",
                             QString("Failed to parse %1 as well picks file").arg(filePath));
        RiaLogging::error(QString("Failed to parse %1 as well picks file").arg(filePath));
    }

    std::map<QString, std::vector<std::pair<double, QString>>>::iterator it;

    for (it = formations.begin(); it != formations.end(); it++)
    {
        cvf::ref<RigWellPathFormations> wellPathFormations = new RigWellPathFormations(it->second, filePath, it->first);
        result[it->first]                                  = wellPathFormations;
    }

    return result;
}

void removeWhiteSpaces(QString* word)
{
    std::string wordStd = word->toStdString();
    wordStd.erase(std::remove_if(wordStd.begin(), wordStd.end(), [](unsigned char x) { return std::isspace(x); }), wordStd.end());

    (*word) = QString(wordStd.c_str());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifWellPathFormationReader::readFileIntoMap(const QString&                                              filePath,
                                                 std::map<QString, std::vector<std::pair<double, QString>>>* formations)
{
    QFile data(filePath);

    if (!data.open(QFile::ReadOnly))
    {
        return;
    }
    QStringList header;

    while (header.size() < 3)
    {
        if (data.atEnd()) return;

        QString line = data.readLine().toLower();
        removeWhiteSpaces(&line);

        header = line.split(';', QString::KeepEmptyParts);
    }

    static const QString wellNameText      = "wellname";
    static const QString surfaceNameText   = "surfacename";
    static const QString measuredDepthText = "md";

    int wellNameIndex      = header.indexOf(wellNameText);
    int surfaceNameIndex   = header.indexOf(surfaceNameText);
    int measuredDepthIndex = header.indexOf(measuredDepthText);

    if (wellNameIndex != -1 && surfaceNameIndex != -1 && measuredDepthIndex != -1)
    {
        do
        {
            QString line = data.readLine();

            QStringList dataLine = line.split(';', QString::KeepEmptyParts);
            if (dataLine.size() != header.size()) continue;

            bool   conversionOk;
            double measuredDepth = dataLine[measuredDepthIndex].toDouble(&conversionOk);
            if (!conversionOk) continue;

            QString wellName    = dataLine[wellNameIndex];
            QString surfaceName = dataLine[surfaceNameIndex];

            (*formations)[wellName].push_back(std::make_pair(measuredDepth, surfaceName));

        } while (!data.atEnd());

        return;
    }

    static const QString unitNameText          = "unitname";
    static const QString measuredDepthToptext  = "topmd";
    static const QString measuredDepthBasetext = "basemd";

    int unitNameIndex          = header.indexOf(unitNameText);
    int measuredDepthTopIndex  = header.indexOf(measuredDepthToptext);
    int measuredDepthBaseIndex = header.indexOf(measuredDepthBasetext);

    if (unitNameIndex != -1 && measuredDepthTopIndex != -1 && measuredDepthBaseIndex != -1)
    {
        do
        {
            QString line = data.readLine();

            QStringList dataLine = line.split(';', QString::KeepEmptyParts);
            if (dataLine.size() != header.size()) continue;

            QString wellName         = dataLine[wellNameIndex];
            QString unitName         = dataLine[unitNameIndex];
            unitName                 = unitName.trimmed();
            double measuredDepthTop  = dataLine[measuredDepthTopIndex].toDouble();
            double measuredDepthBase = dataLine[measuredDepthBaseIndex].toDouble();

            (*formations)[wellName].push_back(std::make_pair(measuredDepthTop, unitName));
            (*formations)[wellName].push_back(std::make_pair(measuredDepthBase, unitName));

        } while (!data.atEnd());
    }
}
