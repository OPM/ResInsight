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

#include "RimStatisticsCase.h"
#include "RimReservoirView.h"
#include "cafPdmUiOrdering.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RigCaseData.h"
#include "RigCaseCellResultsData.h"
#include "RimStatisticsCaseEvaluator.h"
#include "RigMainGrid.h"
#include "cafPdmUiTextEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiPushButtonEditor.h"

namespace caf {
    template<>
    void caf::AppEnum<RimStatisticsCase::PercentileCalcType>::setUp()
    {
        addItem(RimStatisticsCase::NEAREST_OBSERVATION,  "NearestObservationPercentile",  "Nearest Observation");
        addItem(RimStatisticsCase::HISTOGRAM_ESTIMATED,  "HistogramEstimatedPercentile",  "Histogram based estimate");
        setDefault(RimStatisticsCase::NEAREST_OBSERVATION); 
    }
}


CAF_PDM_SOURCE_INIT(RimStatisticsCase, "RimStatisticalCalculation");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStatisticsCase::RimStatisticsCase()
    : RimCase()
{
    CAF_PDM_InitObject("Case Group Statistics", ":/Histogram16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_calculateEditCommand,   "m_editingAllowed", "", "", "", "");
    m_calculateEditCommand.setIOWritable(false);
    m_calculateEditCommand.setIOReadable(false);
    m_calculateEditCommand.setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_calculateEditCommand.setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    m_calculateEditCommand = false;

    CAF_PDM_InitField(&m_selectionSummary, "SelectionSummary", QString(""), "Summary of calculation setup", "", "", "");
    m_selectionSummary.setIOWritable(false);
    m_selectionSummary.setIOReadable(false);
    m_selectionSummary.setUiReadOnly(true);
    m_selectionSummary.setUiEditorTypeName(caf::PdmUiTextEditor::uiEditorTypeName());
    m_selectionSummary.setUiLabelPosition(caf::PdmUiItemInfo::TOP);

    CAF_PDM_InitFieldNoDefault(&m_resultType, "ResultType", "Result Type", "", "", "");
    m_resultType.setIOWritable(false);
    CAF_PDM_InitFieldNoDefault(&m_porosityModel, "PorosityModel", "Porosity Model", "", "", "");
    m_porosityModel.setIOWritable(false);

    CAF_PDM_InitFieldNoDefault(&m_selectedDynamicProperties,   "DynamicPropertiesToCalculate", "Dyn Prop", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_selectedStaticProperties,    "StaticPropertiesToCalculate", "Stat Prop", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_selectedGeneratedProperties, "GeneratedPropertiesToCalculate", "", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_selectedInputProperties,     "InputPropertiesToCalculate", "", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_selectedFractureDynamicProperties,   "FractureDynamicPropertiesToCalculate", "", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_selectedFractureStaticProperties,    "FractureStaticPropertiesToCalculate", "", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_selectedFractureGeneratedProperties, "FractureGeneratedPropertiesToCalculate", "", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_selectedFractureInputProperties,     "FractureInputPropertiesToCalculate", "", "", "", "");

    m_selectedDynamicProperties.setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_selectedStaticProperties.setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_selectedGeneratedProperties.setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_selectedInputProperties.setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    m_selectedFractureDynamicProperties.setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_selectedFractureStaticProperties.setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN); 
    m_selectedFractureGeneratedProperties.setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_selectedFractureInputProperties.setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);

    CAF_PDM_InitField(&m_calculatePercentiles, "CalculatePercentiles", true, "Calculate Percentiles", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_percentileCalculationType, "PercentileCalculationType", "Method", "", "", "");

    CAF_PDM_InitField(&m_lowPercentile, "LowPercentile", 10.0, "Low", "", "", "");
    CAF_PDM_InitField(&m_midPercentile, "MidPercentile", 50.0, "Mid", "", "", "");
    CAF_PDM_InitField(&m_highPercentile, "HighPercentile", 90.0, "High", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStatisticsCase::~RimStatisticsCase()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticsCase::setMainGrid(RigMainGrid* mainGrid)
{
    CVF_ASSERT(mainGrid);
    CVF_ASSERT(this->reservoirData());

    reservoirData()->setMainGrid(mainGrid);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimStatisticsCase::openEclipseGridFile()
{
    if (this->reservoirData()) return true;

    cvf::ref<RigCaseData> eclipseCase = new RigCaseData;

    CVF_ASSERT(parentStatisticsCaseCollection());

    RimIdenticalGridCaseGroup* gridCaseGroup = parentStatisticsCaseCollection()->parentCaseGroup();
    CVF_ASSERT(gridCaseGroup);

    RigMainGrid* mainGrid = gridCaseGroup->mainGrid();

    eclipseCase->setMainGrid(mainGrid);

    eclipseCase->setActiveCellInfo(RifReaderInterface::MATRIX_RESULTS, gridCaseGroup->unionOfActiveCells(RifReaderInterface::MATRIX_RESULTS));
    eclipseCase->setActiveCellInfo(RifReaderInterface::FRACTURE_RESULTS, gridCaseGroup->unionOfActiveCells(RifReaderInterface::FRACTURE_RESULTS));

    this->setReservoirData( eclipseCase.p() );

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCaseCollection* RimStatisticsCase::parentStatisticsCaseCollection()
{
    std::vector<RimCaseCollection*> parentObjects;
    this->parentObjectsOfType(parentObjects);

    if (parentObjects.size() > 0)
    {
        return parentObjects[0];
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticsCase::computeStatistics()
{
    if (this->reservoirData() == NULL)
    {
        openEclipseGridFile();
    }

    RimIdenticalGridCaseGroup* gridCaseGroup = caseGroup();
    CVF_ASSERT(gridCaseGroup);
    gridCaseGroup->computeUnionOfActiveCells();

    std::vector<RimCase*> sourceCases;

    getSourceCases(sourceCases);

    if (sourceCases.size() == 0)
    {
        return;
    }

    // The first source has been read completely from disk, and contains grid and meta data
    // Use this information for all cases in the case group
    size_t timeStepCount = sourceCases.at(0)->results(RifReaderInterface::MATRIX_RESULTS)->cellResults()->maxTimeStepCount();

    RimStatisticsConfig statisticsConfig;

    statisticsConfig.m_calculatePercentiles = m_calculatePercentiles();
    statisticsConfig.m_pMaxPos = m_highPercentile();
    statisticsConfig.m_pMidPos = m_midPercentile();
    statisticsConfig.m_pMinPos = m_lowPercentile();
    statisticsConfig.m_pValMethod = m_percentileCalculationType();

    std::vector<size_t> timeStepIndices;
    for (size_t i = 0; i < timeStepCount; i++)
    {
        timeStepIndices.push_back(i);
    }

    RigCaseData* resultCase = reservoirData();

    QList<RimStatisticsCaseEvaluator::ResSpec > resultSpecification;

    for(size_t pIdx = 0; pIdx < m_selectedDynamicProperties().size(); ++pIdx)
    {
        resultSpecification.append(RimStatisticsCaseEvaluator::ResSpec(RifReaderInterface::MATRIX_RESULTS, RimDefines::DYNAMIC_NATIVE, m_selectedDynamicProperties()[pIdx]));
    }

    for(size_t pIdx = 0; pIdx < m_selectedStaticProperties().size(); ++pIdx)
    {
        resultSpecification.append(RimStatisticsCaseEvaluator::ResSpec(RifReaderInterface::MATRIX_RESULTS, RimDefines::STATIC_NATIVE, m_selectedStaticProperties()[pIdx]));
    }

    for(size_t pIdx = 0; pIdx < m_selectedGeneratedProperties().size(); ++pIdx)
    {
        resultSpecification.append(RimStatisticsCaseEvaluator::ResSpec(RifReaderInterface::MATRIX_RESULTS, RimDefines::GENERATED, m_selectedGeneratedProperties()[pIdx]));
    }

    for(size_t pIdx = 0; pIdx < m_selectedInputProperties().size(); ++pIdx)
    {
        resultSpecification.append(RimStatisticsCaseEvaluator::ResSpec(RifReaderInterface::MATRIX_RESULTS, RimDefines::INPUT_PROPERTY, m_selectedInputProperties()[pIdx]));
    }

    for(size_t pIdx = 0; pIdx < m_selectedFractureDynamicProperties().size(); ++pIdx)
    {
        resultSpecification.append(RimStatisticsCaseEvaluator::ResSpec(RifReaderInterface::FRACTURE_RESULTS, RimDefines::DYNAMIC_NATIVE, m_selectedFractureDynamicProperties()[pIdx]));
    }

    for(size_t pIdx = 0; pIdx < m_selectedFractureStaticProperties().size(); ++pIdx)
    {
        resultSpecification.append(RimStatisticsCaseEvaluator::ResSpec(RifReaderInterface::FRACTURE_RESULTS, RimDefines::STATIC_NATIVE, m_selectedFractureStaticProperties()[pIdx]));
    }

    for(size_t pIdx = 0; pIdx < m_selectedFractureGeneratedProperties().size(); ++pIdx)
    {
        resultSpecification.append(RimStatisticsCaseEvaluator::ResSpec(RifReaderInterface::FRACTURE_RESULTS, RimDefines::GENERATED, m_selectedFractureGeneratedProperties()[pIdx]));
    }

    for(size_t pIdx = 0; pIdx < m_selectedFractureInputProperties().size(); ++pIdx)
    {
        resultSpecification.append(RimStatisticsCaseEvaluator::ResSpec(RifReaderInterface::FRACTURE_RESULTS, RimDefines::INPUT_PROPERTY, m_selectedFractureInputProperties()[pIdx]));
    }

    RimStatisticsCaseEvaluator stat(sourceCases, timeStepIndices, statisticsConfig, resultCase);
    stat.evaluateForResults(resultSpecification);

    // Todo: Is this really the time and place to do the following ? JJS

    for (size_t i = 0; i < reservoirViews().size(); i++)
    {
        RimReservoirView* reservoirView = reservoirViews()[i];
        CVF_ASSERT(reservoirView);

        reservoirView->scheduleGeometryRegen(RivReservoirViewPartMgr::ACTIVE);
        reservoirView->createDisplayModelAndRedraw();
    }

  
    this->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticsCase::getSourceCases(std::vector<RimCase*>& sourceCases)
{
    RimIdenticalGridCaseGroup* gridCaseGroup = caseGroup();
    if (gridCaseGroup)
    {
        size_t caseCount = gridCaseGroup->caseCollection->reservoirs.size();
        for (size_t i = 0; i < caseCount; i++)
        {
            CVF_ASSERT(gridCaseGroup->caseCollection);
            CVF_ASSERT(gridCaseGroup->caseCollection->reservoirs[i]);
            CVF_ASSERT(gridCaseGroup->caseCollection->reservoirs[i]->reservoirData());

            RimCase* sourceCase = gridCaseGroup->caseCollection->reservoirs[i];
            sourceCases.push_back(sourceCase);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIdenticalGridCaseGroup* RimStatisticsCase::caseGroup()
{
    RimCaseCollection* parentCollection = parentStatisticsCaseCollection();
    if (parentCollection)
    {
        return parentCollection->parentCaseGroup();
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticsCase::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) 
{

    updateSelectionSummaryLabel();
    updateSelectionListVisibilities();
    updatePercentileUiVisibility();

    uiOrdering.add(&caseName);
    uiOrdering.add(&m_calculateEditCommand);
    uiOrdering.add(&m_selectionSummary);

    caf::PdmUiGroup * group = uiOrdering.addNewGroup("Properties to consider");
    group->setUiHidden(hasComputedStatistics());
    group->add(&m_resultType);
    group->add(&m_porosityModel);
    group->add(&m_selectedDynamicProperties);
    group->add(&m_selectedStaticProperties);
    group->add(&m_selectedGeneratedProperties);
    group->add(&m_selectedInputProperties);
    group->add(&m_selectedFractureDynamicProperties);
    group->add(&m_selectedFractureStaticProperties);
    group->add(&m_selectedFractureGeneratedProperties);
    group->add(&m_selectedFractureInputProperties);

    group = uiOrdering.addNewGroup("Percentile setup");
    group->setUiHidden(hasComputedStatistics());
    group->add(&m_calculatePercentiles);
    group->add(&m_percentileCalculationType);
    group->add(&m_lowPercentile);
    group->add(&m_midPercentile);
    group->add(&m_highPercentile);
}

QList<caf::PdmOptionItemInfo> toOptionList(const QStringList& varList)
{
    QList<caf::PdmOptionItemInfo> optionList;
    int i;
    for (i = 0; i < varList.size(); ++i)
    {
        optionList.push_back(caf::PdmOptionItemInfo( varList[i], varList[i]));
    }
    return optionList;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimStatisticsCase::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;
    if (useOptionsOnly) *useOptionsOnly = true;

    RimIdenticalGridCaseGroup* idgcg = caseGroup();
    if (!(caseGroup() && caseGroup()->mainCase() && caseGroup()->mainCase()->reservoirData())) 
    {
        return options;
    }

    RigCaseData* caseData = idgcg->mainCase()->reservoirData();

    if (&m_selectedDynamicProperties == fieldNeedingOptions)
    {
        QStringList varList = caseData->results(RifReaderInterface::MATRIX_RESULTS)->resultNames(RimDefines::DYNAMIC_NATIVE);
        return toOptionList(varList);
    } 
    else if (&m_selectedStaticProperties == fieldNeedingOptions)
    {
        QStringList varList = caseData->results(RifReaderInterface::MATRIX_RESULTS)->resultNames(RimDefines::STATIC_NATIVE);
        return toOptionList(varList);
    } 
    else if (&m_selectedGeneratedProperties == fieldNeedingOptions)
    {
        QStringList varList = caseData->results(RifReaderInterface::MATRIX_RESULTS)->resultNames(RimDefines::GENERATED);
        return toOptionList(varList);
    } 
    else if (&m_selectedInputProperties == fieldNeedingOptions)
    {
        QStringList varList = caseData->results(RifReaderInterface::MATRIX_RESULTS)->resultNames(RimDefines::INPUT_PROPERTY);
        return toOptionList(varList);
    }
    else if (&m_selectedFractureDynamicProperties == fieldNeedingOptions)
    {
        QStringList varList = caseData->results(RifReaderInterface::FRACTURE_RESULTS)->resultNames(RimDefines::DYNAMIC_NATIVE);
        return toOptionList(varList);
    } 
    else if (&m_selectedFractureStaticProperties == fieldNeedingOptions)
    {
        QStringList varList = caseData->results(RifReaderInterface::FRACTURE_RESULTS)->resultNames(RimDefines::STATIC_NATIVE);
        return toOptionList(varList);
    } 
    else if (&m_selectedFractureGeneratedProperties == fieldNeedingOptions)
    {
        QStringList varList = caseData->results(RifReaderInterface::FRACTURE_RESULTS)->resultNames(RimDefines::GENERATED);
        return toOptionList(varList);
    } 
    else if (&m_selectedFractureInputProperties == fieldNeedingOptions)
    {
        QStringList varList = caseData->results(RifReaderInterface::FRACTURE_RESULTS)->resultNames(RimDefines::INPUT_PROPERTY);
        return toOptionList(varList);
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticsCase::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (&m_resultType == changedField || &m_porosityModel == changedField)
    {
    }

    if (&m_calculateEditCommand == changedField)
    {
        if (hasComputedStatistics())
        {
            clearComputedStatistics();
        }
        else
        {
            computeStatistics();
        }
        m_calculateEditCommand = false;
    }

     //updateSelectionSummaryLabel();
}

void addPropertySetToHtmlText(QString& html, const QString& heading, const std::vector<QString>& varNames)
{
    if (varNames.size())
    {
        html += "<p><b>" + heading + "</b></p>";
        html += "<p class=indent>";
        for (size_t pIdx = 0; pIdx < varNames.size(); ++pIdx)
        {
            html += varNames[pIdx];
            if ( (pIdx+1)%6 == 0 ) html += "<br>";
            else if (pIdx != varNames.size() -1) html += ", ";
        }
        html += "</p>";
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticsCase::updateSelectionSummaryLabel()
{
    QString html;
    
    html += "<style> "
                "p{ margin-top:0px; margin-bottom:0px;} "
                "p.indent{margin-left:20px; margin-top:0px;} "
                "p.indent2{margin-left:40px; margin-top:0px;} "
            "</style>";

    html += "<p><b>Statistical variables to compute:</b></p>";
    html += "<p class=indent>";
    html += "Min, Max, Range, Mean, Std.dev"; ;
    if (m_calculatePercentiles())
    {
        html += "<br>";
        html += "Percentiles for : " 
            + QString::number(m_lowPercentile()) + ", " 
            + QString::number(m_midPercentile()) + ", " 
            + QString::number(m_highPercentile());
    }
    html += "</p>";

    addPropertySetToHtmlText(html, "Dynamic properties", m_selectedDynamicProperties());
    addPropertySetToHtmlText(html, "Static properties", m_selectedStaticProperties());
    addPropertySetToHtmlText(html, "Generated properties", m_selectedGeneratedProperties());
    addPropertySetToHtmlText(html, "Input properties", m_selectedInputProperties());

    addPropertySetToHtmlText(html, "Dynamic properties, fracture model"  , m_selectedFractureDynamicProperties());
    addPropertySetToHtmlText(html, "Static properties, fracture model"   , m_selectedFractureStaticProperties());
    addPropertySetToHtmlText(html, "Generated properties, fracture model", m_selectedFractureGeneratedProperties());
    addPropertySetToHtmlText(html, "Input properties, fracture model"    , m_selectedFractureInputProperties());

    m_selectionSummary = html;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticsCase::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute)
{
    if (&m_selectionSummary == field)
    {
        caf::PdmUiTextEditorAttribute* textEditAttrib = dynamic_cast<caf::PdmUiTextEditorAttribute*> (attribute);
        textEditAttrib->textMode = caf::PdmUiTextEditorAttribute::HTML;
    }

    if (&m_calculateEditCommand == field)
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*> (attribute);
        attrib->m_buttonText = hasComputedStatistics() ? "Edit (Will DELETE current results)": "Compute";
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticsCase::updateSelectionListVisibilities()
{
    bool isLocked = hasComputedStatistics();
    m_resultType.setUiHidden(isLocked);
    m_porosityModel.setUiHidden(isLocked ); // || !caseGroup()->mainCase()->reservoirData()->results(RifReaderInterface::FRACTURE_RESULTS)->resultCount()

    m_selectedDynamicProperties.setUiHidden(           isLocked || !(m_porosityModel() == RimDefines::MATRIX_MODEL && m_resultType() == RimDefines::DYNAMIC_NATIVE));
    m_selectedStaticProperties.setUiHidden(            isLocked || !(m_porosityModel() == RimDefines::MATRIX_MODEL && m_resultType() == RimDefines::STATIC_NATIVE));
    m_selectedGeneratedProperties.setUiHidden(         isLocked || !(m_porosityModel() == RimDefines::MATRIX_MODEL && m_resultType() == RimDefines::GENERATED));
    m_selectedInputProperties.setUiHidden(             isLocked || !(m_porosityModel() == RimDefines::MATRIX_MODEL && m_resultType() == RimDefines::INPUT_PROPERTY));

    m_selectedFractureDynamicProperties.setUiHidden(   isLocked || !(m_porosityModel() == RimDefines::FRACTURE_MODEL && m_resultType() == RimDefines::DYNAMIC_NATIVE));
    m_selectedFractureStaticProperties.setUiHidden(    isLocked || !(m_porosityModel() == RimDefines::FRACTURE_MODEL && m_resultType() == RimDefines::STATIC_NATIVE));
    m_selectedFractureGeneratedProperties.setUiHidden( isLocked || !(m_porosityModel() == RimDefines::FRACTURE_MODEL && m_resultType() == RimDefines::GENERATED));
    m_selectedFractureInputProperties.setUiHidden(     isLocked || !(m_porosityModel() == RimDefines::FRACTURE_MODEL && m_resultType() == RimDefines::INPUT_PROPERTY));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticsCase::updatePercentileUiVisibility()
{
    bool isLocked = hasComputedStatistics();
    m_calculatePercentiles.setUiHidden(isLocked);
    m_percentileCalculationType.setUiHidden( isLocked || !m_calculatePercentiles());
    m_lowPercentile .setUiHidden(isLocked || !m_calculatePercentiles());
    m_midPercentile .setUiHidden(isLocked || !m_calculatePercentiles());
    m_highPercentile.setUiHidden(isLocked || !m_calculatePercentiles());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimStatisticsCase::hasComputedStatistics() const
{
   if ( reservoirData() 
       && (    reservoirData()->results(RifReaderInterface::MATRIX_RESULTS)->resultCount()
            || reservoirData()->results(RifReaderInterface::FRACTURE_RESULTS)->resultCount()))
   {
       return true;
   }
   else
   {
       return false;
   }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticsCase::clearComputedStatistics()
{
    reservoirData()->results(RifReaderInterface::MATRIX_RESULTS)->clearAllResults();
    reservoirData()->results(RifReaderInterface::FRACTURE_RESULTS)->clearAllResults();

    this->updateConnectedEditors();
}
