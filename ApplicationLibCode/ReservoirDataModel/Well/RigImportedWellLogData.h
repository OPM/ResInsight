/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RigWellLogData.h"

#include <QString>
#include <QStringList>

#include <map>
#include <vector>

//==================================================================================================
///
//==================================================================================================
class RigImportedWellLogData : public RigWellLogData
{
public:
    RigImportedWellLogData();
    ~RigImportedWellLogData() override;

    QStringList wellLogChannelNames() const override;

    std::vector<double> depthValues() const override;
    std::vector<double> tvdMslValues() const override;
    std::vector<double> tvdRkbValues() const override;

    std::vector<double> values( const QString& name ) const override;

    QString wellLogChannelUnitString( const QString& wellLogChannelName ) const override;
    QString depthUnitString() const override;

    bool hasTvdMslChannel() const override;
    bool hasTvdRkbChannel() const override;

    double getMissingValue() const override;

    void setChannelData( const QString& channelName, const std::vector<double>& values );
    void setDepthValues( const std::vector<double>& depthValues );

private:
    std::vector<double>                    m_depthValues;
    std::map<QString, std::vector<double>> m_channelData;

    static const double MISSING_VALUE;
};