/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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

#include "RifStimPlanXmlReader.h"

#include "RiaDefines.h"
#include "RiaEclipseUnitTools.h"
#include "RiaFractureDefines.h"
#include "RiaLogging.h"
#include "RiaTextStringTools.h"

#include "RigStimPlanFractureDefinition.h"

#include "cafAppEnum.h"

#include <QFile>
#include <QStringView>
#include <QXmlStreamReader>

#include <cmath> // Needed for HUGE_VAL on Linux

//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------
bool                                       hasNegativeValues( std::vector<double> xs );
RigStimPlanFractureDefinition::Orientation mapTextToOrientation( const QString text );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigStimPlanFractureDefinition> RifStimPlanXmlReader::readStimPlanXMLFile( const QString&                stimPlanFileName,
                                                                                   double                        conductivityScalingFactor,
                                                                                   MirrorMode                    mirrorMode,
                                                                                   RiaDefines::EclipseUnitSystem requiredUnit,
                                                                                   QString*                      errorMessage )
{
    RiaLogging::info( QString( "Starting to open StimPlan XML file: '%1'" ).arg( stimPlanFileName ) );

    cvf::ref<RigStimPlanFractureDefinition> stimPlanFileData = new RigStimPlanFractureDefinition;
    {
        QFile dataFile( stimPlanFileName );
        if ( !dataFile.open( QFile::ReadOnly ) )
        {
            if ( errorMessage ) ( *errorMessage ) += "Could not open the File: " + ( stimPlanFileName ) + "\n";
            return nullptr;
        }

        QXmlStreamReader xmlStream;
        xmlStream.setDevice( &dataFile );
        xmlStream.readNext();
        readStimplanGridAndTimesteps( xmlStream, stimPlanFileData.p(), mirrorMode, requiredUnit );

        caf::AppEnum<RiaDefines::EclipseUnitSystem> unitSystem = stimPlanFileData->unitSet();

        if ( unitSystem != RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN )
            RiaLogging::info(
                QString( "Setting unit system for StimPlan fracture template %1 to %2" ).arg( stimPlanFileName ).arg( unitSystem.uiText() ) );
        else
            RiaLogging::error( QString( "Found invalid units for %1. Unit system not set." ).arg( stimPlanFileName ) );

        if ( xmlStream.hasError() )
        {
            RiaLogging::error( QString( "Failed to parse file '%1'" ).arg( dataFile.fileName() ) );
            RiaLogging::error( xmlStream.errorString() );
        }
        dataFile.close();
    }

    size_t numberOfYValues = stimPlanFileData->yCount();
    RiaLogging::debug(
        QString( "Grid size X: %1, Y: %2" ).arg( QString::number( stimPlanFileData->xCount() ), QString::number( numberOfYValues ) ) );

    size_t numberOfTimeSteps = stimPlanFileData->timeSteps().size();
    RiaLogging::debug( QString( "Number of time-steps: %1" ).arg( numberOfTimeSteps ) );

    // Start reading from top:
    QFile dataFile( stimPlanFileName );

    if ( !dataFile.open( QFile::ReadOnly ) )
    {
        if ( errorMessage ) ( *errorMessage ) += "Could not open the File: " + ( stimPlanFileName ) + "\n";
        return nullptr;
    }

    QXmlStreamReader xmlStream2;
    xmlStream2.setDevice( &dataFile );
    QString parameter;
    QString unit;

    RiaLogging::info( QString( "Properties available in file:" ) );
    int propertiesElementCount = 0;
    while ( !xmlStream2.atEnd() && propertiesElementCount < 2 )
    {
        xmlStream2.readNext();

        if ( xmlStream2.isStartElement() )
        {
            if ( RiaTextStringTools::isTextEqual( xmlStream2.name(), QString( "properties" ) ) )
            {
                propertiesElementCount++;
            }
            else if ( RiaTextStringTools::isTextEqual( xmlStream2.name(), QString( "property" ) ) )
            {
                unit      = getAttributeValueString( xmlStream2, QString( "uom" ) );
                parameter = getAttributeValueString( xmlStream2, QString( "name" ) );

                RiaLogging::info( QString( "%1 [%2]" ).arg( parameter, unit ) );
            }
            else if ( RiaTextStringTools::isTextEqual( xmlStream2.name(), QString( "time" ) ) )
            {
                double timeStepValue = getAttributeValueDouble( xmlStream2, QString( "value" ) );

                std::vector<std::vector<double>> propertyValuesAtTimestep =
                    stimPlanFileData->generateDataLayoutFromFileDataLayout( getAllDepthDataAtTimeStep( xmlStream2 ) );

                bool valuesOK = stimPlanFileData->numberOfParameterValuesOK( propertyValuesAtTimestep );
                if ( !valuesOK )
                {
                    RiaLogging::error( QString( "Inconsistency detected in reading XML file: '%1'" ).arg( dataFile.fileName() ) );
                    return nullptr;
                }

                if ( parameter.contains( RiaDefines::conductivityResultName(), Qt::CaseInsensitive ) )
                {
                    // Scale all parameters containing conductivity

                    for ( auto& dataAtDepth : propertyValuesAtTimestep )
                    {
                        for ( auto& dataValue : dataAtDepth )
                        {
                            dataValue *= conductivityScalingFactor;
                        }
                    }
                }

                stimPlanFileData->setDataAtTimeValue( parameter, unit, propertyValuesAtTimestep, timeStepValue );
            }
        }
    }

    dataFile.close();

    if ( xmlStream2.hasError() )
    {
        RiaLogging::error( QString( "Failed to parse file: '%1'" ).arg( dataFile.fileName() ) );
        RiaLogging::error( xmlStream2.errorString() );
    }
    else if ( dataFile.error() != QFile::NoError )
    {
        RiaLogging::error( QString( "Cannot read file: '%1'" ).arg( dataFile.fileName() ) );
        RiaLogging::error( dataFile.errorString() );
    }
    else
    {
        RiaLogging::info( QString( "Successfully read XML file: '%1'" ).arg( stimPlanFileName ) );
    }

    return stimPlanFileData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifStimPlanXmlReader::readStimplanGridAndTimesteps( QXmlStreamReader&              xmlStream,
                                                         RigStimPlanFractureDefinition* stimPlanFileData,
                                                         MirrorMode                     mirrorMode,
                                                         RiaDefines::EclipseUnitSystem  requiredUnit )
{
    size_t startNegValuesYs = 0;

    xmlStream.readNext();

    double tvdToTopPerf = HUGE_VAL;
    double tvdToBotPerf = HUGE_VAL;
    double mdToTopPerf  = HUGE_VAL;
    double mdToBotPerf  = HUGE_VAL;
    double formationDip = HUGE_VAL;

    RigStimPlanFractureDefinition::Orientation orientation = RigStimPlanFractureDefinition::Orientation::UNDEFINED;

    int gridSectionCount = 0;

    // First, read time steps and grid to establish data structures for putting data into later.
    while ( !xmlStream.atEnd() && gridSectionCount < 2 )
    {
        xmlStream.readNext();

        if ( xmlStream.isStartElement() )
        {
            RiaDefines::EclipseUnitSystem destinationUnit = requiredUnit;

            if ( RiaTextStringTools::isTextEqual( xmlStream.name(), QString( "grid" ) ) )
            {
                // Support for one grid per file
                if ( gridSectionCount < 1 )
                {
                    QString gridunit = getAttributeValueString( xmlStream, QString( "uom" ) );

                    if ( gridunit.compare( "m", Qt::CaseInsensitive ) == 0 )
                        stimPlanFileData->m_unitSet = RiaDefines::EclipseUnitSystem::UNITS_METRIC;
                    else if ( gridunit.compare( "ft", Qt::CaseInsensitive ) == 0 )
                        stimPlanFileData->m_unitSet = RiaDefines::EclipseUnitSystem::UNITS_FIELD;
                    else
                        stimPlanFileData->m_unitSet = RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;

                    if ( destinationUnit == RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN )
                    {
                        // Use file unit set if requested unit is unknown
                        destinationUnit = stimPlanFileData->m_unitSet;
                    }

                    double tvdToTopPerfFt = getAttributeValueDouble( xmlStream, "TVDToTopPerfFt" );
                    double tvdToBotPerfFt = getAttributeValueDouble( xmlStream, "TVDToBottomPerfFt" );

                    tvdToTopPerf = RifStimPlanXmlReader::valueInRequiredUnitSystem( RiaDefines::EclipseUnitSystem::UNITS_FIELD,
                                                                                    destinationUnit,
                                                                                    tvdToTopPerfFt );
                    tvdToBotPerf = RifStimPlanXmlReader::valueInRequiredUnitSystem( RiaDefines::EclipseUnitSystem::UNITS_FIELD,
                                                                                    destinationUnit,
                                                                                    tvdToBotPerfFt );
                }

                gridSectionCount++;
            }
            else if ( RiaTextStringTools::isTextEqual( xmlStream.name(), QString( "perf" ) ) )
            {
                QString perfUnit = getAttributeValueString( xmlStream, QString( "uom" ) );
                QString fracName = getAttributeValueString( xmlStream, QString( "frac" ) );
            }
            else if ( RiaTextStringTools::isTextEqual( xmlStream.name(), QString( "topTVD" ) ) )
            {
                auto valText = xmlStream.readElementText();
                tvdToTopPerf = valText.toDouble();
            }
            else if ( RiaTextStringTools::isTextEqual( xmlStream.name(), QString( "bottomTVD" ) ) )
            {
                auto valText = xmlStream.readElementText();
                tvdToBotPerf = valText.toDouble();
            }
            else if ( RiaTextStringTools::isTextEqual( xmlStream.name(), QString( "topMD" ) ) )
            {
                auto valText = xmlStream.readElementText();
                mdToTopPerf  = valText.toDouble();
            }
            else if ( RiaTextStringTools::isTextEqual( xmlStream.name(), QString( "bottomMD" ) ) )
            {
                auto valText = xmlStream.readElementText();
                mdToBotPerf  = valText.toDouble();
            }
            else if ( RiaTextStringTools::isTextEqual( xmlStream.name(), QString( "FmDip" ) ) )
            {
                auto valText = xmlStream.readElementText();
                formationDip = valText.toDouble();
            }
            else if ( RiaTextStringTools::isTextEqual( xmlStream.name(), QString( "orientation" ) ) )
            {
                auto valText = xmlStream.readElementText();
                orientation  = mapTextToOrientation( valText.trimmed() );
            }
            else if ( RiaTextStringTools::isTextEqual( xmlStream.name(), QString( "xs" ) ) )
            {
                std::vector<double> gridValuesXs;
                {
                    size_t              dummy;
                    std::vector<double> gridValues;
                    getGriddingValues( xmlStream, gridValues, dummy );

                    gridValuesXs = RifStimPlanXmlReader::valuesInRequiredUnitSystem( stimPlanFileData->m_unitSet, destinationUnit, gridValues );
                }

                stimPlanFileData->m_fileXs = gridValuesXs;

                stimPlanFileData->generateXsFromFileXs( mirrorMode == MirrorMode::MIRROR_AUTO ? !hasNegativeValues( gridValuesXs )
                                                                                              : (bool)mirrorMode );
            }
            else if ( RiaTextStringTools::isTextEqual( xmlStream.name(), QString( "ys" ) ) )
            {
                std::vector<double> gridValuesYs;
                {
                    std::vector<double> gridValues;
                    getGriddingValues( xmlStream, gridValues, startNegValuesYs );

                    gridValuesYs = RifStimPlanXmlReader::valuesInRequiredUnitSystem( stimPlanFileData->m_unitSet, destinationUnit, gridValues );
                }

                // Reorder and change sign
                std::vector<double> ys;
                for ( double y : gridValuesYs )
                {
                    ys.insert( ys.begin(), -y );
                }
                stimPlanFileData->m_Ys = ys;
            }

            else if ( RiaTextStringTools::isTextEqual( xmlStream.name(), QString( "time" ) ) )
            {
                double timeStepValue = getAttributeValueDouble( xmlStream, "value" );
                stimPlanFileData->addTimeStep( timeStepValue );
            }
        }
    }

    if ( tvdToTopPerf != HUGE_VAL )
    {
        stimPlanFileData->setTvdToTopPerf( tvdToTopPerf );
    }

    if ( tvdToBotPerf != HUGE_VAL )
    {
        stimPlanFileData->setTvdToBottomPerf( tvdToBotPerf );
    }

    if ( mdToTopPerf != HUGE_VAL )
    {
        stimPlanFileData->setMdToTopPerf( mdToTopPerf );
    }

    if ( mdToBotPerf != HUGE_VAL )
    {
        stimPlanFileData->setMdToBottomPerf( mdToBotPerf );
    }

    if ( formationDip != HUGE_VAL )
    {
        stimPlanFileData->setFormationDip( formationDip );
    }

    if ( orientation != RigStimPlanFractureDefinition::Orientation::UNDEFINED )
    {
        stimPlanFileData->setOrientation( orientation );
    }

    if ( startNegValuesYs > 0 )
    {
        RiaLogging::error( QString( "Negative depth values detected in XML file" ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>> RifStimPlanXmlReader::getAllDepthDataAtTimeStep( QXmlStreamReader& xmlStream )
{
    std::vector<std::vector<double>> propertyValuesAtTimestep;

    while ( !( xmlStream.isEndElement() && RiaTextStringTools::isTextEqual( xmlStream.name(), QString( "time" ) ) ) )
    {
        xmlStream.readNext();

        if ( RiaTextStringTools::isTextEqual( xmlStream.name(), QString( "depth" ) ) )
        {
            xmlStream.readElementText().toDouble();
            std::vector<double> propertyValuesAtDepth;

            xmlStream.readNext(); // read end depth token
            xmlStream.readNext(); // read cdata section with values

            QString depthDataStr;
            if ( xmlStream.isCDATA() )
            {
                depthDataStr = xmlStream.text().toString();
            }
            else
            {
                QString gridValuesString = xmlStream.readElementText().replace( '\n', ' ' );
                gridValuesString         = gridValuesString.replace( '[', ' ' ).replace( ']', ' ' );

                depthDataStr = gridValuesString;
            }

            QStringList splitted = depthDataStr.split( ' ' );
            for ( int i = 0; i < splitted.size(); i++ )
            {
                QString value = splitted[i];
                if ( value != "" )
                {
                    propertyValuesAtDepth.push_back( value.toDouble() );
                }
            }

            propertyValuesAtTimestep.push_back( propertyValuesAtDepth );
        }
    }
    return propertyValuesAtTimestep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RifStimPlanXmlReader::valuesInRequiredUnitSystem( RiaDefines::EclipseUnitSystem sourceUnit,
                                                                      RiaDefines::EclipseUnitSystem requiredUnit,
                                                                      const std::vector<double>&    values )
{
    if ( sourceUnit == RiaDefines::EclipseUnitSystem::UNITS_FIELD && requiredUnit == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        std::vector<double> convertedValues;
        for ( const auto& valueInFeet : values )
        {
            convertedValues.push_back( RiaEclipseUnitTools::feetToMeter( valueInFeet ) );
        }

        return convertedValues;
    }
    else if ( sourceUnit == RiaDefines::EclipseUnitSystem::UNITS_METRIC && requiredUnit == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
    {
        std::vector<double> convertedValues;
        for ( const auto& valueInMeter : values )
        {
            convertedValues.push_back( RiaEclipseUnitTools::meterToFeet( valueInMeter ) );
        }

        return convertedValues;
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RifStimPlanXmlReader::valueInRequiredUnitSystem( RiaDefines::EclipseUnitSystem sourceUnit,
                                                        RiaDefines::EclipseUnitSystem requiredUnit,
                                                        double                        value )
{
    if ( sourceUnit == RiaDefines::EclipseUnitSystem::UNITS_FIELD && requiredUnit == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        return RiaEclipseUnitTools::feetToMeter( value );
    }
    else if ( sourceUnit == RiaDefines::EclipseUnitSystem::UNITS_METRIC && requiredUnit == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
    {
        return RiaEclipseUnitTools::meterToFeet( value );
    }

    return value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifStimPlanXmlReader::getGriddingValues( QXmlStreamReader& xmlStream, std::vector<double>& gridValues, size_t& startNegValues )
{
    QString gridValuesString = xmlStream.readElementText().replace( '\n', ' ' );
    gridValuesString         = gridValuesString.replace( '[', ' ' ).replace( ']', ' ' );

    for ( const QString& value : RiaTextStringTools::splitSkipEmptyParts( gridValuesString ) )
    {
        if ( value.size() > 0 )
        {
            double gridValue = value.toDouble();
            gridValues.push_back( gridValue );
            if ( gridValue < -RigStimPlanFractureDefinition::THRESHOLD_VALUE ) startNegValues++;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RifStimPlanXmlReader::getAttributeValueDouble( QXmlStreamReader& xmlStream, const QString& parameterName )
{
    double value = HUGE_VAL;
    for ( const QXmlStreamAttribute& attr : xmlStream.attributes() )
    {
        if ( RiaTextStringTools::isTextEqual( attr.name(), parameterName ) )
        {
            value = attr.value().toString().toDouble();
        }
    }
    return value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifStimPlanXmlReader::getAttributeValueString( QXmlStreamReader& xmlStream, const QString& parameterName )
{
    QString parameterValue;
    for ( const QXmlStreamAttribute& attr : xmlStream.attributes() )
    {
        if ( RiaTextStringTools::isTextEqual( attr.name(), parameterName ) )
        {
            parameterValue = attr.value().toString();
        }
    }
    return parameterValue;
}

//--------------------------------------------------------------------------------------------------
/// Internal function
//--------------------------------------------------------------------------------------------------
bool hasNegativeValues( std::vector<double> xs )
{
    return xs[0] < -RigStimPlanFractureDefinition::THRESHOLD_VALUE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigStimPlanFractureDefinition::Orientation mapTextToOrientation( const QString text )
{
    if ( text.compare( "transverse", Qt::CaseInsensitive ) == 0 )
    {
        return RigStimPlanFractureDefinition::Orientation::TRANSVERSE;
    }
    else if ( text.compare( "longitudinal", Qt::CaseInsensitive ) == 0 )
    {
        return RigStimPlanFractureDefinition::Orientation::LONGITUDINAL;
    }
    else
    {
        return RigStimPlanFractureDefinition::Orientation::UNDEFINED;
    }
}
