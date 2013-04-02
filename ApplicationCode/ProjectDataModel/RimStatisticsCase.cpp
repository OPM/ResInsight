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

namespace caf {
    template<>
    void caf::AppEnum<RimStatisticsCase::PercentileCalcType>::setUp()
    {
        addItem(RimStatisticsCase::EXACT,           "ExactPercentile",         "Exact");
        addItem(RimStatisticsCase::HISTOGRAM_ESTIMATED,           "HistogramEstimatedPercentile",         "Estimated (Faster for large case counts)");
        setDefault(RimStatisticsCase::EXACT);
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
    CAF_PDM_InitFieldNoDefault(&m_resultType, "ResultType", "Result Type", "", "", "");
    m_resultType.setIOWritable(false);
    CAF_PDM_InitFieldNoDefault(&m_porosityModel, "PorosityModel", "Porosity Model", "", "", "");
    m_porosityModel.setIOWritable(false);

    CAF_PDM_InitFieldNoDefault(&m_selectedDynamicProperties, "DynamicPropertiesToCalculate", "", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_selectedStaticProperties, "StaticPropertiesToCalculate", "", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_selectedGeneratedProperties, "GeneratedPropertiesToCalculate", "", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_selectedInputProperties, "InputPropertiesToCalculate", "", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_selectedFractureDynamicProperties, "FractureDynamicPropertiesToCalculate", "", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_selectedFractureStaticProperties, "FractureStaticPropertiesToCalculate", "", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_selectedFractureGeneratedProperties, "FractureGeneratedPropertiesToCalculate", "", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_selectedFractureInputProperties, "FractureInputPropertiesToCalculate", "", "", "", "");



    CAF_PDM_InitField(&m_calculatePercentiles, "CalculatePercentiles", true, "Calculate Percentiles", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_percentileCalculationType, "PercentileCalculationType", "Method", "", "", "");

    CAF_PDM_InitField(&m_lowPercentile, "LowPercentile", 10.0, "Low", "", "", "");
    CAF_PDM_InitField(&m_midPercentile, "MidPercentile", 50.0, "Mid", "", "", "");
    CAF_PDM_InitField(&m_highPercentile, "HighPercentile", 90.0, "High", "", "", "");


    updateSelectionListVisibilities();
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

    std::vector<size_t> timeStepIndices;
    for (size_t i = 0; i < timeStepCount; i++)
    {
        timeStepIndices.push_back(i);
    }

    RigCaseData* resultCase = reservoirData();

    QList<QPair<RimDefines::ResultCatType, QString> > resultSpecification;

    //resultSpecification.append(qMakePair(RimDefines::DYNAMIC_NATIVE, QString("PRESSURE")));

    
    {
        QStringList resultNames = sourceCases.at(0)->results(RifReaderInterface::MATRIX_RESULTS)->cellResults()->resultNames(RimDefines::DYNAMIC_NATIVE);
        foreach(QString resultName, resultNames)
        {
            resultSpecification.append(qMakePair(RimDefines::DYNAMIC_NATIVE, resultName));
        }
    }

    {
        QStringList resultNames = sourceCases.at(0)->results(RifReaderInterface::MATRIX_RESULTS)->cellResults()->resultNames(RimDefines::STATIC_NATIVE);
        foreach(QString resultName, resultNames)
        {
            resultSpecification.append(qMakePair(RimDefines::STATIC_NATIVE, resultName));
        }
    }
    

    RimStatisticsCaseEvaluator stat(sourceCases, timeStepIndices, statisticsConfig, resultCase);
    stat.evaluateForResults(resultSpecification);

    for (size_t i = 0; i < reservoirViews().size(); i++)
    {
        RimReservoirView* reservoirView = reservoirViews()[i];
        CVF_ASSERT(reservoirView);

        reservoirView->scheduleGeometryRegen(RivReservoirViewPartMgr::ACTIVE);
        reservoirView->createDisplayModelAndRedraw();
    }

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
void RimStatisticsCase::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) const
{

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
        updateSelectionListVisibilities();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStatisticsCase::updateSelectionListVisibilities()
{
    m_selectedDynamicProperties.setUiHidden(            !(m_porosityModel() == RimDefines::MATRIX_MODEL && m_resultType() == RimDefines::DYNAMIC_NATIVE));
    m_selectedStaticProperties.setUiHidden(             !(m_porosityModel() == RimDefines::MATRIX_MODEL && m_resultType() == RimDefines::STATIC_NATIVE));
    m_selectedGeneratedProperties.setUiHidden(          !(m_porosityModel() == RimDefines::MATRIX_MODEL && m_resultType() == RimDefines::GENERATED));
    m_selectedInputProperties.setUiHidden(              !(m_porosityModel() == RimDefines::MATRIX_MODEL && m_resultType() == RimDefines::INPUT_PROPERTY));

    m_selectedFractureDynamicProperties.setUiHidden(    !(m_porosityModel() == RimDefines::FRACTURE_MODEL && m_resultType() == RimDefines::DYNAMIC_NATIVE));
    m_selectedFractureStaticProperties.setUiHidden(     !(m_porosityModel() == RimDefines::FRACTURE_MODEL && m_resultType() == RimDefines::STATIC_NATIVE));
    m_selectedFractureGeneratedProperties.setUiHidden(  !(m_porosityModel() == RimDefines::FRACTURE_MODEL && m_resultType() == RimDefines::GENERATED));
    m_selectedFractureInputProperties.setUiHidden(      !(m_porosityModel() == RimDefines::FRACTURE_MODEL && m_resultType() == RimDefines::INPUT_PROPERTY));
}
