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
#include "RiaLogging.h"

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

    CAF_PDM_InitField(&parameterForPolygon, "parameterForPolyton", QString(""), "Parameter", "", "", "");
    CAF_PDM_InitField(&timestepForPolygon, "timestepForPolygon", 0, "TimeStep", "", "", "");

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
        setDefaultsBasedOnXMLfile();
    }

    if (&wellPathDepthAtFracture == changedField || &parameterForPolygon == changedField || &timestepForPolygon == changedField)
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

//     if (&parameterForPolygon == changedField || &timestepForPolygon == changedField)
//     {
//         fracturePolygon();
//     }


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
    RiaLogging::info(QString("Starting to open StimPlan XML file: '%1'").arg(fileName()));

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
            RiaLogging::error(QString("Failed to parse file '%1'").arg(dataFile.fileName()));
            RiaLogging::error(xmlStream.errorString());
        }
        dataFile.close();
    }


    size_t numberOfDepthValues = m_stimPlanFractureDefinitionData->depths.size();
    RiaLogging::debug(QString("Grid size X: %1, Y: %2").arg(QString::number(m_stimPlanFractureDefinitionData->gridXs.size()),
        QString::number(numberOfDepthValues)));

    size_t numberOfTimeSteps = m_stimPlanFractureDefinitionData->timeSteps.size();
    RiaLogging::debug(QString("Number of time-steps: %1").arg(numberOfTimeSteps));

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
                
                std::vector<std::vector<double>> propertyValuesAtTimestep = getAllDepthDataAtTimeStep(xmlStream2, startingNegXsValues);
                
                bool valuesOK = numberOfParameterValuesOK(propertyValuesAtTimestep);
                if (!valuesOK)
                {
                    RiaLogging::error(QString("Inconsistency detected in reading XML file: '%1'").arg(dataFile.fileName()));
                    return;
                }

                m_stimPlanFractureDefinitionData->setDataAtTimeValue(parameter, unit, propertyValuesAtTimestep, timeStepValue);
               
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
        RiaLogging::info(QString("Successfully read XML file: '%1'").arg(fileName()));
    }

        RimEclipseView* activeView = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeReservoirView());
    if (!activeView) return;
    activeView->stimPlanColors->loadDataAndUpdate();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::setDefaultsBasedOnXMLfile()
{
    setDepthOfWellPathAtFracture();
    RiaLogging::info(QString("Setting well/fracture intersection depth at %1").arg(wellPathDepthAtFracture));
    timestepForPolygon = static_cast<int>(m_stimPlanFractureDefinitionData->totalNumberTimeSteps() - 1);
    bool polygonPropertySet = setPropertyForPolygonDefault();

    if (polygonPropertySet) RiaLogging::info(QString("Calculating polygon outline based on %1 at timestep %2").arg(parameterForPolygon).arg(m_stimPlanFractureDefinitionData->timeSteps[timestepForPolygon]));
    else                    RiaLogging::info(QString("Property for polygon calculation not set."));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimStimPlanFractureTemplate::setPropertyForPolygonDefault()
{
    //first option: Width
    for (std::pair<QString, QString> property : getStimPlanPropertyNamesUnits())
    {
        if (property.first == "WIDTH")
        {
            parameterForPolygon = property.first + " " + property.second;
            return true;
        }
    }
    //if width not found, use conductivity
    for (std::pair<QString, QString> property : getStimPlanPropertyNamesUnits())
    {
        if (property.first == "CONDUCTIVITY")
        {
            parameterForPolygon = property.first + " " + property.second;
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::loadDataAndUpdate()
{
    QString errorMessage;
    readStimPlanXMLFile(&errorMessage);
    if (errorMessage.size() > 0) RiaLogging::error(errorMessage);

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
QList<caf::PdmOptionItemInfo> RimStimPlanFractureTemplate::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &parameterForPolygon)
    {
        for (std::pair<QString, QString> nameUnit : getStimPlanPropertyNamesUnits())
        {
            options.push_back(caf::PdmOptionItemInfo(nameUnit.first + " [" + nameUnit.second +"]", nameUnit.first + " " + nameUnit.second));
        }
    }

    else if (fieldNeedingOptions == &timestepForPolygon)
    {
        std::vector<double> timeValues = getStimPlanTimeValues();
        int index = 0;
        for (double value : timeValues)
        {
            options.push_back(caf::PdmOptionItemInfo(QString::number(value), index));
            index++;
        }

    }

    return options;

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
        RiaLogging::error(QString("Negative depth values detected in XML file"));
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
void RimStimPlanFractureTemplate::fractureGeometry(std::vector<cvf::Vec3f>* nodeCoords, std::vector<cvf::uint>* triangleIndices, caf::AppEnum< RimDefines::UnitSystem > fractureUnit)
{
    
    if (m_stimPlanFractureDefinitionData.isNull())
    {
        loadDataAndUpdate();
    }


    std::vector<double> xCoords = getNegAndPosXcoords();
    cvf::uint lenXcoords = static_cast<cvf::uint>(xCoords.size());

    std::vector<double> adjustedDepths = adjustedDepthCoordsAroundWellPathPosition();

    if (fractureUnit == fractureTemplateUnit())
    {
        RiaLogging::debug(QString("No conversion necessary for %1").arg(name));
    }

    else if (fractureTemplateUnit() == RimDefines::UNITS_METRIC && fractureUnit == RimDefines::UNITS_FIELD)
    {
        RiaLogging::info(QString("Converting StimPlan geometry from metric to field for fracture template %1").arg(name));
        for (double& value : adjustedDepths) value = RimDefines::meterToFeet(value);
        for (double& value : xCoords)        value = RimDefines::meterToFeet(value);
    }
    else if (fractureTemplateUnit() == RimDefines::UNITS_FIELD && fractureUnit == RimDefines::UNITS_METRIC)
    {
        RiaLogging::info(QString("Converting StimPlan geometry from field to metric for fracture template %1").arg(name));
        for (double& value : adjustedDepths) value = RimDefines::feetToMeter(value);
        for (double& value : xCoords)        value = RimDefines::feetToMeter(value);
    }
    else
    {
        //Should never get here...
        RiaLogging::error(QString("Error: Could not convert units for fracture template %1").arg(name));
        return;
    }

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
std::vector<cvf::Vec3f> RimStimPlanFractureTemplate::fracturePolygon(caf::AppEnum< RimDefines::UnitSystem > fractureUnit)
{
    std::vector<cvf::Vec3f> polygon;
    QString parameterName;
    QString parameterUnit;

    if (parameterForPolygon().split(" ").size() > 1)
    {
        parameterName = parameterForPolygon().split(" ")[0];
        parameterUnit = parameterForPolygon().split(" ")[1];
    }
    else return polygon;

    std::vector<std::vector<double>> dataAtTimeStep = m_stimPlanFractureDefinitionData->getDataAtTimeIndex(parameterName, parameterUnit, timestepForPolygon);

    for (int k = 0; k < dataAtTimeStep.size(); k++)
    {
        for (int i = 0; i < dataAtTimeStep[k].size(); i++)
        {
            if ((dataAtTimeStep[k])[i] < 1e-7)  //polygon should consist of nodes with value 0
            {
                if ((i > 0) && ((dataAtTimeStep[k])[(i - 1)] > 1e-7)) //side neighbour cell different from 0
                {
                    polygon.push_back(cvf::Vec3f(static_cast<float>(m_stimPlanFractureDefinitionData->gridXs[i]),
                        static_cast<float>(m_stimPlanFractureDefinitionData->depths[k]) - wellPathDepthAtFracture, 0.0f));
                }
                else if ((k < dataAtTimeStep.size() - 1) && ((dataAtTimeStep[k + 1])[(i)] > 1e-7))//cell below different from 0
                {
                    polygon.push_back(cvf::Vec3f(static_cast<float>(m_stimPlanFractureDefinitionData->gridXs[i]),
                        static_cast<float>(m_stimPlanFractureDefinitionData->depths[k]) - wellPathDepthAtFracture, 0.0f));
                }
                else if ((k > 0) && ((dataAtTimeStep[k - 1 ])[(i)] > 1e-7))//cell above different from 0
                {
                    polygon.push_back(cvf::Vec3f(static_cast<float>(m_stimPlanFractureDefinitionData->gridXs[i]),
                        static_cast<float>(m_stimPlanFractureDefinitionData->depths[k]) - wellPathDepthAtFracture, 0.0f));
                }
            }
        }
    }

    sortPolygon(polygon);

    std::vector<cvf::Vec3f> negPolygon;
    for (const cvf::Vec3f& node : polygon)
    {
        cvf::Vec3f negNode = node;
        negNode.x() = -negNode.x();
        negPolygon.insert(negPolygon.begin(), negNode);
    }

    for (const cvf::Vec3f& negNode : negPolygon)
    {
        polygon.push_back(negNode);
    }

    //Adding first point last - to close the polygon
    if (polygon.size()>0) polygon.push_back(polygon[0]);


    if (fractureUnit == fractureTemplateUnit())
    {
        RiaLogging::debug(QString("No conversion necessary for %1").arg(name));
    }

    else if (fractureTemplateUnit() == RimDefines::UNITS_METRIC && fractureUnit == RimDefines::UNITS_FIELD)
    {
        RiaLogging::info(QString("Converting StimPlan geometry from metric to field for fracture template %1").arg(name));
        for (cvf::Vec3f& node : polygon)
        {
            float x = RimDefines::meterToFeet(node.x());
            float y = RimDefines::meterToFeet(node.y());
            float z = RimDefines::meterToFeet(node.z());
            node = cvf::Vec3f(x, y, z);
        }
    }
    else if (fractureTemplateUnit() == RimDefines::UNITS_FIELD && fractureUnit == RimDefines::UNITS_METRIC)
    {
        RiaLogging::info(QString("Converting StimPlan geometry from field to metric for fracture template %1").arg(name));
        for (cvf::Vec3f& node : polygon)
        {
            float x = RimDefines::feetToMeter(node.x());
            float y = RimDefines::feetToMeter(node.y());
            float z = RimDefines::feetToMeter(node.z());
            node = cvf::Vec3f(x, y, z);
        }
    }
    else
    {
        //Should never get here...
        RiaLogging::error(QString("Error: Could not convert units for fracture template %1").arg(name));
    }





    return polygon;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::sortPolygon(std::vector<cvf::Vec3f> &polygon)
{
    for (int i = 1; i < polygon.size() - 1; i++)
    {
        cvf::Vec3f lastNode = polygon[i - 1];
        cvf::Vec3f node = polygon[i];
        cvf::Vec3f nextNode = polygon[i + 1];

        if (node.y() == nextNode.y())
        {
            if (lastNode.x() < node.x() && node.x() > nextNode.x())
            {
                polygon[i] = nextNode;
                polygon[i + 1] = node;
            }
            else if (lastNode.x() > node.x() && node.x() < nextNode.x())
            {
                polygon[i] = nextNode;
                polygon[i + 1] = node;
            }
        }
    }
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

    caf::PdmUiGroup* polygonGroup = uiOrdering.addNewGroup("Fracture Polygon Basis");
    polygonGroup->add(&parameterForPolygon);
    polygonGroup->add(&timestepForPolygon);
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

