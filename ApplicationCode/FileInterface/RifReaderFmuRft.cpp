/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-  Equinor ASA
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
#include "RifReaderFmuRft.h"

#include "cvfAssert.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <limits>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderFmuRft::RifReaderFmuRft(const QString& filePath)
    : m_status(UNINITIALIZED)
    , m_filePath(filePath)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RifReaderFmuRft::findSubDirectoriesWithFmuRftData(const QString& filePath)
{
    QStringList subDirsContainingFmuRftData;

    QFileInfo fileInfo(filePath);
    if (!(fileInfo.exists() && fileInfo.isDir() && fileInfo.isReadable()))
    {
        return subDirsContainingFmuRftData;
    }

    if (directoryContainsFmuRftData(filePath))
    {
        subDirsContainingFmuRftData.push_back(filePath);
    }

    QDir dir(filePath);

    QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable, QDir::Name);
    for (QString subDir : subDirs)
    {
        QString absDir = dir.absoluteFilePath(subDir);
        subDirsContainingFmuRftData.append(findSubDirectoriesWithFmuRftData(absDir));
    }

    return subDirsContainingFmuRftData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderFmuRft::directoryContainsFmuRftData(const QString& filePath)
{
    QFileInfo baseFileInfo(filePath);
    if (!(baseFileInfo.exists() && baseFileInfo.isDir() && baseFileInfo.isReadable()))
    {
        return false;
    }

    QDir dir(filePath);
    if (!dir.exists("well_date_rft.txt"))
    {
        return false;
    }
    
    QStringList obsFiles; obsFiles << "*.obs" << "*.txt";
    QFileInfoList fileInfos = dir.entryInfoList(obsFiles, QDir::Files, QDir::Name);

    std::map<QString, int> fileStemCounts;
    for (QFileInfo fileInfo : fileInfos)
    {
        // TODO: 
        // Uses completeBaseName() to support wells with a dot in the name.
        // Not sure if this is necessary or desired
        fileStemCounts[fileInfo.completeBaseName()]++;
        if (fileStemCounts[fileInfo.completeBaseName()] == 2)
        {
            // At least one matching obs and txt file.
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderFmuRft::Status RifReaderFmuRft::status() const
{
    return m_status;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderFmuRft::Status RifReaderFmuRft::initialize(QString* errorMsg)
{
    CVF_ASSERT(errorMsg);

    QFileInfo fileInfo(m_filePath);
    if (!(fileInfo.exists() && fileInfo.isDir() && fileInfo.isReadable()))
    {
        *errorMsg = QString("Directory '%s' does not exist or isn't readable").arg(m_filePath);
        return STATUS_ERROR;
    }

    QDir dir(m_filePath);

    if (STATUS_ERROR == loadWellDates(dir, errorMsg))
    {
        return STATUS_ERROR;
    }
   
    std::map<QString, WellDate> validWellDates;
    for (auto wellDatePair : m_wellDates)
    {
        QString txtFile = QString("%1.txt").arg(wellDatePair.first);
        QString obsFile = QString("%1.obs").arg(wellDatePair.first);
        QFileInfo txtFileInfo(dir.absoluteFilePath(txtFile));
        QFileInfo obsFileInfo(dir.absoluteFilePath(obsFile));
        if (txtFileInfo.exists() && txtFileInfo.isFile() && txtFileInfo.isReadable() &&
            obsFileInfo.exists() && obsFileInfo.isFile() && obsFileInfo.isReadable())
        {
            validWellDates.insert(wellDatePair);
        }
    }
    m_wellDates.swap(validWellDates);

    if (m_wellDates.empty())
    {
        *errorMsg = QString("'%1' contains no valid FMU RFT data").arg(m_filePath);
        return STATUS_WARNING;
    }

    return STATUS_OK;
}



//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> RifReaderFmuRft::availableTimeSteps(const QString& wellName) const
{
    std::set<QDateTime> dates;
    for (auto wellDatePair : m_wellDates)
    {
        dates.insert(wellDatePair.second.date);
    }
    return  dates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RifReaderFmuRft::wellNamesWithRftData() const
{
    std::set<QString> wellNames;
    for (auto wellDatePair : m_wellDates)
    {
        wellNames.insert(wellDatePair.first);
    }
    return wellNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderFmuRft::Status RifReaderFmuRft::observations(const QString&            wellName,
                                                      WellDate*                 wellDate,
                                                      std::vector<Observation>* observations,
                                                      QString*                  errorMsg) const
{
    CVF_ASSERT(observations && errorMsg);

    auto it = m_wellDates.find(wellName);
    if (it == m_wellDates.end())
    {
        *errorMsg = QString("No such well name '%1'").arg(wellName);
        return STATUS_ERROR;
    }

    *wellDate = it->second;

    QDir      dir(m_filePath);

    QString txtFileName = dir.absoluteFilePath(QString("%1.txt").arg(wellName));
    QString obsFileName = dir.absoluteFilePath(QString("%1.obs").arg(wellName));

    if (STATUS_ERROR == readTxtFile(txtFileName, observations, errorMsg))
    {
        return STATUS_ERROR;
    }

    if (STATUS_ERROR == readObsFile(obsFileName, observations, errorMsg))
    {
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderFmuRft::Status RifReaderFmuRft::loadWellDates(QDir& dir, QString* errorMsg)
{
    CVF_ASSERT(errorMsg);

    QFileInfo wellDateFileInfo(dir.absoluteFilePath("well_date_rft.txt"));
    if (!(wellDateFileInfo.exists() && wellDateFileInfo.isFile() && wellDateFileInfo.isReadable()))
    {
        *errorMsg = QString("well_date_rft.txt cannot be found at '%s'").arg(m_filePath);
        return STATUS_ERROR;
    }

    {
        QFile wellDateFile(wellDateFileInfo.absoluteFilePath());
        if (!wellDateFile.open(QIODevice::Text | QIODevice::ReadOnly))
        {
            *errorMsg = QString("Could not read '%1'").arg(wellDateFileInfo.absoluteFilePath());
            return STATUS_ERROR;
        }
        QTextStream fileStream(&wellDateFile);
        while (true)
        {
            QString line = fileStream.readLine();
            if (line.isNull())
            {
                break;
            }
            else
            {
                QTextStream lineStream(&line);

                QString wellName;
                int     day, month, year, measurementIndex;

                lineStream >> wellName >> day >> month >> year >> measurementIndex;
                if (lineStream.status() != QTextStream::Ok)
                {
                    *errorMsg = QString("Failed to parse '%1'").arg(wellDateFileInfo.absoluteFilePath());
                    return STATUS_ERROR;
                }

                QDateTime dateTime(QDate(year, month, day));
                WellDate  wellDate = {wellName, dateTime, measurementIndex};
                m_wellDates[wellName] = wellDate;
            }
        }
    }
  
    return STATUS_OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderFmuRft::Status RifReaderFmuRft::readTxtFile(const QString& fileName, std::vector<Observation>* observations, QString* errorMsg) const
{
    CVF_ASSERT(observations);

    QFile file(fileName);
    if (!(file.open(QIODevice::Text | QIODevice::ReadOnly)))
    {
        *errorMsg = QString("Could not open '%1'").arg(fileName);
        return STATUS_ERROR;
    }
    QTextStream stream(&file);
    while (true)
    {
        QString line = stream.readLine();
        if (line.isNull())
        {
            break;
        }
        else
        {
            QTextStream lineStream(&line);

            double  utmx, utmy, mdrkb, tvdmsl;
            QString formationName;

            lineStream >> utmx >> utmy >> mdrkb >> tvdmsl >> formationName;

            if (lineStream.status() != QTextStream::Ok)
            {
                *errorMsg = QString("Failed to parse '%1'").arg(fileName);
                return STATUS_ERROR;
            }

            Observation observation = {utmx,
                                       utmy,
                                       mdrkb,
                                       tvdmsl,
                                       -std::numeric_limits<double>::infinity(),
                                       -std::numeric_limits<double>::infinity(),
                                       formationName};
            observations->push_back(observation);
        }
    }
    return STATUS_OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderFmuRft::Status RifReaderFmuRft::readObsFile(const QString& fileName, std::vector<Observation>* observations, QString* errorMsg) const
{
    CVF_ASSERT(observations);

    QFile file(fileName);
    if (!(file.open(QIODevice::Text | QIODevice::ReadOnly)))
    {
        *errorMsg = QString("Could not open '%1'").arg(fileName);
        return STATUS_ERROR;
    }

    size_t lineNumber = 0u;

    QTextStream stream(&file);
    while (true)
    {
        if (lineNumber >= observations->size())
        {
            *errorMsg = QString("Too many lines in '%1'").arg(fileName);
            return STATUS_ERROR;
        }

        QString line = stream.readLine();
        if (line.isNull())
        {
            break;
        }
        else
        {
            QTextStream lineStream(&line);

            double  pressure, pressure_error;

            lineStream >> pressure >> pressure_error;

            if (lineStream.status() != QTextStream::Ok)
            {
                *errorMsg = QString("Failed to parse '%1'").arg(fileName);
                return STATUS_ERROR;
            }

            Observation& observation   = observations->at(lineNumber);
            observation.pressure       = pressure;
            observation.pressure_error = pressure_error;
        }
        lineNumber++;
    }

    if (lineNumber != observations->size())
    {
        *errorMsg = QString("Not enough lines in '%1'").arg(fileName);
        return STATUS_ERROR;
    }
    return STATUS_OK;
}
