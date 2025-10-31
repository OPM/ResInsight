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

#include "RifThermalFractureReader.h"

#include "RiaDefines.h"
#include "RiaFractureDefines.h"
#include "RiaTextStringTools.h"
#include "RiaThermalFractureDefines.h"

#include "RigThermalFractureDefinition.h"

#include "RifFileParseTools.h"

#include "cafAssert.h"

#include <QDateTime>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::shared_ptr<RigThermalFractureDefinition>, QString> RifThermalFractureReader::readFractureCsvFile( const QString& filePath )
{
    QFile file( filePath );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        return std::make_pair( nullptr, QString( "Unable to open file: %1" ).arg( filePath ) );
    }

    std::shared_ptr<RigThermalFractureDefinition> definition = std::make_shared<RigThermalFractureDefinition>();

    QString separator = ",";

    auto appendPropertyValues = [definition]( int nodeIndex, int valueOffset, const QStringList& values, double conductivityFactor )
    {
        CAF_ASSERT( valueOffset <= values.size() );
        for ( int i = valueOffset; i < values.size(); i++ )
        {
            bool   isOk  = false;
            double value = values[i].toDouble( &isOk );
            if ( isOk )
            {
                int propertyIndex = i - valueOffset;

                // Convert conductivity from Darcy to milliDarcy
                if ( definition->getPropertyIndex( "Conductivity" ) == propertyIndex )
                {
                    value *= conductivityFactor;
                }

                definition->appendPropertyValue( propertyIndex, nodeIndex, value );
            }
        }
    };

    QTextStream in( &file );
    int         lineNumber = 1;

    // The two items in the csv is name and timestep
    const int valueOffset        = 2;
    int       nodeIndex          = 0;
    bool      isFirstHeader      = true;
    bool      isValidNode        = false;
    double    conductivityFactor = 1.0;
    while ( !in.atEnd() )
    {
        QString line = in.readLine();
        if ( lineNumber == 1 )
        {
            // The first line is the name of the fracture
            definition->setName( line );
        }
        else if ( isHeaderLine( line ) )
        {
            QStringList headerValues = RifFileParseTools::splitLineAndTrim( line, separator );
            if ( isFirstHeader )
            {
                // Create the result vector when encountering the first header
                for ( int i = valueOffset; i < headerValues.size(); i++ )
                {
                    auto [name, unit] = parseNameAndUnit( headerValues[i] );
                    if ( !name.isEmpty() && !unit.isEmpty() )
                    {
                        // Special handling for Conductivity: change unit from Darcy to Milli
                        if ( name.contains( RiaDefines::conductivityResultName(), Qt::CaseInsensitive ) )
                        {
                            // Check if the conductivity unit needs conversion to milliDarcy
                            if ( !unit.contains( "mD" ) )
                            {
                                conductivityFactor = 1.0e3;
                            }

                            // Use the preferred internal unit for conductivity
                            unit = RiaDefines::unitStringConductivity( detectUnitSystem( definition ) );
                        }

                        definition->addProperty( name, unit );
                    }
                }

                // Detect unit system
                RiaDefines::EclipseUnitSystem unitSystem = detectUnitSystem( definition );
                if ( unitSystem == RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN )
                {
                    return std::make_pair( nullptr, QString( "Unknown unit system found in file: %1" ).arg( filePath ) );
                }

                // Verify that all values have consistent units:
                // all values should be either metric or field, and mixing is not allowed
                bool isUnitsConsistent = checkUnits( definition, unitSystem );
                if ( !isUnitsConsistent )
                {
                    return std::make_pair( nullptr, QString( "Inconsistent units found in file: %1" ).arg( filePath ) );
                }

                definition->setUnitSystem( unitSystem );
                isFirstHeader = false;
            }
            else if ( isValidNode )
            {
                nodeIndex++;
            }
        }
        else if ( isCenterNodeLine( line ) )
        {
            // The first node is the center node
            auto values = RifFileParseTools::splitLineAndTrim( line, separator );

            // Second is the timestamp
            QDateTime dateTime = parseDateTime( values[1] );
            definition->addTimeStep( dateTime.toSecsSinceEpoch() );

            //
            appendPropertyValues( nodeIndex, valueOffset, values, conductivityFactor );
            isValidNode = true;
        }
        else if ( isInternalNodeLine( line ) || isPerimeterNodeLine( line ) )
        {
            auto values = RifFileParseTools::splitLineAndTrim( line, separator );
            appendPropertyValues( nodeIndex, valueOffset, values, conductivityFactor );
            isValidNode = true;
        }
        else
        {
            isValidNode = false;
        }

        lineNumber++;
    }

    if ( definition->numTimeSteps() == 0 )
    {
        return std::make_pair( nullptr, QString( "No time steps found: %1" ).arg( filePath ) );
    }

    return std::make_pair( definition, "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDateTime RifThermalFractureReader::parseDateTime( const QString& dateString )
{
    QString   dateFormat = "dd.MM.yyyy hh:mm:ss";
    QDateTime dateTime   = QDateTime::fromString( dateString, dateFormat );

    // Sometimes the datetime field is missing time
    if ( !dateTime.isValid() )
    {
        QString dateFormat = "dd.MM.yyyy";
        dateTime           = QDateTime::fromString( dateString, dateFormat );
    }

    // Sometimes the datetime field has different year definition
    if ( !dateTime.isValid() )
    {
        QString dateFormat = "dd-MM-yyyy hh:mm:ss";
        dateTime           = QDateTime::fromString( dateString, dateFormat );
    }

    if ( !dateTime.isValid() )
    {
        QString dateFormat = "dd-MM-yyyy";
        dateTime           = QDateTime::fromString( dateString, dateFormat );
    }

    return dateTime;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifThermalFractureReader::isHeaderLine( const QString& line )
{
    return line.contains( "XCoord" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifThermalFractureReader::isCenterNodeLine( const QString& line )
{
    return line.contains( "Centre Node" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifThermalFractureReader::isInternalNodeLine( const QString& line )
{
    return line.contains( "Internal Node" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifThermalFractureReader::isPerimeterNodeLine( const QString& line )
{
    std::vector<QString> validPerimeterNames = { "Perimeter Node", "Bottom Node", "Top Node", "Right Node", "Left Node" };

    bool result =
        std::any_of( validPerimeterNames.begin(), validPerimeterNames.end(), [line]( const QString& str ) { return line.contains( str ); } );

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<QString, QString> RifThermalFractureReader::parseNameAndUnit( const QString& value )
{
    // Expected values: "name(unit)" or "name (unit)"
    QRegularExpression re( "(\\w*)\\s?\\(([^\\)]+)\\)" );

    QRegularExpressionMatch match = re.match( value );
    if ( match.hasMatch() )
    {
        QString name = match.captured( 1 );
        QString unit = match.captured( 2 );
        return std::make_pair( name, unit );
    }

    return std::make_pair( "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RifThermalFractureReader::detectUnitSystem( std::shared_ptr<const RigThermalFractureDefinition> definition )
{
    // Use XCoord property to determine expected unit for entire file
    QString targetName    = "XCoord";
    auto    namesAndUnits = definition->getPropertyNamesUnits();
    auto    res = std::find_if( namesAndUnits.begin(), namesAndUnits.end(), [&]( const auto& val ) { return val.first == targetName; } );

    if ( res != namesAndUnits.end() )
    {
        QString unit = res->second;
        if ( unit == RiaDefines::getExpectedThermalFractureUnit( targetName, RiaDefines::EclipseUnitSystem::UNITS_METRIC ) )
            return RiaDefines::EclipseUnitSystem::UNITS_METRIC;

        if ( unit == RiaDefines::getExpectedThermalFractureUnit( targetName, RiaDefines::EclipseUnitSystem::UNITS_FIELD ) )
            return RiaDefines::EclipseUnitSystem::UNITS_FIELD;
    }

    return RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifThermalFractureReader::checkUnits( std::shared_ptr<const RigThermalFractureDefinition> definition,
                                           RiaDefines::EclipseUnitSystem                       unitSystem )
{
    auto namesAndUnits = definition->getPropertyNamesUnits();
    for ( auto [name, unit] : namesAndUnits )
    {
        auto expectedUnit = RiaDefines::getExpectedThermalFractureUnit( name, unitSystem );
        if ( expectedUnit != unit ) return false;
    }

    return true;
}
