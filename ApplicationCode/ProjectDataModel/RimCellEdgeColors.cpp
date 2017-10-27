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
#include "RigFlowDiagResults.h"

#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimLegendConfig.h"
#include "RimReservoirCellResultsStorage.h"

#include "cafPdmUiListEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "cvfMath.h"

namespace caf
{
    template<>
    void AppEnum< RimCellEdgeColors::PropertyType >::setUp()
    {
        addItem(RimCellEdgeColors::MULTI_AXIS_STATIC_PROPERTY,  "MULTI_AXIS_STATIC_PROPERTY",   "Multi Axis Static Property");
        addItem(RimCellEdgeColors::ANY_SINGLE_PROPERTY,         "ANY_SINGLE_PROPERTY",          "Any Single Property");
        setDefault(RimCellEdgeColors::MULTI_AXIS_STATIC_PROPERTY);
    }
}


CAF_PDM_SOURCE_INIT(RimCellEdgeColors, "CellEdgeResultSlot");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellEdgeColors::RimCellEdgeColors()
{
    CAF_PDM_InitObject("Cell Edge Result", ":/EdgeResult_1.png", "", "");

    CAF_PDM_InitField(&enableCellEdgeColors, "EnableCellEdgeColors", true, "Enable cell edge results", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_propertyType, "propertyType", "Property Type", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_resultVariable, "CellEdgeVariable", "Result property", "", "", "");
    CAF_PDM_InitField(&useXVariable, "UseXVariable", true, "Use X values", "", "", "");
    CAF_PDM_InitField(&useYVariable, "UseYVariable", true, "Use Y values", "", "", "");
    CAF_PDM_InitField(&useZVariable, "UseZVariable", true, "Use Z values", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_legendConfig, "LegendDefinition", "Legend Definition", ":/Legend.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_singleVarEdgeResultColors, "SingleVarEdgeResult", "Result Property", ":/CellResult.png", "", "");
    m_singleVarEdgeResultColors = new RimEclipseCellColors();

    m_resultVariable.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());

    m_legendConfig = new RimLegendConfig();

    m_ignoredResultScalar = cvf::UNDEFINED_DOUBLE;
    resetResultIndices();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellEdgeColors::~RimCellEdgeColors()
{
    delete m_legendConfig();
    delete m_singleVarEdgeResultColors;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::setReservoirView(RimEclipseView* ownerReservoirView)
{
    m_reservoirView = ownerReservoirView;
    m_singleVarEdgeResultColors->setReservoirView(ownerReservoirView);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::loadResult()
{
    CVF_ASSERT(m_reservoirView && m_reservoirView->currentGridCellResults());

    if (isUsingSingleVariable())
    {
        m_singleVarEdgeResultColors->loadResult();;
        
        size_t resultindex = m_singleVarEdgeResultColors->scalarResultIndex();
        for (int cubeFaceIdx = 0; cubeFaceIdx < 6; ++cubeFaceIdx)
        {
            m_resultNameToIndexPairs[cubeFaceIdx] = std::make_pair(m_singleVarEdgeResultColors->resultVariable(), resultindex);
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
             size_t resultindex = m_reservoirView->currentGridCellResults()->findOrLoadScalarResult(RiaDefines::STATIC_NATIVE, vars[i]);
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

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::initAfterRead()
{
    m_singleVarEdgeResultColors->initAfterRead();
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
            varList = m_reservoirView->currentGridCellResults()->resultNames(RiaDefines::STATIC_NATIVE);

            //TODO: Must also handle input properties
            //varList += m_reservoirView->gridCellResults()->resultNames(RiaDefines::INPUT_PROPERTY);

            QList<caf::PdmOptionItemInfo> options;

            std::map<QString, caf::FixedArray<QString, 6> > varBaseNameToVarsMap;

            int i;
            for (i = 0; i < varList.size(); ++i)
            {
                if (RiaDefines::isPerCellFaceResult(varList[i])) continue;

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

                options.push_back(caf::PdmOptionItemInfo( optionUiName, QVariant(it->first)));

            }

            options.push_front(caf::PdmOptionItemInfo(RiaDefines::undefinedResultName(), ""));
            
            if (useOptionsOnly) *useOptionsOnly = true;

            return options;
        }
    }

    return QList<caf::PdmOptionItemInfo>();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_propertyType);

    if (isUsingSingleVariable())
    {
        m_singleVarEdgeResultColors->uiOrdering(uiConfigName,uiOrdering );
    }
    else
    {
        uiOrdering.add(&m_resultVariable);

        uiOrdering.add(&useXVariable);
        uiOrdering.add(&useYVariable);
        uiOrdering.add(&useZVariable);
    }

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCellEdgeColors::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    uiTreeOrdering.add(legendConfig());
    uiTreeOrdering.skipRemainingChildren(true);
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
        varList = m_reservoirView->currentGridCellResults()->resultNames(RiaDefines::STATIC_NATIVE);
        //TODO: Must handle Input properties

        int i;
        for (i = 0; i < varList.size(); ++i)
        {
            if (RiaDefines::isPerCellFaceResult(varList[i])) continue;

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
bool RimCellEdgeColors::isUsingSingleVariable() const
{
    return (m_propertyType == ANY_SINGLE_PROPERTY);
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
    if (isUsingSingleVariable())
    {
        isStatic = m_singleVarEdgeResultColors->resultType() == RiaDefines::STATIC_NATIVE;
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

    if (isUsingSingleVariable() && m_singleVarEdgeResultColors->isFlowDiagOrInjectionFlooding())
    {
        return true;
    }

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

    if (isUsingSingleVariable() && singleVarEdgeResultColors()->isFlowDiagOrInjectionFlooding())
    {
        int currentTimeStep = m_reservoirView->currentTimeStep();

        RigFlowDiagResults* fldResults = singleVarEdgeResultColors()->flowDiagSolution()->flowDiagResults();
        RigFlowDiagResultAddress resAddr = singleVarEdgeResultColors()->flowDiagResAddress();

        fldResults->minMaxScalarValues(resAddr, currentTimeStep, &globalMin, &globalMax);
    }
    else
    {
        size_t resultIndices[6];
        this->gridScalarIndices(resultIndices);

        size_t idx;
        for (idx = 0; idx < 6; idx++)
        {
            if (resultIndices[idx] == cvf::UNDEFINED_SIZE_T) continue;

            {
                double cMin, cMax;
                m_reservoirView->currentGridCellResults()->minMaxCellScalarValues(resultIndices[idx], cMin, cMax);

                globalMin = CVF_MIN(globalMin, cMin);
                globalMax = CVF_MAX(globalMax, cMax);
            }

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
            m_reservoirView->currentGridCellResults()->posNegClosestToZero(resultIndices[idx], localPos, localNeg);

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
    m_singleVarEdgeResultColors->setEclipseCase(eclipseCase);
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
    if (isUsingSingleVariable())
    {
        return m_singleVarEdgeResultColors->resultVariable();
    }
    else
    {
        return m_resultVariable;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimCellEdgeColors::resultVariableUiName() const
{
    if (isUsingSingleVariable())
    {
        return m_singleVarEdgeResultColors->resultVariableUiName();
    }
    else
    {
        return m_resultVariable;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimCellEdgeColors::resultVariableUiShortName() const
{
    if (isUsingSingleVariable())
    {
        return m_singleVarEdgeResultColors->resultVariableUiShortName();
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimCellEdgeColors::hasCategoryResult() const
{
   return isUsingSingleVariable() && m_singleVarEdgeResultColors->hasCategoryResult();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCellColors* RimCellEdgeColors::singleVarEdgeResultColors()
{
    return m_singleVarEdgeResultColors;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimLegendConfig* RimCellEdgeColors::legendConfig()
{
    if (isUsingSingleVariable())
    {
        return m_singleVarEdgeResultColors->legendConfig();
    }
    else
    {
        return m_legendConfig;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellEdgeColors::PropertyType RimCellEdgeColors::propertyType() const
{
    return m_propertyType();
}

