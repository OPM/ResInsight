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
    : m_filePath(filePath)
{
    loadWellNames();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderFmuRft::valid() const
{
    QFileInfo fileInfo(m_filePath);
    return fileInfo.exists() && fileInfo.isDir() && fileInfo.isReadable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> RifReaderFmuRft::availableTimeSteps(const QString& wellName) const
{

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::set<QString>& RifReaderFmuRft::wellNamesWithRftData() const {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QDateTime, std::vector<RifReaderFmuRftObservation>> RifReaderFmuRft::observations(const QString& wellName) const {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderFmuRft::loadWellNames()
{
    if (valid())
    {
        QFileInfo fileInfo(m_filePath);
        QDir dir = fileInfo.dir();

        QStringList obsFileFilter;
        obsFileFilter << "*.obs";

        QStringList obsFiles = dir.entryList(obsFileFilter, QDir::Files, QDir::Name);

        for (QString obsFile : obsFiles)
        {
            QFileInfo obsFileInfo(dir.absoluteFilePath(obsFile));
            if (obsFileInfo.isFile() && obsFileInfo.isReadable())
            {
                QString baseName = obsFileInfo.completeBaseName();
                QString txtFile = QString("%1.%2").arg(baseName).arg("txt");
                QFileInfo txtFileInfo(dir.absoluteFilePath(txtFile));
                if (txtFileInfo.exists() && txtFileInfo.isFile() && txtFileInfo.isReadable())
                {
                    m_wellNames.insert(baseName);
                }
                else
                {
                    // TODO: Report warning
                }
            }
            else
            {
                // TODO: Report warning
            }
        }
    }
    // TODO: Report error
}
