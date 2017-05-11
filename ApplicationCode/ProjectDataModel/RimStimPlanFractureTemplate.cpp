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
#include "RigStimPlanFracTemplateCell.h"
#include "RimStimPlanColors.h"
#include "RimStimPlanLegendConfig.h"

#include "RivWellFracturePartMgr.h"

#include "cafPdmObject.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiFilePathEditor.h"

#include "cvfVector3.h"

#include <QFileInfo>
#include <QMessageBox>

#include <algorithm>
#include <vector>
#include <cmath>
#include "RigFractureTransCalc.h"



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
    CAF_PDM_InitField(&activeTimeStepIndex, "activeTimeStepIndex", 0, "Active TimeStep Index", "", "", "");
    CAF_PDM_InitField(&showStimPlanMesh, "showStimPlanMesh", true, "Show StimPlan Mesh", "", "", "");

    //TODO: Is this correct way of doing this...?
    wellCenterStimPlanCellIJ = std::make_pair(0, 0);
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

    if (&activeTimeStepIndex == changedField)
    {
        setupStimPlanCells();

        //Changes to this parameters should change all fractures with this fracture template attached. 
        RimProject* proj;
        this->firstAncestorOrThisOfType(proj);
        if (proj)
        {
            std::vector<RimFracture*> fractures;
            proj->descendantsIncludingThisOfType(fractures);
            for (RimFracture* fracture : fractures)
            {
                if (fracture->attachedFractureDefinition() == this)
                {
                        fracture->stimPlanTimeIndexToPlot = activeTimeStepIndex;
                        fracture->setRecomputeGeometryFlag();
                }
            }
            proj->createDisplayModelAndRedrawAllViews();
        }
    }


    if (&wellPathDepthAtFracture == changedField || &parameterForPolygon == changedField || &activeTimeStepIndex == changedField || &showStimPlanMesh == changedField)
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

                if (parameter == "CONDUCTIVITY")
                {
                    if (unit == "md-ft")
                    {
                        fractureTemplateUnit = RimDefines::UNITS_FIELD;
                        RiaLogging::info(QString("Setting unit system to Field for StimPlan fracture template %1").arg(name));
                    }
                    if (unit == "md-m")
                    {
                        fractureTemplateUnit = RimDefines::UNITS_METRIC;
                        RiaLogging::info(QString("Setting unit system to Metric for StimPlan fracture template %1").arg(name));
                    }
                }


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
    activeTimeStepIndex = static_cast<int>(m_stimPlanFractureDefinitionData->totalNumberTimeSteps() - 1);
    bool polygonPropertySet = setPropertyForPolygonDefault();

    if (polygonPropertySet) RiaLogging::info(QString("Calculating polygon outline based on %1 at timestep %2").arg(parameterForPolygon).arg(m_stimPlanFractureDefinitionData->timeSteps[activeTimeStepIndex]));
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
            parameterForPolygon = property.first;
            return true;
        }
    }
    //if width not found, use conductivity
    for (std::pair<QString, QString> property : getStimPlanPropertyNamesUnits())
    {
        if (property.first == "CONDUCTIVITY")
        {
            parameterForPolygon = property.first;
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

    setupStimPlanCells();

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
std::vector<std::vector<double>> RimStimPlanFractureTemplate::getMirroredDataAtTimeIndex(const QString& resultName, const QString& unitName, size_t timeStepIndex) const
{
    std::vector<std::vector<double>> notMirrordedData = m_stimPlanFractureDefinitionData->getDataAtTimeIndex(resultName, unitName, timeStepIndex);
    std::vector<std::vector<double>> mirroredData;

    for (std::vector<double> depthData : notMirrordedData)
    {
        std::vector<double> mirrordDepthData = RivWellFracturePartMgr::mirrorDataAtSingleDepth(depthData);
        mirroredData.push_back(mirrordDepthData);
    }


    return mirroredData;
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
            //options.push_back(caf::PdmOptionItemInfo(nameUnit.first + " [" + nameUnit.second + "]", nameUnit.first + " " + nameUnit.second));
            options.push_back(caf::PdmOptionItemInfo(nameUnit.first, nameUnit.first));
        }
    }

    else if (fieldNeedingOptions == &activeTimeStepIndex)
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
QString RimStimPlanFractureTemplate::getUnitForStimPlanParameter(QString parameterName)
{
    QString unit;
    bool found = false;
    bool foundMultiple = false;

    for (std::pair<QString, QString> nameUnit : getStimPlanPropertyNamesUnits())
    {
        if (nameUnit.first == parameterName)
        {
            unit =  nameUnit.second;
            if (found) foundMultiple = true;
            found = true;
        }
    }

    if (foundMultiple)  RiaLogging::error(QString("Multiple units found for same parameter"));
    if (!found)         RiaLogging::error(QString("Unit for parameter not found"));
    return unit;
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
        adjustedDepth = -adjustedDepth;
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
void RimStimPlanFractureTemplate::getStimPlanDataAsPolygonsAndValues(std::vector<std::vector<cvf::Vec3d> > &cellsAsPolygons, std::vector<double> &parameterValues, const QString& resultName, const QString& unitName, size_t timeStepIndex)
{

    std::vector<std::vector<double>> propertyValuesAtTimeStep = getMirroredDataAtTimeIndex(resultName, unitName, timeStepIndex);

    cellsAsPolygons.clear();
    parameterValues.clear();

    //TODO: Code partly copied from RivWellFracturePartMgr - can this be combined in some function?
    std::vector<double> depthCoordsAtNodes = adjustedDepthCoordsAroundWellPathPosition();
    std::vector<double> xCoordsAtNodes = getNegAndPosXcoords();

    //Cells are around nodes instead of between nodes
    std::vector<double> xCoords;
    for (int i = 0; i < xCoordsAtNodes.size() - 1; i++) xCoords.push_back((xCoordsAtNodes[i] + xCoordsAtNodes[i + 1]) / 2);
    std::vector<double> depthCoords;
    for (int i = 0; i < depthCoordsAtNodes.size() - 1; i++) depthCoords.push_back((depthCoordsAtNodes[i] + depthCoordsAtNodes[i + 1]) / 2);

    for (int i = 0; i < xCoords.size() - 1; i++)
    {
        for (int j = 0; j < depthCoords.size() - 1; j++)
        {
            std::vector<cvf::Vec3d> cellAsPolygon;
            cellAsPolygon.push_back(cvf::Vec3d(static_cast<float>(xCoords[i]), static_cast<float>(depthCoords[j]), 0.0));
            cellAsPolygon.push_back(cvf::Vec3d(static_cast<float>(xCoords[i + 1]), static_cast<float>(depthCoords[j]), 0.0));
            cellAsPolygon.push_back(cvf::Vec3d(static_cast<float>(xCoords[i + 1]), static_cast<float>(depthCoords[j + 1]), 0.0));
            cellAsPolygon.push_back(cvf::Vec3d(static_cast<float>(xCoords[i]), static_cast<float>(depthCoords[j + 1]), 0.0));
            cellsAsPolygons.push_back(cellAsPolygon);
            //TODO: Values for both neg and pos x values...
            parameterValues.push_back(propertyValuesAtTimeStep[j+1][i+1]); //TODO test that this value exsist...

        }
    }


}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::setupStimPlanCells()
{
    RimEclipseView* activeView = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeReservoirView());
    if (!activeView) return;
    QString resultNameFromColors = activeView->stimPlanColors->resultName();
    QString resultUnitFromColors = activeView->stimPlanColors->unit();

    std::vector<RigStimPlanFracTemplateCell> stimPlanCells;

    bool wellCenterStimPlanCellFound = false;

    std::vector<std::vector<double>> displayPropertyValuesAtTimeStep = getMirroredDataAtTimeIndex(resultNameFromColors, resultUnitFromColors, activeTimeStepIndex);

    QString condUnit;
    if (fractureTemplateUnit == RimDefines::UNITS_METRIC) condUnit = "md-m";
    if (fractureTemplateUnit == RimDefines::UNITS_FIELD)  condUnit = "md-ft";
    std::vector<std::vector<double>> conductivityValuesAtTimeStep = getMirroredDataAtTimeIndex("CONDUCTIVITY", condUnit, activeTimeStepIndex);

    std::vector<double> depthCoordsAtNodes = adjustedDepthCoordsAroundWellPathPosition();
    std::vector<double> xCoordsAtNodes = getNegAndPosXcoords();

    std::vector<double> xCoords;
    for (int i = 0; i < xCoordsAtNodes.size() - 1; i++) xCoords.push_back((xCoordsAtNodes[i] + xCoordsAtNodes[i + 1]) / 2);
    std::vector<double> depthCoords;
    for (int i = 0; i < depthCoordsAtNodes.size() - 1; i++) depthCoords.push_back((depthCoordsAtNodes[i] + depthCoordsAtNodes[i + 1]) / 2);

    for (int i = 0; i < xCoords.size() - 1; i++)
    {
        for (int j = 0; j < depthCoords.size() - 1; j++)
        {
            std::vector<cvf::Vec3d> cellPolygon;
            cellPolygon.push_back(cvf::Vec3d(static_cast<float>(xCoords[i]), static_cast<float>(depthCoords[j]), 0.0));
            cellPolygon.push_back(cvf::Vec3d(static_cast<float>(xCoords[i + 1]), static_cast<float>(depthCoords[j]), 0.0));
            cellPolygon.push_back(cvf::Vec3d(static_cast<float>(xCoords[i + 1]), static_cast<float>(depthCoords[j + 1]), 0.0));
            cellPolygon.push_back(cvf::Vec3d(static_cast<float>(xCoords[i]), static_cast<float>(depthCoords[j + 1]), 0.0));

            RigStimPlanFracTemplateCell stimPlanCell(cellPolygon, i, j);
            if (conductivityValuesAtTimeStep.size() > 0) //Assuming vector to be of correct length, or no values
            {
                stimPlanCell.setConductivityValue(conductivityValuesAtTimeStep[j + 1][i + 1]);
            }
            else
            {
                stimPlanCell.setConductivityValue(cvf::UNDEFINED_DOUBLE);
            }

            if (displayPropertyValuesAtTimeStep.size() > 0)
            {
                stimPlanCell.setDisplayValue(displayPropertyValuesAtTimeStep[j + 1][i + 1]);
            }
            else
            {
                stimPlanCell.setDisplayValue(cvf::UNDEFINED_DOUBLE);
            }

            if (cellPolygon[0].x() < 0.0 && cellPolygon[1].x() > 0.0)
            {
                if (cellPolygon[1].y() > 0.0 && cellPolygon[2].y() < 0.0)
                {
                    wellCenterStimPlanCellIJ = std::make_pair(stimPlanCell.getI(), stimPlanCell.getJ());
                    RiaLogging::debug(QString("Setting wellCenterStimPlanCell at cell %1, %2").
                        arg(QString::number(stimPlanCell.getI()), QString::number(stimPlanCell.getJ())));
                    
                    wellCenterStimPlanCellFound = true;
                }
            }

            stimPlanCells.push_back(stimPlanCell);
        }
    }

    if (!wellCenterStimPlanCellFound)
    {
        RiaLogging::error("Did not find stim plan cell at well crossing!");
    }


    m_stimPlanCells = stimPlanCells;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<RigStimPlanFracTemplateCell>& RimStimPlanFractureTemplate::getStimPlanCells()
{
    return m_stimPlanCells;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimStimPlanFractureTemplate::getStimPlanRowPolygon(size_t i) //Row with constant depth
{
    std::vector<cvf::Vec3d> rowPolygon;

    std::vector<double> depthCoordsAtNodes = adjustedDepthCoordsAroundWellPathPosition();
    std::vector<double> xCoordsAtNodes = getNegAndPosXcoords();

    std::vector<double> xCoords;
    for (int i = 0; i < xCoordsAtNodes.size() - 1; i++) xCoords.push_back((xCoordsAtNodes[i] + xCoordsAtNodes[i + 1]) / 2);
    std::vector<double> depthCoords;
    for (int i = 0; i < depthCoordsAtNodes.size() - 1; i++) depthCoords.push_back((depthCoordsAtNodes[i] + depthCoordsAtNodes[i + 1]) / 2);

    rowPolygon.push_back(cvf::Vec3d(static_cast<float>(xCoords[0]), static_cast<float>(depthCoords[i]), 0.0));
    rowPolygon.push_back(cvf::Vec3d(static_cast<float>(xCoords.back()), static_cast<float>(depthCoords[i]), 0.0));
    rowPolygon.push_back(cvf::Vec3d(static_cast<float>(xCoords.back()), static_cast<float>(depthCoords[i+1]), 0.0));
    rowPolygon.push_back(cvf::Vec3d(static_cast<float>(xCoords[0]), static_cast<float>(depthCoords[i+1]), 0.0));
    
    return rowPolygon;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimStimPlanFractureTemplate::getStimPlanColPolygon(size_t j)
{
    std::vector<cvf::Vec3d> colPolygon;

    std::vector<double> depthCoordsAtNodes = adjustedDepthCoordsAroundWellPathPosition();
    std::vector<double> xCoordsAtNodes = getNegAndPosXcoords();

    std::vector<double> xCoords;
    for (int i = 0; i < xCoordsAtNodes.size() - 1; i++) xCoords.push_back((xCoordsAtNodes[i] + xCoordsAtNodes[i + 1]) / 2);
    std::vector<double> depthCoords;
    for (int i = 0; i < depthCoordsAtNodes.size() - 1; i++) depthCoords.push_back((depthCoordsAtNodes[i] + depthCoordsAtNodes[i + 1]) / 2);

    colPolygon.push_back(cvf::Vec3d(static_cast<float>(xCoords[j]), static_cast<float>(depthCoords[0]), 0.0));
    colPolygon.push_back(cvf::Vec3d(static_cast<float>(xCoords[j+1]), static_cast<float>(depthCoords[0]), 0.0));
    colPolygon.push_back(cvf::Vec3d(static_cast<float>(xCoords[j+1]), static_cast<float>(depthCoords.back()), 0.0));
    colPolygon.push_back(cvf::Vec3d(static_cast<float>(xCoords[j]), static_cast<float>(depthCoords.back()), 0.0));
    
    return colPolygon;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::pair<size_t, size_t> RimStimPlanFractureTemplate::getStimPlanCellAtWellCenter()
{
    return wellCenterStimPlanCellIJ;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimStimPlanFractureTemplate::getGlobalIndexFromIJ(size_t i, size_t j)
{
    size_t length_I = stimPlanGridNumberOfRows() - 1;
    size_t globIndex = j * length_I + i;

    return globIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigStimPlanFracTemplateCell& RimStimPlanFractureTemplate::stimPlanCellFromIndex(size_t index) const
{
    if (index < m_stimPlanCells.size())
    {
        const RigStimPlanFracTemplateCell& cell = m_stimPlanCells[index];
        return cell;
    }
    else
    {
        //TODO: Better error handling?
        RiaLogging::error("Requesting non-existent StimPlanCell");
        RiaLogging::error("Returning cell 0, results will be invalid");
        const RigStimPlanFracTemplateCell& cell = m_stimPlanCells[0];
        return cell;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3f> RimStimPlanFractureTemplate::fracturePolygon(caf::AppEnum< RimDefines::UnitSystem > fractureUnit)
{
    std::vector<cvf::Vec3f> polygon;

    QString parameterName = parameterForPolygon;
    QString parameterUnit = getUnitForStimPlanParameter(parameterName);
  
    std::vector<std::vector<double>> dataAtTimeStep = m_stimPlanFractureDefinitionData->getDataAtTimeIndex(parameterName, parameterUnit, activeTimeStepIndex);

    std::vector<double> adjustedDepths = adjustedDepthCoordsAroundWellPathPosition();

    for (int k = 0; k < dataAtTimeStep.size(); k++)
    {
        for (int i = 0; i < dataAtTimeStep[k].size(); i++)
        {
            if ((dataAtTimeStep[k])[i] < 1e-7)  //polygon should consist of nodes with value 0
            {
                if ((i > 0) && ((dataAtTimeStep[k])[(i - 1)] > 1e-7)) //side neighbour cell different from 0
                {
                    polygon.push_back(cvf::Vec3f(static_cast<float>(m_stimPlanFractureDefinitionData->gridXs[i]),
                        static_cast<float>(adjustedDepths[k]), 0.0f));
                }
                else if ((k < dataAtTimeStep.size() - 1) && ((dataAtTimeStep[k + 1])[(i)] > 1e-7))//cell below different from 0
                {
                    polygon.push_back(cvf::Vec3f(static_cast<float>(m_stimPlanFractureDefinitionData->gridXs[i]),
                        static_cast<float>(adjustedDepths[k]), 0.0f));
                }
                else if ((k > 0) && ((dataAtTimeStep[k - 1 ])[(i)] > 1e-7))//cell above different from 0
                {
                    polygon.push_back(cvf::Vec3f(static_cast<float>(m_stimPlanFractureDefinitionData->gridXs[i]),
                        static_cast<float>(adjustedDepths[k]), 0.0f));
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
    if (polygon.size() == 0) return;

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
size_t RimStimPlanFractureTemplate::stimPlanGridNumberOfColums()
{
    return getNegAndPosXcoords().size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimStimPlanFractureTemplate::stimPlanGridNumberOfRows()
{
    return adjustedDepthCoordsAroundWellPathPosition().size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimFractureTemplate::defineUiOrdering(uiConfigName, uiOrdering);

    uiOrdering.add(&name);
    uiOrdering.add(&showStimPlanMesh);

    caf::PdmUiGroup* fileGroup = uiOrdering.addNewGroup("Input");
    fileGroup->add(&m_stimPlanFileName);
    fileGroup->add(&activeTimeStepIndex);
    fileGroup->add(&wellPathDepthAtFracture);

    caf::PdmUiGroup* geometryGroup = uiOrdering.addNewGroup("Geometry");
    geometryGroup->add(&orientation);
    geometryGroup->add(&azimuthAngle);

    caf::PdmUiGroup* propertyGroup = uiOrdering.addNewGroup("Properties");
    propertyGroup->add(&fractureConductivity);
    propertyGroup->add(&skinFactor);
    propertyGroup->add(&perforationLength);
    propertyGroup->add(&perforationEfficiency);
    propertyGroup->add(&wellRadius);

    caf::PdmUiGroup* polygonGroup = uiOrdering.addNewGroup("Fracture Polygon Basis");
    polygonGroup->add(&parameterForPolygon);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute)
{
    RimFractureTemplate::defineEditorAttribute(field, uiConfigName, attribute);

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

