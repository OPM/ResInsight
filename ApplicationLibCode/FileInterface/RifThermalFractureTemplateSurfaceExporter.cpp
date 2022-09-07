/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022-    Equinor ASA
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

#include "RifThermalFractureTemplateSurfaceExporter.h"

#include "RigThermalFractureDefinition.h"
#include "RimThermalFractureTemplate.h"

#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifThermalFractureTemplateSurfaceExporter::writeToFile( RimThermalFractureTemplate* fractureTemplate,
                                                             int                         timeStepIndex,
                                                             const QString&              filePath )
{
    auto fractureData = fractureTemplate->fractureDefinition();
    auto numNodes     = fractureData->numNodes();
    auto numTimeSteps = fractureData->numTimeSteps();
    auto properties   = fractureData->getPropertyNamesUnits();

    if ( timeStepIndex < 0 || timeStepIndex >= static_cast<int>( numTimeSteps ) ) return false;

    QFile file( filePath );
    if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        QTextStream out( &file );

        out.setRealNumberPrecision( 16 );

        auto nameAndUnits  = fractureData->getPropertyNamesUnits();
        int  numProperties = nameAndUnits.size();

        for ( auto [name, unit] : nameAndUnits )
        {
            out << name << " ";
        }

        out << "\n";

        for ( int nodeIndex = 0; nodeIndex < static_cast<int>( numNodes ); nodeIndex++ )
        {
            for ( int propertyIndex = 0; propertyIndex < numProperties; propertyIndex++ )
            {
                double value = fractureData->getPropertyValue( propertyIndex, nodeIndex, timeStepIndex );
                out << value;
                if ( propertyIndex < numProperties - 1 ) out << " ";
            }

            out << "\n";
        }
    }
    file.close();

    return true;
}
