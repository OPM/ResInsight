/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimEclipseCellColors.h"

#include "RiaColorTables.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigFlowDiagResults.h"
#include "RigFormationNames.h"

#include "RimCellEdgeColors.h"
#include "RimEclipseCase.h"
#include "RimEclipseFaultColors.h"
#include "RimEclipseView.h"
#include "RimRegularLegendConfig.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimTernaryLegendConfig.h"
#include "RimViewController.h"
#include "RimViewLinker.h"

#include "RiuMainWindow.h"

#include "cafPdmUiTreeOrdering.h"

#include <QMessageBox>

CAF_PDM_SOURCE_INIT(RimEclipseCellColors, "ResultSlot");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCellColors::RimEclipseCellColors()
{
    CAF_PDM_InitObject("Cell Result", ":/CellResult.png", "", "");

    CAF_PDM_InitFieldNoDefault(&obsoleteField_legendConfig, "LegendDefinition", "Color Legend", "", "", "");
    this->obsoleteField_legendConfig.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitFieldNoDefault(&m_legendConfigData, "ResultVarLegendDefinitionList", "", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_ternaryLegendConfig, "TernaryLegendDefinition", "Ternary Color Legend", "", "", "");
    this->m_ternaryLegendConfig = new RimTernaryLegendConfig();

    CAF_PDM_InitFieldNoDefault(&m_legendConfigPtrField, "LegendDefinitionPtrField", "Color Legend PtrField", "", "", "");

    // Make sure we have a created legend for the default/undefined result variable
    changeLegendConfig(this->resultVariable());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCellColors::~RimEclipseCellColors()
{
    CVF_ASSERT(obsoleteField_legendConfig() == nullptr);

    m_legendConfigData.deleteAllChildObjects();

    delete m_ternaryLegendConfig();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                            const QVariant&            oldValue,
                                            const QVariant&            newValue)
{
    RimEclipseResultDefinition::fieldChangedByUi(changedField, oldValue, newValue);

    // Update of legend config must happen after RimEclipseResultDefinition::fieldChangedByUi(), as this function modifies
    // this->resultVariable()
    if (changedField == &m_resultVariableUiField)
    {
        if (oldValue != newValue)
        {
            changeLegendConfig(this->resultVariable());
        }

        if (newValue != RiaDefines::undefinedResultName())
        {
            if (m_reservoirView) m_reservoirView->hasUserRequestedAnimation = true;
        }

        RimEclipseFaultColors* faultColors = dynamic_cast<RimEclipseFaultColors*>(this->parentField()->ownerObject());
        if (faultColors)
        {
            faultColors->updateConnectedEditors();
        }

        RimCellEdgeColors* cellEdgeColors = dynamic_cast<RimCellEdgeColors*>(this->parentField()->ownerObject());
        if (cellEdgeColors)
        {
            cellEdgeColors->updateConnectedEditors();
        }
    }

    if (m_reservoirView) m_reservoirView->createDisplayModelAndRedraw();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::changeLegendConfig(QString resultVarNameOfNewLegend)
{
    if (resultVarNameOfNewLegend != RiaDefines::ternarySaturationResultName())
    {
        QString legendResultVariable;

        if (this->m_legendConfigPtrField())
        {
            legendResultVariable = this->m_legendConfigPtrField()->resultVariableName();
        }

        if (!this->m_legendConfigPtrField() || legendResultVariable != resultVarNameOfNewLegend)
        {
            bool found = false;
            for (size_t i = 0; i < m_legendConfigData.size(); i++)
            {
                if (m_legendConfigData[i]->resultVariableName() == resultVarNameOfNewLegend)
                {
                    this->m_legendConfigPtrField = m_legendConfigData[i];
                    found                        = true;
                    break;
                }
            }

            // Not found ?
            if (!found)
            {
                RimRegularLegendConfig* newLegend = new RimRegularLegendConfig;
                newLegend->resultVariableName     = resultVarNameOfNewLegend;
                m_legendConfigData.push_back(newLegend);

                this->m_legendConfigPtrField = newLegend;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::initAfterRead()
{
    RimEclipseResultDefinition::initAfterRead();

    if (this->m_legendConfigPtrField() && this->m_legendConfigPtrField()->resultVariableName == "")
    {
        this->m_legendConfigPtrField()->resultVariableName = this->resultVariable();
    }

    if (obsoleteField_legendConfig)
    {
        // The current legend config is NOT stored in <ResultVarLegendDefinitionList> in ResInsight up to v 1.3.7-dev
        RimRegularLegendConfig* obsoleteLegend = obsoleteField_legendConfig();

        // set to nullptr before pushing into container
        obsoleteField_legendConfig = nullptr;

        m_legendConfigData.push_back(obsoleteLegend);
        m_legendConfigPtrField = obsoleteLegend;
    }

    changeLegendConfig(this->resultVariable());

    updateIconState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimEclipseResultDefinition::defineUiOrdering(uiConfigName, uiOrdering);

    if (uiConfigName == "AddLegendLevels")
    {
        legendConfig()->uiOrdering("NumIntervalsOnly", uiOrdering);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    if (this->resultVariable() == RiaDefines::ternarySaturationResultName())
    {
        uiTreeOrdering.add(m_ternaryLegendConfig());
    }
    else
    {
        uiTreeOrdering.add(m_legendConfigPtrField());
    }

    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::updateLegendCategorySettings()
{
    changeLegendConfig(this->resultVariable());

    if (this->hasCategoryResult())
    {
        legendConfig()->setMappingMode(RimRegularLegendConfig::CATEGORY_INTEGER);
        legendConfig()->setColorRange(RimRegularLegendConfig::CATEGORY);
    }
    else
    {
        if (legendConfig()->mappingMode() == RimRegularLegendConfig::CATEGORY_INTEGER)
        {
            legendConfig()->setMappingMode(RimRegularLegendConfig::LINEAR_CONTINUOUS);
        }

        if (legendConfig()->colorRange() == RimRegularLegendConfig::CATEGORY)
        {
            legendConfig()->setColorRange(RimRegularLegendConfig::NORMAL);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::uiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    defineUiTreeOrdering(uiTreeOrdering, uiConfigName);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::setReservoirView(RimEclipseView* ownerReservoirView)
{
    m_reservoirView = ownerReservoirView;
    if (ownerReservoirView)
    {
        this->setEclipseCase(ownerReservoirView->eclipseCase());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimEclipseCellColors::reservoirView()
{
    return m_reservoirView;
}

bool operator<(const cvf::Color3ub first, const cvf::Color3ub second)
{
    if (first.r() != second.r()) return first.r() < second.r();
    if (first.g() != second.g()) return first.g() < second.g();
    if (first.b() != second.b()) return first.b() < second.b();

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class TupleCompare
{
public:
    bool operator()(const std::tuple<QString, int, cvf::Color3ub>& t1, const std::tuple<QString, int, cvf::Color3ub>& t2) const
    {
        using namespace std;
        if (get<0>(t1) != get<0>(t2)) return get<0>(t1) < get<0>(t2);
        if (get<1>(t1) != get<1>(t2)) return get<1>(t1) < get<1>(t2);
        if (get<2>(t1) != get<2>(t2)) return get<2>(t1) < get<2>(t2);

        return false;
    }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::updateLegendData(RimEclipseCase*         rimEclipseCase,
                                            int                     currentTimeStep,
                                            RimRegularLegendConfig* legendConfig,
                                            RimTernaryLegendConfig* ternaryLegendConfig)
{
    if (!legendConfig) legendConfig = this->legendConfig();
    if (!ternaryLegendConfig) ternaryLegendConfig = this->m_ternaryLegendConfig();

    if (this->hasResult())
    {
        if (this->isFlowDiagOrInjectionFlooding())
        {
            CVF_ASSERT(currentTimeStep >= 0);

            double                   globalMin, globalMax;
            double                   globalPosClosestToZero, globalNegClosestToZero;
            RigFlowDiagResults*      flowResultsData = this->flowDiagSolution()->flowDiagResults();
            RigFlowDiagResultAddress resAddr         = this->flowDiagResAddress();

            flowResultsData->minMaxScalarValues(resAddr, currentTimeStep, &globalMin, &globalMax);
            flowResultsData->posNegClosestToZero(resAddr, currentTimeStep, &globalPosClosestToZero, &globalNegClosestToZero);

            double localMin, localMax;
            double localPosClosestToZero, localNegClosestToZero;
            if (this->hasDynamicResult())
            {
                flowResultsData->minMaxScalarValues(resAddr, currentTimeStep, &localMin, &localMax);
                flowResultsData->posNegClosestToZero(resAddr, currentTimeStep, &localPosClosestToZero, &localNegClosestToZero);
            }
            else
            {
                localMin = globalMin;
                localMax = globalMax;

                localPosClosestToZero = globalPosClosestToZero;
                localNegClosestToZero = globalNegClosestToZero;
            }

            CVF_ASSERT(legendConfig);

            legendConfig->disableAllTimeStepsRange(true);
            legendConfig->setClosestToZeroValues(
                globalPosClosestToZero, globalNegClosestToZero, localPosClosestToZero, localNegClosestToZero);
            legendConfig->setAutomaticRanges(globalMin, globalMax, localMin, localMax);

            if (this->hasCategoryResult() && m_reservoirView)
            {
                std::set<std::tuple<QString, int, cvf::Color3ub>, TupleCompare> categories;
                // std::set<std::tuple<QString, int, cvf::Color3ub> > categories;

                std::vector<QString> tracerNames = this->flowDiagSolution()->tracerNames();
                int                  tracerIndex = 0;
                for (const auto& tracerName : tracerNames)
                {
                    RimSimWellInView* well =
                        m_reservoirView->wellCollection()->findWell(RimFlowDiagSolution::removeCrossFlowEnding(tracerName));
                    cvf::Color3ub color(cvf::Color3::GRAY);
                    if (well) color = cvf::Color3ub(well->wellPipeColor());

                    categories.insert(std::make_tuple(tracerName, tracerIndex, color));
                    ++tracerIndex;
                }

                std::vector<std::tuple<QString, int, cvf::Color3ub>> reverseCategories;
                for (auto tupIt = categories.rbegin(); tupIt != categories.rend(); ++tupIt)
                {
                    reverseCategories.push_back(*tupIt);
                }

                legendConfig->setCategoryItems(reverseCategories);
            }
        }
        else
        {
            CVF_ASSERT(rimEclipseCase);
            if (!rimEclipseCase) return;

            RigEclipseCaseData* eclipseCase = rimEclipseCase->eclipseCaseData();
            CVF_ASSERT(eclipseCase);
            if (!eclipseCase) return;

            RigCaseCellResultsData* cellResultsData = eclipseCase->results(this->porosityModel());
            CVF_ASSERT(cellResultsData);

            double globalMin, globalMax;
            double globalPosClosestToZero, globalNegClosestToZero;

            cellResultsData->minMaxCellScalarValues(this->eclipseResultAddress(), globalMin, globalMax);
            cellResultsData->posNegClosestToZero(this->eclipseResultAddress(), globalPosClosestToZero, globalNegClosestToZero);

            double localMin, localMax;
            double localPosClosestToZero, localNegClosestToZero;
            if (this->hasDynamicResult() && currentTimeStep >= 0)
            {
                cellResultsData->minMaxCellScalarValues(this->eclipseResultAddress(), currentTimeStep, localMin, localMax);
                cellResultsData->posNegClosestToZero(
                    this->eclipseResultAddress(), currentTimeStep, localPosClosestToZero, localNegClosestToZero);
            }
            else
            {
                localMin = globalMin;
                localMax = globalMax;

                localPosClosestToZero = globalPosClosestToZero;
                localNegClosestToZero = globalNegClosestToZero;
            }

            CVF_ASSERT(legendConfig);

            legendConfig->disableAllTimeStepsRange(false);
            legendConfig->setClosestToZeroValues(
                globalPosClosestToZero, globalNegClosestToZero, localPosClosestToZero, localNegClosestToZero);
            legendConfig->setAutomaticRanges(globalMin, globalMax, localMin, localMax);

            if (this->hasCategoryResult())
            {
                if (this->resultType() == RiaDefines::FORMATION_NAMES)
                {
                    const std::vector<QString>& fnVector = eclipseCase->activeFormationNames()->formationNames();
                    legendConfig->setNamedCategoriesInverse(fnVector);
                }
                else if (this->resultType() == RiaDefines::DYNAMIC_NATIVE &&
                         this->resultVariable() == RiaDefines::completionTypeResultName())
                {
                    const std::vector<int>& visibleCategories =
                        cellResultsData->uniqueCellScalarValues(this->eclipseResultAddress());

                    std::vector<RiaDefines::WellPathComponentType> supportedCompletionTypes = {
                        RiaDefines::WELL_PATH, RiaDefines::FISHBONES, RiaDefines::PERFORATION_INTERVAL, RiaDefines::FRACTURE};

                    RiaColorTables::WellPathComponentColors colors = RiaColorTables::wellPathComponentColors();

                    std::vector<std::tuple<QString, int, cvf::Color3ub>> categories;
                    for (auto completionType : supportedCompletionTypes)
                    {
                        if (std::find(visibleCategories.begin(), visibleCategories.end(), completionType) !=
                            visibleCategories.end())
                        {
                            QString categoryText = caf::AppEnum<RiaDefines::WellPathComponentType>::uiText(completionType);
                            categories.push_back(std::make_tuple(categoryText, completionType, colors[completionType]));
                        }
                    }

                    legendConfig->setCategoryItems(categories);
                }
                else
                {
                    legendConfig->setIntegerCategories(cellResultsData->uniqueCellScalarValues(this->eclipseResultAddress()));
                }
            }
        }
    }

    // Ternary legend update
    {
        CVF_ASSERT(rimEclipseCase);
        if (!rimEclipseCase) return;

        RigEclipseCaseData* eclipseCase = rimEclipseCase->eclipseCaseData();
        CVF_ASSERT(eclipseCase);
        if (!eclipseCase) return;

        RigCaseCellResultsData* cellResultsData = eclipseCase->results(this->porosityModel());

        size_t maxTimeStepCount = cellResultsData->maxTimeStepCount();
        if (this->isTernarySaturationSelected() && maxTimeStepCount > 1)
        {
            RigCaseCellResultsData* gridCellResults = this->currentGridCellResults();
            {
                RigEclipseResultAddress resAddr(RiaDefines::DYNAMIC_NATIVE, "SOIL");

                if (gridCellResults->ensureKnownResultLoaded(resAddr))
                {
                    double globalMin = 0.0;
                    double globalMax = 1.0;
                    double localMin  = 0.0;
                    double localMax  = 1.0;

                    cellResultsData->minMaxCellScalarValues(resAddr, globalMin, globalMax);
                    cellResultsData->minMaxCellScalarValues(resAddr, currentTimeStep, localMin, localMax);

                    ternaryLegendConfig->setAutomaticRanges(
                        RimTernaryLegendConfig::TERNARY_SOIL_IDX, globalMin, globalMax, localMin, localMax);
                }
            }

            {
                RigEclipseResultAddress resAddr(RiaDefines::DYNAMIC_NATIVE, "SGAS");

                if (gridCellResults->ensureKnownResultLoaded(resAddr))
                {
                    double globalMin = 0.0;
                    double globalMax = 1.0;
                    double localMin  = 0.0;
                    double localMax  = 1.0;

                    cellResultsData->minMaxCellScalarValues(resAddr, globalMin, globalMax);
                    cellResultsData->minMaxCellScalarValues(resAddr, currentTimeStep, localMin, localMax);

                    ternaryLegendConfig->setAutomaticRanges(
                        RimTernaryLegendConfig::TERNARY_SGAS_IDX, globalMin, globalMax, localMin, localMax);
                }
            }

            {
                RigEclipseResultAddress resAddr(RiaDefines::DYNAMIC_NATIVE, "SWAT");

                if (gridCellResults->ensureKnownResultLoaded(resAddr))
                {
                    double globalMin = 0.0;
                    double globalMax = 1.0;
                    double localMin  = 0.0;
                    double localMax  = 1.0;

                    cellResultsData->minMaxCellScalarValues(resAddr, globalMin, globalMax);
                    cellResultsData->minMaxCellScalarValues(resAddr, currentTimeStep, localMin, localMax);

                    ternaryLegendConfig->setAutomaticRanges(
                        RimTernaryLegendConfig::TERNARY_SWAT_IDX, globalMin, globalMax, localMin, localMax);
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::setResultVariable(const QString& val)
{
    RimEclipseResultDefinition::setResultVariable(val);

    this->changeLegendConfig(val);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRegularLegendConfig* RimEclipseCellColors::legendConfig()
{
    return m_legendConfigPtrField;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimTernaryLegendConfig* RimEclipseCellColors::ternaryLegendConfig()
{
    return m_ternaryLegendConfig;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::updateIconState()
{
    if (m_reservoirView)
    {
        RimViewController* viewController = m_reservoirView->viewController();
        if (viewController && viewController->isResultColorControlled())
        {
            updateUiIconFromState(false);
        }
        else
        {
            updateUiIconFromState(true);
        }
    }
    uiCapability()->updateConnectedEditors();
}
