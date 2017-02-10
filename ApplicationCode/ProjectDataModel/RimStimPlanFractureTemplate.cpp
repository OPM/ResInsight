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

#include "RimFracture.h"
#include "RimProject.h"

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

    CAF_PDM_InitField(&m_stimPlanFileName, "StimPlanFileName", QString(""), "StimPlan File Name", "", "", "");
    m_stimPlanFileName.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());

    CAF_PDM_InitField(&wellPathDepthAtFracture, "WellPathDepthAtFracture", 0.0, "Depth of Well Path at Fracture", "", "", "");


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

    if (&m_stimPlanFileName == changedField)
    {
        updateUiTreeName();
        QString errorMessage;
        readStimPlanXMLFile(&errorMessage);
        if (!errorMessage.isEmpty())
        {
            QMessageBox::warning(nullptr, "StimPlanFile", errorMessage);
        }
    }

    if (&wellPathDepthAtFracture == changedField)
    {
        RimProject* proj;
        this->firstAncestorOrThisOfType(proj);
        if (proj)
        {
            //Regenerate geometry
            std::vector<RimFracture*> fractures;
            proj->descendantsIncludingThisOfType(fractures);

            for (RimFracture* fracture : fractures)
            {
                if (fracture->attachedFractureDefinition() == this)
                {
                    fracture->setRecomputeGeometryFlag();
                }
            }
        }

        proj->createDisplayModelAndRedrawAllViews();
    }


}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::updateUiTreeName()
{
    this->uiCapability()->setUiName(fileNameWithOutPath());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::setFileName(const QString& fileName)
{
    m_stimPlanFileName = fileName;

    updateUiTreeName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QString& RimStimPlanFractureTemplate::fileName()
{
    return m_stimPlanFileName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimStimPlanFractureTemplate::fileNameWithOutPath()
{
    QFileInfo stimplanfileFileInfo(m_stimPlanFileName());
    return stimplanfileFileInfo.fileName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::readStimPlanXMLFile(QString * errorMessage)
{
    QFile dataFile(m_stimPlanFileName());

    if (!dataFile.open(QFile::ReadOnly))
    {
        if (errorMessage) (*errorMessage) += "Could not open the File: " + (m_stimPlanFileName()) + "\n";
        return;
    }

    m_stimPlanFractureDefinitionData = new RigStimPlanFractureDefinition;

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
                m_stimPlanFractureDefinitionData->gridXs = getGriddingValues(xmlStream);
            }

            else if (xmlStream.name() == "ys")
            {
                m_stimPlanFractureDefinitionData->gridYs = getGriddingValues(xmlStream);

            }

            else if (xmlStream.name() == "property")
            {
                parameter = getAttributeValueString(xmlStream, "name");
            }

            else if (xmlStream.name() == "time")
            {
                double timeStepValue = getAttributeValueDouble(xmlStream, "value");
                m_stimPlanFractureDefinitionData->timeSteps.push_back(timeStepValue);
                
                std::vector<std::vector<double>> propertyValuesAtTimestep = getAllDepthDataAtTimeStep(xmlStream);
                
                if (parameter == "CONDUCTIVITY")
                {
                    m_stimPlanFractureDefinitionData->conductivities.push_back(propertyValuesAtTimestep);
                }

                if (parameter == "PERMEABILITY")
                {
                    m_stimPlanFractureDefinitionData->permeabilities.push_back(propertyValuesAtTimestep);
                }
                if (parameter == "WIDTH")
                {
                    m_stimPlanFractureDefinitionData->widths.push_back(propertyValuesAtTimestep);
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
            m_stimPlanFractureDefinitionData->depths.push_back(depth);
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
        if (value.size()>0) gridValues.push_back(value.toDouble());
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
    
    if (m_stimPlanFractureDefinitionData.isNull())
    {
        return;
        //TODO: try to read in data again if missing... 
    }

    for (const double& depth : adjustedDepthCoordsAroundWellPathPosition())
    {
        for (const double& negXcoord : getNegAndPosXcoords())
        {
            cvf::Vec3f node = cvf::Vec3f(negXcoord, depth, 0);
            nodeCoords->push_back(node);
        }
    }


    cvf::uint lenXcoords = static_cast<cvf::uint>(getNegAndPosXcoords().size());

    for (cvf::uint k = 0; k < adjustedDepthCoordsAroundWellPathPosition().size()-1; k++)
    {
        for (cvf::uint i = 0; i < lenXcoords-1; i++)
        {
            //Upper triangle
            triangleIndices->push_back(i + k*lenXcoords);
            triangleIndices->push_back((i+1) + k*lenXcoords);
            triangleIndices->push_back((i+1) + (k+1)*lenXcoords);
            //Lower triangle
            triangleIndices->push_back(i + k*lenXcoords);
            triangleIndices->push_back((i + 1) + (k+1)*lenXcoords);
            triangleIndices->push_back((i) + (k+1)*lenXcoords);

        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStimPlanFractureTemplate::getNegAndPosXcoords()
{
    std::vector<double> allXcoords;
    for (const double& xCoord : m_stimPlanFractureDefinitionData->gridXs)
    {
        if (xCoord > 1e-5)
        {
            double negXcoord = -xCoord;
            allXcoords.insert(allXcoords.begin(), negXcoord);
        }
    }
    for (const double& xCoord : m_stimPlanFractureDefinitionData->gridXs)
    {
        allXcoords.push_back(xCoord);
    }

    return allXcoords;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double>  RimStimPlanFractureTemplate::adjustedDepthCoordsAroundWellPathPosition()
{
    std::vector<double> depthRelativeToWellPath;

    for (const double& depth : m_stimPlanFractureDefinitionData->depths)
    {
        double adjustedDepth = depth - wellPathDepthAtFracture();
        depthRelativeToWellPath.push_back(adjustedDepth);
    }
    return depthRelativeToWellPath;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3f> RimStimPlanFractureTemplate::fracturePolygon()
{
     std::vector<cvf::Vec3f> polygon;

     //TODO: Handle multiple time-step and properties
     std::vector<std::vector<double>> ConductivitiesAtTimeStep = m_stimPlanFractureDefinitionData->conductivities[0];

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
                         polygon.push_back(cvf::Vec3f(static_cast<float>(m_stimPlanFractureDefinitionData->gridXs[i]),
                             static_cast<float>(m_stimPlanFractureDefinitionData->depths[k]), 0.0f));
                     }
                 }
                 else
                 {
                     polygon.push_back(cvf::Vec3f(static_cast<float>(m_stimPlanFractureDefinitionData->gridXs[i]),
                         static_cast<float>(m_stimPlanFractureDefinitionData->depths[k]), 0.0f));
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
    fileGroup->add(&m_stimPlanFileName);

    caf::PdmUiGroup* geometryGroup = uiOrdering.addNewGroup("Fracture geometry");
    geometryGroup->add(&orientation);
    geometryGroup->add(&azimuthAngle);

    caf::PdmUiGroup* propertyGroup = uiOrdering.addNewGroup("Fracture properties");
    propertyGroup->add(&fractureConductivity);
    propertyGroup->add(&skinFactor);
}

