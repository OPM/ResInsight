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

    std::vector<QString> wellNames;
    std::vector<QString> formationNames;

    std::vector<double> mdTop;
    std::vector<double> mdBase;

    std::vector<double> tvdTop;
    std::vector<double> tvdBase;

    readFile(filePath, &wellNames, &formationNames, &mdTop, &mdBase, &tvdTop, &tvdBase);

    bool mdIsPresent = true;
    bool tvdIsPresent = true;
    
    if (mdTop.empty() || mdBase.empty())
    {
        mdIsPresent = false;
    }
    
    if (tvdTop.empty() || tvdBase.empty())
    {
        tvdIsPresent = false;
    }

    if (wellNames.empty() || formationNames.empty())
    {
        QMessageBox::warning(RiuMainWindow::instance(), "Import failure",
                             QString("Failed to parse %1 as a well pick file").arg(filePath));
        RiaLogging::error(QString("Failed to parse %1 as a well pick file").arg(filePath));

        return result;
    }
    else if (!(mdIsPresent || tvdIsPresent))
    {
        QMessageBox::warning(RiuMainWindow::instance(), "Import failure",
                             QString("Failed to parse %1 as a well pick file. Neither MD or TVD is present.").arg(filePath));
        RiaLogging::error(QString("Failed to parse %1 as a well pick file. Neither MD or TVD is present.").arg(filePath));

        return result;
    }

    CVF_ASSERT(wellNames.size() == formationNames.size());

    std::map<QString, std::vector<RigWellPathFormation>> formations;

    for (size_t i = 0; i < wellNames.size(); i++)
    {
        RigWellPathFormation formation;
        formation.formationName = formationNames[i];

        if (mdIsPresent)
        {
            formation.mdTop = mdTop[i];
            formation.mdBase = mdBase[i];
        }

        if (tvdIsPresent)
        {
            formation.tvdTop = tvdTop[i];
            formation.tvdBase = tvdBase[i];
        }

        if (!formations.count(wellNames[i]))
        {
            formations[wellNames[i]] = std::vector<RigWellPathFormation>();
        }

        formations[wellNames[i]].push_back(formation);
    }

    for (const std::pair<QString, std::vector<RigWellPathFormation>>& formation : formations)
    {
        cvf::ref<RigWellPathFormations> wellPathFormations = new RigWellPathFormations(formation.second, filePath, formation.first);
        result[formation.first] = wellPathFormations;
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
void RifWellPathFormationReader::readFile(const QString& filePath, std::vector<QString>* wellNames,
                                          std::vector<QString>* formationNames, std::vector<double>* mdTop,
                                          std::vector<double>* mdBase, std::vector<double>* tvdTop, 
                                          std::vector<double>* tvdBase)
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

    static const QString wellNameText = "wellname";
    static const QString surfaceNameText = "surfacename";
    static const QString measuredDepthText = "md";

    int wellNameIndex = header.indexOf(wellNameText);
    int surfaceNameIndex = header.indexOf(surfaceNameText);
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

            QString wellName = dataLine[wellNameIndex];
            QString surfaceName = dataLine[surfaceNameIndex];

            wellNames->push_back(wellName);
            formationNames->push_back(surfaceName);
            mdTop->push_back(measuredDepth);

        } while (!data.atEnd());

        return;
    }

    static const QString unitNameText = "unitname";
    static const QString measuredDepthToptext = "topmd";
    static const QString measuredDepthBasetext = "basemd";
    static const QString trueVerticalDepthToptext = "toptvdss";
    static const QString trueVerticalDepthBasetext = "basetvdss";

    int unitNameIndex = header.indexOf(unitNameText);

    int measuredDepthTopIndex = header.indexOf(measuredDepthToptext);
    int measuredDepthBaseIndex = header.indexOf(measuredDepthBasetext);

    int trueVerticalDepthTopIndex = header.indexOf(trueVerticalDepthToptext);
    int trueVerticalDepthBaseIndex = header.indexOf(trueVerticalDepthBasetext);

    bool mdIsPresent = true;
    bool tvdIsPresent = true;
    
    if (measuredDepthTopIndex == -1 || measuredDepthBaseIndex == -1)
    {
        mdIsPresent = false;
    }

    if (trueVerticalDepthTopIndex == -1 || trueVerticalDepthBaseIndex == -1)
    {
        tvdIsPresent = false;
    }

    if (unitNameIndex != -1 && (mdIsPresent || tvdIsPresent))
    {
        do
        {
            QString line = data.readLine();

            QStringList dataLine = line.split(';', QString::KeepEmptyParts);
            if (dataLine.size() != header.size()) continue;

            QString wellName = dataLine[wellNameIndex];
            QString unitName = dataLine[unitNameIndex];
            unitName = unitName.trimmed();

            if (mdIsPresent)
            {
                double mdTopValue = dataLine[measuredDepthTopIndex].toDouble();
                double mdBaseValue = dataLine[measuredDepthBaseIndex].toDouble();
                
                mdTop->push_back(mdTopValue);
                mdBase->push_back(mdBaseValue);
            }

            if (tvdIsPresent)
            {
                double tvdTopValue = dataLine[trueVerticalDepthTopIndex].toDouble();
                double tvdBaseValue = dataLine[trueVerticalDepthBaseIndex].toDouble();

                tvdTop->push_back(-tvdTopValue);
                tvdBase->push_back(-tvdBaseValue);
            }

            wellNames->push_back(wellName);
            formationNames->push_back(unitName);

        } while (!data.atEnd());
    }
}
