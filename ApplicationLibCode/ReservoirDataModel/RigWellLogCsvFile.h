/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RigWellLogFile.h"

#include "RiaDefines.h"

#include <QStringList>

#include <map>
#include <vector>

class RigWellPath;

//==================================================================================================
///
//==================================================================================================
class RigWellLogCsvFile : public RigWellLogFile
{
public:
    RigWellLogCsvFile();
    ~RigWellLogCsvFile() override;

    bool open( const QString& fileName, RigWellPath* wellPath, QString* errorMessage );

    QStringList wellLogChannelNames() const override;

    std::vector<double> depthValues() const override;
    std::vector<double> tvdMslValues() const override;
    std::vector<double> tvdRkbValues() const override;

    std::vector<double> values( const QString& name ) const override;

    QString                   wellLogChannelUnitString( const QString& wellLogChannelName ) const override;
    RiaDefines::DepthUnitType depthUnit() const;

    bool hasTvdMslChannel() const override;
    bool hasTvdRkbChannel() const override;

    double getMissingValue() const override;

private:
    void    close();
    QString depthUnitString() const override;

    static RigWellPath*        resampleWellPath( const RigWellPath& wellPath, double samplingInterval );
    static std::vector<double> resampleMeasuredDepths( const std::vector<double>& measuredDepths, double samplingInterval );

    QStringList                            m_wellLogChannelNames;
    QString                                m_depthLogName;
    QString                                m_tvdMslLogName;
    QString                                m_tvdRkbLogName;
    std::map<QString, std::vector<double>> m_values;
    std::map<QString, QString>             m_units;
};
