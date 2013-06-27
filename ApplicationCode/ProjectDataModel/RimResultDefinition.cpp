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

//#include "RiaStdInclude.h"

#include "RimResultDefinition.h"
#include "RimReservoirCellResultsCacher.h"

#include "RimReservoirView.h"
#include "RimCase.h"
#include "RigCaseCellResultsData.h"
#include "RigCaseData.h"
#include "RigMainGrid.h"
#include "cafPdmUiListEditor.h"


#include "cafPdmFieldCvfMat4d.h"
#include "cafPdmFieldCvfColor.h"
#include "RimResultSlot.h"
#include "RimCellEdgeResultSlot.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCellPropertyFilterCollection.h"
#include "RimWellCollection.h"
#include "Rim3dOverlayInfoConfig.h"

CAF_PDM_SOURCE_INIT(RimResultDefinition, "ResultDefinition");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimResultDefinition::RimResultDefinition() 
    //: m_gridScalarResultIndex(cvf::UNDEFINED_SIZE_T)
{
    CAF_PDM_InitObject("Result Definition", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_resultType,     "ResultType",           "Type", "", "", "");
    m_resultType.setUiHidden(true);
    CAF_PDM_InitFieldNoDefault(&m_porosityModel,  "PorosityModelType",    "Type", "", "", "");
    m_porosityModel.setUiHidden(true);
    CAF_PDM_InitField(&m_resultVariable, "ResultVariable", RimDefines::undefinedResultName(), "Variable", "", "", "" );
    m_resultVariable.setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_resultTypeUiField,     "MResultType",           "Type", "", "", "");
    m_resultTypeUiField.setIOReadable(false);
    m_resultTypeUiField.setIOWritable(false);
    CAF_PDM_InitFieldNoDefault(&m_porosityModelUiField,  "MPorosityModelType",    "Type", "", "", "");
    m_porosityModelUiField.setIOReadable(false);
    m_porosityModelUiField.setIOWritable(false);
    CAF_PDM_InitField(&m_resultVariableUiField, "MResultVariable", RimDefines::undefinedResultName(), "Variable", "", "", "" );
    m_resultVariableUiField.setIOReadable(false);
    m_resultVariableUiField.setIOWritable(false);


    m_resultVariableUiField.setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());
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

    // TODO: This code is executed before reservoir is read, and then porosity model is never set to zero
    if (m_reservoirView->eclipseCase() &&
        m_reservoirView->eclipseCase()->reservoirData() &&
        m_reservoirView->eclipseCase()->reservoirData()->activeCellInfo(RifReaderInterface::FRACTURE_RESULTS) )
    {
        if (m_reservoirView->eclipseCase()->reservoirData()->activeCellInfo(RifReaderInterface::FRACTURE_RESULTS)->globalActiveCellCount() == 0)
        {
            m_porosityModelUiField.setUiHidden(true);
        }
    }
}

QStringList RimResultDefinition::getResultVariableListForCurrentUIFieldSettings()
{
    if (!m_reservoirView || !m_reservoirView->eclipseCase() ) return QStringList();

    RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(m_porosityModelUiField());

    return m_reservoirView->eclipseCase()->results(porosityModel)->cellResults()->resultNames(m_resultTypeUiField());
}


RimReservoirCellResultsStorage* RimResultDefinition::currentGridCellResults() const
{
    if (!m_reservoirView || !m_reservoirView->eclipseCase()) return NULL;

    RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(m_porosityModel());

    return m_reservoirView->eclipseCase()->results(porosityModel);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimResultDefinition::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (   &m_resultTypeUiField == changedField 
        || &m_porosityModelUiField == changedField )
    {
        QStringList varList = getResultVariableListForCurrentUIFieldSettings();

        // If the user are seeing the list with the actually selected result, select that result in the list. Otherwise select nothing.
        if (   m_resultTypeUiField() == m_resultType() 
            && m_porosityModelUiField() == m_porosityModel() 
            && varList.contains(resultVariable()))
        {
            m_resultVariableUiField = resultVariable();
        }
        else
        {
            m_resultVariableUiField = "";
        }
    }
  
    if (&m_resultVariableUiField == changedField)
    {
        m_porosityModel = m_porosityModelUiField;
        m_resultType = m_resultTypeUiField;
        m_resultVariable = m_resultVariableUiField;

        loadResult();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimResultDefinition::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    if (fieldNeedingOptions == &m_resultVariableUiField)
    {
        if (this->currentGridCellResults())
        {
            QStringList varList = getResultVariableListForCurrentUIFieldSettings();
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
    size_t gridScalarResultIndex = cvf::UNDEFINED_SIZE_T;

    const RimReservoirCellResultsStorage* gridCellResults = this->currentGridCellResults();
    if (gridCellResults) gridScalarResultIndex = gridCellResults->cellResults()->findScalarResultIndex(m_resultType(), m_resultVariable());

    return gridScalarResultIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimResultDefinition::loadResult()
{
    RimReservoirCellResultsStorage* gridCellResults = this->currentGridCellResults();
    if (gridCellResults)
    {
        gridCellResults->findOrLoadScalarResult(m_resultType(), m_resultVariable);
    }
   
}


//--------------------------------------------------------------------------------------------------
/// Returns whether the result requested by the definition is a single frame result 
/// The result needs to be loaded before asking
//--------------------------------------------------------------------------------------------------
bool RimResultDefinition::hasStaticResult() const
{
    const RimReservoirCellResultsStorage* gridCellResults = this->currentGridCellResults();
    size_t gridScalarResultIndex = this->gridScalarIndex();

    if (hasResult() && gridCellResults->cellResults()->timeStepCount(gridScalarResultIndex) == 1 )
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

    if (this->currentGridCellResults() && this->currentGridCellResults()->cellResults())
    {
        const RigCaseCellResultsData* gridCellResults = this->currentGridCellResults()->cellResults();
        size_t gridScalarResultIndex = gridCellResults->findScalarResultIndex(m_resultType(), m_resultVariable());
        return gridScalarResultIndex != cvf::UNDEFINED_SIZE_T;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// Returns whether the result requested by the definition is a multi frame result 
/// The result needs to be loaded before asking
//--------------------------------------------------------------------------------------------------
bool RimResultDefinition::hasDynamicResult() const
{
    if (hasResult())
    {
        if (m_resultType() == RimDefines::DYNAMIC_NATIVE)
        {
            return true;
        }

        if (this->currentGridCellResults() && this->currentGridCellResults()->cellResults())
        {
            const RigCaseCellResultsData* gridCellResults = this->currentGridCellResults()->cellResults();
            size_t gridScalarResultIndex = this->gridScalarIndex();
            if (gridCellResults->timeStepCount(gridScalarResultIndex) > 1 )
            {
                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirView* RimResultDefinition::reservoirView()
{
    return m_reservoirView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimResultDefinition::initAfterRead()
{
    m_porosityModelUiField = m_porosityModel;
    m_resultTypeUiField = m_resultType;
    m_resultVariableUiField = m_resultVariable;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimResultDefinition::setResultType(RimDefines::ResultCatType val)
{
    m_resultType = val;
    m_resultTypeUiField = val;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimResultDefinition::setPorosityModel(RimDefines::PorosityModelType val)
{
    m_porosityModel = val;
    m_porosityModelUiField = val;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimResultDefinition::setResultVariable(const QString& val)
{
    m_resultVariable = val;
    m_resultVariableUiField = val;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimResultDefinition::setPorosityModelUiFieldHidden(bool hide)
{
    m_porosityModelUiField.setUiHidden(true);
}
