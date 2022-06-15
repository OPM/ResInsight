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

#include <QString>

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigThermalFractureResult
{
public:
    RigThermalFractureResult( const QString& name, const QString& unit )
        : m_name( name )
        , m_unit( unit )
    {
    }

    QString name() { return m_name; }
    QString unit() { return m_unit; }

    void appendValue( int nodeIndex, int timeStepIndex, double value )
    {
        if ( nodeIndex >= static_cast<int>( parameterValues.size() ) )
            parameterValues.push_back( { value } );
        else
            parameterValues[nodeIndex].push_back( value );
    }

    double getValue( int nodeIndex, int timeStepIndex ) const { return parameterValues[nodeIndex][timeStepIndex]; }

    size_t numNodes() const { return parameterValues.size(); }

private:
    QString m_name;
    QString m_unit;

    // Vector for each time step for each node
    std::vector<std::vector<double>> parameterValues;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RigThermalFractureDefinition
{
public:
    RigThermalFractureDefinition();
    ~RigThermalFractureDefinition();

    void    setName( const QString& name );
    QString name() const;

    const std::vector<double>& timeSteps() const;
    void                       addTimeStep( double timeStep );
    size_t                     numTimeSteps() const;

    size_t numNodes() const;

    void addProperty( const QString& name, const QString& unit );

    std::vector<std::pair<QString, QString>> getPropertyNamesUnits() const;

    void appendPropertyValue( int propertyIndex, int nodeIndex, int timeStepIndex, double value );

    double getPropertyValue( int propertyIndex, int nodeIndex, int timeStepIndex ) const;

private:
    QString m_name;

    std::vector<double>                   m_timeSteps;
    std::vector<RigThermalFractureResult> m_results;
};
