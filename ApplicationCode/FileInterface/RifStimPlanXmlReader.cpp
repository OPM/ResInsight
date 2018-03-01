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

#include "RiaEclipseUnitTools.h"
#include "RiaFractureDefines.h"
#include "RiaLogging.h"
#include "RigStimPlanFractureDefinition.h"

#include <QFile>
#include <QXmlStreamReader>

#include <cmath> // Needed for HUGE_VAL on Linux


//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------
bool hasNegativeValues(std::vector<double> xs);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigStimPlanFractureDefinition> RifStimPlanXmlReader::readStimPlanXMLFile(const QString& stimPlanFileName, 
                                                                                  double conductivityScalingFactor,
                                                                                  double xScaleFactor,
                                                                                  double yScaleFactor,
                                                                                  double wellPathInterationY,
                                                                                  MirrorMode mirrorMode,
                                                                                  RiaEclipseUnitTools::UnitSystem requiredUnit,
                                                                                  QString * errorMessage)
{
    RiaLogging::info(QString("Starting to open StimPlan XML file: '%1'").arg(stimPlanFileName));

    cvf::ref<RigStimPlanFractureDefinition> stimPlanFileData = new RigStimPlanFractureDefinition;
    {
        QFile dataFile(stimPlanFileName);
        if (!dataFile.open(QFile::ReadOnly))
        {
            if (errorMessage) (*errorMessage) += "Could not open the File: " + (stimPlanFileName) + "\n";
            return nullptr;
        }

        QXmlStreamReader xmlStream;
        xmlStream.setDevice(&dataFile);
        xmlStream.readNext();
        readStimplanGridAndTimesteps(xmlStream, stimPlanFileData.p(), mirrorMode, requiredUnit);

        if(xScaleFactor != 1.0) stimPlanFileData->scaleXs(xScaleFactor);
        if(yScaleFactor != 1.0) stimPlanFileData->scaleYs(yScaleFactor, wellPathInterationY);

        RiaEclipseUnitTools::UnitSystemType unitSystem = stimPlanFileData->unitSet();

        if (unitSystem != RiaEclipseUnitTools::UNITS_UNKNOWN)
            RiaLogging::info(QString("Setting unit system for StimPlan fracture template %1 to %2").arg(stimPlanFileName).arg(unitSystem.uiText()));
        else 
            RiaLogging::error(QString("Found invalid units for %1. Unit system not set.").arg(stimPlanFileName));


        if (xmlStream.hasError())
        {
            RiaLogging::error(QString("Failed to parse file '%1'").arg(dataFile.fileName()));
            RiaLogging::error(xmlStream.errorString());
        }
        dataFile.close();
    }


    size_t numberOfYValues = stimPlanFileData->yCount();
    RiaLogging::debug(QString("Grid size X: %1, Y: %2").arg(QString::number(stimPlanFileData->xCount()),
        QString::number(numberOfYValues)));

    size_t numberOfTimeSteps = stimPlanFileData->timeSteps().size();
    RiaLogging::debug(QString("Number of time-steps: %1").arg(numberOfTimeSteps));

    //Start reading from top:
    QFile dataFile(stimPlanFileName);

    if (!dataFile.open(QFile::ReadOnly))
    {
        if (errorMessage) (*errorMessage) += "Could not open the File: " + (stimPlanFileName) + "\n";
        return nullptr;
    }
    
    QXmlStreamReader xmlStream2;
    xmlStream2.setDevice(&dataFile);
    QString parameter;
    QString unit;

    RiaLogging::info(QString("Properties available in file:"));
    while (!xmlStream2.atEnd())
    {
        xmlStream2.readNext();

        if (xmlStream2.isStartElement())
        {
            if (xmlStream2.name() == "property")
            {
                unit      = getAttributeValueString(xmlStream2, "uom");
                parameter = getAttributeValueString(xmlStream2, "name");

                RiaLogging::info(QString("%1 [%2]").arg(parameter, unit));

            }
            else if (xmlStream2.name() == "time")
            {
                double timeStepValue = getAttributeValueDouble(xmlStream2, "value");
                
                std::vector<std::vector<double>> propertyValuesAtTimestep = 
                    stimPlanFileData->generateDataLayoutFromFileDataLayout(getAllDepthDataAtTimeStep(xmlStream2));

                bool valuesOK = stimPlanFileData->numberOfParameterValuesOK(propertyValuesAtTimestep);
                if (!valuesOK)
                {
                    RiaLogging::error(QString("Inconsistency detected in reading XML file: '%1'").arg(dataFile.fileName()));
                    return nullptr;
                }

                if (parameter.contains(RiaDefines::conductivityResultName(), Qt::CaseInsensitive))
                {
                    // Scale all parameters containing conductivity

                    for (auto& dataAtDepth : propertyValuesAtTimestep)
                    {
                        for (auto& dataValue : dataAtDepth)
                        {
                            dataValue *= conductivityScalingFactor;
                        }
                    }
                }

                stimPlanFileData->setDataAtTimeValue(parameter, unit, propertyValuesAtTimestep, timeStepValue);
            }
        }
    }

    dataFile.close();

    if (xmlStream2.hasError())
    {
        RiaLogging::error(QString("Failed to parse file: '%1'").arg(dataFile.fileName()));
        RiaLogging::error(xmlStream2.errorString());
    }
    else if (dataFile.error() != QFile::NoError)
    {
        RiaLogging::error(QString("Cannot read file: '%1'").arg(dataFile.fileName()));
        RiaLogging::error(dataFile.errorString());
    }
    else
    {
        RiaLogging::info(QString("Successfully read XML file: '%1'").arg(stimPlanFileName));
    }

    return stimPlanFileData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifStimPlanXmlReader::readStimplanGridAndTimesteps(QXmlStreamReader &xmlStream,
                                                        RigStimPlanFractureDefinition* stimPlanFileData,
                                                        MirrorMode mirrorMode,
                                                        RiaEclipseUnitTools::UnitSystem requiredUnit)
{
    size_t startNegValuesYs = 0;
    QString gridunit = "unknown";

    xmlStream.readNext();

    //First, read time steps and grid to establish data structures for putting data into later. 
    while (!xmlStream.atEnd())
    {
        xmlStream.readNext();

        if (xmlStream.isStartElement())
        {
            RiaEclipseUnitTools::UnitSystem destinationUnit = requiredUnit;

            if (xmlStream.name() == "grid")
            {
                gridunit = getAttributeValueString(xmlStream, "uom");

                if (gridunit == "m")       stimPlanFileData->m_unitSet = RiaEclipseUnitTools::UNITS_METRIC;
                else if (gridunit == "ft") stimPlanFileData->m_unitSet = RiaEclipseUnitTools::UNITS_FIELD;
                else                       stimPlanFileData->m_unitSet = RiaEclipseUnitTools::UNITS_UNKNOWN;

                if (destinationUnit == RiaEclipseUnitTools::UNITS_UNKNOWN)
                {
                    // Use file unit set if requested unit is unknown
                    destinationUnit = stimPlanFileData->m_unitSet;
                }

                double tvdToTopPerfFt = getAttributeValueDouble(xmlStream, "TVDToTopPerfFt");
                double tvdToBotPerfFt = getAttributeValueDouble(xmlStream, "TVDToBottomPerfFt");

                double tvdToTopPerfRequestedUnit = RifStimPlanXmlReader::valueInRequiredUnitSystem(RiaEclipseUnitTools::UNITS_FIELD, destinationUnit, tvdToTopPerfFt);
                double tvdToBotPerfRequestedUnit = RifStimPlanXmlReader::valueInRequiredUnitSystem(RiaEclipseUnitTools::UNITS_FIELD, destinationUnit, tvdToBotPerfFt);

                stimPlanFileData->setTvdToTopPerf(tvdToTopPerfRequestedUnit);
                stimPlanFileData->setTvdToBottomPerf(tvdToBotPerfRequestedUnit);
            }

            if (xmlStream.name() == "xs")
            {
                std::vector<double> gridValuesXs;
                {
                    size_t dummy;
                    std::vector<double> gridValues;
                    getGriddingValues(xmlStream, gridValues, dummy);

                    gridValuesXs = RifStimPlanXmlReader::valuesInRequiredUnitSystem(stimPlanFileData->m_unitSet, destinationUnit, gridValues);
                }

                stimPlanFileData->m_fileXs = gridValuesXs;

                stimPlanFileData->generateXsFromFileXs(mirrorMode == MIRROR_AUTO ? !hasNegativeValues(gridValuesXs) : (bool)mirrorMode);
            }

            else if (xmlStream.name() == "ys")
            {
                std::vector<double> gridValuesYs;
                {
                    std::vector<double> gridValues;
                    getGriddingValues(xmlStream, gridValues, startNegValuesYs);

                    gridValuesYs = RifStimPlanXmlReader::valuesInRequiredUnitSystem(stimPlanFileData->m_unitSet, destinationUnit, gridValues);
                }

                // Reorder and change sign
                std::vector<double> ys;
                for (double y : gridValuesYs)
                {
                    ys.insert(ys.begin(), -y);
                }
                stimPlanFileData->m_Ys = ys;
            }

            else if (xmlStream.name() == "time")
            {
                double timeStepValue = getAttributeValueDouble(xmlStream, "value");
                stimPlanFileData->addTimeStep(timeStepValue);
            }
        }
    }

    if (startNegValuesYs > 0)
    {
        RiaLogging::error(QString("Negative depth values detected in XML file"));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>>  RifStimPlanXmlReader::getAllDepthDataAtTimeStep(QXmlStreamReader &xmlStream)
{
    std::vector<std::vector<double>> propertyValuesAtTimestep;

    while (!(xmlStream.isEndElement() && xmlStream.name() == "time"))
    {
        xmlStream.readNext();

        if (xmlStream.name() == "depth")
        {
            double depth = xmlStream.readElementText().toDouble();
            std::vector<double> propertyValuesAtDepth;

            xmlStream.readNext(); //read end depth token
            xmlStream.readNext(); //read cdata section with values
            if (xmlStream.isCDATA())
            {
                QString depthDataStr = xmlStream.text().toString();
                QStringList splitted = depthDataStr.split(' ');
                for (int i = 0; i < splitted.size(); i++)
                {
                    QString value = splitted[i];
                    if ( value != "")
                    {
                        propertyValuesAtDepth.push_back(value.toDouble());
                    }
                }
            }
            propertyValuesAtTimestep.push_back(propertyValuesAtDepth);
        }
    }
    return propertyValuesAtTimestep;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RifStimPlanXmlReader::valuesInRequiredUnitSystem(RiaEclipseUnitTools::UnitSystem sourceUnit,
                                                                     RiaEclipseUnitTools::UnitSystem requiredUnit,
                                                                     const std::vector<double>&      values)
{
    if (sourceUnit == RiaEclipseUnitTools::UNITS_FIELD && requiredUnit == RiaEclipseUnitTools::UNITS_METRIC)
    {
        std::vector<double> convertedValues;
        for (const auto &valueInFeet : values)
        {
            convertedValues.push_back(RiaEclipseUnitTools::feetToMeter(valueInFeet));
        }

        return convertedValues;
    }
    else if (sourceUnit == RiaEclipseUnitTools::UNITS_METRIC && requiredUnit == RiaEclipseUnitTools::UNITS_FIELD)
    {
        std::vector<double> convertedValues;
        for (const auto &valueInMeter : values)
        {
            convertedValues.push_back(RiaEclipseUnitTools::meterToFeet(valueInMeter));
        }

        return convertedValues;
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RifStimPlanXmlReader::valueInRequiredUnitSystem(RiaEclipseUnitTools::UnitSystem sourceUnit,
                                                       RiaEclipseUnitTools::UnitSystem requiredUnit,
                                                       double                          value)
{
    if (sourceUnit == RiaEclipseUnitTools::UNITS_FIELD && requiredUnit == RiaEclipseUnitTools::UNITS_METRIC)
    {
        return RiaEclipseUnitTools::feetToMeter(value);
    }
    else if (sourceUnit == RiaEclipseUnitTools::UNITS_METRIC && requiredUnit == RiaEclipseUnitTools::UNITS_FIELD)
    {
        return RiaEclipseUnitTools::meterToFeet(value);
    }

    return value;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifStimPlanXmlReader::getGriddingValues(QXmlStreamReader &xmlStream, std::vector<double>& gridValues, size_t& startNegValues)
{
    QString gridValuesString = xmlStream.readElementText().replace('\n', ' ');
    for (QString value : gridValuesString.split(' '))
    {
        if (value.size() > 0)
        {
            double gridValue = value.toDouble();
            gridValues.push_back(gridValue);
            if(gridValue < -RigStimPlanFractureDefinition::THRESHOLD_VALUE) startNegValues++;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RifStimPlanXmlReader::getAttributeValueDouble(QXmlStreamReader &xmlStream, QString parameterName)
{
    double value = HUGE_VAL;
    for (const QXmlStreamAttribute &attr : xmlStream.attributes())
    {
        if (attr.name() == parameterName)
        {
            value = attr.value().toString().toDouble();
        }
    }
    return value;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RifStimPlanXmlReader::getAttributeValueString(QXmlStreamReader &xmlStream, QString parameterName)
{
    QString parameterValue;
    for (const QXmlStreamAttribute &attr : xmlStream.attributes())
    {
        if (attr.name() == parameterName)
        {
            parameterValue = attr.value().toString();
        }
    }
    return parameterValue;
}

//--------------------------------------------------------------------------------------------------
/// Internal function
//--------------------------------------------------------------------------------------------------
bool hasNegativeValues(std::vector<double> xs)
{
    return xs[0] < -RigStimPlanFractureDefinition::THRESHOLD_VALUE;
}
