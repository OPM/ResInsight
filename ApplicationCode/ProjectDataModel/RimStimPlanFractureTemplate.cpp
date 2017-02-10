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

#include "RimStimPlanFractureTemplate.h"

#include "RigStimPlanFractureDefinition.h"

#include "cafPdmObject.h"
#include "cafPdmUiFilePathEditor.h"

#include "cvfVector3.h"

#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>



CAF_PDM_SOURCE_INIT(RimStimPlanFractureTemplate, "RimStimPlanFractureTemplate");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStimPlanFractureTemplate::RimStimPlanFractureTemplate(void)
{
    CAF_PDM_InitObject("Fracture Template", ":/FractureTemplate16x16.png", "", "");

    CAF_PDM_InitField(&m_StimPlanFileName, "StimPlanFileName", QString(""), "StimPlan File Name", "", "", "");
    m_StimPlanFileName.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());


}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStimPlanFractureTemplate::~RimStimPlanFractureTemplate()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{

    if (&m_StimPlanFileName == changedField)
    {
        updateUiTreeName();
        QString errorMessage;
        readStimPlanXMLFile(&errorMessage);
        if (!errorMessage.isEmpty())
        {
            QMessageBox::warning(nullptr, "StimPlanFile", errorMessage);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::updateUiTreeName()
{
    this->uiCapability()->setUiName(fileNameWoPath());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::setFileName(const QString& fileName)
{
    m_StimPlanFileName = fileName;

    updateUiTreeName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QString& RimStimPlanFractureTemplate::fileName()
{
    return m_StimPlanFileName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimStimPlanFractureTemplate::fileNameWoPath()
{
    QFileInfo stimplanfileFileInfo(m_StimPlanFileName());
    return stimplanfileFileInfo.fileName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::readStimPlanXMLFile(QString * errorMessage)
{
    QFile dataFile(m_StimPlanFileName());

    if (!dataFile.open(QFile::ReadOnly))
    {
        if (errorMessage) (*errorMessage) += "Could not open the File: " + (m_StimPlanFileName()) + "\n";
        return;
    }

    m_StimPlanFractureDefinitionData = new RigStimPlanFractureDefinition;

    QXmlStreamReader xmlStream;
    xmlStream.setDevice(&dataFile);
    xmlStream.readNext();

    QString parameter;

    while (!xmlStream.atEnd())
    {
        xmlStream.readNext();

        if (xmlStream.isStartElement())
        {

           if (xmlStream.name() == "xs")
            {   
                m_StimPlanFractureDefinitionData->gridXs = getGriddingValues(xmlStream);
            }

            else if (xmlStream.name() == "ys")
            {
                m_StimPlanFractureDefinitionData->gridYs = getGriddingValues(xmlStream);

            }

            else if (xmlStream.name() == "property")
            {
                parameter = getAttributeValueString(xmlStream, "name");
            }

            else if (xmlStream.name() == "time")
            {
                double timeStepValue = getAttributeValueDouble(xmlStream, "value");
                m_StimPlanFractureDefinitionData->timeSteps.push_back(timeStepValue);
                
                std::vector<std::vector<double>> propertyValuesAtTimestep = getAllDepthDataAtTimeStep(xmlStream);
                
                if (parameter == "CONDUCTIVITY")
                {
                    m_StimPlanFractureDefinitionData->conductivities.push_back(propertyValuesAtTimestep);
                }

                if (parameter == "PERMEABILITY")
                {
                    m_StimPlanFractureDefinitionData->permeabilities.push_back(propertyValuesAtTimestep);
                }
                if (parameter == "WIDTH")
                {
                    m_StimPlanFractureDefinitionData->widths.push_back(propertyValuesAtTimestep);
                }
            }
        }
    }

    dataFile.close();

    if (xmlStream.hasError())
    {
        qDebug() << "Error: Failed to parse file " << dataFile.fileName();
        qDebug() << xmlStream.errorString();
    }
    else if (dataFile.error() != QFile::NoError)
    {
        qDebug() << "Error: Cannot read file " << dataFile.fileName();
        qDebug() << dataFile.errorString();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>> RimStimPlanFractureTemplate::getAllDepthDataAtTimeStep(QXmlStreamReader &xmlStream)
{
    std::vector<std::vector<double>> propertyValuesAtTimestep;

    while (!(xmlStream.isEndElement() && xmlStream.name() == "time"))
    {
        xmlStream.readNext();
        if (xmlStream.name() == "depth")
        {
            double depth = xmlStream.readElementText().toDouble();
            m_StimPlanFractureDefinitionData->depths.push_back(depth);
            std::vector<double> propertyValuesAtDepth;

            xmlStream.readNext(); //read end depth token
            xmlStream.readNext(); //read cdata section with values
            if (xmlStream.isCDATA())
            {
                QString depthDataStr = xmlStream.text().toString();
                for (QString value : depthDataStr.split(' '))
                {
                    propertyValuesAtDepth.push_back(value.toDouble());
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
std::vector<double> RimStimPlanFractureTemplate::getGriddingValues(QXmlStreamReader &xmlStream)
{
    std::vector<double> gridValues;
    QString gridValuesString = xmlStream.readElementText().replace('\n', ' ');
    for (QString value : gridValuesString.split(' '))
    {
        gridValues.push_back(value.toDouble());
    }

    return gridValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimStimPlanFractureTemplate::getAttributeValueDouble(QXmlStreamReader &xmlStream, QString parameterName)
{
    double value = cvf::UNDEFINED_DOUBLE;
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
QString RimStimPlanFractureTemplate::getAttributeValueString(QXmlStreamReader &xmlStream, QString parameterName)
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
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::fractureGeometry(std::vector<cvf::Vec3f>* nodeCoords, std::vector<cvf::uint>* triangleIndices)
{
    //TODO
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3f> RimStimPlanFractureTemplate::fracturePolygon()
{
     std::vector<cvf::Vec3f> polygon;

     //TODO: Handle multiple time-step and properties
     std::vector<std::vector<double>> ConductivitiesAtTimeStep = m_StimPlanFractureDefinitionData->conductivities[0];

     for (int k = 0; k < ConductivitiesAtTimeStep.size(); k++)
     {
         for (int i = 0; i < ConductivitiesAtTimeStep[k].size(); i++)
         {
             if ((ConductivitiesAtTimeStep[k])[i] > 1e-7)
             {
                 if ((i < ConductivitiesAtTimeStep[k].size() - 1))
                 {
                     if ((ConductivitiesAtTimeStep[k])[(i + 1)] < 1e-7)
                     {
                         polygon.push_back(cvf::Vec3f(static_cast<float>(m_StimPlanFractureDefinitionData->gridXs[i]),
                             static_cast<float>(m_StimPlanFractureDefinitionData->depths[k]), 0.0f));
                     }
                 }
                 else
                 {
                     polygon.push_back(cvf::Vec3f(static_cast<float>(m_StimPlanFractureDefinitionData->gridXs[i]),
                         static_cast<float>(m_StimPlanFractureDefinitionData->depths[k]), 0.0f));
                 }
             }
         }
     }

     std::vector<cvf::Vec3f> negPolygon;

     for (const auto& node : polygon)
     {
         cvf::Vec3f negNode = node;
         negNode.x() = -negNode.x();
         negPolygon.insert(negPolygon.begin(), negNode);
     }

     for (const auto& negNode : negPolygon)
     {
         polygon.push_back(negNode);
     }

     return polygon;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimFractureTemplate::defineUiOrdering(uiConfigName, uiOrdering);

    uiOrdering.add(&name);

    caf::PdmUiGroup* fileGroup = uiOrdering.addNewGroup("File");
    fileGroup->add(&m_StimPlanFileName);

    caf::PdmUiGroup* geometryGroup = uiOrdering.addNewGroup("Fracture geometry");
    geometryGroup->add(&orientation);
    geometryGroup->add(&azimuthAngle);

    caf::PdmUiGroup* propertyGroup = uiOrdering.addNewGroup("Fracture properties");
    propertyGroup->add(&skinFactor);
}

