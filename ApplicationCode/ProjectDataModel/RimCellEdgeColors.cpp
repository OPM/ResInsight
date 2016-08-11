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

#include "RimCellEdgeColors.h"

#include "RigCaseCellResultsData.h"

#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimLegendConfig.h"
#include "RimReservoirCellResultsStorage.h"

#include "cafPdmUiListEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "cvfMath.h"


CAF_PDM_SOURCE_INIT(RimCellEdgeColors, "CellEdgeResultSlot");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellEdgeColors::RimCellEdgeColors()
{
    CAF_PDM_InitObject("Cell Edge Result", ":/EdgeResult_1.png", "", "");

    CAF_PDM_InitField(&enableCellEdgeColors, "EnableCellEdgeColors", true, "Enable cell edge results", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_resultVariable, "CellEdgeVariable", "Result property", "", "", "");
    CAF_PDM_InitField(&useXVariable, "UseXVariable", true, "Use X values", "", "", "");
    CAF_PDM_InitField(&useYVariable, "UseYVariable", true, "Use Y values", "", "", "");
    CAF_PDM_InitField(&useZVariable, "UseZVariable", true, "Use Z values", "", "", "");

    CAF_PDM_InitFieldNoDefault(&legendConfig, "LegendDefinition", "Legend Definition", ":/Legend.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_customFaultResultColors, "CustomEdgeResult", "Custom Edge Result", ":/CellResult.png", "", "");
    m_customFaultResultColors = new RimEclipseCellColors();

    m_resultVariable.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());

    legendConfig = new RimLegendConfig();

    m_ignoredResultScalar = cvf::UNDEFINED_DOUBLE;
    resetResultIndices();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellEdgeColors::~RimCellEdgeColors()
{
    delete legendConfig();
    delete m_customFaultResultColors;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::setReservoirView(RimEclipseView* ownerReservoirView)
{
    m_reservoirView = ownerReservoirView;
    this->legendConfig()->setReservoirView(ownerReservoirView);
    m_customFaultResultColors->setReservoirView(ownerReservoirView);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::loadResult()
{
    CVF_ASSERT(m_reservoirView && m_reservoirView->currentGridCellResults());

    if (m_resultVariable == RimCellEdgeColors::customEdgeResultUiText())
    {
        size_t resultindex = m_reservoirView->currentGridCellResults()->findOrLoadScalarResult(m_customFaultResultColors->resultType(), m_customFaultResultColors->resultVariable());

        for (int cubeFaceIdx = 0; cubeFaceIdx < 6; ++cubeFaceIdx)
        {
            m_resultNameToIndexPairs[cubeFaceIdx] = std::make_pair(m_customFaultResultColors->resultVariable(), resultindex);
        }
    }
    else
    {
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
                 if (   ((cubeFaceIdx == 0 || cubeFaceIdx == 1) && useXVariable())
                     || ((cubeFaceIdx == 2 || cubeFaceIdx == 3) && useYVariable())
                     || ((cubeFaceIdx == 4 || cubeFaceIdx == 5) && useZVariable()))
                 {
                     QString varEnd = EdgeFaceEnum::textFromIndex(cubeFaceIdx);

                     if (vars[i].endsWith(varEnd))
                     {
                         m_resultNameToIndexPairs[cubeFaceIdx] = std::make_pair(vars[i], resultindex);
                     }
                 }
             }
        }
    }

    updateFieldVisibility();

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::initAfterRead()
{
    m_customFaultResultColors->initAfterRead();

    updateFieldVisibility();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    loadResult();

    if (m_reservoirView) m_reservoirView->createDisplayModelAndRedraw();
}

namespace caf
{
    template<>
    void RimCellEdgeColors::EdgeFaceEnum::setUp()
    {
        addItem(RimCellEdgeColors::X,       "X",    "");
        addItem(RimCellEdgeColors::NEG_X,   "X-",   "");
        addItem(RimCellEdgeColors::Y,       "Y",    "");
        addItem(RimCellEdgeColors::NEG_Y,   "Y-",   "");
        addItem(RimCellEdgeColors::Z,       "Z",    "");
        addItem(RimCellEdgeColors::NEG_Z,   "Z-",   "");
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

QList<caf::PdmOptionItemInfo> RimCellEdgeColors::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    if (fieldNeedingOptions == &m_resultVariable)
    {
        if (m_reservoirView && m_reservoirView->currentGridCellResults())
        {
            QStringList varList;
            varList = m_reservoirView->currentGridCellResults()->cellResults()->resultNames(RimDefines::STATIC_NATIVE);

            //TODO: Must also handle input properties
            //varList += m_reservoirView->gridCellResults()->resultNames(RimDefines::INPUT_PROPERTY);

            QList<caf::PdmOptionItemInfo> optionList;

            std::map<QString, caf::FixedArray<QString, 6> > varBaseNameToVarsMap;

            int i;
            for (i = 0; i < varList.size(); ++i)
            {
                if (RimDefines::isPerCellFaceResult(varList[i])) continue;

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

            std::map<QString, caf::FixedArray<QString, 6> >::iterator it;

            for (it = varBaseNameToVarsMap.begin(); it != varBaseNameToVarsMap.end(); ++it)
            {
                QString optionUiName = it->first;
                optionUiName += " (";
                int cubeFaceIdx;
                bool firstText = true;
                for (cubeFaceIdx = 0; cubeFaceIdx < 6; ++cubeFaceIdx)
                {
                    if (!it->second[cubeFaceIdx].isEmpty())
                    {
                        if (firstText)
                        {
                            optionUiName += it->second[cubeFaceIdx];
                            firstText = false;
                        }
                        else
                        {
                            optionUiName += QString(", ") + it->second[cubeFaceIdx];
                        }
                    }
                }
                optionUiName += ")";

                optionList.push_back(caf::PdmOptionItemInfo( optionUiName, QVariant(it->first)));

            }

            optionList.push_front(caf::PdmOptionItemInfo(RimDefines::undefinedResultName(), ""));
            
            optionList.push_back(caf::PdmOptionItemInfo(RimCellEdgeColors::customEdgeResultUiText(), RimCellEdgeColors::customEdgeResultUiText()));

            if (useOptionsOnly) *useOptionsOnly = true;

            return optionList;
        }
    }

    return QList<caf::PdmOptionItemInfo>();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_resultVariable);

    if (m_resultVariable().compare(RimCellEdgeColors::customEdgeResultUiText()) == 0)
    {
        caf::PdmUiGroup* group1 = uiOrdering.addNewGroup("Custom Edge Result");
        group1->add(&(m_customFaultResultColors->m_resultTypeUiField));
        group1->add(&(m_customFaultResultColors->m_porosityModelUiField));
        group1->add(&(m_customFaultResultColors->m_resultVariableUiField));
    }
    else
    {
        uiOrdering.add(&useXVariable);
        uiOrdering.add(&useYVariable);
        uiOrdering.add(&useZVariable);
    }

    uiOrdering.setForgetRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    uiTreeOrdering.add(legendConfig());
    uiTreeOrdering.setForgetRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RimCellEdgeColors::findResultVariableNames()
{
    QStringList varNames;
    
    if (m_reservoirView && m_reservoirView->currentGridCellResults() && !m_resultVariable().isEmpty())
    {
        QStringList varList;
        varList = m_reservoirView->currentGridCellResults()->cellResults()->resultNames(RimDefines::STATIC_NATIVE);
        //TODO: Must handle Input properties

        int i;
        for (i = 0; i < varList.size(); ++i)
        {
            if (RimDefines::isPerCellFaceResult(varList[i])) continue;

            if (varList[i].startsWith(m_resultVariable))
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
void RimCellEdgeColors::gridScalarIndices(size_t resultIndices[6])
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
void RimCellEdgeColors::gridScalarResultNames(std::vector<QString>* resultNames)
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
void RimCellEdgeColors::cellEdgeMetaData(std::vector<RimCellEdgeMetaData>* metaDataVector)
{
    CVF_ASSERT(metaDataVector);

    size_t resultIndices[6];
    this->gridScalarIndices(resultIndices);

    std::vector<QString> resultNames;
    this->gridScalarResultNames(&resultNames);

    bool isStatic = true;
    if (m_resultVariable == RimCellEdgeColors::customEdgeResultUiText())
    {
        isStatic = m_customFaultResultColors->resultType() == RimDefines::STATIC_NATIVE;
    }

    for (size_t i = 0; i < 6; i++)
    {
        RimCellEdgeMetaData metaData;
        metaData.m_resultIndex = resultIndices[i];
        metaData.m_resultVariable = resultNames[i];
        metaData.m_isStatic = isStatic;

        metaDataVector->push_back(metaData);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::updateFieldVisibility()
{
    m_customFaultResultColors->updateFieldVisibility();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::resetResultIndices()
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
bool RimCellEdgeColors::hasResult() const
{
    if (!enableCellEdgeColors()) return false;

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
void RimCellEdgeColors::updateIgnoredScalarValue()
{
    if (m_resultVariable == "MULT" || m_resultVariable == "riMULT")
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
void RimCellEdgeColors::minMaxCellEdgeValues(double& min, double& max)
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::posNegClosestToZero(double& pos, double& neg)
{
    pos = HUGE_VAL;
    neg = -HUGE_VAL;

    size_t resultIndices[6];
    this->gridScalarIndices(resultIndices);

    size_t idx;
    for (idx = 0; idx < 6; idx++)
    {
        if (resultIndices[idx] == cvf::UNDEFINED_SIZE_T) continue;

        {
            double localPos, localNeg;
            m_reservoirView->currentGridCellResults()->cellResults()->posNegClosestToZero(resultIndices[idx], localPos, localNeg);

            if (localPos > 0 && localPos < pos) pos = localPos;
            if (localNeg < 0 && localNeg > neg) neg = localNeg;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::setEclipseCase(RimEclipseCase* eclipseCase)
{
    m_customFaultResultColors->setEclipseCase(eclipseCase);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::setResultVariable(const QString& variableName)
{
    m_resultVariable = variableName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimCellEdgeColors::resultVariable() const
{
    if (m_resultVariable == RimCellEdgeColors::customEdgeResultUiText())
    {
        return m_customFaultResultColors->resultVariable();
    }
    else
    {
        return m_resultVariable;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCellEdgeColors::objectToggleField()
{
   return &enableCellEdgeColors;
}

