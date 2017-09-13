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

#include "Rim3dOverlayInfoConfig.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseNativeVisibleCellsStatCalc.h"
#include "RigFemNativeVisibleCellsStatCalc.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigGeoMechCaseData.h"
#include "RigMainGrid.h"
#include "RigStatisticsDataCache.h"

#include "RimCellEdgeColors.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseFaultColors.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimEclipseWellCollection.h"
#include "RimFaultCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimView.h"

#include "RiuViewer.h"
#include "RigFlowDiagResults.h"
#include "RigFlowDiagVisibleCellsStatCalc.h"

CAF_PDM_SOURCE_INIT(Rim3dOverlayInfoConfig, "View3dOverlayInfoConfig");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

namespace caf
{
template<>
void caf::AppEnum<Rim3dOverlayInfoConfig::StatisticsTimeRangeType>::setUp()
{
    addItem(Rim3dOverlayInfoConfig::ALL_TIMESTEPS, "ALL_TIMESTEPS", "All Time Steps");
    addItem(Rim3dOverlayInfoConfig::CURRENT_TIMESTEP, "CURRENT_TIMESTEP", "Current Time Step");
    setDefault(Rim3dOverlayInfoConfig::ALL_TIMESTEPS);
}
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

namespace caf
{
template<>
void caf::AppEnum<Rim3dOverlayInfoConfig::StatisticsCellRangeType>::setUp()
{
    addItem(Rim3dOverlayInfoConfig::ALL_CELLS, "ALL_CELLS", "All Active Cells");
    addItem(Rim3dOverlayInfoConfig::VISIBLE_CELLS, "VISIBLE_CELLS", "Visible Cells");
    setDefault(Rim3dOverlayInfoConfig::ALL_CELLS);
}
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim3dOverlayInfoConfig::Rim3dOverlayInfoConfig() 
{
    CAF_PDM_InitObject("Info Box", ":/InfoBox16x16.png", "", "");

    CAF_PDM_InitField(&active,              "Active",               true,   "Active",   "", "", "");
    active.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&showAnimProgress,    "ShowAnimProgress",     true,   "Animation progress",   "", "", "");
    CAF_PDM_InitField(&showCaseInfo,        "ShowInfoText",         true,   "Case Info",   "", "", "");
    CAF_PDM_InitField(&showResultInfo,      "ShowResultInfo",       true,   "Result Info",   "", "", "");
    CAF_PDM_InitField(&showHistogram,       "ShowHistogram",        true,   "Histogram",   "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_statisticsTimeRange, "StatisticsTimeRange", "Statistics Time Range", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_statisticsCellRange, "StatisticsCellRange", "Statistics Cell Range", "", "", "");
    //m_statisticsCellRange.uiCapability()->setUiHidden(true);

    m_isVisCellStatUpToDate = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim3dOverlayInfoConfig::~Rim3dOverlayInfoConfig()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (m_viewDef->propertyFilterCollection() && m_viewDef->propertyFilterCollection()->hasActiveDynamicFilters() &&
        m_statisticsCellRange() == VISIBLE_CELLS && m_statisticsTimeRange() == ALL_TIMESTEPS)
    {
        displayPropertyFilteredStatisticsMessage(false);
        if (changedField == &m_statisticsTimeRange) m_statisticsTimeRange = CURRENT_TIMESTEP;
        if (changedField == &m_statisticsCellRange) m_statisticsCellRange = ALL_CELLS;
    }


    this->update3DInfo();

    if (m_viewDef && m_viewDef->viewer())
    {
        m_viewDef->viewer()->update();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::setPosition(cvf::Vec2ui position)
{
    m_position = position;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::update3DInfo()
{
    this->updateUiIconFromToggleField();

    if (!m_viewDef) return;
    if (!m_viewDef->viewer()) return;
    
    if (!this->active())
    {
        m_viewDef->viewer()->showInfoText(false);
        m_viewDef->viewer()->showHistogram(false);
        m_viewDef->viewer()->showAnimationProgress(false);
        
        return;
    }

    m_viewDef->viewer()->showInfoText(showCaseInfo() || showResultInfo());
    m_viewDef->viewer()->showHistogram(false);
    m_viewDef->viewer()->showAnimationProgress(showAnimProgress());
    
    m_isVisCellStatUpToDate = false;

    if (m_viewDef->propertyFilterCollection() && m_viewDef->propertyFilterCollection()->hasActiveDynamicFilters() &&
        m_statisticsCellRange() == VISIBLE_CELLS && m_statisticsTimeRange() == ALL_TIMESTEPS)
    {
        displayPropertyFilteredStatisticsMessage(true);
        m_statisticsTimeRange = CURRENT_TIMESTEP;
    }

    RimEclipseView * reservoirView = dynamic_cast<RimEclipseView*>(m_viewDef.p());
    if (reservoirView) updateEclipse3DInfo(reservoirView);
    RimGeoMechView * geoMechView = dynamic_cast<RimGeoMechView*>(m_viewDef.p());
    if (geoMechView) updateGeoMech3DInfo(geoMechView);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* Rim3dOverlayInfoConfig::objectToggleField()
{
    return &active;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* visGroup = uiOrdering.addNewGroup("Visibility");

    visGroup->add(&showAnimProgress);
    visGroup->add(&showCaseInfo);
    visGroup->add(&showResultInfo);
    visGroup->add(&showHistogram);

    caf::PdmUiGroup* statGroup = uiOrdering.addNewGroup("Statistics Options");
    RimEclipseView * eclipseView = dynamic_cast<RimEclipseView*>(m_viewDef.p());

    if ( !eclipseView || eclipseView->cellResult()->resultType() != RiaDefines::FLOW_DIAGNOSTICS ) // 
    {
        statGroup->add(&m_statisticsTimeRange);
    }
    statGroup->add(&m_statisticsCellRange);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::setReservoirView(RimView* ownerReservoirView)
{
    m_viewDef = ownerReservoirView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::updateEclipse3DInfo(RimEclipseView * eclipseView)
{
    double min = HUGE_VAL, max = HUGE_VAL;
    double p10 = HUGE_VAL, p90 = HUGE_VAL;
    double mean = HUGE_VAL;
    double sum = 0.0;
    const std::vector<size_t>* histogram = NULL;

    bool isResultsInfoRelevant  = eclipseView->hasUserRequestedAnimation() && eclipseView->cellResult()->hasResult();

    if (showHistogram() || showResultInfo())
    {
        if (isResultsInfoRelevant)
        {
            size_t scalarIndex = eclipseView->cellResult()->scalarResultIndex();
            if (scalarIndex != cvf::UNDEFINED_SIZE_T)
            {

            if (m_statisticsCellRange == ALL_CELLS)
            {
                if (m_statisticsTimeRange == ALL_TIMESTEPS)
                {
                    eclipseView->currentGridCellResults()->minMaxCellScalarValues(scalarIndex, min, max);
                    eclipseView->currentGridCellResults()->p10p90CellScalarValues(scalarIndex, p10, p90);
                    eclipseView->currentGridCellResults()->meanCellScalarValues(scalarIndex, mean);
                    eclipseView->currentGridCellResults()->sumCellScalarValues(scalarIndex, sum);
                    histogram = &(eclipseView->currentGridCellResults()->cellScalarValuesHistogram(scalarIndex));
                }
                else if (m_statisticsTimeRange == CURRENT_TIMESTEP )
                {
                    int currentTimeStep = eclipseView->currentTimeStep();
                    if (eclipseView->cellResult()->hasStaticResult())
                    {
                        currentTimeStep = 0;
                    }

                    eclipseView->currentGridCellResults()->minMaxCellScalarValues(scalarIndex, currentTimeStep, min, max);
                    eclipseView->currentGridCellResults()->p10p90CellScalarValues(scalarIndex, currentTimeStep, p10, p90);
                    eclipseView->currentGridCellResults()->meanCellScalarValues(scalarIndex, currentTimeStep, mean);
                    eclipseView->currentGridCellResults()->sumCellScalarValues(scalarIndex, currentTimeStep, sum);
                    histogram = &(eclipseView->currentGridCellResults()->cellScalarValuesHistogram(scalarIndex, currentTimeStep));
                }
                else
                {
                    CVF_ASSERT(false);
                }
            }
            else if ( m_statisticsCellRange == VISIBLE_CELLS)
            {
                updateVisCellStatsIfNeeded();
                if (m_statisticsTimeRange == ALL_TIMESTEPS)
                {
                    // TODO: Only valid if we have no dynamic property filter
                    m_visibleCellStatistics->meanCellScalarValues(mean);
                    m_visibleCellStatistics->minMaxCellScalarValues(min, max);
                    m_visibleCellStatistics->p10p90CellScalarValues(p10, p90);
                    m_visibleCellStatistics->sumCellScalarValues(sum);

                    histogram = &(m_visibleCellStatistics->cellScalarValuesHistogram());
                }
                else if (m_statisticsTimeRange == CURRENT_TIMESTEP)
                {
                    int currentTimeStep = eclipseView->currentTimeStep();
                    if (eclipseView->cellResult()->hasStaticResult())
                    {
                        currentTimeStep = 0;
                    }

                    m_visibleCellStatistics->meanCellScalarValues(currentTimeStep, mean);
                    m_visibleCellStatistics->minMaxCellScalarValues(currentTimeStep, min, max);
                    m_visibleCellStatistics->p10p90CellScalarValues(currentTimeStep, p10, p90);
                    m_visibleCellStatistics->sumCellScalarValues(currentTimeStep, sum);

                    histogram = &(m_visibleCellStatistics->cellScalarValuesHistogram(currentTimeStep));
                }
            }
            }
            else if (eclipseView->cellResult()->resultType() == RiaDefines::FLOW_DIAGNOSTICS || eclipseView->cellResult->resultType() == RiaDefines::INJECTION_FLOODING)
            {
                if ( m_statisticsTimeRange == CURRENT_TIMESTEP ||  m_statisticsTimeRange == ALL_TIMESTEPS) // All timesteps is ignored
                {
                    int currentTimeStep = eclipseView->currentTimeStep();

                    if ( m_statisticsCellRange == ALL_CELLS )
                    {
                        RigFlowDiagResults* fldResults = eclipseView->cellResult()->flowDiagSolution()->flowDiagResults();
                        RigFlowDiagResultAddress resAddr = eclipseView->cellResult()->flowDiagResAddress();

                        fldResults->minMaxScalarValues(resAddr, currentTimeStep, &min, &max);
                        fldResults->p10p90ScalarValues(resAddr, currentTimeStep, &p10, &p90);
                        fldResults->meanScalarValue(resAddr, currentTimeStep, &mean);
                        fldResults->sumScalarValue(resAddr, currentTimeStep, &sum);
                        histogram = &(fldResults->scalarValuesHistogram(resAddr, currentTimeStep));
                    }
                    else if (m_statisticsCellRange == VISIBLE_CELLS )
                    {
                        updateVisCellStatsIfNeeded();

                        m_visibleCellStatistics->meanCellScalarValues(currentTimeStep, mean);
                        m_visibleCellStatistics->minMaxCellScalarValues(currentTimeStep, min, max);
                        m_visibleCellStatistics->p10p90CellScalarValues(currentTimeStep, p10, p90);
                        m_visibleCellStatistics->sumCellScalarValues(currentTimeStep, sum);

                        histogram = &(m_visibleCellStatistics->cellScalarValuesHistogram(currentTimeStep));
                    }
                }
            }
        }
    }

    QString infoText;

    if (showCaseInfo())
    {
        QString caseName;
        QString totCellCount;
        QString activeCellCountText;
        QString fractureActiveCellCount;
        QString iSize, jSize, kSize;
        QString zScale;

        if (eclipseView->mainGrid())
        {
            caseName = eclipseView->eclipseCase()->caseUserDescription();
            totCellCount = QString::number(eclipseView->mainGrid()->globalCellArray().size());
            size_t mxActCellCount = eclipseView->eclipseCase()->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL)->reservoirActiveCellCount();
            size_t frActCellCount = eclipseView->eclipseCase()->eclipseCaseData()->activeCellInfo(RiaDefines::FRACTURE_MODEL)->reservoirActiveCellCount();
            if (frActCellCount > 0)  activeCellCountText += "Matrix : ";
            activeCellCountText += QString::number(mxActCellCount);
            if (frActCellCount > 0)  activeCellCountText += " Fracture : " + QString::number(frActCellCount);

            iSize = QString::number(eclipseView->mainGrid()->cellCountI());
            jSize = QString::number(eclipseView->mainGrid()->cellCountJ());
            kSize = QString::number(eclipseView->mainGrid()->cellCountK());

            zScale = QString::number(eclipseView->scaleZ());

        }

        infoText += QString(
            "<p><b><center>-- %1 --</center></b><p>  "
            "<b>Cell count. Total:</b> %2 <b>Active:</b> %3 <br>"
            "<b>Main Grid I,J,K:</b> %4, %5, %6 <b>Z-Scale:</b> %7<br>").arg(caseName, totCellCount, activeCellCountText, iSize, jSize, kSize, zScale);
    }

    if (showResultInfo())
    {

        if (eclipseView->cellResult()->isTernarySaturationSelected())
        {
            QString propName = eclipseView->cellResult()->resultVariableUiShortName();
            infoText += QString("<b>Cell Property:</b> %1 ").arg(propName);
        }

        if (isResultsInfoRelevant)
        {
            QString propName = eclipseView->cellResult()->resultVariableUiShortName();
            QString timeRangeText =  m_statisticsTimeRange().uiText();
            if ( eclipseView->cellResult()->resultType() == RiaDefines::FLOW_DIAGNOSTICS )
            {
                timeRangeText = caf::AppEnum<StatisticsTimeRangeType>::uiText(CURRENT_TIMESTEP);
            }

            infoText += QString("<b>Cell Property:</b> %1 ").arg(propName);
            infoText += QString("<br><b>Statistics:</b> ") + timeRangeText + " and " + m_statisticsCellRange().uiText();
            infoText += QString("<table border=0 cellspacing=5 >"
                                "<tr> <td>Min</td> <td>P10</td> <td>Mean</td> <td>P90</td> <td>Max</td> <td>Sum</td> </tr>"
                                "<tr> <td>%1</td>  <td> %2</td> <td>  %3</td> <td> %4</td> <td> %5</td> <td> %6</td> </tr>"
                                "</table>").arg(min).arg(p10).arg(mean).arg(p90).arg(max).arg(sum);

            if (eclipseView->faultResultSettings()->hasValidCustomResult())
            {
                QString faultMapping;
                bool isShowingGrid = eclipseView->faultCollection()->isGridVisualizationMode();
                if (!isShowingGrid)
                {
                    if (eclipseView->faultCollection()->faultResult() == RimFaultCollection::FAULT_BACK_FACE_CULLING)
                    {
                        faultMapping = "Cells behind fault";
                    }
                    else if (eclipseView->faultCollection()->faultResult() == RimFaultCollection::FAULT_FRONT_FACE_CULLING)
                    {
                        faultMapping = "Cells in front of fault";
                    }
                    else
                    {
                        faultMapping = "Cells in front and behind fault";
                    }
                }
                else
                {
                    faultMapping = "Cells in front and behind fault";
                }

                infoText += QString("<b>Fault results: </b> %1<br>").arg(faultMapping);
                infoText += QString("<b>Fault Property:</b> %1 <br>").arg(eclipseView->faultResultSettings()->customFaultResult()->resultVariableUiShortName());
            }
        }

        if (eclipseView->hasUserRequestedAnimation() && eclipseView->cellEdgeResult()->hasResult())
        {
            double min, max;
            QString cellEdgeName = eclipseView->cellEdgeResult()->resultVariableUiShortName();
            eclipseView->cellEdgeResult()->minMaxCellEdgeValues(min, max);
            infoText += QString("<b>Cell Edge Property:</b> %1 ").arg(cellEdgeName);
            infoText += QString("<table border=0 cellspacing=5 >"
                                "<tr> <td>Min</td> <td></td> <td></td> <td></td> <td>Max</td> </tr>"
                                "<tr> <td>%1</td>  <td></td> <td></td> <td></td> <td> %2</td></tr>"
                                "</table>").arg(min).arg(max);

        }
        
    }

    if (!infoText.isEmpty())
    {
        eclipseView->viewer()->setInfoText(infoText);
    }

    if (showHistogram())
    {
        if (isResultsInfoRelevant && histogram)
        {
            eclipseView->viewer()->showHistogram(true);
            eclipseView->viewer()->setHistogram(min, max, *histogram);
            eclipseView->viewer()->setHistogramPercentiles(p10, p90, mean);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::updateGeoMech3DInfo(RimGeoMechView * geoMechView)
{
    RimGeoMechCase* geoMechCase = geoMechView->geoMechCase();
    RigGeoMechCaseData* caseData = geoMechCase ? geoMechCase->geoMechData() : NULL;
    bool isResultsInfoRelevant = caseData && geoMechView->hasUserRequestedAnimation() && geoMechView->cellResultResultDefinition()->hasResult();

    // Retreive result stats if needed

    double min = HUGE_VAL, max = HUGE_VAL;
    double p10 = HUGE_VAL, p90 = HUGE_VAL;
    double mean = HUGE_VAL;
    double sum = 0.0;
    const std::vector<size_t>* histogram = NULL;

    if (showResultInfo() || showHistogram())
    {
        if (isResultsInfoRelevant)
        {
            RigFemResultAddress resAddress = geoMechView->cellResultResultDefinition()->resultAddress();
            if (m_statisticsCellRange == ALL_CELLS)
            {
                if (m_statisticsTimeRange == ALL_TIMESTEPS)
                {
                    caseData->femPartResults()->meanScalarValue(resAddress, &mean);
                    caseData->femPartResults()->minMaxScalarValues(resAddress, &min, &max);
                    caseData->femPartResults()->p10p90ScalarValues(resAddress, &p10, &p90);
                    caseData->femPartResults()->sumScalarValue(resAddress, &sum);

                    histogram = &(caseData->femPartResults()->scalarValuesHistogram(resAddress));
                }
                else if (m_statisticsTimeRange == CURRENT_TIMESTEP)
                {
                    int timeStepIdx = geoMechView->currentTimeStep();
                    caseData->femPartResults()->meanScalarValue(resAddress, timeStepIdx, &mean);
                    caseData->femPartResults()->minMaxScalarValues(resAddress, timeStepIdx, &min, &max);
                    caseData->femPartResults()->p10p90ScalarValues(resAddress, timeStepIdx, &p10, &p90);
                    caseData->femPartResults()->sumScalarValue(resAddress, timeStepIdx, &sum);

                    histogram = &(caseData->femPartResults()->scalarValuesHistogram(resAddress, timeStepIdx));
                }
            }
            else if (m_statisticsCellRange == VISIBLE_CELLS)
            {
                this->updateVisCellStatsIfNeeded();

                if (m_statisticsTimeRange == ALL_TIMESTEPS)
                {
                    // TODO: Only valid if we have no dynamic property filter
                    m_visibleCellStatistics->meanCellScalarValues(mean);
                    m_visibleCellStatistics->minMaxCellScalarValues(min, max);
                    m_visibleCellStatistics->p10p90CellScalarValues(p10, p90);
                    m_visibleCellStatistics->sumCellScalarValues(sum);

                    histogram = &(m_visibleCellStatistics->cellScalarValuesHistogram());
                }
                else if (m_statisticsTimeRange == CURRENT_TIMESTEP)
                {
                    int timeStepIdx = geoMechView->currentTimeStep();
                    m_visibleCellStatistics->meanCellScalarValues(timeStepIdx, mean);
                    m_visibleCellStatistics->minMaxCellScalarValues(timeStepIdx, min, max);
                    m_visibleCellStatistics->p10p90CellScalarValues(timeStepIdx, p10, p90);
                    m_visibleCellStatistics->sumCellScalarValues(timeStepIdx, sum);

                    histogram = &(m_visibleCellStatistics->cellScalarValuesHistogram(timeStepIdx));
                }
            }
        }
    }

    // Compose text

    QString infoText;

    if (showCaseInfo())
    {

        RigFemPartCollection* femParts = caseData ? caseData->femParts() : NULL;

        if (femParts)
        {
            QString caseName = geoMechCase->caseUserDescription();
            QString cellCount = QString("%1").arg(femParts->totalElementCount());
            QString zScale = QString::number(geoMechView->scaleZ());

            infoText = QString(
                "<p><b><center>-- %1 --</center></b><p>"
                "<b>Cell count:</b> %2 <b>Z-Scale:</b> %3<br>").arg(caseName, cellCount, zScale);
        }
    }

    if (showResultInfo())
    {

        if (isResultsInfoRelevant)
        {
            {
                QString resultPos;
                QString fieldName = geoMechView->cellResultResultDefinition()->resultFieldUiName();
                QString compName = geoMechView->cellResultResultDefinition()->resultComponentUiName();

                switch (geoMechView->cellResultResultDefinition()->resultPositionType())
                {
                    case RIG_NODAL:
                    resultPos = "Nodal";
                    break;

                    case RIG_ELEMENT_NODAL:
                    resultPos = "Element nodal";
                    break;

                    case RIG_INTEGRATION_POINT:
                    resultPos = "Integration point";
                    break;

                    default:
                    break;
                }

                infoText += QString("<b>Cell result:</b> %1, %2, %3").arg(resultPos).arg(fieldName).arg(compName);
            }
            {
                infoText += QString("<br><b>Statistics:</b> ") + m_statisticsTimeRange().uiText() + " and " + m_statisticsCellRange().uiText();
                infoText += QString("<table border=0 cellspacing=5 >"
                                    "<tr> <td>Min</td> <td>P10</td> <td>Mean</td> <td>P90</td> <td>Max</td> <td>Sum</td> </tr>"
                                    "<tr> <td>%1</td>  <td> %2</td> <td> %3</td>  <td> %4</td> <td> %5</td> <td> %6</td> </tr>"
                                    "</table>").arg(min).arg(p10).arg(mean).arg(p90).arg(max).arg(sum);

            }
        }
    }

    if (!infoText.isEmpty())
    {
        geoMechView->viewer()->setInfoText(infoText);
    }

    // Populate histogram

    if (showHistogram())
    {
        if (isResultsInfoRelevant)
        {
            geoMechView->viewer()->showHistogram(true);
            geoMechView->viewer()->setHistogram(min, max, *histogram);
            geoMechView->viewer()->setHistogramPercentiles(p10, p90, mean);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::updateVisCellStatsIfNeeded()
{
    RimEclipseView * eclipseView = dynamic_cast<RimEclipseView*>(m_viewDef.p());
    RimGeoMechView * geoMechView = dynamic_cast<RimGeoMechView*>(m_viewDef.p());

    if (!m_isVisCellStatUpToDate)
    {
        cvf::ref<RigStatisticsCalculator> calc;
        if (geoMechView)
        {
            RigFemResultAddress resAddress = geoMechView->cellResultResultDefinition()->resultAddress();
            calc = new RigFemNativeVisibleCellsStatCalc(geoMechView->geoMechCase()->geoMechData(),
                                                        resAddress,
                                                        geoMechView->currentTotalCellVisibility().p());
          
        }
        else if (eclipseView)
        {
            if ( eclipseView->cellResult()->resultType() == RiaDefines::FLOW_DIAGNOSTICS )
            {
                RigFlowDiagResultAddress resAddr = eclipseView->cellResult()->flowDiagResAddress();
                RigFlowDiagResults* fldResults = eclipseView->cellResult()->flowDiagSolution()->flowDiagResults();
                calc = new RigFlowDiagVisibleCellsStatCalc(fldResults, 
                                                           resAddr,
                                                           eclipseView->currentTotalCellVisibility().p());

            }
            else
            {
                size_t scalarIndex = eclipseView->cellResult()->scalarResultIndex();
                calc = new RigEclipseNativeVisibleCellsStatCalc(eclipseView->currentGridCellResults(),
                                                                scalarIndex,
                                                                eclipseView->currentTotalCellVisibility().p());
            }
        }

        m_visibleCellStatistics = new RigStatisticsDataCache(calc.p());
        m_isVisCellStatUpToDate = true;
    }
}

#include <QMessageBox>

void Rim3dOverlayInfoConfig::displayPropertyFilteredStatisticsMessage(bool showSwitchToCurrentTimestep)
{
    static bool isShowing = false;

    QString switchString;
    if (showSwitchToCurrentTimestep)
    {
        switchString = QString("<br>"
                               "Switching to statistics for <b>Current Time Step</b>");
    }

    if (!isShowing)
    {
        isShowing = true;
        QMessageBox::information(m_viewDef->viewer()->layoutWidget(),
                                 QString("ResInsight"),
                                 QString("Statistics not available<br>"
                                 "<br>"
                                 "Statistics calculations of <b>Visible Cells</b> for <b>All Time Steps</b> is not supported<br>"
                                 "when you have an active Property filter on a time varying result.<br>")
                                 + switchString);
        isShowing = false;
    }
}