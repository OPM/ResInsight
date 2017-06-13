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

#include "RigStimPlanFractureDefinition.h"
#include "RiaLogging.h"
#include <QFile>
#include <QXmlStreamReader>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigStimPlanFractureDefinition> RifStimPlanXmlReader::readStimPlanXMLFile(const QString& stimPlanFileName, QString * errorMessage)
{
    RiaLogging::info(QString("Starting to open StimPlan XML file: '%1'").arg(stimPlanFileName));

    cvf::ref<RigStimPlanFractureDefinition> stimPlanFileData = new RigStimPlanFractureDefinition;
    size_t startingNegXsValues = 0;
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
        startingNegXsValues = readStimplanGridAndTimesteps(xmlStream, stimPlanFileData.p());

        if (xmlStream.hasError())
        {
            RiaLogging::error(QString("Failed to parse file '%1'").arg(dataFile.fileName()));
            RiaLogging::error(xmlStream.errorString());
        }
        dataFile.close();
    }


    size_t numberOfDepthValues = stimPlanFileData->depths.size();
    RiaLogging::debug(QString("Grid size X: %1, Y: %2").arg(QString::number(stimPlanFileData->gridXs.size()),
        QString::number(numberOfDepthValues)));

    size_t numberOfTimeSteps = stimPlanFileData->timeSteps.size();
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

                if (parameter == "CONDUCTIVITY")
                {
                    if (unit == "md-ft")
                    {
                        stimPlanFileData->unitSet = RimUnitSystem::UNITS_FIELD;
                        RiaLogging::info(QString("Setting unit system to Field for StimPlan fracture template %1").arg(stimPlanFileName));
                    }
                    if (unit == "md-m")
                    {
                        stimPlanFileData->unitSet = RimUnitSystem::UNITS_METRIC;
                        RiaLogging::info(QString("Setting unit system to Metric for StimPlan fracture template %1").arg(stimPlanFileName));
                    }
                }


            }
            else if (xmlStream2.name() == "time")
            {
                double timeStepValue = getAttributeValueDouble(xmlStream2, "value");
                
                std::vector<std::vector<double>> propertyValuesAtTimestep = getAllDepthDataAtTimeStep(xmlStream2, startingNegXsValues);
                
                bool valuesOK = stimPlanFileData->numberOfParameterValuesOK(propertyValuesAtTimestep);
                if (!valuesOK)
                {
                    RiaLogging::error(QString("Inconsistency detected in reading XML file: '%1'").arg(dataFile.fileName()));
                    return nullptr;
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
size_t RifStimPlanXmlReader::readStimplanGridAndTimesteps(QXmlStreamReader &xmlStream, RigStimPlanFractureDefinition* stimPlanFileData)
{

    size_t startNegValuesXs = 0;
    size_t startNegValuesYs = 0;

    xmlStream.readNext();

    //First, read time steps and grid to establish data structures for putting data into later. 
    while (!xmlStream.atEnd())
    {
        xmlStream.readNext();

        if (xmlStream.isStartElement())
        {

            if (xmlStream.name() == "xs")
            {
                std::vector<double> gridValues;
                getGriddingValues(xmlStream, gridValues, startNegValuesXs);
                stimPlanFileData->gridXs = gridValues;
            }

            else if (xmlStream.name() == "ys")
            {
                std::vector<double> gridValues;
                getGriddingValues(xmlStream, gridValues, startNegValuesYs);
                stimPlanFileData->gridYs = gridValues;

                stimPlanFileData->reorderYgridToDepths();
            }

            else if (xmlStream.name() == "time")
            {
                double timeStepValue = getAttributeValueDouble(xmlStream, "value");
                if (!stimPlanFileData->timeStepExisist(timeStepValue))
                {
                    stimPlanFileData->timeSteps.push_back(timeStepValue);
                }
            }
        }
    }

    if (startNegValuesYs > 0)
    {
        RiaLogging::error(QString("Negative depth values detected in XML file"));
    }
    return startNegValuesXs;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>>  RifStimPlanXmlReader::getAllDepthDataAtTimeStep(QXmlStreamReader &xmlStream, size_t startingNegValuesXs)
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
                for (int i = 0; i < depthDataStr.split(' ').size(); i++)
                {
                    if (i < startingNegValuesXs) continue;
                    else 
                    {
                        QString value = depthDataStr.split(' ')[i];
                        if ( value != "")
                        {
                            propertyValuesAtDepth.push_back(value.toDouble());
                        }
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
void RifStimPlanXmlReader::getGriddingValues(QXmlStreamReader &xmlStream, std::vector<double>& gridValues, size_t& startNegValues)
{
    QString gridValuesString = xmlStream.readElementText().replace('\n', ' ');
    for (QString value : gridValuesString.split(' '))
    {
        if (value.size() > 0)
        {
            double gridValue = value.toDouble();
            if (gridValue > -1e-5) //tolerance of 1e-5 
            {
                gridValues.push_back(gridValue);
            }
            else startNegValues++;
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




