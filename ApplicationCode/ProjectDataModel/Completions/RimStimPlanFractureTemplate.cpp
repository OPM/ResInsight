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
#include "RigFractureTransCalc.h"
#include "RigFractureGrid.h"

#include "RimEclipseView.h"
#include "RimFracture.h"
#include "RimProject.h"
#include "RigFractureCell.h"
#include "RimStimPlanColors.h"
#include "RimStimPlanLegendConfig.h"
#include "RimFractureContainment.h"

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
#include "RifStimPlanXmlReader.h"



CAF_PDM_SOURCE_INIT(RimStimPlanFractureTemplate, "RimStimPlanFractureTemplate");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStimPlanFractureTemplate::RimStimPlanFractureTemplate(void)
{
    CAF_PDM_InitObject("Fracture Template", ":/FractureTemplate16x16.png", "", "");

    CAF_PDM_InitField(&m_stimPlanFileName, "StimPlanFileName", QString(""), "File Name", "", "", "");
    m_stimPlanFileName.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_wellPathDepthAtFracture, "WellPathDepthAtFracture", 0.0, "Well/Fracture Intersection Depth", "", "", "");
    m_wellPathDepthAtFracture.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_parameterForPolygon, "parameterForPolyton", QString(""), "Parameter", "", "", "");
    CAF_PDM_InitField(&m_activeTimeStepIndex, "activeTimeStepIndex", 0, "Active TimeStep Index", "", "", "");
    CAF_PDM_InitField(&m_showStimPlanMesh, "showStimPlanMesh", true, "Show StimPlan Mesh", "", "", "");

    m_fractureGrid = new RigFractureGrid();
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

    if (&m_activeTimeStepIndex == changedField)
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
                if (fracture->fractureTemplate() == this)
                {
                        fracture->stimPlanTimeIndexToPlot = m_activeTimeStepIndex;
                        fracture->clearDisplayGeometryCache();
                }
            }
            proj->createDisplayModelAndRedrawAllViews();
        }
    }


    if (&m_wellPathDepthAtFracture == changedField || &m_parameterForPolygon == changedField || &m_activeTimeStepIndex == changedField || &m_showStimPlanMesh == changedField)
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
                if (fracture->fractureTemplate() == this)
                {
                    fracture->clearDisplayGeometryCache();
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
void RimStimPlanFractureTemplate::setDefaultsBasedOnXMLfile()
{
    setDepthOfWellPathAtFracture();
    RiaLogging::info(QString("Setting well/fracture intersection depth at %1").arg(m_wellPathDepthAtFracture));
    m_activeTimeStepIndex = static_cast<int>(m_stimPlanFractureDefinitionData->totalNumberTimeSteps() - 1);
    bool polygonPropertySet = setPropertyForPolygonDefault();

    if (polygonPropertySet) RiaLogging::info(QString("Calculating polygon outline based on %1 at timestep %2").arg(m_parameterForPolygon).arg(m_stimPlanFractureDefinitionData->timeSteps[m_activeTimeStepIndex]));
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
            m_parameterForPolygon = property.first;
            return true;
        }
    }
    //if width not found, use conductivity
    for (std::pair<QString, QString> property : getStimPlanPropertyNamesUnits())
    {
        if (property.first == "CONDUCTIVITY")
        {
            m_parameterForPolygon = property.first;
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
    m_stimPlanFractureDefinitionData = RifStimPlanXmlReader::readStimPlanXMLFile( m_stimPlanFileName(), &errorMessage);
    if (errorMessage.size() > 0) RiaLogging::error(errorMessage);

    if (m_stimPlanFractureDefinitionData.notNull())
    {
        fractureTemplateUnit = m_stimPlanFractureDefinitionData->unitSet;
    }
    else
    {
        fractureTemplateUnit = RimUnitSystem::UNITS_UNKNOWN; 
    }

    setupStimPlanCells();

    // Todo: Must update all views using this fracture template
    RimEclipseView* activeView = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeReservoirView());
    if (activeView) activeView->stimPlanColors->loadDataAndUpdate();

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

    if (fieldNeedingOptions == &m_parameterForPolygon)
    {
        for (std::pair<QString, QString> nameUnit : getStimPlanPropertyNamesUnits())
        {
            //options.push_back(caf::PdmOptionItemInfo(nameUnit.first + " [" + nameUnit.second + "]", nameUnit.first + " " + nameUnit.second));
            options.push_back(caf::PdmOptionItemInfo(nameUnit.first, nameUnit.first));
        }
    }

    else if (fieldNeedingOptions == &m_activeTimeStepIndex)
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
void RimStimPlanFractureTemplate::setDepthOfWellPathAtFracture()
{
    if (!m_stimPlanFractureDefinitionData.isNull())
    {
        double firstDepth = m_stimPlanFractureDefinitionData->depths[0];
        double lastDepth  = m_stimPlanFractureDefinitionData->depths[m_stimPlanFractureDefinitionData->depths.size()-1];
        double averageDepth = (firstDepth + lastDepth) / 2;
        m_wellPathDepthAtFracture = averageDepth;
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
void RimStimPlanFractureTemplate::fractureTriangleGeometry(std::vector<cvf::Vec3f>* nodeCoords, 
                                                           std::vector<cvf::uint>* triangleIndices, 
                                                           RimUnitSystem::UnitSystem neededUnit)
{
    
    if (m_stimPlanFractureDefinitionData.isNull())
    {
        loadDataAndUpdate();
    }


    std::vector<double> xCoords = m_stimPlanFractureDefinitionData->getNegAndPosXcoords();
    cvf::uint lenXcoords = static_cast<cvf::uint>(xCoords.size());

    std::vector<double> adjustedDepths = m_stimPlanFractureDefinitionData->adjustedDepthCoordsAroundWellPathPosition(m_wellPathDepthAtFracture());

    if (neededUnit == fractureTemplateUnit())
    {
        RiaLogging::debug(QString("No conversion necessary for %1").arg(name));
    }

    else if (fractureTemplateUnit() == RimUnitSystem::UNITS_METRIC && neededUnit == RimUnitSystem::UNITS_FIELD)
    {
        RiaLogging::info(QString("Converting StimPlan geometry from metric to field for fracture template %1").arg(name));
        for (double& value : adjustedDepths) value = RimUnitSystem::meterToFeet(value);
        for (double& value : xCoords)        value = RimUnitSystem::meterToFeet(value);
    }
    else if (fractureTemplateUnit() == RimUnitSystem::UNITS_FIELD && neededUnit == RimUnitSystem::UNITS_METRIC)
    {
        RiaLogging::info(QString("Converting StimPlan geometry from field to metric for fracture template %1").arg(name));
        for (double& value : adjustedDepths) value = RimUnitSystem::feetToMeter(value);
        for (double& value : xCoords)        value = RimUnitSystem::feetToMeter(value);
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
        propertyNamesUnits = m_stimPlanFractureDefinitionData->getStimPlanPropertyNamesUnits();
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
/// OBSOLETE ! Only used for upscaling code
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::getStimPlanDataAsPolygonsAndValues(std::vector<std::vector<cvf::Vec3d> > &cellsAsPolygons, 
                                                                     std::vector<double> &parameterValues, 
                                                                     const QString& resultName, 
                                                                     const QString& unitName, 
                                                                     size_t timeStepIndex)
{
    std::vector< std::vector<double> > propertyValuesAtTimeStep = m_stimPlanFractureDefinitionData->getMirroredDataAtTimeIndex(resultName, unitName, timeStepIndex);

    cellsAsPolygons.clear();
    parameterValues.clear();

    //TODO: Code partly copied from RivWellFracturePartMgr - can this be combined in some function?
    std::vector<double> depthCoordsAtNodes = m_stimPlanFractureDefinitionData->adjustedDepthCoordsAroundWellPathPosition(m_wellPathDepthAtFracture());
    std::vector<double> xCoordsAtNodes = m_stimPlanFractureDefinitionData->getNegAndPosXcoords();

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

    std::vector<RigFractureCell> stimPlanCells;
    std::pair<size_t, size_t> wellCenterStimPlanCellIJ = std::make_pair(0,0);

    bool wellCenterStimPlanCellFound = false;

    std::vector<std::vector<double>> displayPropertyValuesAtTimeStep = m_stimPlanFractureDefinitionData->getMirroredDataAtTimeIndex(resultNameFromColors, resultUnitFromColors, m_activeTimeStepIndex);

    QString condUnit;
    if (fractureTemplateUnit == RimUnitSystem::UNITS_METRIC) condUnit = "md-m";
    if (fractureTemplateUnit == RimUnitSystem::UNITS_FIELD)  condUnit = "md-ft";
    std::vector<std::vector<double>> conductivityValuesAtTimeStep = m_stimPlanFractureDefinitionData->getMirroredDataAtTimeIndex("CONDUCTIVITY", condUnit, m_activeTimeStepIndex);

    std::vector<double> depthCoordsAtNodes = m_stimPlanFractureDefinitionData->adjustedDepthCoordsAroundWellPathPosition(m_wellPathDepthAtFracture());
    std::vector<double> xCoordsAtNodes = m_stimPlanFractureDefinitionData->getNegAndPosXcoords();

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

            RigFractureCell stimPlanCell(cellPolygon, i, j);
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

    m_fractureGrid->setFractureCells(stimPlanCells);
    m_fractureGrid->setWellCenterFractureCellIJ(wellCenterStimPlanCellIJ);
    m_fractureGrid->setICellCount(m_stimPlanFractureDefinitionData->getNegAndPosXcoords().size() - 2);
    m_fractureGrid->setJCellCount(m_stimPlanFractureDefinitionData->adjustedDepthCoordsAroundWellPathPosition(m_wellPathDepthAtFracture()).size() - 2);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigFractureGrid* RimStimPlanFractureTemplate::fractureGrid() const
{
    return m_fractureGrid.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3f> RimStimPlanFractureTemplate::fractureBorderPolygon(RimUnitSystem::UnitSystem fractureUnit)
{
    std::vector<cvf::Vec3f> polygon;

    QString parameterName = m_parameterForPolygon;
    QString parameterUnit = getUnitForStimPlanParameter(parameterName);
  
    std::vector<std::vector<double>> dataAtTimeStep = m_stimPlanFractureDefinitionData->getDataAtTimeIndex(parameterName, parameterUnit, m_activeTimeStepIndex);

    std::vector<double> adjustedDepths = m_stimPlanFractureDefinitionData->adjustedDepthCoordsAroundWellPathPosition(m_wellPathDepthAtFracture());

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

    else if (fractureTemplateUnit() == RimUnitSystem::UNITS_METRIC && fractureUnit == RimUnitSystem::UNITS_FIELD)
    {
        RiaLogging::info(QString("Converting StimPlan geometry from metric to field for fracture template %1").arg(name));
        for (cvf::Vec3f& node : polygon)
        {
            float x = RimUnitSystem::meterToFeet(node.x());
            float y = RimUnitSystem::meterToFeet(node.y());
            float z = RimUnitSystem::meterToFeet(node.z());
            node = cvf::Vec3f(x, y, z);
        }
    }
    else if (fractureTemplateUnit() == RimUnitSystem::UNITS_FIELD && fractureUnit == RimUnitSystem::UNITS_METRIC)
    {
        RiaLogging::info(QString("Converting StimPlan geometry from field to metric for fracture template %1").arg(name));
        for (cvf::Vec3f& node : polygon)
        {
            float x = RimUnitSystem::feetToMeter(node.x());
            float y = RimUnitSystem::feetToMeter(node.y());
            float z = RimUnitSystem::feetToMeter(node.z());
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
void RimStimPlanFractureTemplate::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimFractureTemplate::defineUiOrdering(uiConfigName, uiOrdering);

    uiOrdering.add(&name);
    uiOrdering.add(&m_showStimPlanMesh);

    caf::PdmUiGroup* fileGroup = uiOrdering.addNewGroup("Input");
    fileGroup->add(&m_stimPlanFileName);
    fileGroup->add(&m_activeTimeStepIndex);
    fileGroup->add(&m_wellPathDepthAtFracture);

    caf::PdmUiGroup* geometryGroup = uiOrdering.addNewGroup("Geometry");
    geometryGroup->add(&orientationType);
    geometryGroup->add(&azimuthAngle);

    caf::PdmUiGroup* trGr = uiOrdering.addNewGroup("Fracture Truncation");
    m_fractureContainment()->defineUiOrdering(uiConfigName, *trGr);

    caf::PdmUiGroup* propertyGroup = uiOrdering.addNewGroup("Properties");
    propertyGroup->add(&conductivityType);
    propertyGroup->add(&skinFactor);
    propertyGroup->add(&perforationLength);
    propertyGroup->add(&perforationEfficiency);
    propertyGroup->add(&wellDiameter);

    caf::PdmUiGroup* polygonGroup = uiOrdering.addNewGroup("Fracture Polygon Basis");
    polygonGroup->add(&m_parameterForPolygon);
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

    if (field == &m_wellPathDepthAtFracture)
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

