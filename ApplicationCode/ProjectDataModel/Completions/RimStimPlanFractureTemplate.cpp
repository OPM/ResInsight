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

#include "RifStimPlanXmlReader.h"

#include "RigStimPlanFractureDefinition.h"
#include "RigFractureGrid.h"

#include "RigFractureCell.h"
#include "RimEclipseView.h"
#include "RimFracture.h"
#include "RimFractureContainment.h"
#include "RimProject.h"
#include "RimStimPlanColors.h"
#include "RimStimPlanLegendConfig.h"
#include "RimTools.h"

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

    CAF_PDM_InitField(&m_borderPolygonResultName, "parameterForPolyton", QString(""), "Parameter", "", "", "");
    m_borderPolygonResultName.uiCapability()->setUiHidden(true);

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
        updateFractureGrid();

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


    if (&m_wellPathDepthAtFracture == changedField || &m_borderPolygonResultName == changedField || &m_activeTimeStepIndex == changedField || &m_showStimPlanMesh == changedField)
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
void RimStimPlanFractureTemplate::updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath)
{
    m_stimPlanFileName = RimTools::relocateFile(m_stimPlanFileName(), newProjectPath, oldProjectPath, nullptr, nullptr);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::setDefaultsBasedOnXMLfile()
{
    setDepthOfWellPathAtFracture();
    RiaLogging::info(QString("Setting well/fracture intersection depth at %1").arg(m_wellPathDepthAtFracture));
    m_activeTimeStepIndex = static_cast<int>(m_stimPlanFractureDefinitionData->totalNumberTimeSteps() - 1);
    bool polygonPropertySet = setBorderPolygonResultNameToDefault();

    if (polygonPropertySet) RiaLogging::info(QString("Calculating polygon outline based on %1 at timestep %2").arg(m_borderPolygonResultName).arg(m_stimPlanFractureDefinitionData->timeSteps()[m_activeTimeStepIndex]));
    else                    RiaLogging::info(QString("Property for polygon calculation not set."));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimStimPlanFractureTemplate::setBorderPolygonResultNameToDefault()
{
    //first option: Width
    for (std::pair<QString, QString> property : resultNamesWithUnit())
    {
        if (property.first == "WIDTH")
        {
            m_borderPolygonResultName = property.first;
            return true;
        }
    }
    //if width not found, use conductivity
    for (std::pair<QString, QString> property : resultNamesWithUnit())
    {
        if (property.first == "CONDUCTIVITY")
        {
            m_borderPolygonResultName = property.first;
            return true;
        }
    }
    //else: Set to first property
    if (resultNamesWithUnit().size() > 0)
    {
        m_borderPolygonResultName = resultNamesWithUnit()[0].first;
        return true;
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
        fractureTemplateUnit = m_stimPlanFractureDefinitionData->unitSet();
    }
    else
    {
        fractureTemplateUnit = RiaEclipseUnitTools::UNITS_UNKNOWN; 
    }

    updateFractureGrid();

    // Todo: Must update all views using this fracture template
    RimEclipseView* activeView = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeReservoirView());
    if (activeView) activeView->stimPlanColors->loadDataAndUpdate();

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimStimPlanFractureTemplate::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_borderPolygonResultName)
    {
        for (std::pair<QString, QString> nameUnit : resultNamesWithUnit())
        {
            //options.push_back(caf::PdmOptionItemInfo(nameUnit.first + " [" + nameUnit.second + "]", nameUnit.first + " " + nameUnit.second));
            options.push_back(caf::PdmOptionItemInfo(nameUnit.first, nameUnit.first));
        }
    }

    else if (fieldNeedingOptions == &m_activeTimeStepIndex)
    {
        std::vector<double> timeValues = timeSteps();
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
        double firstDepth = m_stimPlanFractureDefinitionData->minDepth();
        double lastDepth  = m_stimPlanFractureDefinitionData->maxDepth();
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

    for (std::pair<QString, QString> nameUnit : resultNamesWithUnit())
    {
        if (nameUnit.first == parameterName)
        {
            unit =  nameUnit.second;
            if (found) foundMultiple = true;
            found = true;
        }
    }

    if (foundMultiple)  RiaLogging::error(QString("Multiple units found for same parameter"));
    if (!found)         RiaLogging::error(QString("Requested unit / parameter not found for %1 template").arg(name()));
    return unit;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RimStimPlanFractureTemplate::timeSteps() 
{
    if (m_stimPlanFractureDefinitionData.isNull()) loadDataAndUpdate();
    return m_stimPlanFractureDefinitionData->timeSteps();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, QString> > RimStimPlanFractureTemplate::resultNamesWithUnit() const
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
void RimStimPlanFractureTemplate::computeMinMax(const QString& resultName, const QString& unitName, double* minValue, double* maxValue, double* posClosestToZero, double* negClosestToZero) const
{
    if (m_stimPlanFractureDefinitionData.notNull())
    {
        m_stimPlanFractureDefinitionData->computeMinMax(resultName, unitName, minValue, maxValue, posClosestToZero, negClosestToZero);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>> RimStimPlanFractureTemplate::resultValues(const QString& resultName, const QString& unitName, size_t timeStepIndex) const
{
    return m_stimPlanFractureDefinitionData->getDataAtTimeIndex(resultName, unitName, timeStepIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStimPlanFractureTemplate::fractureGridResults(const QString& resultName, const QString& unitName, size_t timeStepIndex) const
{
   return m_stimPlanFractureDefinitionData->fractureGridResults(resultName, unitName, timeStepIndex);
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
void RimStimPlanFractureTemplate::updateFractureGrid()
{
    m_fractureGrid = m_stimPlanFractureDefinitionData->createFractureGrid(m_activeTimeStepIndex,
                                                                          fractureTemplateUnit,
                                                                          m_wellPathDepthAtFracture);
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::fractureTriangleGeometry(std::vector<cvf::Vec3f>* nodeCoords, 
                                                           std::vector<cvf::uint>* triangleIndices, 
                                                           RiaEclipseUnitTools::UnitSystem neededUnit)
{

    if (m_stimPlanFractureDefinitionData.isNull())
    {
        loadDataAndUpdate();
    }

    m_stimPlanFractureDefinitionData->createFractureTriangleGeometry(m_wellPathDepthAtFracture,
                                                                     neededUnit,
                                                                     name,
                                                                     nodeCoords,
                                                                     triangleIndices);
    return;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3f> RimStimPlanFractureTemplate::fractureBorderPolygon(RiaEclipseUnitTools::UnitSystem neededUnit)
{

    QString parameterName = m_borderPolygonResultName;
    QString parameterUnit = getUnitForStimPlanParameter(parameterName);
  
    return m_stimPlanFractureDefinitionData->createFractureBorderPolygon(parameterName,
                                                                         parameterUnit,
                                                                         m_activeTimeStepIndex,
                                                                         m_wellPathDepthAtFracture,
                                                                         neededUnit,
                                                                         name);
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
        if ( !m_stimPlanFractureDefinitionData.isNull() && (m_stimPlanFractureDefinitionData->depthCount() > 0) )
        {
            caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);
            if ( myAttr )
            {
                myAttr->m_minimum = m_stimPlanFractureDefinitionData->minDepth();
                myAttr->m_maximum = m_stimPlanFractureDefinitionData->maxDepth();
            }
        }
    }
}

