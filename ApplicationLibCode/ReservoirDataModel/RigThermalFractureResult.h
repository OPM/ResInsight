/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 -     Equinor ASA
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

#include "cvfVector3.h"

#include <QString>

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigThermalFractureResult
{
public:
    RigThermalFractureResult( const QString& name, const QString& unit );

    QString name() const;
    QString unit() const;

    void   appendValue( int nodeIndex, double value );
    double getValue( int nodeIndex, int timeStepIndex ) const;

    size_t numNodes() const;

private:
    QString m_name;
    QString m_unit;

    // Vector for each time step for each node
    std::vector<std::vector<double>> m_parameterValues;
};
