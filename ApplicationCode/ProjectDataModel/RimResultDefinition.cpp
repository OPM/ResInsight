/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RIStdInclude.h"

#include "RimResultDefinition.h"

#include "RimReservoirView.h"
#include "RimReservoir.h"
#include "RigReservoirCellResults.h"
#include "RigReservoir.h"
#include "RigMainGrid.h"
#include "cafPdmUiListEditor.h"


CAF_PDM_SOURCE_INIT(RimResultDefinition, "ResultDefinition");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimResultDefinition::RimResultDefinition() 
    : m_gridScalarResultIndex(cvf::UNDEFINED_SIZE_T)
{
    CAF_PDM_InitObject("Result Definition", "", "", "");

    CAF_PDM_InitFieldNoDefault(&resultType,     "ResultType",       "Type", "", "", "");
    CAF_PDM_InitField(&resultVariable, "ResultVariable", RimDefines::undefinedResultName(), "Variable", "", "", "" );

    resultVariable.setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimResultDefinition::~RimResultDefinition()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimResultDefinition::setReservoirView(RimReservoirView* ownerReservoirView)
{
    m_reservoirView = ownerReservoirView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimResultDefinition::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &resultType)
    {
        resultVariable = RimDefines::undefinedResultName();
    }

    loadResult();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimResultDefinition::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    if (fieldNeedingOptions == &resultVariable)
    {
        if (m_reservoirView && m_reservoirView->gridCellResults())
        {
            /*
            QStringList varList;
            if (resultType() == RimDefines::DYNAMIC_NATIVE)
            {
                varList = readerInterface->dynamicResultNames();

                if (!varList.contains("SOIL", Qt::CaseInsensitive))
                {
                    // SOIL will be computed  in RigReservoirCellResults::loadOrComputeSOIL() if SGAS and SWAT is present
                    if (varList.contains("SGAS", Qt::CaseInsensitive) && varList.contains("SWAT", Qt::CaseInsensitive))
                    {
                        varList.push_back("SOIL");
                    }
                }
            }
            else if (resultType == RimDefines::STATIC_NATIVE)
            {
                varList = readerInterface->staticResultNames();
            }
            else if (resultType == RimDefines::GENERATED)
            {
               varList = m_reservoirView->eclipseCase()->reservoirData()->mainGrid()->results()->resultNames(resultType());
            }
            else if (resultType == RimDefines::INPUT_PROPERTY)
            {
                varList = readerInterface->inputPropertyNames();
            }
            */
            
            QStringList varList = m_reservoirView->gridCellResults()->resultNames(resultType());
            QList<caf::PdmOptionItemInfo> optionList;
            int i;
            for (i = 0; i < varList.size(); ++i)
            {
                optionList.push_back(caf::PdmOptionItemInfo( varList[i], varList[i]));
            }
            optionList.push_front(caf::PdmOptionItemInfo( RimDefines::undefinedResultName(), RimDefines::undefinedResultName() ));

            if (useOptionsOnly) *useOptionsOnly = true;

            return optionList;
        }
    }

    return QList<caf::PdmOptionItemInfo>();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RimResultDefinition::gridScalarIndex() const
{
    if (m_gridScalarResultIndex ==  cvf::UNDEFINED_SIZE_T)
    {
        const RigReservoirCellResults* gridCellResults = m_reservoirView->gridCellResults();
        if (gridCellResults) m_gridScalarResultIndex = gridCellResults->findScalarResultIndex(resultType(), resultVariable());
    }
    return m_gridScalarResultIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimResultDefinition::loadResult()
{
    RigReservoirCellResults* gridCellResults = m_reservoirView->gridCellResults();
    if (gridCellResults)
    {
        m_gridScalarResultIndex = gridCellResults->findOrLoadScalarResult(resultType(), resultVariable);
    }
    else
    {
        m_gridScalarResultIndex = cvf::UNDEFINED_SIZE_T;
    }
}


//--------------------------------------------------------------------------------------------------
/// Returns whether the result requested by the definition is a single frame result 
/// The result needs to be loaded before asking
//--------------------------------------------------------------------------------------------------
bool RimResultDefinition::hasStaticResult() const
{
    const RigReservoirCellResults* gridCellResults = m_reservoirView->gridCellResults();
    if (hasResult() && gridCellResults->timeStepCount(m_gridScalarResultIndex) == 1 )
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
/// Returns whether the result requested by the definition is loaded or possible to load from the result file
//--------------------------------------------------------------------------------------------------
bool RimResultDefinition::hasResult() const
{
    if (m_gridScalarResultIndex != cvf::UNDEFINED_SIZE_T) return true;

    const RigReservoirCellResults* gridCellResults = m_reservoirView->gridCellResults();
    if (gridCellResults)
    {
        m_gridScalarResultIndex = gridCellResults->findScalarResultIndex(resultType(), resultVariable());
        return m_gridScalarResultIndex != cvf::UNDEFINED_SIZE_T;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// Returns whether the result requested by the definition is a multi frame result 
/// The result needs to be loaded before asking
//--------------------------------------------------------------------------------------------------
bool RimResultDefinition::hasDynamicResult() const
{
    const RigReservoirCellResults* gridCellResults = m_reservoirView->gridCellResults();
    if (hasResult() && gridCellResults->timeStepCount(m_gridScalarResultIndex) > 1 )
        return true;
    else
        return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirView* RimResultDefinition::reservoirView()
{
    return m_reservoirView;
}
