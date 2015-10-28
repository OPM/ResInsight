/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimWellLogExtractionCurve.h"

#include "RiaApplication.h"

#include "RigCaseCellResultsData.h"
#include "RigCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigGeoMechWellLogExtractor.h"
#include "RigResultAccessorFactory.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechResultDefinition.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellLogPlotCurve.h"
#include "RimWellLogPlotTrack.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimEclipseView.h"
#include "RimEclipseCellColors.h"
#include "RimGeoMechView.h"
#include "RimGeoMechCellColors.h"

#include "RiuWellLogPlotCurve.h"
#include "RiuWellLogTrackPlot.h"

#include "cafPdmUiTreeOrdering.h"

//==================================================================================================
///  
///  
//==================================================================================================

CAF_PDM_SOURCE_INIT(RimWellLogExtractionCurve, "RimWellLogExtractionCurve");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogExtractionCurve::RimWellLogExtractionCurve()
{
    CAF_PDM_InitObject("Well Log Curve", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellPath, "CurveWellPath", "Well Path", "", "", "");
    m_wellPath.uiCapability()->setUiChildrenHidden(true);
    //m_wellPath.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_case, "CurveCase", "Case", "", "", "");
    m_case.uiCapability()->setUiChildrenHidden(true);
    //m_case.uiCapability()->setUiHidden(true);
    CAF_PDM_InitFieldNoDefault(&m_eclipseResultDefinition, "CurveEclipseResult", "", "", "", "");
    m_eclipseResultDefinition.uiCapability()->setUiHidden(true);
    m_eclipseResultDefinition.uiCapability()->setUiChildrenHidden(true);
    m_eclipseResultDefinition = new RimEclipseResultDefinition;

    CAF_PDM_InitFieldNoDefault(&m_geomResultDefinition, "CurveGeomechResult", "", "", "", "");
    m_geomResultDefinition.uiCapability()->setUiHidden(true);
    m_geomResultDefinition.uiCapability()->setUiChildrenHidden(true);
    m_geomResultDefinition = new RimGeoMechResultDefinition;

    CAF_PDM_InitField(&m_timeStep, "CurveTimeStep", 0,"Time Step", "", "", "");

    // Add some space before name to indicate these belong to the Auto Name field
    CAF_PDM_InitField(&m_addCaseNameToCurveName, "AddCaseNameToCurveName", true, "   Case Name", "", "", "");
    CAF_PDM_InitField(&m_addPropertyToCurveName, "AddPropertyToCurveName", true, "   Property", "", "", "");
    CAF_PDM_InitField(&m_addWellNameToCurveName, "AddWellNameToCurveName", true, "   Well Name", "", "", "");
    CAF_PDM_InitField(&m_addTimestepToCurveName, "AddTimestepToCurveName", false, "   Timestep", "", "", "");
    CAF_PDM_InitField(&m_addDateToCurveName, "AddDateToCurveName", true, "   Date", "", "", "");

    updateOptionSensitivity();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogExtractionCurve::~RimWellLogExtractionCurve()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::setWellPath(RimWellPath* wellPath)
{
    m_wellPath = wellPath;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::setPropertiesFromView(RimView* view)
{
    m_case = view ? view->ownerCase() : NULL;

    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());
    m_eclipseResultDefinition->setEclipseCase(eclipseCase);
    m_geomResultDefinition->setGeoMechCase(geomCase);

    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(view);
    if (eclipseView)
    {
        m_eclipseResultDefinition->setResultType(eclipseView->cellResult()->resultType());
        m_eclipseResultDefinition->setResultVariable(eclipseView->cellResult()->resultVariable());
        m_timeStep = eclipseView->currentTimeStep();
    }

    RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>(view);
    if (geoMechView)
    {
        m_geomResultDefinition->setResultAddress(geoMechView->cellResult()->resultAddress());
        m_timeStep = geoMechView->currentTimeStep();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimWellLogPlotCurve::fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == &m_case)
    {
        this->updatePlotData();
    }    
    else if (changedField == &m_wellPath)
    {
        this->updatePlotData();
    }
    else if (changedField == &m_timeStep)
    {
        this->updatePlotData();
    }

    if (changedField == &m_addCaseNameToCurveName ||
        changedField == &m_addPropertyToCurveName ||
        changedField == &m_addWellNameToCurveName ||
        changedField == &m_addTimestepToCurveName ||
        changedField == &m_addDateToCurveName)
    {
        this->uiCapability()->updateConnectedEditors();
        updateCurveName();
        updatePlotTitle();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::updatePlotData()
{
    RimWellLogPlotCurve::updatePlotConfiguration();

    if (isCurveVisible())
    {
        // Make sure we have set correct case data into the result definitions.

        RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());
        m_eclipseResultDefinition->setEclipseCase(eclipseCase);
        m_geomResultDefinition->setGeoMechCase(geomCase);

        RimWellLogPlotCollection* wellLogCollection = NULL;
        this->firstAnchestorOrThisOfType(wellLogCollection);
        CVF_ASSERT(wellLogCollection);

        cvf::ref<RigEclipseWellLogExtractor> eclExtractor = wellLogCollection->findOrCreateExtractor(m_wellPath, eclipseCase);
        cvf::ref<RigGeoMechWellLogExtractor> geomExtractor = wellLogCollection->findOrCreateExtractor(m_wellPath, geomCase);

        std::vector<double> values;
        std::vector<double> measuredDepthValues;
        std::vector<double> tvDepthValues;

        if (eclExtractor.notNull())
        {
            RimWellLogPlot* wellLogPlot;
            firstAnchestorOrThisOfType(wellLogPlot);
            CVF_ASSERT(wellLogPlot);

            measuredDepthValues = eclExtractor->measuredDepth();
            if (wellLogPlot->depthType() == RimWellLogPlot::TRUE_VERTICAL_DEPTH)
            {
                tvDepthValues = eclExtractor->trueVerticalDepth();
            }

            RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(m_eclipseResultDefinition->porosityModel());
            m_eclipseResultDefinition->loadResult();

            cvf::ref<RigResultAccessor> resAcc = RigResultAccessorFactory::createResultAccessor(
                eclipseCase->reservoirData(), 0,
                porosityModel,
                m_timeStep,
                m_eclipseResultDefinition->resultVariable());

            if (resAcc.notNull())
            {
                eclExtractor->curveData(resAcc.p(), &values);
            }
        }
        else if (geomExtractor.notNull()) // geomExtractor
        {
            RimWellLogPlot* wellLogPlot;
            firstAnchestorOrThisOfType(wellLogPlot);
            CVF_ASSERT(wellLogPlot);

            measuredDepthValues =  geomExtractor->measuredDepth();
            if (wellLogPlot->depthType() == RimWellLogPlot::TRUE_VERTICAL_DEPTH)
            {
                tvDepthValues = geomExtractor->trueVerticalDepth();
            }

            m_geomResultDefinition->loadResult();

            geomExtractor->curveData(m_geomResultDefinition->resultAddress(), m_timeStep, &values);
        }

        m_curveData = new RigWellLogCurveData;
        if (values.size() && measuredDepthValues.size())
        {
            if (!tvDepthValues.size())
            {
                m_curveData->setValuesAndMD(values, measuredDepthValues, true);
            }
            else
            {
                m_curveData->setValuesWithTVD(values, measuredDepthValues, tvDepthValues);
            }
        }

        m_qwtPlotCurve->setCurveData(m_curveData.p());
        zoomAllOwnerTrackAndPlot();

        if (m_ownerQwtTrack) m_ownerQwtTrack->replot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogExtractionCurve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
   QList<caf::PdmOptionItemInfo> optionList;

    if (fieldNeedingOptions == &m_wellPath)
    {
        RimProject* proj = RiaApplication::instance()->project();
        if (proj->activeOilField()->wellPathCollection())
        {
            caf::PdmChildArrayField<RimWellPath*>& wellPaths =  proj->activeOilField()->wellPathCollection()->wellPaths;

            for (size_t i = 0; i< wellPaths.size(); i++)
            {
                optionList.push_back(caf::PdmOptionItemInfo(wellPaths[i]->name(), QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(wellPaths[i]))));
            }

            if (optionList.size() > 0)
            {
                optionList.push_front(caf::PdmOptionItemInfo("None", QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(NULL))));
            }
        }
    }
    else if (fieldNeedingOptions == &m_case)
    {
        RimProject* proj = RiaApplication::instance()->project();
        std::vector<RimCase*> cases;

        proj->allCases(cases);

        for (size_t i = 0; i< cases.size(); i++)
        {
            optionList.push_back(caf::PdmOptionItemInfo(cases[i]->caseUserDescription(), QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(cases[i]))));
        }

        if (optionList.size() > 0)
        {
            optionList.push_front(caf::PdmOptionItemInfo("None", QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(NULL))));
        }
    }
    else if (fieldNeedingOptions == &m_timeStep)
    {
        QStringList timeStepNames;

        if (m_case)
        {
            timeStepNames = m_case->timeStepStrings();
        }

        for (int i = 0; i < timeStepNames.size(); i++)
        {
            optionList.push_back(caf::PdmOptionItemInfo(timeStepNames[i], i));
        }
    }

    return optionList;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup("Curve Data");
    curveDataGroup->add(&m_wellPath);

    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());

    curveDataGroup->add(&m_wellPath);

    curveDataGroup->add(&m_case);
    if (eclipseCase)
    {
        curveDataGroup->add(&(m_eclipseResultDefinition->m_resultTypeUiField));
        curveDataGroup->add(&(m_eclipseResultDefinition->m_porosityModelUiField));
        curveDataGroup->add(&(m_eclipseResultDefinition->m_resultVariableUiField));

        if (m_eclipseResultDefinition->hasDynamicResult())
        {
            curveDataGroup->add(&m_timeStep);
        }
    }
    else if (geomCase)
    {
        curveDataGroup->add(&(m_geomResultDefinition->m_resultPositionTypeUiField));
        curveDataGroup->add(&(m_geomResultDefinition->m_resultVariableUiField));

        curveDataGroup->add(&m_timeStep);
    }

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup("Appearance");
    appearanceGroup->add(&m_curveColor);
    appearanceGroup->add(&m_curveName);
    appearanceGroup->add(&m_autoName);
    if (m_autoName)
    {
        appearanceGroup->add(&m_addWellNameToCurveName);
        appearanceGroup->add(&m_addCaseNameToCurveName);
        appearanceGroup->add(&m_addPropertyToCurveName);
        appearanceGroup->add(&m_addDateToCurveName);
        appearanceGroup->add(&m_addTimestepToCurveName);
    }


    uiOrdering.setForgetRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::initAfterRead()
{
    RimWellLogPlotCurve::initAfterRead();

    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());

    m_eclipseResultDefinition->setEclipseCase(eclipseCase);
    m_geomResultDefinition->setGeoMechCase(geomCase);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    uiTreeOrdering.setForgetRemainingFields(true);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogExtractionCurve::createCurveName()
{
    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());
    QString generatedCurveName;

    if (m_addWellNameToCurveName && m_wellPath)
    {
        generatedCurveName += wellName();
    }

    if (m_addCaseNameToCurveName && m_case())
    {
        if (!generatedCurveName.isEmpty())
        {
            generatedCurveName += ", ";
        }

        generatedCurveName += m_case->caseUserDescription();
    }

    if (m_addPropertyToCurveName)
    {
        if (!generatedCurveName.isEmpty())
        {
            generatedCurveName += ",";
        }

        generatedCurveName += wellLogChannelName();
    }

    if (m_addTimestepToCurveName || m_addDateToCurveName)
    {
        size_t maxTimeStep = 0;
        
        if (eclipseCase)
        {
            RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(m_eclipseResultDefinition->porosityModel());
            if (eclipseCase->reservoirData())
            {
                maxTimeStep = eclipseCase->reservoirData()->results(porosityModel)->maxTimeStepCount();
            }
        }
        else if (geomCase)
        {
            if (geomCase->geoMechData())
            {
                maxTimeStep = geomCase->geoMechData()->femPartResults()->frameCount();
            }
        }

        if (m_addDateToCurveName)
        {
            QString dateString = wellDate();
            if (!dateString.isEmpty())
            {
                if (!generatedCurveName.isEmpty())
                {
                    generatedCurveName += ", ";
                }

                generatedCurveName += dateString;
            }
        }

        if (m_addTimestepToCurveName)
        {
            if (!generatedCurveName.isEmpty())
            {
                generatedCurveName += ", ";
            }

            generatedCurveName += QString("[%1/%2]").arg(m_timeStep()).arg(maxTimeStep);
        }
    }

    return generatedCurveName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogExtractionCurve::wellLogChannelName() const
{
    RimGeoMechCase* geoMechCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());

    QString name;
    if (eclipseCase)
    {
        name = m_eclipseResultDefinition->resultVariable();
    }
    else if (geoMechCase)
    {
        QString resCompName = m_geomResultDefinition->resultComponentUiName();
        if (resCompName.isEmpty())
        {
            name = m_geomResultDefinition->resultFieldUiName();
        }
        else
        {
            name = m_geomResultDefinition->resultFieldUiName() + "." + resCompName;
        }
    }

    return name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogExtractionCurve::wellName() const
{
    return m_wellPath->name();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogExtractionCurve::wellDate() const
{
    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());

    QStringList timeStepNames;

    if (eclipseCase)
    {
        RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(m_eclipseResultDefinition->porosityModel());
        if (eclipseCase->reservoirData())
        {
            timeStepNames = eclipseCase->timeStepStrings();
        }
    }
    else if (geomCase)
    {
        if (geomCase->geoMechData())
        {
            timeStepNames = geomCase->timeStepStrings();
        }
    }

    return (m_timeStep < timeStepNames.size()) ? timeStepNames[m_timeStep] : "";
}
