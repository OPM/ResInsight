/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 -     Equinor ASA
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

#include <QDateTime>
#include <QString>

#include <memory>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigPressureDepthData
{
public:
    RigPressureDepthData();
    ~RigPressureDepthData();

    void    setWellName( const QString& name );
    QString wellName() const;

    void      setTimeStep( const QDateTime& timeStep );
    QDateTime timeStep() const;

    void                                   addPressureAtDepth( double pressure, double depth );
    std::vector<std::pair<double, double>> getPressureDepthValues() const;

    std::vector<double> tvdmsl() const;
    std::vector<double> pressure() const;

private:
    QString                                m_wellName;
    QDateTime                              m_timeStep;
    std::vector<std::pair<double, double>> m_values;
};
