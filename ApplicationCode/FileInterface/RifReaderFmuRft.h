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

struct RifReaderFmuRftObservation
{
    double utmx;
    double utmy;
    double mdrkb;
    double tvdmsl;

    double pressure;
    double pressure_error;

    QString formation;
};

//==================================================================================================
//
//
//==================================================================================================
class RifReaderFmuRft : public cvf::Object
{
public:
    RifReaderFmuRft(const QString& filePath);
    ~RifReaderFmuRft() = default;

    bool valid() const;

    std::set<QDateTime>      availableTimeSteps(const QString& wellName) const;
    const std::set<QString>& wellNamesWithRftData() const;

    std::map<QDateTime, std::vector<RifReaderFmuRftObservation>> observations(const QString& wellName) const;

private:
    void loadWellNames();

private:
    QString                             m_filePath;
    std::map<QString, QDateTime>        m_wellNames;
};



