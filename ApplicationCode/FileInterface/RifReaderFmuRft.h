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
#pragma once

#include "RifEclipseRftAddress.h"

#include <map>
#include <string>
#include <vector>

#include "cvfBase.h"
#include "cvfObject.h"

#include <QDateTime>
#include <QDir>

//==================================================================================================
//
//
//==================================================================================================
class RifReaderFmuRft : public cvf::Object
{
public:
    enum Status
    {
        UNINITIALIZED  = 0,
        STATUS_OK      = 1,
        STATUS_WARNING = 2,
        STATUS_ERROR   = 3
    };

    struct Observation
    {
        double utmx;
        double utmy;
        double mdrkb;
        double tvdmsl;

        double pressure;
        double pressure_error;

        QString formation;
    };

    struct WellDate
    {
        QString   wellName;
        QDateTime date;
        int       measurementIndex;
    };

public:
    RifReaderFmuRft(const QString& filePath);
    ~RifReaderFmuRft() = default;

    static QStringList findSubDirectoriesWithFmuRftData(const QString& filePath);
    static bool        directoryContainsFmuRftData(const QString& filePath);

    Status status() const;
    Status initialize(QString* errorMsg);

    std::set<QDateTime> availableTimeSteps(const QString& wellName) const;
    std::set<QString>   wellNamesWithRftData() const;

    Status observations(const QString& wellName, WellDate* wellDate, std::vector<Observation>* observations, QString* errorMsg) const;

private:
    Status loadWellDates(QDir& dir, QString* errorMsg);
    Status readTxtFile(const QString& txtFileName, std::vector<Observation>* observations, QString* errorMsg) const;
    Status readObsFile(const QString& obsFileName, std::vector<Observation>* observations, QString* errorMsg) const;
private:
    Status                      m_status;
    QString                     m_filePath;
    std::map<QString, WellDate> m_wellDates;
};
