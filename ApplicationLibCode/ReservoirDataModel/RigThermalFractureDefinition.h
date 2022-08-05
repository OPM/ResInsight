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

#include "RigThermalFractureResult.h"

#include "cvfBoundingBox.h"
#include "cvfVector3.h"

#include <QString>

#include <vector>

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

    void appendPropertyValue( int propertyIndex, int nodeIndex, double value );

    double getPropertyValue( int propertyIndex, int nodeIndex, int timeStepIndex ) const;

    int getPropertyIndex( const QString& name ) const;

    std::vector<cvf::Vec3d> relativeCoordinates( int timeStepIndex ) const;

    cvf::Vec3d centerPosition() const;

    double minDepth( int timeStepIndex ) const;
    double maxDepth( int timeStepIndex ) const;

    void setUnitSystem( RiaDefines::EclipseUnitSystem unitSystem );

    RiaDefines::EclipseUnitSystem unitSystem() const;

private:
    cvf::BoundingBox getBoundingBox( int timeStepIndex ) const;

    QString m_name;

    RiaDefines::EclipseUnitSystem         m_unitSystem;
    std::vector<double>                   m_timeSteps;
    std::vector<RigThermalFractureResult> m_results;
};
