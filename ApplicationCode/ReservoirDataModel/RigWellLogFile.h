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

#include "RiaDefines.h"

#include "cvfBase.h"
#include "cvfObject.h"

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
class RigWellLogFile : public cvf::Object
{
public:
    RigWellLogFile();
    virtual ~RigWellLogFile();

    bool open(const QString& fileName, QString* errorMessage);

    QString     wellName() const;
    QString     date() const;
    QStringList wellLogChannelNames() const;

    std::vector<double> depthValues() const;
    std::vector<double> tvdMslValues() const;
    std::vector<double> values(const QString& name) const;

    QString wellLogChannelUnitString(const QString& wellLogChannelName, RiaDefines::DepthUnitType displayDepthUnit) const;
    RiaDefines::DepthUnitType depthUnit() const;

    bool        hasTvdChannel() const;

private:
    void    close();
    QString depthUnitString() const;

    NRLib::Well*    m_wellLogFile;
    QStringList     m_wellLogChannelNames;
    QString         m_depthLogName;
    QString         m_tvdMslLogName;
};
