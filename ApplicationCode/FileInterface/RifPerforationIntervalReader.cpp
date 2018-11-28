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

#include "RifPerforationIntervalReader.h"

#include <QFile>
#include <QDate>

const QString PERFORATION_KEY("perforation");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<QString, std::vector<RifPerforationInterval> > RifPerforationIntervalReader::readPerforationIntervals(const QStringList& filePaths)
{
    std::map<QString, std::vector<RifPerforationInterval>> perforationIntervals;

    foreach (QString filePath, filePaths)
    {
        readFileIntoMap(filePath, &perforationIntervals);
    }

    return perforationIntervals;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<QString, std::vector<RifPerforationInterval> > RifPerforationIntervalReader::readPerforationIntervals(const QString& filePath)
{
    std::map<QString, std::vector<RifPerforationInterval> > perforationIntervals;

    readFileIntoMap(filePath, &perforationIntervals);

    return perforationIntervals;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifPerforationIntervalReader::readFileIntoMap(const QString& filePath, std::map<QString, std::vector<RifPerforationInterval>>* perforations)
{
    QFile data(filePath);

    if (!data.open(QFile::ReadOnly))
    {
        return;
    }

    QString wellName;

    do {
        QString line = data.readLine();

        if (line.startsWith("--"))
        {
            // Skip comment
            continue;
        }

        // Replace any tabs with spaces to enable splitting on spaces
        line.replace("\t", " ");
        QStringList parts = line.split(" ", QString::SkipEmptyParts);

        if (line.startsWith("WELLNAME"))
        {
            // Save current well name
            if (parts.size() > 1)
            {
                wellName = parts[1].trimmed();
            }
        }
        else if (parts.size() >= 6)
        {
            RifPerforationInterval interval;

            int mdStartIndex;

            if (parts[3] == PERFORATION_KEY)
            {
                interval.date = QDate::fromString(QString("%1 %2 %3").arg(parts[0]).arg(parts[1]).arg(parts[2]), "dd MMM yyyy");
                interval.startOfHistory = false;

                mdStartIndex = 4;
            }
            else if (parts[1] == PERFORATION_KEY)
            {
                interval.startOfHistory = true;

                mdStartIndex = 2;
            }
            else
            {
                continue;
            }

            interval.startMD    = parts[mdStartIndex].toDouble();
            interval.endMD      = parts[mdStartIndex + 1].toDouble();
            interval.diameter   = parts[mdStartIndex + 2].toDouble();
            interval.skinFactor = parts[mdStartIndex + 3].toDouble();

            auto match = perforations->find(wellName);
            if (match == perforations->end())
            {
                (*perforations)[wellName] = std::vector<RifPerforationInterval>();
            }
            (*perforations)[wellName].push_back(interval);
        }
    } while (!data.atEnd());
}
