/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "Rim3dWellLogExtractionCurve.h"

#include "RigWellLogFile.h"

#include "RiaExtractionTools.h"
#include "RiaCurveDataTools.h"
#include "RigEclipseCaseData.h"
#include "RigGeoMechCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigGeoMechWellLogExtractor.h"
#include "RigResultAccessorFactory.h"
#include "RigCaseCellResultsData.h"
#include "RigFemPartResultsCollection.h"
#include "RigWellPath.h"
#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"
#include "Rim3dView.h"
#include "RimWellLogExtractionCurveNameConfig.h"
#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimTools.h"
#include "RimWellLogFile.h"
#include "RimWellLogFileChannel.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellPath.h"

#include "cafUtils.h"

#include <QFileInfo>

#include <set>

//==================================================================================================
///
///
//==================================================================================================

CAF_PDM_SOURCE_INIT(Rim3dWellLogExtractionCurve, "Rim3dWellLogExtractionCurve");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dWellLogExtractionCurve::Rim3dWellLogExtractionCurve()
{
    CAF_PDM_InitObject("3d Well Log Extraction Curve", ":/WellLogCurve16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_case, "CurveCase", "Case", "", "", "");
    m_case.uiCapability()->setUiTreeChildrenHidden(true);
    m_case = nullptr;

    CAF_PDM_InitField(&m_timeStep, "CurveTimeStep", -1, "Time Step", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_eclipseResultDefinition, "CurveEclipseResult", "", "", "", "");
    m_eclipseResultDefinition.uiCapability()->setUiHidden(true);
    m_eclipseResultDefinition.uiCapability()->setUiTreeChildrenHidden(true);
    m_eclipseResultDefinition = new RimEclipseResultDefinition;
    m_eclipseResultDefinition->findField("MResultType")->uiCapability()->setUiName("Result Type");

    CAF_PDM_InitFieldNoDefault(&m_geomResultDefinition, "CurveGeomechResult", "", "", "", "");
    m_geomResultDefinition.uiCapability()->setUiHidden(true);
    m_geomResultDefinition.uiCapability()->setUiTreeChildrenHidden(true);
    m_geomResultDefinition = new RimGeoMechResultDefinition;
    m_geomResultDefinition->setAddWellPathDerivedResults(true);

    CAF_PDM_InitFieldNoDefault(&m_nameConfig, "NameConfig", "", "", "", "");
    m_nameConfig = new RimWellLogExtractionCurveNameConfig(this);    
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dWellLogExtractionCurve::~Rim3dWellLogExtractionCurve() 
{
    delete m_geomResultDefinition;
    delete m_eclipseResultDefinition;
    delete m_nameConfig;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogExtractionCurve::setPropertiesFromView(Rim3dView* view)
{
    if (!view) return;

    m_case = view->ownerCase();

    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());
    m_eclipseResultDefinition->setEclipseCase(eclipseCase);
    m_geomResultDefinition->setGeoMechCase(geomCase);

    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(view);
    if (eclipseView)
    {
        m_eclipseResultDefinition->simpleCopy(eclipseView->cellResult());
    }

    RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>(view);
    if (geoMechView)
    {
        m_geomResultDefinition->setResultAddress(geoMechView->cellResultResultDefinition()->resultAddress());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dWellLogExtractionCurve::resultPropertyString() const
{
    RimGeoMechCase* geoMechCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());

    QString name;
    if (eclipseCase)
    {
        name = caf::Utils::makeValidFileBasename(m_eclipseResultDefinition->resultVariableUiName());
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
bool Rim3dWellLogExtractionCurve::followAnimationTimeStep() const
{
    return m_timeStep() == -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogExtractionCurve::curveValuesAndMds(std::vector<double>* values, std::vector<double>* measuredDepthValues) const
{
    CVF_ASSERT(m_timeStep() >= 0);

    return this->curveValuesAndMdsAtTimeStep(values, measuredDepthValues, m_timeStep());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogExtractionCurve::curveValuesAndMdsAtTimeStep(std::vector<double>* values, std::vector<double>* measuredDepthValues, int timeStep) const
{
    CAF_ASSERT(values != nullptr);
    CAF_ASSERT(measuredDepthValues != nullptr);

    cvf::ref<RigEclipseWellLogExtractor> eclExtractor;
    cvf::ref<RigGeoMechWellLogExtractor> geomExtractor;

    RimWellPath* wellPath;
    firstAncestorOrThisOfType(wellPath);

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case());

    if (eclipseCase)
    {
        eclExtractor = RiaExtractionTools::wellLogExtractorEclipseCase(wellPath, eclipseCase);
    }
    else
    {
        RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case());
        if (geomCase)
        {
            geomExtractor = RiaExtractionTools::wellLogExtractorGeoMechCase(wellPath, geomCase);
        }
    }

    if (eclExtractor.notNull() && eclipseCase)
    {
        *measuredDepthValues = eclExtractor->measuredDepth();

        m_eclipseResultDefinition->loadResult();

        cvf::ref<RigResultAccessor> resAcc = RigResultAccessorFactory::createFromResultDefinition(eclipseCase->eclipseCaseData(),
                                                                                                  0,
                                                                                                  timeStep,
                                                                                                  m_eclipseResultDefinition);
        if (resAcc.notNull())
        {
            eclExtractor->curveData(resAcc.p(), values);
        }
    }
    else if (geomExtractor.notNull())
    {
        *measuredDepthValues = geomExtractor->measuredDepth();

        RimWellLogExtractionCurve::findAndLoadWbsParametersFromLasFiles(wellPath, geomExtractor.p());

        m_geomResultDefinition->loadResult();
        geomExtractor->setRkbDiff(rkbDiff());
        geomExtractor->curveData(m_geomResultDefinition->resultAddress(), timeStep, values);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> Rim3dWellLogExtractionCurve::findCurveValueRange()
{
    double foundMinValue = std::numeric_limits<float>::infinity();
    double foundMaxValue = -std::numeric_limits<float>::infinity();
    if (m_case())
    {
        std::set<int> timeStepsToCheck;
        if (followAnimationTimeStep())
        {
            // Check all time steps to avoid range changing during animation.
            for (int i = 0; i < m_case->timeStepStrings().size(); ++i)
            {
                timeStepsToCheck.insert(i);
            }
        }
        else
        {
            timeStepsToCheck.insert(m_timeStep());
        }

        for (int timeStep : timeStepsToCheck)
        {
            std::vector<double> values;
            std::vector<double> measuredDepths;
            this->curveValuesAndMdsAtTimeStep(&values, &measuredDepths, timeStep);

            for (double value : values)
            {
                if (RiaCurveDataTools::isValidValue(value, false))
                {
                    foundMinValue = std::min(foundMinValue, value);
                    foundMaxValue = std::max(foundMaxValue, value);
                }
            }
        }
    }
    return std::make_pair(foundMinValue, foundMaxValue);
}

QString Rim3dWellLogExtractionCurve::name() const
{
    return m_nameConfig()->name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dWellLogExtractionCurve::createAutoName() const
{
    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());

    QStringList autoName;

    if (!m_nameConfig->customName().isEmpty())
    {
        autoName.push_back(m_nameConfig->customName());
    }

    QStringList generatedAutoTags;

    if (m_nameConfig->addWellName())
    {
        RimWellPath* wellPath;
        this->firstAncestorOrThisOfTypeAsserted(wellPath);
        if (!wellPath->name().isEmpty())
        {
            generatedAutoTags += wellPath->name();
        }
    }

    if (m_nameConfig->addCaseName() && m_case())
    {
        generatedAutoTags.push_back(m_case->caseUserDescription());
    }

    if (m_nameConfig->addProperty() && !resultPropertyString().isEmpty())
    {
        generatedAutoTags.push_back(resultPropertyString());
    }

    if (m_nameConfig->addTimeStep() || m_nameConfig->addDate())
    {
        bool addTimeStep = m_nameConfig->addTimeStep() && m_timeStep() != -1;
        size_t maxTimeStep = 0;

        if (eclipseCase)
        {
            addTimeStep = addTimeStep && m_eclipseResultDefinition->resultType() != RiaDefines::STATIC_NATIVE;
            RigEclipseCaseData* data = eclipseCase->eclipseCaseData();
            if (data)
            {
                maxTimeStep = data->results(m_eclipseResultDefinition->porosityModel())->maxTimeStepCount();
            }
        }
        else if (geomCase)
        {
            RigGeoMechCaseData* data = geomCase->geoMechData();
            if (data)
            {
                maxTimeStep = data->femPartResults()->frameCount();
            }
        }

        if (m_nameConfig->addDate())
        {
            QString dateString = wellDate();
            if (!dateString.isEmpty())
            {
                generatedAutoTags.push_back(dateString);
            }
        }

        if (addTimeStep)
        {
            generatedAutoTags.push_back(QString("[%1/%2]").arg(m_timeStep() + 1).arg(maxTimeStep));            
        }
    }
    if (!generatedAutoTags.empty())
    {
        autoName.push_back(generatedAutoTags.join(", "));
    }
    return autoName.join(": ");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double Rim3dWellLogExtractionCurve::rkbDiff() const
{
    RimWellPath* wellPath;
    firstAncestorOrThisOfType(wellPath);

    if (wellPath && wellPath->wellPathGeometry())
    {
        return wellPath->wellPathGeometry()->rkbDiff();
    }

    return HUGE_VAL;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dWellLogExtractionCurve::isShowingTimeDependentResultInView(const Rim3dView* gridView) const
{
    if (showInView(gridView))
    {
        if (dynamic_cast<const RimEclipseCase*>(m_case()))
        {
            return m_eclipseResultDefinition->hasDynamicResult();
        }
        else if (dynamic_cast<const RimGeoMechCase*>(m_case()))
        {
            return m_geomResultDefinition->hasResult();
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dWellLogExtractionCurve::showInView(const Rim3dView* gridView) const
{
    if (isShowingCurve())
    {
        if (dynamic_cast<const RimEclipseCase*>(m_case()))
        {
            return dynamic_cast<const RimEclipseView*>(gridView) != nullptr;
        }
        else if (dynamic_cast<const RimGeoMechCase*>(m_case()))
        {
            return dynamic_cast<const RimGeoMechView*>(gridView) != nullptr;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* Rim3dWellLogExtractionCurve::userDescriptionField()
{
    return m_nameConfig()->nameField();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogExtractionCurve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{   
    if (changedField == &m_case)
    {
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case());
        RimGeoMechCase* geoMechCase = dynamic_cast<RimGeoMechCase*>(m_case());
        if (eclipseCase)
        {
            m_eclipseResultDefinition->setEclipseCase(eclipseCase);
        }
        else if (geoMechCase)
        {
            m_geomResultDefinition->setGeoMechCase(geoMechCase);
        }

        this->resetMinMaxValuesAndUpdateUI();
    }
    else if (changedField == &m_timeStep)
    {
        this->resetMinMaxValuesAndUpdateUI();
    }
    Rim3dWellLogCurve::fieldChangedByUi(changedField, oldValue, newValue);     
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> Rim3dWellLogExtractionCurve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                                 bool*                      useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    options = Rim3dWellLogCurve::calculateValueOptions(fieldNeedingOptions, useOptionsOnly);

    if (fieldNeedingOptions == &m_case)
    {
        RimTools::caseOptionItems(&options);

        options.push_front(caf::PdmOptionItemInfo("None", nullptr));
    }
    else if (fieldNeedingOptions == &m_timeStep)
    {
        QStringList timeStepNames;

        if (m_case)
        {
            timeStepNames = m_case->timeStepStrings();
        }
        options.push_back(caf::PdmOptionItemInfo(QString("Follow Animation Time Step"), -1));
        for (int i = 0; i < timeStepNames.size(); i++)
        {
            options.push_back(caf::PdmOptionItemInfo(timeStepNames[i], i));
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogExtractionCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup("Curve Data");

    curveDataGroup->add(&m_case);

    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());

    if (eclipseCase)
    {
        m_eclipseResultDefinition->uiOrdering(uiConfigName, *curveDataGroup);
    }
    else if (geomCase)
    {
        m_geomResultDefinition->uiOrdering(uiConfigName, *curveDataGroup);
    }

    if ((eclipseCase && m_eclipseResultDefinition->hasDynamicResult())
        || geomCase)
    {
        curveDataGroup->add(&m_timeStep);
    }

    Rim3dWellLogCurve::configurationUiOrdering(uiOrdering);

    m_nameConfig()->createUiGroup(uiConfigName, uiOrdering);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dWellLogExtractionCurve::initAfterRead()
{
    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());

    m_eclipseResultDefinition->setEclipseCase(eclipseCase);
    m_geomResultDefinition->setGeoMechCase(geomCase);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString Rim3dWellLogExtractionCurve::wellDate() const
{
    RimGeoMechCase* geomCase = dynamic_cast<RimGeoMechCase*>(m_case.value());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());

    QStringList timeStepNames;

    if (eclipseCase)
    {
        if (m_eclipseResultDefinition->resultType() == RiaDefines::STATIC_NATIVE)
        {
            return QString();
        }
        if (eclipseCase->eclipseCaseData())
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
    if (m_timeStep == -1)
    {
        return QString("Animation Time Step");
    }
    return (m_timeStep >= 0 && m_timeStep < timeStepNames.size()) ? timeStepNames[m_timeStep] : "";
}
