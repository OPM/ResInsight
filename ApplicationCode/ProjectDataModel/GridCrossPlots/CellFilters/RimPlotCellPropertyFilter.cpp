/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RimPlotCellPropertyFilter.h"

#include "RimEclipseResultDefinition.h"
#include "RimGeoMechResultDefinition.h"

#include "RigActiveCellInfo.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RimEclipseCase.h"
#include "cafPdmUiDoubleSliderEditor.h"

CAF_PDM_SOURCE_INIT(RimPlotCellPropertyFilter, "RimPlotCellPropertyFilter");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotCellPropertyFilter::RimPlotCellPropertyFilter()
{
    CAF_PDM_InitObject("Plot Cell Property Filter", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_resultDefinition, "ResultDefinition", "Result Definition", "", "", "");

    // Set to hidden to avoid this item to been displayed as a child item
    // Fields in this object are displayed using defineUiOrdering()
    m_resultDefinition.uiCapability()->setUiHidden(true);
    m_resultDefinition.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitField(&m_lowerBound, "LowerBound", 0.0, "Min", "", "", "");
    m_lowerBound.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_upperBound, "UpperBound", 0.0, "Max", "", "", "");
    m_upperBound.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCellPropertyFilter::setResultDefinition(caf::PdmObject* resultDefinition)
{
    m_resultDefinition = resultDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCellPropertyFilter::setValueRange(double lowerBound, double upperBound)
{
    m_lowerBound = lowerBound;
    m_upperBound = upperBound;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultDefinition* RimPlotCellPropertyFilter::eclipseResultDefinition()
{
    caf::PdmObject* pdmObj = m_resultDefinition;

    return dynamic_cast<RimEclipseResultDefinition*>(pdmObj);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotCellPropertyFilter::updateCellVisibilityFromFilter(size_t timeStepIndex, cvf::UByteArray* visibleCells)
{
    CVF_ASSERT(visibleCells);

    RimEclipseResultDefinition* resDef = eclipseResultDefinition();
    if (resDef)
    {
        resDef->loadResult();

        RimEclipseCase* eclCase = resDef->eclipseCase();
        if (!eclCase) return;

        eclCase->ensureReservoirCaseIsOpen();
        RigEclipseCaseData* eclipseCaseData = eclCase->eclipseCaseData();
        if (!eclipseCaseData) return;

        RigCaseCellResultsData* cellResultsData = resDef->currentGridCellResults();
        if (!cellResultsData) return;

        const std::vector<double>& cellResultValues = cellResultsData->cellScalarResults(resDef->eclipseResultAddress(), timeStepIndex);

        if (cellResultValues.empty()) return;

        const RigActiveCellInfo* actCellInfo = cellResultsData->activeCellInfo();
        size_t                   cellCount   = actCellInfo->reservoirCellCount();

        bool isUsingGlobalActiveIndex = cellResultsData->isUsingGlobalActiveIndex(resDef->eclipseResultAddress());

        double lowerBound = m_lowerBound;
        double upperBound = m_upperBound;

        for (size_t reservoirCellIndex = 0; reservoirCellIndex < cellCount; ++reservoirCellIndex)
        {
            if (!actCellInfo->isActive(reservoirCellIndex)) continue;

            size_t cellResultIndex = reservoirCellIndex;
            if (isUsingGlobalActiveIndex)
            {
                cellResultIndex = actCellInfo->cellResultIndex(reservoirCellIndex);
            }

            if (cellResultIndex != cvf::UNDEFINED_SIZE_T && cellResultIndex < cellResultValues.size())
            {
                if ((*visibleCells)[reservoirCellIndex])
                {
                    double scalarValue = cellResultValues[cellResultIndex];
                    if (lowerBound <= scalarValue && scalarValue <= upperBound)
                    {
                        if (filterType() == RimPlotCellFilter::EXCLUDE)
                        {
                            (*visibleCells)[reservoirCellIndex] = false;
                        }
                    }
                    else
                    {
                        if (filterType() == RimPlotCellFilter::INCLUDE)
                        {
                            (*visibleCells)[reservoirCellIndex] = false;
                        }
                    }
                }
            }
        }
    }
}
