/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RiaDefines.h"

#include <QStringList>
#include <vector>

namespace NRLib
{
class Well;
}

class RimWellLogCurve;

//==================================================================================================
///
//==================================================================================================
class RigWellLogLasFile : public RigWellLogData
{
public:
    RigWellLogLasFile();
    ~RigWellLogLasFile() override;

    bool open( const QString& fileName, QString* errorMessage );

    QString     wellName() const;
    QString     date() const;
    QStringList wellLogChannelNames() const override;

    std::vector<double> depthValues() const override;
    std::vector<double> tvdMslValues() const override;
    std::vector<double> tvdRkbValues() const override;

    std::vector<double> values( const QString& name ) const override;

    QString wellLogChannelUnitString( const QString& wellLogChannelName ) const override;

    bool hasTvdMslChannel() const override;
    bool hasTvdRkbChannel() const override;

    double getMissingValue() const override;

private:
    void    close();
    QString depthUnitString() const override;

    NRLib::Well* m_wellLogFile;
    QStringList  m_wellLogChannelNames;
    QString      m_depthLogName;
    QString      m_tvdMslLogName;
    QString      m_tvdRkbLogName;
};
