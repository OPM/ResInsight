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

#include "RiaApplication.h"

#include "RigStimPlanFractureDefinition.h"

#include "RimEclipseView.h"
#include "RimFracture.h"
#include "RimProject.h"
#include "RimStimPlanColors.h"
#include "RimStimPlanLegendConfig.h"

#include "cafPdmObject.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiFilePathEditor.h"

#include "cvfVector3.h"

#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>

#include <algorithm>



CAF_PDM_SOURCE_INIT(RimStimPlanFractureTemplate, "RimStimPlanFractureTemplate");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStimPlanFractureTemplate::RimStimPlanFractureTemplate(void)
{
    CAF_PDM_InitObject("Fracture Template", ":/FractureTemplate16x16.png", "", "");

    CAF_PDM_InitField(&m_stimPlanFileName, "StimPlanFileName", QString(""), "File Name", "", "", "");
    m_stimPlanFileName.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());

    CAF_PDM_InitField(&wellPathDepthAtFracture, "WellPathDepthAtFracture", 0.0, "Well/Fracture Intersection Depth", "", "", "");
    wellPathDepthAtFracture.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());
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
    RimFractureTemplate::fieldChangedByUi(changedField, oldValue, newValue);

    if (&m_stimPlanFileName == changedField)
    {
        updateUiTreeName();
        loadDataAndUpdate();
/*
        QString errorMessage;
        readStimPlanXMLFile(&errorMessage);
        if (!errorMessage.isEmpty())
        {
            QMessageBox::warning(nullptr, "StimPlanFile", errorMessage);
        }
*/
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
    m_stimPlanFractureDefinitionData = new RigStimPlanFractureDefinition;
    size_t startingNegXsValues = 0;
    {
        QFile dataFile(m_stimPlanFileName());
        if (!dataFile.open(QFile::ReadOnly))
        {
            if (errorMessage) (*errorMessage) += "Could not open the File: " + (m_stimPlanFileName()) + "\n";
            return;
        }

        QXmlStreamReader xmlStream;
        xmlStream.setDevice(&dataFile);
        xmlStream.readNext();
        startingNegXsValues = readStimplanGridAndTimesteps(xmlStream);
        if (xmlStream.hasError())
        {
            qDebug() << "Error: Failed to parse file " << dataFile.fileName();
            qDebug() << xmlStream.errorString();
        }
        dataFile.close();
    }

    size_t numberOfDepthValues;
    numberOfDepthValues = m_stimPlanFractureDefinitionData->depths.size();
    size_t numberOfTimeSteps;
    numberOfTimeSteps = m_stimPlanFractureDefinitionData->timeSteps.size();

    //Start reading from top:
    QFile dataFile(m_stimPlanFileName());

    if (!dataFile.open(QFile::ReadOnly))
    {
        if (errorMessage) (*errorMessage) += "Could not open the File: " + (m_stimPlanFileName()) + "\n";
        return;
    }
    
    QXmlStreamReader xmlStream2;
    xmlStream2.setDevice(&dataFile);
    QString parameter;
    QString unit;

    while (!xmlStream2.atEnd())
    {
        xmlStream2.readNext();

        if (xmlStream2.isStartElement())
        {
            if (xmlStream2.name() == "property")
            {
                unit      = getAttributeValueString(xmlStream2, "uom");
                parameter = getAttributeValueString(xmlStream2, "name");
                //Width - convert to cm from mm?

            }
            else if (xmlStream2.name() == "time")
            {
                double timeStepValue = getAttributeValueDouble(xmlStream2, "value");
                
                std::vector<std::vector<double>> propertyValuesAtTimestep = getAllDepthDataAtTimeStep(xmlStream2, startingNegXsValues);
                
                bool valuesOK = numberOfParameterValuesOK(propertyValuesAtTimestep);
                if (!valuesOK)
                {
                    qDebug() << "Inconsistency detected in reading XML file!";
                    return;
                }

                m_stimPlanFractureDefinitionData->setDataAtTimeValue(parameter, unit, propertyValuesAtTimestep, timeStepValue);
               
            }
        }
    }

    dataFile.close();

    if (xmlStream2.hasError())
    {
        qDebug() << "Error: Failed to parse file " << dataFile.fileName();
        qDebug() << xmlStream2.errorString();
    }
    else if (dataFile.error() != QFile::NoError)
    {
        qDebug() << "Error: Cannot read file " << dataFile.fileName();
        qDebug() << dataFile.errorString();
    }

    setDepthOfWellPathAtFracture();
    
    RimEclipseView* activeView = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeReservoirView());
    if (!activeView) return;
    activeView->stimPlanColors->loadDataAndUpdate();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::loadDataAndUpdate()
{
    QString errorMessage;
    readStimPlanXMLFile(&errorMessage);
    qDebug() << errorMessage;

    updateConnectedEditors();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>> RimStimPlanFractureTemplate::getDataAtTimeIndex(const QString& resultName, const QString& unitName, size_t timeStepIndex) const
{
    return m_stimPlanFractureDefinitionData->getDataAtTimeIndex(resultName, unitName, timeStepIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimStimPlanFractureTemplate::readStimplanGridAndTimesteps(QXmlStreamReader &xmlStream)
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
                m_stimPlanFractureDefinitionData->gridXs = gridValues;
            }

            else if (xmlStream.name() == "ys")
            {
                std::vector<double> gridValues;
                getGriddingValues(xmlStream, gridValues, startNegValuesYs);
                m_stimPlanFractureDefinitionData->gridYs = gridValues;

                m_stimPlanFractureDefinitionData->reorderYgridToDepths();
            }

            else if (xmlStream.name() == "time")
            {
                double timeStepValue = getAttributeValueDouble(xmlStream, "value");
                if (!m_stimPlanFractureDefinitionData->timeStepExisist(timeStepValue))
                {
                    m_stimPlanFractureDefinitionData->timeSteps.push_back(timeStepValue);
                }
            }
        }
    }

    if (startNegValuesYs > 0)
    {
        qDebug() << "Error in reading XML file. Negative depth values detected";
    }
    return startNegValuesXs;
        
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>>  RimStimPlanFractureTemplate::getAllDepthDataAtTimeStep(QXmlStreamReader &xmlStream, size_t startingNegValuesXs)
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
bool RimStimPlanFractureTemplate::numberOfParameterValuesOK(std::vector<std::vector<double>> propertyValuesAtTimestep)
{
    size_t depths = m_stimPlanFractureDefinitionData->depths.size();
    size_t gridXvalues = m_stimPlanFractureDefinitionData->gridXs.size();

    if (propertyValuesAtTimestep.size() != depths)  return false;
    for (std::vector<double> valuesAtDepthVector : propertyValuesAtTimestep)
    {
        if (valuesAtDepthVector.size() != gridXvalues) return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::setDepthOfWellPathAtFracture()
{
    if (!m_stimPlanFractureDefinitionData.isNull())
    {
        double firstDepth = m_stimPlanFractureDefinitionData->depths[0];
        double lastDepth  = m_stimPlanFractureDefinitionData->depths[m_stimPlanFractureDefinitionData->depths.size()-1];
        double averageDepth = (firstDepth + lastDepth) / 2;
        wellPathDepthAtFracture = averageDepth;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::getGriddingValues(QXmlStreamReader &xmlStream, std::vector<double>& gridValues, size_t& startNegValues)
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
        loadDataAndUpdate();
    }


    std::vector<double> xCoords = getNegAndPosXcoords();
    cvf::uint lenXcoords = static_cast<cvf::uint>(xCoords.size());

    std::vector<double> adjustedDepths = adjustedDepthCoordsAroundWellPathPosition();

    for (cvf::uint k = 0; k < adjustedDepths.size(); k++)
    {
        for (cvf::uint i = 0; i < lenXcoords; i++)
        {
            cvf::Vec3f node = cvf::Vec3f(xCoords[i], adjustedDepths[k], 0);
            nodeCoords->push_back(node);

            if (i < lenXcoords - 1 && k < adjustedDepths.size() - 1)
            {
                if (xCoords[i] < 1e-5)
                {
                    //Upper triangle
                    triangleIndices->push_back(i + k*lenXcoords);
                    triangleIndices->push_back((i + 1) + k*lenXcoords);
                    triangleIndices->push_back((i + 1) + (k + 1)*lenXcoords);
                    //Lower triangle
                    triangleIndices->push_back(i + k*lenXcoords);
                    triangleIndices->push_back((i + 1) + (k + 1)*lenXcoords);
                    triangleIndices->push_back((i)+(k + 1)*lenXcoords);

                }
                else
                {
                    //Upper triangle
                    triangleIndices->push_back(i + k*lenXcoords);
                    triangleIndices->push_back((i + 1) + k*lenXcoords);
                    triangleIndices->push_back((i)+(k + 1)*lenXcoords);
                    //Lower triangle
                    triangleIndices->push_back((i + 1) + k*lenXcoords);
                    triangleIndices->push_back((i + 1) + (k + 1)*lenXcoords);
                    triangleIndices->push_back((i) + (k + 1)*lenXcoords);
                }

            }
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
std::vector<double> RimStimPlanFractureTemplate::getStimPlanTimeValues()
{
    if (m_stimPlanFractureDefinitionData.isNull()) loadDataAndUpdate();
    return m_stimPlanFractureDefinitionData->timeSteps;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, QString> > RimStimPlanFractureTemplate::getStimPlanPropertyNamesUnits() const
{
    std::vector<std::pair<QString, QString> >  propertyNamesUnits;
    if (m_stimPlanFractureDefinitionData.notNull())
    {
        std::vector<RigStimPlanData > allStimPlanData = m_stimPlanFractureDefinitionData->stimPlanData;
        for (RigStimPlanData stimPlanDataEntry : allStimPlanData)
        {
            propertyNamesUnits.push_back(std::make_pair(stimPlanDataEntry.resultName, stimPlanDataEntry.unit));
        }
    }
    return propertyNamesUnits;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::computeMinMax(const QString& resultName, const QString& unitName, double* minValue, double* maxValue) const
{
    if (m_stimPlanFractureDefinitionData.notNull())
    {
        m_stimPlanFractureDefinitionData->computeMinMax(resultName, unitName, minValue, maxValue);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3f> RimStimPlanFractureTemplate::fracturePolygon()
{
    std::vector<cvf::Vec3f> polygon;

    //TODO: Handle multiple time-step and properties
    std::vector<std::vector<double>> dataAtTimeStep = m_stimPlanFractureDefinitionData->getDataAtTimeIndex(getStimPlanPropertyNamesUnits()[0].first, getStimPlanPropertyNamesUnits()[0].second, 0);

    for (int k = 0; k < dataAtTimeStep.size(); k++)
    {
        for (int i = 0; i < dataAtTimeStep[k].size(); i++)
        {
            if ((dataAtTimeStep[k])[i] > 1e-7)
            {
                if ((i < dataAtTimeStep[k].size() - 1))
                {
                    if ((dataAtTimeStep[k])[(i + 1)] < 1e-7)
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
    fileGroup->add(&wellPathDepthAtFracture);

    caf::PdmUiGroup* geometryGroup = uiOrdering.addNewGroup("Geometry");
    geometryGroup->add(&orientation);
    geometryGroup->add(&azimuthAngle);

    caf::PdmUiGroup* propertyGroup = uiOrdering.addNewGroup("Properties");
    propertyGroup->add(&fractureConductivity);
    propertyGroup->add(&skinFactor);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute)
{
    if (field == &m_stimPlanFileName)
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_fileSelectionFilter = "StimPlan Xml Files(*.xml);;All Files (*.*)";
        }
    }

    if (field == &wellPathDepthAtFracture)
    {
        if (!m_stimPlanFractureDefinitionData.isNull() && (m_stimPlanFractureDefinitionData->depths.size()>0))
        {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_minimum = m_stimPlanFractureDefinitionData->depths[0];
            myAttr->m_maximum = m_stimPlanFractureDefinitionData->depths[m_stimPlanFractureDefinitionData->depths.size() - 1];
        }

        }
    }
}

