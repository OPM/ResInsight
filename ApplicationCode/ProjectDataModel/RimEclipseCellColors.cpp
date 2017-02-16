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

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigFlowDiagResults.h"
#include "RigFormationNames.h"

#include "RimCellEdgeColors.h"
#include "RimEclipseCase.h"
#include "RimEclipseFaultColors.h"
#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimEclipseWellCollection.h"
#include "RimLegendConfig.h"
#include "RimTernaryLegendConfig.h"
#include "RimViewController.h"
#include "RimViewLinker.h"

#include "RiuMainWindow.h"

#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT(RimEclipseCellColors, "ResultSlot");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCellColors::RimEclipseCellColors()
{
    CAF_PDM_InitObject("Cell Result", ":/CellResult.png", "", "");

    CAF_PDM_InitFieldNoDefault(&obsoleteField_legendConfig, "LegendDefinition", "Legend Definition", "", "", "");
    this->obsoleteField_legendConfig.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitFieldNoDefault(&m_legendConfigData, "ResultVarLegendDefinitionList", "", "", "", "");

    CAF_PDM_InitFieldNoDefault(&ternaryLegendConfig, "TernaryLegendDefinition", "Ternary Legend Definition", "", "", "");
    this->ternaryLegendConfig = new RimTernaryLegendConfig();

    CAF_PDM_InitFieldNoDefault(&m_legendConfigPtrField, "LegendDefinitionPtrField", "Legend Definition PtrField", "", "", "");

    // Make sure we have a created legend for the default/undefined result variable
    changeLegendConfig(this->resultVariable());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCellColors::~RimEclipseCellColors()
{
    CVF_ASSERT(obsoleteField_legendConfig() == NULL);

    m_legendConfigData.deleteAllChildObjects();

    delete ternaryLegendConfig();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimEclipseResultDefinition::fieldChangedByUi(changedField, oldValue, newValue);

    // Update of legend config must happen after RimEclipseResultDefinition::fieldChangedByUi(), as this function modifies this->resultVariable()
    if (changedField == &m_resultVariableUiField)
    {
        if (oldValue != newValue)
        {
            changeLegendConfig(this->resultVariable());
        }

        if (newValue != RimDefines::undefinedResultName())
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
    if (resultVarNameOfNewLegend != RimDefines::ternarySaturationResultName())
    {
        bool found = false;

        QString legendResultVariable;

        if (this->m_legendConfigPtrField())
        {
            legendResultVariable = this->m_legendConfigPtrField()->resultVariableName();
        }

        if (!this->m_legendConfigPtrField() || legendResultVariable != resultVarNameOfNewLegend)
        {
            for (size_t i = 0; i < m_legendConfigData.size(); i++)
            {
                if (m_legendConfigData[i]->resultVariableName() == resultVarNameOfNewLegend)
                {
                    this->m_legendConfigPtrField = m_legendConfigData[i];
                    found = true;
                    break;
                }
            }

            // Not found ?
            if (!found)
            {
                    RimLegendConfig* newLegend = new RimLegendConfig;
                    newLegend->resultVariableName = resultVarNameOfNewLegend;
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
        RimLegendConfig* obsoleteLegend = obsoleteField_legendConfig();

        // set to NULL before pushing into container
        obsoleteField_legendConfig = NULL;

        m_legendConfigData.push_back(obsoleteLegend);
        m_legendConfigPtrField = obsoleteLegend;
    }

    changeLegendConfig(this->resultVariable());

    updateIconState();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    if (this->resultVariable() == RimDefines::ternarySaturationResultName())
    {
        uiTreeOrdering.add(ternaryLegendConfig());
    }
    else
    {
        uiTreeOrdering.add(m_legendConfigPtrField());
    }

   uiTreeOrdering.setForgetRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::updateLegendCategorySettings()
{
    changeLegendConfig(this->resultVariable());

    if (this->hasCategoryResult())
    {
        legendConfig()->setMappingMode(RimLegendConfig::CATEGORY_INTEGER);
        legendConfig()->setColorRangeMode(RimLegendConfig::CATEGORY);
    }
    else
    {
        if (legendConfig()->mappingMode() == RimLegendConfig::CATEGORY_INTEGER)
        {
            legendConfig()->setMappingMode(RimLegendConfig::LINEAR_CONTINUOUS);
        }

        if (legendConfig()->colorRangeMode() == RimLegendConfig::CATEGORY)
        {
            legendConfig()->setColorRangeMode(RimLegendConfig::NORMAL);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::setReservoirView(RimEclipseView* ownerReservoirView)
{
    this->setEclipseCase(ownerReservoirView->eclipseCase());

    m_reservoirView = ownerReservoirView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimEclipseCellColors::reservoirView()
{
    return m_reservoirView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::updateLegendData(size_t currentTimeStep)
{
    if (this->resultType() == RimDefines::FLOW_DIAGNOSTICS)
    {
        double globalMin, globalMax;
        double globalPosClosestToZero, globalNegClosestToZero;
        RigFlowDiagResults* flowResultsData = this->flowDiagSolution()->flowDiagResults();
        RigFlowDiagResultAddress resAddr = this->flowDiagResAddress();

        int integerTimeStep = static_cast<int>(currentTimeStep);

        flowResultsData->minMaxScalarValues(resAddr, integerTimeStep, &globalMin, &globalMax);
        flowResultsData->posNegClosestToZero(resAddr, integerTimeStep, &globalPosClosestToZero, &globalNegClosestToZero);

        double localMin, localMax;
        double localPosClosestToZero, localNegClosestToZero;
        if (this->hasDynamicResult())
        {
            flowResultsData->minMaxScalarValues(resAddr, integerTimeStep, &localMin, &localMax);
            flowResultsData->posNegClosestToZero(resAddr, integerTimeStep, &localPosClosestToZero, &localNegClosestToZero);
        }
        else
        {
            localMin = globalMin;
            localMax = globalMax;

            localPosClosestToZero = globalPosClosestToZero;
            localNegClosestToZero = globalNegClosestToZero;
        }

        CVF_ASSERT(this->legendConfig());

        this->legendConfig()->disableAllTimeStepsRange(true);
        this->legendConfig()->setClosestToZeroValues(globalPosClosestToZero, globalNegClosestToZero, localPosClosestToZero, localNegClosestToZero);
        this->legendConfig()->setAutomaticRanges(globalMin, globalMax, localMin, localMax);

        if (this->hasCategoryResult())
        {
            std::vector<std::tuple<QString, int, cvf::Color3ub>> categories;

            std::vector<QString> tracerNames = this->flowDiagSolution()->tracerNames();

            // Loop through the wells to get same ordering as the wells in tree view
            for (size_t i = 0; i < m_reservoirView->wellCollection()->wells().size(); i++)
            {
                size_t reverseIndex = m_reservoirView->wellCollection()->wells().size() - i - 1;

                RimEclipseWell* well = m_reservoirView->wellCollection()->wells()[reverseIndex];
                QString wellName = well->name();

                auto tracer = std::find(begin(tracerNames), end(tracerNames), wellName);
                if (tracer != end(tracerNames))
                {
                    // The category value is defined as the index of the tracer name in the tracer name vector
                    size_t categoryValue = std::distance(begin(tracerNames), tracer);

                    cvf::Color3ub color(cvf::Color3::SEA_GREEN);
                    color = cvf::Color3ub(well->wellPipeColor());

                    categories.push_back(std::make_tuple(wellName, static_cast<int>(categoryValue), color));
                }
            }

            this->legendConfig()->setCategoryItems(categories);
        }
    }
    else
    {
        RimEclipseCase* rimEclipseCase = nullptr;
        this->firstAncestorOrThisOfType(rimEclipseCase);
        CVF_ASSERT(rimEclipseCase);
        if (!rimEclipseCase) return;

        RigEclipseCaseData* eclipseCase = rimEclipseCase->reservoirData();
        CVF_ASSERT(eclipseCase);
        if (!eclipseCase) return;

        RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(this->porosityModel());
        RigCaseCellResultsData* cellResultsData = eclipseCase->results(porosityModel);
        CVF_ASSERT(cellResultsData);

        double globalMin, globalMax;
        double globalPosClosestToZero, globalNegClosestToZero;
        cellResultsData->minMaxCellScalarValues(this->scalarResultIndex(), globalMin, globalMax);
        cellResultsData->posNegClosestToZero(this->scalarResultIndex(), globalPosClosestToZero, globalNegClosestToZero);

        double localMin, localMax;
        double localPosClosestToZero, localNegClosestToZero;
        if (this->hasDynamicResult())
        {
            cellResultsData->minMaxCellScalarValues(this->scalarResultIndex(), currentTimeStep, localMin, localMax);
            cellResultsData->posNegClosestToZero(this->scalarResultIndex(), currentTimeStep, localPosClosestToZero, localNegClosestToZero);
        }
        else
        {
            localMin = globalMin;
            localMax = globalMax;

            localPosClosestToZero = globalPosClosestToZero;
            localNegClosestToZero = globalNegClosestToZero;
        }

        CVF_ASSERT(this->legendConfig());

        this->legendConfig()->disableAllTimeStepsRange(false);
        this->legendConfig()->setClosestToZeroValues(globalPosClosestToZero, globalNegClosestToZero, localPosClosestToZero, localNegClosestToZero);
        this->legendConfig()->setAutomaticRanges(globalMin, globalMax, localMin, localMax);

        if (this->hasCategoryResult())
        {
            if (this->resultType() != RimDefines::FORMATION_NAMES)
            {
                this->legendConfig()->setIntegerCategories(cellResultsData->uniqueCellScalarValues(this->scalarResultIndex()));
            }
            else
            {
                const std::vector<QString>& fnVector = eclipseCase->activeFormationNames()->formationNames();
                this->legendConfig()->setNamedCategoriesInverse(fnVector);
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
RimLegendConfig* RimEclipseCellColors::legendConfig()
{
    return m_legendConfigPtrField;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCellColors::updateIconState()
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

    uiCapability()->updateConnectedEditors();
}

