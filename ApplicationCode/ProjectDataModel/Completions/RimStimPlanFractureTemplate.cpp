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
#include "RiaCompletionTypeCalculationScheduler.h"
#include "RiaFractureDefines.h"
#include "RiaLogging.h"
#include "RiaWeightedGeometricMeanCalculator.h"
#include "RiaWeightedMeanCalculator.h"

#include "RifStimPlanXmlReader.h"

#include "RigFractureGrid.h"
#include "RigStimPlanFractureDefinition.h"
#include "RigWellPathStimplanIntersector.h"

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

#include "cvfMath.h"
#include "cvfVector3.h"

#include <QFileInfo>

#include <algorithm>
#include <cmath>
#include <vector>

static std::vector<double> EMPTY_DOUBLE_VECTOR;

CAF_PDM_SOURCE_INIT(RimStimPlanFractureTemplate, "RimStimPlanFractureTemplate");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanFractureTemplate::RimStimPlanFractureTemplate()
{
    // clang-format off
    
    CAF_PDM_InitObject("Fracture Template", ":/FractureTemplate16x16.png", "", "");

    CAF_PDM_InitField(&m_stimPlanFileName,          "StimPlanFileName", QString(""), "File Name", "", "", "");
    m_stimPlanFileName.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_wellPathDepthAtFracture,   "WellPathDepthAtFracture", 0.0, "Well/Fracture Intersection Depth", "", "", "");
    m_wellPathDepthAtFracture.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_borderPolygonResultName,   "BorderPolygonResultName", QString(""), "Parameter", "", "", "");
    m_borderPolygonResultName.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_activeTimeStepIndex,           "ActiveTimeStepIndex", 0, "Active TimeStep Index", "", "", "");
    CAF_PDM_InitField(&m_conductivityResultNameOnFile,  "ConductivityResultName", QString(""), "Active Conductivity Result Name", "", "", "");

    CAF_PDM_InitField(&m_showStimPlanMesh_OBSOLETE, "ShowStimPlanMesh", true, "", "", "", "");
    m_showStimPlanMesh_OBSOLETE.uiCapability()->setUiHidden(true);

    m_fractureGrid = new RigFractureGrid();
    m_readError    = false;

    // clang-format on
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanFractureTemplate::~RimStimPlanFractureTemplate() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimStimPlanFractureTemplate::activeTimeStepIndex()
{
    return m_activeTimeStepIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                   const QVariant&            oldValue,
                                                   const QVariant&            newValue)
{
    RimFractureTemplate::fieldChangedByUi(changedField, oldValue, newValue);

    if (&m_stimPlanFileName == changedField)
    {
        m_readError = false;
        loadDataAndUpdate();
        setDefaultsBasedOnXMLfile();
    }

    if (&m_activeTimeStepIndex == changedField)
    {
        // Changes to this parameters should change all fractures with this fracture template attached.
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
                    fracture->setStimPlanTimeIndexToPlot(m_activeTimeStepIndex);
                }
            }
            proj->scheduleCreateDisplayModelAndRedrawAllViews();
        }
    }

    if (&m_wellPathDepthAtFracture == changedField || &m_borderPolygonResultName == changedField ||
        &m_activeTimeStepIndex == changedField || &m_stimPlanFileName == changedField ||
        &m_conductivityResultNameOnFile == changedField)
    {
        updateFractureGrid();

        RimProject* proj;
        this->firstAncestorOrThisOfType(proj);
        if (proj)
        {
            proj->scheduleCreateDisplayModelAndRedrawAllViews();
        }
    }

    if (changedField == &m_scaleApplyButton)
    {
        m_scaleApplyButton = false;
        onLoadDataAndUpdateGeometryHasChanged();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::setFileName(const QString& fileName)
{
    m_stimPlanFileName = fileName;
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
void RimStimPlanFractureTemplate::updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath)
{
    m_stimPlanFileName = RimTools::relocateFile(m_stimPlanFileName(), newProjectPath, oldProjectPath, nullptr, nullptr);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::setDefaultsBasedOnXMLfile()
{
    if (m_stimPlanFractureDefinitionData.isNull()) return;

    computeDepthOfWellPathAtFracture();
    computePerforationLength();

    RiaLogging::info(QString("Setting well/fracture intersection depth at %1").arg(m_wellPathDepthAtFracture));

    m_activeTimeStepIndex = static_cast<int>(m_stimPlanFractureDefinitionData->totalNumberTimeSteps() - 1);

    bool polygonPropertySet = setBorderPolygonResultNameToDefault();
    if (polygonPropertySet)
        RiaLogging::info(QString("Calculating polygon outline based on %1 at timestep %2")
                             .arg(m_borderPolygonResultName)
                             .arg(m_stimPlanFractureDefinitionData->timeSteps()[m_activeTimeStepIndex]));
    else
        RiaLogging::info(QString("Property for polygon calculation not set."));

    if (!m_stimPlanFractureDefinitionData->conductivityResultNames().isEmpty())
    {
        m_conductivityResultNameOnFile = m_stimPlanFractureDefinitionData->conductivityResultNames().front();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanFractureTemplate::setBorderPolygonResultNameToDefault()
{
    // first option: Width
    for (std::pair<QString, QString> property : uiResultNamesWithUnit())
    {
        if (property.first == "WIDTH")
        {
            m_borderPolygonResultName = property.first;
            return true;
        }
    }

    // if width not found, use conductivity
    if (hasConductivity())
    {
        m_borderPolygonResultName = m_stimPlanFractureDefinitionData->conductivityResultNames().first();
        return true;
    }

    // else: Set to first property
    if (!uiResultNamesWithUnit().empty())
    {
        m_borderPolygonResultName = uiResultNamesWithUnit()[0].first;
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

    if (m_readError) return;

    m_stimPlanFractureDefinitionData = RifStimPlanXmlReader::readStimPlanXMLFile(m_stimPlanFileName(),
                                                                                 m_conductivityScaleFactor(),
                                                                                 m_halfLengthScaleFactor(),
                                                                                 m_heightScaleFactor(),
                                                                                 -m_wellPathDepthAtFracture(),
                                                                                 RifStimPlanXmlReader::MIRROR_AUTO,
                                                                                 fractureTemplateUnit(),
                                                                                 &errorMessage);
    if (errorMessage.size() > 0) RiaLogging::error(errorMessage);

    if (m_stimPlanFractureDefinitionData.notNull())
    {
        setDefaultConductivityResultIfEmpty();

        if (fractureTemplateUnit() == RiaEclipseUnitTools::UNITS_UNKNOWN)
        {
            setFractureTemplateUnit(m_stimPlanFractureDefinitionData->unitSet());
        }

        m_readError = false;
    }
    else
    {
        setFractureTemplateUnit(RiaEclipseUnitTools::UNITS_UNKNOWN);
        m_readError = true;
    }

    updateFractureGrid();

    for (RimFracture* fracture : fracturesUsingThisTemplate())
    {
        fracture->clearCachedNonDarcyProperties();
    }

    if (widthResultValues().empty())
    {
        m_fractureWidthType = USER_DEFINED_WIDTH;
    }

    // Todo: Must update all views using this fracture template
    RimEclipseView* activeView = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeReservoirView());
    if (activeView) activeView->fractureColors()->loadDataAndUpdate();

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimStimPlanFractureTemplate::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                                 bool*                      useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_fractureWidthType)
    {
        options.push_back(caf::PdmOptionItemInfo(caf::AppEnum<WidthEnum>::uiText(USER_DEFINED_WIDTH), USER_DEFINED_WIDTH));

        if (!widthResultValues().empty())
        {
            options.push_back(caf::PdmOptionItemInfo(caf::AppEnum<WidthEnum>::uiText(WIDTH_FROM_FRACTURE), WIDTH_FROM_FRACTURE));
        }
    }

    if (fieldNeedingOptions == &m_betaFactorType)
    {
        options.push_back(
            caf::PdmOptionItemInfo(caf::AppEnum<BetaFactorEnum>::uiText(USER_DEFINED_BETA_FACTOR), USER_DEFINED_BETA_FACTOR));

        if (isBetaFactorAvailableOnFile())
        {
            options.push_back(caf::PdmOptionItemInfo(caf::AppEnum<BetaFactorEnum>::uiText(BETA_FACTOR_FROM_FRACTURE),
                              BETA_FACTOR_FROM_FRACTURE));
        }
    }

    if (fieldNeedingOptions == &m_borderPolygonResultName)
    {
        for (std::pair<QString, QString> nameUnit : uiResultNamesWithUnit())
        {
            options.push_back(caf::PdmOptionItemInfo(nameUnit.first, nameUnit.first));
        }
    }
    else if (fieldNeedingOptions == &m_activeTimeStepIndex)
    {
        std::vector<double> timeValues = timeSteps();
        int                 index      = 0;
        for (double value : timeValues)
        {
            options.push_back(caf::PdmOptionItemInfo(QString::number(value), index));
            index++;
        }
    }
    else if (fieldNeedingOptions == &m_conductivityResultNameOnFile)
    {
        if (m_stimPlanFractureDefinitionData.notNull())
        {
            QStringList conductivityResultNames = m_stimPlanFractureDefinitionData->conductivityResultNames();
            for (const auto& resultName : conductivityResultNames)
            {
                options.push_back(caf::PdmOptionItemInfo(resultName, resultName));
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::computeDepthOfWellPathAtFracture()
{
    if (!m_stimPlanFractureDefinitionData.isNull())
    {
        double firstTvd = m_stimPlanFractureDefinitionData->topPerfTvd();
        double lastTvd  = m_stimPlanFractureDefinitionData->bottomPerfTvd();

        if (firstTvd != HUGE_VAL && lastTvd != HUGE_VAL)
        {
            m_wellPathDepthAtFracture = (firstTvd + lastTvd) / 2;
        }
        else
        {
            firstTvd                  = m_stimPlanFractureDefinitionData->minDepth();
            lastTvd                   = m_stimPlanFractureDefinitionData->maxDepth();
            m_wellPathDepthAtFracture = (firstTvd + lastTvd) / 2;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::computePerforationLength()
{
    if (!m_stimPlanFractureDefinitionData.isNull())
    {
        double firstTvd = m_stimPlanFractureDefinitionData->topPerfTvd();
        double lastTvd  = m_stimPlanFractureDefinitionData->bottomPerfTvd();

        if (firstTvd != HUGE_VAL && lastTvd != HUGE_VAL)
        {
            m_perforationLength = cvf::Math::abs(firstTvd - lastTvd);
        }
    }

    if (fractureTemplateUnit() == RiaEclipseUnitTools::UNITS_METRIC && m_perforationLength < 10)
    {
        m_perforationLength = 10;
    }
    else if (fractureTemplateUnit() == RiaEclipseUnitTools::UNITS_FIELD &&
             m_perforationLength < RiaEclipseUnitTools::meterToFeet(10))
    {
        m_perforationLength = std::round(RiaEclipseUnitTools::meterToFeet(10));
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>
    RimStimPlanFractureTemplate::fractureGridResultsForUnitSystem(const QString&                  resultName,
                                                                  const QString&                  unitName,
                                                                  size_t                          timeStepIndex,
                                                                  RiaEclipseUnitTools::UnitSystem requiredUnitSystem) const
{
    auto resultValues = m_stimPlanFractureDefinitionData->fractureGridResults(resultName, unitName, m_activeTimeStepIndex);

    if (fractureTemplateUnit() == RiaEclipseUnitTools::UNITS_METRIC)
    {
        for (auto& v : resultValues)
        {
            v = RiaEclipseUnitTools::convertToMeter(v, unitName);
        }
    }
    else if (fractureTemplateUnit() == RiaEclipseUnitTools::UNITS_FIELD)
    {
        for (auto& v : resultValues)
        {
            v = RiaEclipseUnitTools::convertToFeet(v, unitName);
        }
    }

    return resultValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
WellFractureIntersectionData RimStimPlanFractureTemplate::wellFractureIntersectionData(const RimFracture* fractureInstance) const
{
    WellFractureIntersectionData values;

    if (m_fractureGrid.notNull())
    {
        if (orientationType() == ALONG_WELL_PATH)
        {
            CVF_ASSERT(fractureInstance);

            RimWellPath* rimWellPath = nullptr;
            fractureInstance->firstAncestorOrThisOfType(rimWellPath);

            if (rimWellPath && rimWellPath->wellPathGeometry())
            {
                double totalLength              = 0.0;
                double weightedConductivity     = 0.0;
                double weightedWidth            = 0.0;
                double weightedBetaFactorOnFile = 0.0;
                double conversionFactorForBeta  = 1.0;

                {
                    std::vector<double> widthResultValues;
                    {
                        auto nameUnit     = widthParameterNameAndUnit();
                        widthResultValues = fractureGridResultsForUnitSystem(
                            nameUnit.first, nameUnit.second, m_activeTimeStepIndex, fractureTemplateUnit());
                    }

                    std::vector<double> conductivityResultValues;
                    {
                        auto nameUnit            = conductivityParameterNameAndUnit();
                        conductivityResultValues = fractureGridResultsForUnitSystem(
                            nameUnit.first, nameUnit.second, m_activeTimeStepIndex, fractureTemplateUnit());
                    }

                    std::vector<double> betaFactorResultValues;
                    {
                        auto nameUnit          = betaFactorParameterNameAndUnit();
                        betaFactorResultValues = m_stimPlanFractureDefinitionData->fractureGridResults(
                            nameUnit.first, nameUnit.second, m_activeTimeStepIndex);

                        QString trimmedUnit = nameUnit.second.trimmed().toLower();
                        if (trimmedUnit == "/m")
                        {
                            conversionFactorForBeta = 1.01325E+08;
                        }
                        else if (trimmedUnit == "/cm")
                        {
                            conversionFactorForBeta = 1.01325E+06;
                        }
                        else if (trimmedUnit == "/ft")
                        {
                            conversionFactorForBeta = 3.088386E+07;
                        }
                    }

                    RiaWeightedMeanCalculator<double>  widthCalc;
                    RiaWeightedMeanCalculator<double>  conductivityCalc;
                    RiaWeightedGeometricMeanCalculator betaFactorCalc;

                    RigWellPathStimplanIntersector intersector(rimWellPath->wellPathGeometry(), fractureInstance);
                    for (const auto& v : intersector.intersections())
                    {
                        size_t fractureGlobalCellIndex = v.first;
                        double intersectionLength      = v.second.computeLength();

                        if (fractureGlobalCellIndex < widthResultValues.size())
                        {
                            widthCalc.addValueAndWeight(widthResultValues[fractureGlobalCellIndex], intersectionLength);
                        }

                        if (fractureGlobalCellIndex < conductivityResultValues.size())
                        {
                            conductivityCalc.addValueAndWeight(conductivityResultValues[fractureGlobalCellIndex],
                                                               intersectionLength);
                        }

                        if (fractureGlobalCellIndex < betaFactorResultValues.size())
                        {
                            betaFactorCalc.addValueAndWeight(betaFactorResultValues[fractureGlobalCellIndex], intersectionLength);
                        }
                    }
                    if (conductivityCalc.validAggregatedWeight())
                    {
                        weightedConductivity = conductivityCalc.weightedMean();
                    }
                    if (widthCalc.validAggregatedWeight())
                    {
                        weightedWidth = widthCalc.weightedMean();
                        totalLength   = widthCalc.aggregatedWeight();
                    }
                    if (betaFactorCalc.validAggregatedWeight())
                    {
                        weightedBetaFactorOnFile = betaFactorCalc.weightedMean();
                    }
                }

                if (totalLength > 1e-7)
                {
                    values.m_width        = weightedWidth;
                    values.m_conductivity = weightedConductivity;

                    double betaFactorForcheimer          = weightedBetaFactorOnFile / conversionFactorForBeta;
                    values.m_betaFactorInForcheimerUnits = betaFactorForcheimer;
                }

                if (weightedWidth > 1e-7)
                {
                    values.m_permeability = weightedConductivity / weightedWidth;
                }
            }
        }
        else
        {
            std::pair<size_t, size_t> wellCellIJ    = m_fractureGrid->fractureCellAtWellCenter();
            size_t                    wellCellIndex = m_fractureGrid->getGlobalIndexFromIJ(wellCellIJ.first, wellCellIJ.second);
            const RigFractureCell&    wellCell      = m_fractureGrid->cellFromIndex(wellCellIndex);

            double conductivity   = wellCell.getConductivityValue();
            values.m_conductivity = conductivity;

            auto nameUnit = widthParameterNameAndUnit();
            if (!nameUnit.first.isEmpty())
            {
                double widthInRequiredUnit = HUGE_VAL;
                {
                    auto resultValues = fractureGridResultsForUnitSystem(
                        nameUnit.first, nameUnit.second, m_activeTimeStepIndex, fractureTemplateUnit());

                    if (wellCellIndex < resultValues.size())
                    {
                        widthInRequiredUnit = resultValues[wellCellIndex];
                    }
                }

                if (widthInRequiredUnit != HUGE_VAL && fabs(widthInRequiredUnit) > 1e-20)
                {
                    values.m_width        = widthInRequiredUnit;
                    values.m_permeability = conductivity / widthInRequiredUnit;
                }
            }
        }
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<QString, QString> RimStimPlanFractureTemplate::widthParameterNameAndUnit() const
{
    if (m_stimPlanFractureDefinitionData.notNull())
    {
        std::vector<std::pair<QString, QString>> propertyNamesUnitsOnFile =
            m_stimPlanFractureDefinitionData->getStimPlanPropertyNamesUnits();

        for (const auto& nameUnit : propertyNamesUnitsOnFile)
        {
            if (nameUnit.first.contains("effective width", Qt::CaseInsensitive))
            {
                return nameUnit;
            }

            if (nameUnit.first.contains("width", Qt::CaseInsensitive))
            {
                return nameUnit;
            }
        }
    }

    return std::pair<QString, QString>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<QString, QString> RimStimPlanFractureTemplate::conductivityParameterNameAndUnit() const
{
    if (m_stimPlanFractureDefinitionData.notNull())
    {
        std::vector<std::pair<QString, QString>> propertyNamesUnitsOnFile =
            m_stimPlanFractureDefinitionData->getStimPlanPropertyNamesUnits();

        for (const auto& nameUnit : propertyNamesUnitsOnFile)
        {
            if (nameUnit.first.contains(m_conductivityResultNameOnFile, Qt::CaseInsensitive))
            {
                return nameUnit;
            }
        }
    }

    return std::pair<QString, QString>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<QString, QString> RimStimPlanFractureTemplate::betaFactorParameterNameAndUnit() const
{
    if (m_stimPlanFractureDefinitionData.notNull())
    {
        std::vector<std::pair<QString, QString>> propertyNamesUnitsOnFile =
            m_stimPlanFractureDefinitionData->getStimPlanPropertyNamesUnits();

        for (const auto& nameUnit : propertyNamesUnitsOnFile)
        {
            if (nameUnit.first.contains("beta", Qt::CaseInsensitive))
            {
                return nameUnit;
            }
        }
    }

    return std::pair<QString, QString>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanFractureTemplate::isBetaFactorAvailableOnFile() const
{
    auto nameAndUnit = betaFactorParameterNameAndUnit();

    return !nameAndUnit.first.isEmpty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::setDefaultConductivityResultIfEmpty()
{
    if (m_conductivityResultNameOnFile().isEmpty())
    {
        if (!m_stimPlanFractureDefinitionData->conductivityResultNames().isEmpty())
        {
            m_conductivityResultNameOnFile = m_stimPlanFractureDefinitionData->conductivityResultNames().front();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanFractureTemplate::mapUiResultNameToFileResultName(const QString& uiResultName) const
{
    QString fileResultName;

    if (uiResultName == RiaDefines::conductivityResultName())
    {
        fileResultName = m_conductivityResultNameOnFile();
    }
    else
    {
        fileResultName = uiResultName;
    }

    return fileResultName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanFractureTemplate::showStimPlanMesh() const
{
    return m_showStimPlanMesh_OBSOLETE();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::convertToUnitSystem(RiaEclipseUnitTools::UnitSystem neededUnit)
{
    if (m_fractureTemplateUnit() == neededUnit) return;

    setFractureTemplateUnit(neededUnit);
    RimFractureTemplate::convertToUnitSystem(neededUnit);

    m_readError = false;
    loadDataAndUpdate();

    if (m_stimPlanFractureDefinitionData.isNull()) return;

    if (neededUnit == RiaEclipseUnitTools::UNITS_FIELD)
    {
        m_wellPathDepthAtFracture = RiaEclipseUnitTools::meterToFeet(m_wellPathDepthAtFracture);
    }
    else if (neededUnit == RiaEclipseUnitTools::UNITS_METRIC)
    {
        m_wellPathDepthAtFracture = RiaEclipseUnitTools::feetToMeter(m_wellPathDepthAtFracture);
    }

    m_activeTimeStepIndex   = static_cast<int>(m_stimPlanFractureDefinitionData->totalNumberTimeSteps() - 1);
    bool polygonPropertySet = setBorderPolygonResultNameToDefault();

    if (polygonPropertySet)
        RiaLogging::info(QString("Calculating polygon outline based on %1 at timestep %2")
                             .arg(m_borderPolygonResultName)
                             .arg(m_stimPlanFractureDefinitionData->timeSteps()[m_activeTimeStepIndex]));
    else
        RiaLogging::info(QString("Property for polygon calculation not set."));

    if (!m_stimPlanFractureDefinitionData->conductivityResultNames().isEmpty())
    {
        m_conductivityResultNameOnFile = m_stimPlanFractureDefinitionData->conductivityResultNames().front();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::onLoadDataAndUpdateGeometryHasChanged()
{
    loadDataAndUpdate();

    RimProject* proj;
    this->firstAncestorOrThisOfType(proj);
    if (proj)
    {
        proj->scheduleCreateDisplayModelAndRedrawAllViews();
        RiaCompletionTypeCalculationScheduler::instance()->scheduleRecalculateCompletionTypeAndRedrawAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStimPlanFractureTemplate::timeSteps()
{
    if (m_stimPlanFractureDefinitionData.notNull())
    {
        return m_stimPlanFractureDefinitionData->timeSteps();
    }

    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, QString>> RimStimPlanFractureTemplate::uiResultNamesWithUnit() const
{
    std::vector<std::pair<QString, QString>> propertyNamesAndUnits;

    if (m_stimPlanFractureDefinitionData.notNull())
    {
        QString conductivityUnit = "mD/s";

        std::vector<std::pair<QString, QString>> tmp;

        std::vector<std::pair<QString, QString>> propertyNamesUnitsOnFile =
            m_stimPlanFractureDefinitionData->getStimPlanPropertyNamesUnits();
        for (const auto& nameUnitPair : propertyNamesUnitsOnFile)
        {
            if (nameUnitPair.first.contains(RiaDefines::conductivityResultName(), Qt::CaseInsensitive))
            {
                conductivityUnit = nameUnitPair.second;
            }
            else
            {
                tmp.push_back(nameUnitPair);
            }
        }

        propertyNamesAndUnits.push_back(std::make_pair(RiaDefines::conductivityResultName(), conductivityUnit));

        for (const auto& nameUnitPair : tmp)
        {
            propertyNamesAndUnits.push_back(nameUnitPair);
        }
    }

    return propertyNamesAndUnits;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>>
    RimStimPlanFractureTemplate::resultValues(const QString& uiResultName, const QString& unitName, size_t timeStepIndex) const
{
    if (m_stimPlanFractureDefinitionData.notNull())
    {
        QString fileResultName = mapUiResultNameToFileResultName(uiResultName);

        return m_stimPlanFractureDefinitionData->getDataAtTimeIndex(fileResultName, unitName, timeStepIndex);
    }

    return std::vector<std::vector<double>>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStimPlanFractureTemplate::fractureGridResults(const QString& uiResultName,
                                                                     const QString& unitName,
                                                                     size_t         timeStepIndex) const
{
    if (m_stimPlanFractureDefinitionData.notNull())
    {
        QString fileResultName = mapUiResultNameToFileResultName(uiResultName);

        return m_stimPlanFractureDefinitionData->fractureGridResults(fileResultName, unitName, timeStepIndex);
    }

    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanFractureTemplate::hasConductivity() const
{
    if (m_stimPlanFractureDefinitionData.notNull() && !m_stimPlanFractureDefinitionData->conductivityResultNames().isEmpty())
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanFractureTemplate::resultValueAtIJ(const QString& uiResultName,
                                                    const QString& unitName,
                                                    size_t         timeStepIndex,
                                                    size_t         i,
                                                    size_t         j)
{
    auto values = resultValues(uiResultName, unitName, timeStepIndex);

    if (values.empty()) return HUGE_VAL;

    size_t adjustedI = i + 1;
    size_t adjustedJ = j + 1;

    if (adjustedI >= fractureGrid()->iCellCount() || adjustedJ >= fractureGrid()->jCellCount())
    {
        return HUGE_VAL;
    }

    return values[adjustedJ][adjustedI];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimStimPlanFractureTemplate::widthResultValues() const
{
    std::vector<double> resultValues;

    auto nameUnit = widthParameterNameAndUnit();
    if (!nameUnit.first.isEmpty())
    {
        resultValues =
            fractureGridResultsForUnitSystem(nameUnit.first, nameUnit.second, m_activeTimeStepIndex, fractureTemplateUnit());
    }

    return resultValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::appendDataToResultStatistics(const QString&     uiResultName,
                                                               const QString&     unit,
                                                               MinMaxAccumulator& minMaxAccumulator,
                                                               PosNegAccumulator& posNegAccumulator) const
{
    if (m_stimPlanFractureDefinitionData.notNull())
    {
        QString fileResultName = mapUiResultNameToFileResultName(uiResultName);

        m_stimPlanFractureDefinitionData->appendDataToResultStatistics(
            fileResultName, unit, minMaxAccumulator, posNegAccumulator);
    }
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
    m_fractureGrid = nullptr;

    if (m_stimPlanFractureDefinitionData.notNull())
    {
        m_fractureGrid = m_stimPlanFractureDefinitionData->createFractureGrid(
            m_conductivityResultNameOnFile, m_activeTimeStepIndex, m_wellPathDepthAtFracture, m_fractureTemplateUnit());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::fractureTriangleGeometry(std::vector<cvf::Vec3f>* nodeCoords,
                                                           std::vector<cvf::uint>*  triangleIndices) const
{
    if (m_stimPlanFractureDefinitionData.notNull())
    {
        m_stimPlanFractureDefinitionData->createFractureTriangleGeometry(
            m_wellPathDepthAtFracture, name(), nodeCoords, triangleIndices);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_name);
    uiOrdering.add(&m_id);

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Input");
        group->add(&m_stimPlanFileName);
        group->add(&m_activeTimeStepIndex);
        group->add(&m_wellPathDepthAtFracture);
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Geometry");
        group->add(&m_orientationType);
        group->add(&m_azimuthAngle);
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Fracture Truncation");
        group->setCollapsedByDefault(true);
        m_fractureContainment()->uiOrdering(uiConfigName, *group);
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Properties");
        group->add(&m_conductivityResultNameOnFile);
        group->add(&m_conductivityType);
        group->add(&m_skinFactor);
        group->add(&m_perforationLength);
        group->add(&m_perforationEfficiency);
        group->add(&m_wellDiameter);
    }

    if (widthResultValues().empty())
    {
        m_fractureWidthType = USER_DEFINED_WIDTH;
    }

    RimFractureTemplate::defineUiOrdering(uiConfigName, uiOrdering);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanFractureTemplate::defineEditorAttribute(const caf::PdmFieldHandle* field,
                                                        QString                    uiConfigName,
                                                        caf::PdmUiEditorAttribute* attribute)
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
        if (!m_stimPlanFractureDefinitionData.isNull() && (m_stimPlanFractureDefinitionData->yCount() > 0))
        {
            caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);
            if (myAttr)
            {
                myAttr->m_minimum = m_stimPlanFractureDefinitionData->minDepth();
                myAttr->m_maximum = m_stimPlanFractureDefinitionData->maxDepth();
            }
        }
    }
}
