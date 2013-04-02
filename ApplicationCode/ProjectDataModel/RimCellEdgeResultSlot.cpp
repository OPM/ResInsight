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

#include "RiaStdInclude.h"

#include "RimCellEdgeResultSlot.h"
#include "RimLegendConfig.h"
#include "RimReservoirView.h"
#include "RimCase.h"
#include "RimReservoirView.h"
#include "RigCaseCellResultsData.h"
#include "RigCaseData.h"

#include "cafPdmUiListEditor.h"


CAF_PDM_SOURCE_INIT(RimCellEdgeResultSlot, "CellEdgeResultSlot");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellEdgeResultSlot::RimCellEdgeResultSlot()
{
    CAF_PDM_InitObject("Cell Edge Result", "", "", "");

    CAF_PDM_InitFieldNoDefault(&resultVariable, "CellEdgeVariable", "Result variable", "", "", "");
    CAF_PDM_InitFieldNoDefault(&legendConfig, "LegendDefinition", "Legend Definition", ":/Legend.png", "", "");

    resultVariable.setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());

    legendConfig = new RimLegendConfig();

    m_ignoredResultScalar = cvf::UNDEFINED_DOUBLE;
    resetResultIndices();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellEdgeResultSlot::~RimCellEdgeResultSlot()
{
    delete legendConfig();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeResultSlot::setReservoirView(RimReservoirView* ownerReservoirView)
{
    m_reservoirView = ownerReservoirView;
    this->legendConfig()->setReservoirView(ownerReservoirView);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeResultSlot::loadResult()
{
    CVF_ASSERT(m_reservoirView && m_reservoirView->currentGridCellResults());

    resetResultIndices();
    QStringList vars = findResultVariableNames();
    updateIgnoredScalarValue();

    int i;
    for (i = 0; i < vars.size(); ++i)
    {
         size_t resultindex = m_reservoirView->currentGridCellResults()->findOrLoadScalarResult(RimDefines::STATIC_NATIVE, vars[i]);
         int cubeFaceIdx;
         for (cubeFaceIdx = 0; cubeFaceIdx < 6; ++cubeFaceIdx)
         {
             QString varEnd = EdgeFaceEnum::textFromIndex(cubeFaceIdx);

             if (vars[i].endsWith(varEnd))
             {
                 m_resultNameToIndexPairs[cubeFaceIdx] = std::make_pair(vars[i], resultindex);
             }
         }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeResultSlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &resultVariable)
    {
        loadResult();
    }

    if (m_reservoirView) m_reservoirView->createDisplayModelAndRedraw();
}

namespace caf
{
    template<>
    void RimCellEdgeResultSlot::EdgeFaceEnum::setUp()
    {
        addItem(RimCellEdgeResultSlot::X,       "X",    "");
        addItem(RimCellEdgeResultSlot::NEG_X,   "X-",   "");
        addItem(RimCellEdgeResultSlot::Y,       "Y",    "");
        addItem(RimCellEdgeResultSlot::NEG_Y,   "Y-",   "");
        addItem(RimCellEdgeResultSlot::Z,       "Z",    "");
        addItem(RimCellEdgeResultSlot::NEG_Z,   "Z-",   "");
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

QList<caf::PdmOptionItemInfo> RimCellEdgeResultSlot::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    if (fieldNeedingOptions == &resultVariable)
    {
        if (m_reservoirView && m_reservoirView->currentGridCellResults())
        {
            QStringList varList;
            varList = m_reservoirView->currentGridCellResults()->cellResults()->resultNames(RimDefines::STATIC_NATIVE);

            //TODO: Must also handle input properties
            //varList += m_reservoirView->gridCellResults()->resultNames(RimDefines::INPUT_PROPERTY);

            QList<caf::PdmOptionItemInfo> optionList;

            std::map<QString, caf::fvector<QString, 6> > varBaseNameToVarsMap;

            int i;
            for (i = 0; i < varList.size(); ++i)
            {
                size_t cubeFaceIdx;
                for (cubeFaceIdx = 0; cubeFaceIdx < EdgeFaceEnum::size(); ++cubeFaceIdx)
                {
                    QString varEnd = EdgeFaceEnum::textFromIndex(cubeFaceIdx);
                    if (varList[i].endsWith(varEnd))
                    {
                        QStringList splits = varList[i].split(varEnd);
                        QString variableBasename = splits.front();
                        varBaseNameToVarsMap[variableBasename][cubeFaceIdx] = varList[i];
                    }
                }
            }

            std::map<QString, caf::fvector<QString, 6> >::iterator it;

            for (it = varBaseNameToVarsMap.begin(); it != varBaseNameToVarsMap.end(); ++it)
            {
                QString optionUiName = it->first;
                optionUiName += " (";
                int cubeFaceIdx;
                bool firstText = true;
                for (cubeFaceIdx = 0; cubeFaceIdx < 6; ++cubeFaceIdx)
                {
                    if (!it->second.v[cubeFaceIdx].isEmpty())
                    {
                        if (firstText)
                        {
                            optionUiName += it->second.v[cubeFaceIdx];
                            firstText = false;
                        }
                        else
                        {
                            optionUiName += QString(", ") + it->second.v[cubeFaceIdx];
                        }
                    }
                }
                optionUiName += ")";

                optionList.push_back(caf::PdmOptionItemInfo( optionUiName, QVariant(it->first)));

            }

            optionList.push_front(caf::PdmOptionItemInfo( RimDefines::undefinedResultName(), "" ));

            if (useOptionsOnly) *useOptionsOnly = true;

            return optionList;
        }
    }

    return QList<caf::PdmOptionItemInfo>();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RimCellEdgeResultSlot::findResultVariableNames()
{
    QStringList varNames;
    
    if (m_reservoirView && m_reservoirView->currentGridCellResults() && !resultVariable().isEmpty())
    {
        QStringList varList;
        varList = m_reservoirView->currentGridCellResults()->cellResults()->resultNames(RimDefines::STATIC_NATIVE);
        //TODO: Must handle Input properties

        int i;
        for (i = 0; i < varList.size(); ++i)
        {
            if (varList[i].contains(resultVariable))
            {               
                varNames.append(varList[i]);
            }
        }
    }
    return varNames;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeResultSlot::gridScalarIndices(size_t resultIndices[6])
{
    int cubeFaceIndex;
    for (cubeFaceIndex = 0; cubeFaceIndex < 6; ++cubeFaceIndex)
    {
        resultIndices[cubeFaceIndex] = m_resultNameToIndexPairs[cubeFaceIndex].second;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeResultSlot::gridScalarResultNames(QStringList* resultNames)
{
    CVF_ASSERT(resultNames);

    int cubeFaceIndex;
    for (cubeFaceIndex = 0; cubeFaceIndex < 6; ++cubeFaceIndex)
    {
        resultNames->push_back(m_resultNameToIndexPairs[cubeFaceIndex].first);
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeResultSlot::resetResultIndices()
{
    int cubeFaceIndex;
    for (cubeFaceIndex = 0; cubeFaceIndex < 6; ++cubeFaceIndex)
    {
        m_resultNameToIndexPairs[cubeFaceIndex].second = cvf::UNDEFINED_SIZE_T;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimCellEdgeResultSlot::hasResult() const
{
    bool hasResult = false;
    int cubeFaceIndex;
    for (cubeFaceIndex = 0; cubeFaceIndex < 6; ++cubeFaceIndex)
    {
        hasResult |=  ((m_resultNameToIndexPairs[cubeFaceIndex].second) != cvf::UNDEFINED_SIZE_T);
    }

    return hasResult;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeResultSlot::updateIgnoredScalarValue()
{
    if (resultVariable == "MULT")
    {
        m_ignoredResultScalar = 1.0;
    }
    else
    {
        m_ignoredResultScalar = cvf::UNDEFINED_DOUBLE;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeResultSlot::minMaxCellEdgeValues(double& min, double& max)
{
    double globalMin, globalMax;
    globalMin = HUGE_VAL;
    globalMax = -HUGE_VAL;

    size_t resultIndices[6];
    this->gridScalarIndices(resultIndices);

    size_t idx;
    for (idx = 0; idx < 6; idx++)
    {
        if (resultIndices[idx] == cvf::UNDEFINED_SIZE_T) continue;

        {
            double cMin, cMax;
            m_reservoirView->currentGridCellResults()->cellResults()->minMaxCellScalarValues(resultIndices[idx], cMin, cMax);

            globalMin = CVF_MIN(globalMin, cMin);
            globalMax = CVF_MAX(globalMax, cMax);
        }

    }

    min = globalMin;
    max = globalMax;
}

