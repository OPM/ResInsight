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

#include <QDir>
#include <QFileInfo>

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
RifReaderFmuRft::Status RifReaderFmuRft::status() const
{
    return m_status;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderFmuRft::Status RifReaderFmuRft::initialize(QString* errorMsg)
{
    CAF_ASSERT(errorMsg);

    QFileInfo fileInfo(m_filePath);
    if (!fileInfo.exists() && fileInfo.isDir() && fileInfo.isReadable())
    {
        *errorMsg = QString("Directory '%s' does not exist or isn't readable").arg(m_filePath);
        return STATUS_ERROR;
    }

    QDir dir(fileInfo.dir());

    if (STATUS_ERROR == readWellDates(dir, errorMsg))
    {
        return STATUS_ERROR;
    }
   
    std::map<QString, std::vector<WellDate>> validWellDates;
    for (auto wellDatePair : m_wellDates)
    {
        QString txtFile = QString("%1.txt").arg(wellDatePair.first.wellName);
        QString obsFile = QString("%1.obs").arg(wellDatePair.first.wellName);
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
    for (const WellDate& wellDate : m_wellDates[wellName])
    {
        dates.insert(wellDate.date);
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
        wellNames.insert(wellDate.wellName);
    }
    return wellNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderFmuRft::Status RifReaderFmuRft::observations(const QString&                                 wellName,
                                                      std::map<QDateTime, std::vector<Observation>>* observations,
                                                      QString*                                       errorMsg) const
{

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderFmuRft::Status RifReaderFmuRft::readWellDates(QDir& dir, QString* errorMsg)
{
    CAF_ASSERT(errorMsg);

    QFileInfo wellDateFileInfo(dir.absoluteFilePath("well_date_rft.txt"));
    if (!(wellDateFileInfo.exists() && wellDateFileInfo.isFile() && wellDateFileInfo.isReadable()))
    {
        *errorMsg = QString("well_date_rft.txt cannot be found at '%s'").arg(m_filePath);
        return STATUS_ERROR;
    }

    QStringList wellTimeStepLines;
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
                wellTimeStepLines.push_back(line);
            }
        }
    }

    for (QString line : wellTimeStepLines)
    {
        QTextStream lineStream(&line);

        QString wellName;
        int day, month, year, measurementIndex;

        lineStream >> wellName >> day >> month >> year;
        if (lineStream.status() != QTextStream::Ok)
        {
            *errorMsg = QString("Failed to parse '%1'").arg(wellDateFileInfo.absoluteFilePath();
            return STATUS_ERROR;
        }

        QDateTime dateTime(QDate(year, month, day));
        WellDate wellDate = { wellName, dateTime, measurementIndex };
        m_wellDates[wellName].push_back(wellDate);
    }

    return STATUS_OK;
}
