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

#include "RiaQDateTimeTools.h"

#include "RicGridStatisticsDialog.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseNativeVisibleCellsStatCalc.h"
#include "RigFemNativeVisibleCellsStatCalc.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigFlowDiagResults.h"
#include "RigFlowDiagVisibleCellsStatCalc.h"
#include "RigGeoMechCaseData.h"
#include "RigMainGrid.h"
#include "RigStatisticsDataCache.h"

#include "Rim2dIntersectionView.h"
#include "Rim2dIntersectionViewCollection.h"
#include "Rim3dView.h"
#include "RimCase.h"
#include "RimCellEdgeColors.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseContourMapProjection.h"
#include "RimEclipseContourMapView.h"
#include "RimEclipseFaultColors.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimFaultInViewCollection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechContourMapProjection.h"
#include "RimGeoMechContourMapView.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimSimWellInViewCollection.h"
#include "RimTools.h"

#include "RiuViewer.h"

#include <QApplication>
#include <QMessageBox>

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
} // namespace caf
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
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dOverlayInfoConfig::Rim3dOverlayInfoConfig()
{
    CAF_PDM_InitObject("Info Box", ":/InfoBox16x16.png", "", "");

    CAF_PDM_InitField(&m_active, "Active", true, "Active", "", "", "");
    m_active.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_showAnimProgress, "ShowAnimProgress", true, "Animation progress", "", "", "");
    CAF_PDM_InitField(&m_showCaseInfo, "ShowInfoText", true, "Case Info", "", "", "");
    CAF_PDM_InitField(&m_showResultInfo, "ShowResultInfo", true, "Result Info", "", "", "");
    CAF_PDM_InitField(&m_showHistogram, "ShowHistogram", true, "Histogram", "", "", "");
    CAF_PDM_InitField(&m_showVolumeWeightedMean, "ShowVolumeWeightedMean", true, "Mobile Volume Weighted Mean", "", "", "");
    CAF_PDM_InitField(&m_showVersionInfo, "ShowVersionInfo", true, "Version Info", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_statisticsTimeRange, "StatisticsTimeRange", "Statistics Time Range", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_statisticsCellRange, "StatisticsCellRange", "Statistics Cell Range", "", "", "");
    // m_statisticsCellRange.uiCapability()->setUiHidden(true);

    m_isVisCellStatUpToDate = false;

    m_gridStatisticsDialog = std::unique_ptr<RicGridStatisticsDialog>(new RicGridStatisticsDialog(nullptr));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dOverlayInfoConfig::~Rim3dOverlayInfoConfig() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                              const QVariant&            oldValue,
                                              const QVariant&            newValue)
{
    if (hasInvalidStatisticsCombination())
    {
        displayPropertyFilteredStatisticsMessage(false);
        if (changedField == &m_statisticsTimeRange) m_statisticsTimeRange = CURRENT_TIMESTEP;
        if (changedField == &m_statisticsCellRange) m_statisticsCellRange = ALL_CELLS;
    }

    if (changedField == &m_showResultInfo)
    {
        if (!m_showResultInfo())
        {
            m_showVolumeWeightedMean = false;
            m_showVolumeWeightedMean.uiCapability()->setUiReadOnly(true);
        }
        else
        {
            m_showVolumeWeightedMean = true;
            m_showVolumeWeightedMean.uiCapability()->setUiReadOnly(false);
        }
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
Rim3dOverlayInfoConfig::HistogramData Rim3dOverlayInfoConfig::histogramData()
{
    auto eclipseView       = dynamic_cast<RimEclipseView*>(m_viewDef.p());
    auto geoMechView       = dynamic_cast<RimGeoMechView*>(m_viewDef.p());
    auto eclipseContourMap = dynamic_cast<RimEclipseContourMapView*>(eclipseView);
    auto geoMechContourMap = dynamic_cast<RimGeoMechContourMapView*>(geoMechView);

    if (eclipseContourMap)
        return histogramData(eclipseContourMap);
    else if (geoMechContourMap)
        return histogramData(geoMechContourMap);
    else if (eclipseView)
        return histogramData(eclipseView);
    else if (geoMechView)
        return histogramData(geoMechView);
    return HistogramData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dOverlayInfoConfig::timeStepText()
{
    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_viewDef.p());
    RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>(m_viewDef.p());

    if (eclipseView) return timeStepText(eclipseView);
    if (geoMechView) return timeStepText(geoMechView);
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dOverlayInfoConfig::caseInfoText()
{
    auto eclipseView = dynamic_cast<RimEclipseView*>(m_viewDef.p());
    auto geoMechView = dynamic_cast<RimGeoMechView*>(m_viewDef.p());

    if (eclipseView) return caseInfoText(eclipseView);
    if (geoMechView) return caseInfoText(geoMechView);
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dOverlayInfoConfig::resultInfoText(const HistogramData& histData)
{
    auto eclipseView = dynamic_cast<RimEclipseView*>(m_viewDef.p());
    auto geoMechView = dynamic_cast<RimGeoMechView*>(m_viewDef.p());

    if (eclipseView) return resultInfoText(histData, eclipseView, m_showVolumeWeightedMean());
    if (geoMechView) return resultInfoText(histData, geoMechView);
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage Rim3dOverlayInfoConfig::statisticsDialogScreenShotImage()
{
    if (m_gridStatisticsDialog->isVisible())
    {
        return m_gridStatisticsDialog->screenShotImage();
    }
    return QImage();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dOverlayInfoConfig::showAnimProgress() const
{
    return m_showAnimProgress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dOverlayInfoConfig::showCaseInfo() const
{
    return m_showCaseInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dOverlayInfoConfig::showResultInfo() const
{
    return m_showResultInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dOverlayInfoConfig::isActive() const
{
    return m_active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::setIsActive(bool active)
{
    m_active = active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dOverlayInfoConfig::showVersionInfo() const
{
    return m_showVersionInfo();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dOverlayInfoConfig::HistogramData Rim3dOverlayInfoConfig::histogramData(RimEclipseContourMapView* contourMap)
{
    HistogramData histData;

    if (contourMap)
    {
        bool isResultsInfoRelevant = contourMap->contourMapProjection()->numberOfValidCells() > 0u;

        if (isResultsInfoRelevant)
        {
            histData.min  = contourMap->contourMapProjection()->minValue();
            histData.max  = contourMap->contourMapProjection()->maxValue();
            histData.mean = contourMap->contourMapProjection()->meanValue();
            histData.sum  = contourMap->contourMapProjection()->sumAllValues();
        }
    }
    return histData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dOverlayInfoConfig::HistogramData Rim3dOverlayInfoConfig::histogramData(RimGeoMechContourMapView* contourMap)
{
    HistogramData histData;

    if (contourMap)
    {
        bool isResultsInfoRelevant = contourMap->contourMapProjection()->numberOfValidCells() > 0u;

        if (isResultsInfoRelevant)
        {
            histData.min  = contourMap->contourMapProjection()->minValue();
            histData.max  = contourMap->contourMapProjection()->maxValue();
            histData.mean = contourMap->contourMapProjection()->meanValue();
            histData.sum  = contourMap->contourMapProjection()->sumAllValues();
        }
    }
    return histData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dOverlayInfoConfig::HistogramData Rim3dOverlayInfoConfig::histogramData(RimEclipseView* eclipseView)
{
    HistogramData histData;

    if (eclipseView)
    {
        bool isResultsInfoRelevant = eclipseView->hasUserRequestedAnimation() && eclipseView->cellResult()->hasResult();

        if (isResultsInfoRelevant)
        {
            RigEclipseResultAddress eclResAddr = eclipseView->cellResult()->eclipseResultAddress();

            if (eclResAddr.isValid())
            {
                if (m_statisticsCellRange == ALL_CELLS)
                {
                    if (m_statisticsTimeRange == ALL_TIMESTEPS)
                    {
                        eclipseView->currentGridCellResults()->minMaxCellScalarValues(eclResAddr, histData.min, histData.max);
                        eclipseView->currentGridCellResults()->p10p90CellScalarValues(eclResAddr, histData.p10, histData.p90);
                        eclipseView->currentGridCellResults()->meanCellScalarValues(eclResAddr, histData.mean);
                        eclipseView->currentGridCellResults()->sumCellScalarValues(eclResAddr, histData.sum);
                        eclipseView->currentGridCellResults()->mobileVolumeWeightedMean(eclResAddr, histData.weightedMean);
                        histData.histogram = &(eclipseView->currentGridCellResults()->cellScalarValuesHistogram(eclResAddr));
                    }
                    else if (m_statisticsTimeRange == CURRENT_TIMESTEP)
                    {
                        int currentTimeStep = eclipseView->currentTimeStep();
                        if (eclipseView->cellResult()->hasStaticResult())
                        {
                            currentTimeStep = 0;
                        }

                        eclipseView->currentGridCellResults()->minMaxCellScalarValues(
                            eclResAddr, currentTimeStep, histData.min, histData.max);
                        eclipseView->currentGridCellResults()->p10p90CellScalarValues(
                            eclResAddr, currentTimeStep, histData.p10, histData.p90);
                        eclipseView->currentGridCellResults()->meanCellScalarValues(eclResAddr, currentTimeStep, histData.mean);
                        eclipseView->currentGridCellResults()->sumCellScalarValues(eclResAddr, currentTimeStep, histData.sum);
                        eclipseView->currentGridCellResults()->mobileVolumeWeightedMean(
                            eclResAddr, currentTimeStep, histData.weightedMean);

                        histData.histogram =
                            &(eclipseView->currentGridCellResults()->cellScalarValuesHistogram(eclResAddr, currentTimeStep));
                    }
                    else
                    {
                        CVF_ASSERT(false);
                    }
                }
                else if (m_statisticsCellRange == VISIBLE_CELLS)
                {
                    updateVisCellStatsIfNeeded();
                    if (m_statisticsTimeRange == ALL_TIMESTEPS)
                    {
                        // TODO: Only valid if we have no dynamic property filter
                        m_visibleCellStatistics->meanCellScalarValues(histData.mean);
                        m_visibleCellStatistics->minMaxCellScalarValues(histData.min, histData.max);
                        m_visibleCellStatistics->p10p90CellScalarValues(histData.p10, histData.p90);
                        m_visibleCellStatistics->sumCellScalarValues(histData.sum);
                        m_visibleCellStatistics->mobileVolumeWeightedMean(histData.weightedMean);

                        histData.histogram = &(m_visibleCellStatistics->cellScalarValuesHistogram());
                    }
                    else if (m_statisticsTimeRange == CURRENT_TIMESTEP)
                    {
                        int currentTimeStep = eclipseView->currentTimeStep();
                        if (eclipseView->cellResult()->hasStaticResult())
                        {
                            currentTimeStep = 0;
                        }

                        m_visibleCellStatistics->meanCellScalarValues(currentTimeStep, histData.mean);
                        m_visibleCellStatistics->minMaxCellScalarValues(currentTimeStep, histData.min, histData.max);
                        m_visibleCellStatistics->p10p90CellScalarValues(currentTimeStep, histData.p10, histData.p90);
                        m_visibleCellStatistics->sumCellScalarValues(currentTimeStep, histData.sum);
                        m_visibleCellStatistics->mobileVolumeWeightedMean(currentTimeStep, histData.weightedMean);

                        histData.histogram = &(m_visibleCellStatistics->cellScalarValuesHistogram(currentTimeStep));
                    }
                }
            }
            else if (eclipseView->cellResult()->isFlowDiagOrInjectionFlooding())
            {
                if (m_statisticsTimeRange == CURRENT_TIMESTEP ||
                    m_statisticsTimeRange == ALL_TIMESTEPS) // All timesteps is ignored
                {
                    int currentTimeStep = eclipseView->currentTimeStep();

                    if (m_statisticsCellRange == ALL_CELLS)
                    {
                        RigFlowDiagResults*      fldResults = eclipseView->cellResult()->flowDiagSolution()->flowDiagResults();
                        RigFlowDiagResultAddress resAddr    = eclipseView->cellResult()->flowDiagResAddress();

                        fldResults->minMaxScalarValues(resAddr, currentTimeStep, &histData.min, &histData.max);
                        fldResults->p10p90ScalarValues(resAddr, currentTimeStep, &histData.p10, &histData.p90);
                        fldResults->meanScalarValue(resAddr, currentTimeStep, &histData.mean);
                        fldResults->sumScalarValue(resAddr, currentTimeStep, &histData.sum);
                        fldResults->mobileVolumeWeightedMean(resAddr, currentTimeStep, &histData.weightedMean);

                        histData.histogram = &(fldResults->scalarValuesHistogram(resAddr, currentTimeStep));
                    }
                    else if (m_statisticsCellRange == VISIBLE_CELLS)
                    {
                        updateVisCellStatsIfNeeded();

                        m_visibleCellStatistics->meanCellScalarValues(currentTimeStep, histData.mean);
                        m_visibleCellStatistics->minMaxCellScalarValues(currentTimeStep, histData.min, histData.max);
                        m_visibleCellStatistics->p10p90CellScalarValues(currentTimeStep, histData.p10, histData.p90);
                        m_visibleCellStatistics->sumCellScalarValues(currentTimeStep, histData.sum);
                        m_visibleCellStatistics->mobileVolumeWeightedMean(currentTimeStep, histData.weightedMean);

                        histData.histogram = &(m_visibleCellStatistics->cellScalarValuesHistogram(currentTimeStep));
                    }
                }
            }
        }
    }
    return histData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dOverlayInfoConfig::HistogramData Rim3dOverlayInfoConfig::histogramData(RimGeoMechView* geoMechView)
{
    HistogramData histData;

    if (geoMechView)
    {
        RimGeoMechCase*     geoMechCase = geoMechView->geoMechCase();
        RigGeoMechCaseData* caseData    = geoMechCase ? geoMechCase->geoMechData() : nullptr;
        bool                isResultsInfoRelevant =
            caseData && geoMechView->hasUserRequestedAnimation() && geoMechView->cellResultResultDefinition()->hasResult();

        if (isResultsInfoRelevant)
        {
            RigFemResultAddress resAddress = geoMechView->cellResultResultDefinition()->resultAddress();
            if (m_statisticsCellRange == ALL_CELLS)
            {
                if (m_statisticsTimeRange == ALL_TIMESTEPS)
                {
                    caseData->femPartResults()->meanScalarValue(resAddress, &histData.mean);
                    caseData->femPartResults()->minMaxScalarValues(resAddress, &histData.min, &histData.max);
                    caseData->femPartResults()->p10p90ScalarValues(resAddress, &histData.p10, &histData.p90);
                    caseData->femPartResults()->sumScalarValue(resAddress, &histData.sum);

                    histData.histogram = &(caseData->femPartResults()->scalarValuesHistogram(resAddress));
                }
                else if (m_statisticsTimeRange == CURRENT_TIMESTEP)
                {
                    int timeStepIdx = geoMechView->currentTimeStep();
                    caseData->femPartResults()->meanScalarValue(resAddress, timeStepIdx, &histData.mean);
                    caseData->femPartResults()->minMaxScalarValues(resAddress, timeStepIdx, &histData.min, &histData.max);
                    caseData->femPartResults()->p10p90ScalarValues(resAddress, timeStepIdx, &histData.p10, &histData.p90);
                    caseData->femPartResults()->sumScalarValue(resAddress, timeStepIdx, &histData.sum);

                    histData.histogram = &(caseData->femPartResults()->scalarValuesHistogram(resAddress, timeStepIdx));
                }
            }
            else if (m_statisticsCellRange == VISIBLE_CELLS)
            {
                this->updateVisCellStatsIfNeeded();

                if (m_statisticsTimeRange == ALL_TIMESTEPS)
                {
                    // TODO: Only valid if we have no dynamic property filter
                    m_visibleCellStatistics->meanCellScalarValues(histData.mean);
                    m_visibleCellStatistics->minMaxCellScalarValues(histData.min, histData.max);
                    m_visibleCellStatistics->p10p90CellScalarValues(histData.p10, histData.p90);
                    m_visibleCellStatistics->sumCellScalarValues(histData.sum);

                    histData.histogram = &(m_visibleCellStatistics->cellScalarValuesHistogram());
                }
                else if (m_statisticsTimeRange == CURRENT_TIMESTEP)
                {
                    int timeStepIdx = geoMechView->currentTimeStep();
                    m_visibleCellStatistics->meanCellScalarValues(timeStepIdx, histData.mean);
                    m_visibleCellStatistics->minMaxCellScalarValues(timeStepIdx, histData.min, histData.max);
                    m_visibleCellStatistics->p10p90CellScalarValues(timeStepIdx, histData.p10, histData.p90);
                    m_visibleCellStatistics->sumCellScalarValues(timeStepIdx, histData.sum);

                    histData.histogram = &(m_visibleCellStatistics->cellScalarValuesHistogram(timeStepIdx));
                }
            }
        }
    }
    return histData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dOverlayInfoConfig::caseInfoText(RimEclipseView* eclipseView)
{
    QString infoText;

    if (eclipseView)
    {
        QString caseName = eclipseView->eclipseCase()->caseUserDescription();

        RimEclipseContourMapView* contourMap = dynamic_cast<RimEclipseContourMapView*>(eclipseView);
        if (contourMap && contourMap->contourMapProjection())
        {
            QString   totCellCount        = QString::number(contourMap->contourMapProjection()->numberOfCells());
            cvf::uint validCellCount      = contourMap->contourMapProjection()->numberOfValidCells();
            QString   activeCellCountText = QString::number(validCellCount);
            QString   iSize               = QString::number(contourMap->contourMapProjection()->numberOfElementsIJ().x());
            QString   jSize               = QString::number(contourMap->contourMapProjection()->numberOfElementsIJ().y());
            QString   aggregationType     = contourMap->contourMapProjection()->resultAggregationText();
            QString   weightingParameterString;
            if (contourMap->contourMapProjection()->weightingParameter() != "None")
            {
                weightingParameterString +=
                    QString(" (Weight: %1)").arg(contourMap->contourMapProjection()->weightingParameter());
            }

            infoText += QString("<p><b>-- Contour Map: %1 --</b><p>  "
                                "<b>Sample Count. Total:</b> %2 <b>Valid Results:</b> %3 <br>"
                                "<b>Projection Type:</b> %4%5<br>")
                            .arg(caseName, totCellCount, activeCellCountText, aggregationType, weightingParameterString);
        }
        else if (eclipseView->mainGrid())
        {
            QString totCellCount   = QString::number(eclipseView->mainGrid()->globalCellArray().size());
            size_t  mxActCellCount = eclipseView->eclipseCase()
                                        ->eclipseCaseData()
                                        ->activeCellInfo(RiaDefines::MATRIX_MODEL)
                                        ->reservoirActiveCellCount();
            size_t frActCellCount = eclipseView->eclipseCase()
                                        ->eclipseCaseData()
                                        ->activeCellInfo(RiaDefines::FRACTURE_MODEL)
                                        ->reservoirActiveCellCount();

            QString activeCellCountText;
            if (frActCellCount > 0) activeCellCountText += "Matrix : ";
            activeCellCountText += QString::number(mxActCellCount);
            if (frActCellCount > 0) activeCellCountText += " Fracture : " + QString::number(frActCellCount);

            QString iSize = QString::number(eclipseView->mainGrid()->cellCountI());
            QString jSize = QString::number(eclipseView->mainGrid()->cellCountJ());
            QString kSize = QString::number(eclipseView->mainGrid()->cellCountK());

            QString zScale = QString::number(eclipseView->scaleZ());
            infoText += QString("<p><b>-- %1 --</b><p>  "
                                "<b>Cell count. Total:</b> %2 <b>Active:</b> %3 <br>"
                                "<b>Main Grid I,J,K:</b> %4, %5, %6 <b>Z-Scale:</b> %7<br>")
                            .arg(caseName, totCellCount, activeCellCountText, iSize, jSize, kSize, zScale);
        }
    }

    return infoText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dOverlayInfoConfig::caseInfoText(RimGeoMechView* geoMechView)
{
    QString infoText;

    if (geoMechView)
    {
        RimGeoMechCase*       geoMechCase = geoMechView->geoMechCase();
        RigGeoMechCaseData*   caseData    = geoMechCase ? geoMechCase->geoMechData() : nullptr;
        RigFemPartCollection* femParts    = caseData ? caseData->femParts() : nullptr;

        if (femParts)
        {
            QString                   caseName   = geoMechCase->caseUserDescription();
            RimGeoMechContourMapView* contourMap = dynamic_cast<RimGeoMechContourMapView*>(geoMechView);

            if (contourMap && contourMap->contourMapProjection())
            {
                QString   totCellCount        = QString::number(contourMap->contourMapProjection()->numberOfCells());
                cvf::uint validCellCount      = contourMap->contourMapProjection()->numberOfValidCells();
                QString   activeCellCountText = QString::number(validCellCount);
                QString   iSize               = QString::number(contourMap->contourMapProjection()->numberOfElementsIJ().x());
                QString   jSize               = QString::number(contourMap->contourMapProjection()->numberOfElementsIJ().y());
                QString   aggregationType     = contourMap->contourMapProjection()->resultAggregationText();

                infoText += QString("<p><b>-- Contour Map: %1 --</b><p>  "
                                    "<b>Sample Count. Total:</b> %2 <b>Valid Results:</b> %3 <br>"
                                    "<b>Projection Type:</b> %4<br>")
                                .arg(caseName, totCellCount, activeCellCountText, aggregationType);
            }
            else
            {
                QString cellCount = QString("%1").arg(femParts->totalElementCount());
                QString zScale    = QString::number(geoMechView->scaleZ());

                infoText = QString("<p><b>-- %1 --</b><p>"
                                   "<b>Cell count:</b> %2 <b>Z-Scale:</b> %3<br>")
                               .arg(caseName, cellCount, zScale);
            }
        }
    }
    return infoText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dOverlayInfoConfig::resultInfoText(const HistogramData& histData,
                                               RimEclipseView*      eclipseView,
                                               bool                 showVolumeWeightedMean)
{
    QString infoText;

    RimEclipseContourMapView* contourMap = dynamic_cast<RimEclipseContourMapView*>(eclipseView);

    if (contourMap)
    {
        bool isResultsInfoRelevant = contourMap->contourMapProjection()->numberOfValidCells() > 0u;
        if (isResultsInfoRelevant)
        {
            QString propName      = eclipseView->cellResult()->resultVariableUiShortName();
            QString diffResString = eclipseView->cellResult()->diffResultUiName();
            if (!contourMap->contourMapProjection()->isColumnResult())
            {
                infoText += QString("<b>Cell Property:</b> %1<br>").arg(propName);
            }
            if (!diffResString.isEmpty())
            {
                infoText += QString("%1<br>").arg(diffResString);
            }
            infoText += QString("<br><b>Statistics:</b> Current Time Step and Visible Cells");
            infoText += QString("<table border=0 cellspacing=5 >"
                                "<tr> <td>Min</td> <td>Mean</td> <td>Max</td> <td>Sum</td> </tr>"
                                "<tr> <td>%1</td>  <td> %2</td> <td>  %3</td> <td> %4</td> </tr>"
                                "</table>")
                            .arg(histData.min)
                            .arg(histData.mean)
                            .arg(histData.max)
                            .arg(histData.sum);
        }
    }
    else if (eclipseView)
    {
        bool isResultsInfoRelevant = eclipseView->hasUserRequestedAnimation() && eclipseView->cellResult()->hasResult();

        if (eclipseView->cellResult()->isTernarySaturationSelected())
        {
            QString propName = eclipseView->cellResult()->resultVariableUiShortName();
            infoText += QString("<b>Cell Property:</b> %1 ").arg(propName);
        }

        if (isResultsInfoRelevant)
        {
            QString propName      = eclipseView->cellResult()->resultVariableUiShortName();
            QString diffResString = eclipseView->cellResult()->diffResultUiName();
            QString timeRangeText = m_statisticsTimeRange().uiText();
            if (eclipseView->cellResult()->isFlowDiagOrInjectionFlooding())
            {
                timeRangeText = caf::AppEnum<StatisticsTimeRangeType>::uiText(CURRENT_TIMESTEP);
            }

            infoText += QString("<b>Cell Property:</b> %1<br>").arg(propName);
            if (!diffResString.isEmpty())
            {
                infoText += QString("%1<br>").arg(diffResString);
            }
            infoText += QString("<br><b>Statistics:</b> ") + timeRangeText + " and " + m_statisticsCellRange().uiText();
            infoText += QString("<table border=0 cellspacing=5 >"
                                "<tr> <td>Min</td> <td>P90</td> <td>Mean</td> <td>P10</td> <td>Max</td> <td>Sum</td> </tr>"
                                "<tr> <td>%1</td>  <td> %2</td> <td>  %3</td> <td> %4</td> <td> %5</td> <td> %6</td> </tr>"
                                "</table>")
                            .arg(histData.min)
                            .arg(histData.p10)
                            .arg(histData.mean)
                            .arg(histData.p90)
                            .arg(histData.max)
                            .arg(histData.sum);

            if (eclipseView->faultResultSettings()->hasValidCustomResult())
            {
                QString faultMapping;
                bool    isShowingGrid = eclipseView->faultCollection()->isGridVisualizationMode();
                if (!isShowingGrid)
                {
                    if (eclipseView->faultCollection()->faultResult() == RimFaultInViewCollection::FAULT_BACK_FACE_CULLING)
                    {
                        faultMapping = "Cells behind fault";
                    }
                    else if (eclipseView->faultCollection()->faultResult() == RimFaultInViewCollection::FAULT_FRONT_FACE_CULLING)
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
                infoText += QString("<b>Fault Property:</b> %1 <br>")
                                .arg(eclipseView->faultResultSettings()->customFaultResult()->resultVariableUiShortName());
            }
        }

        if (eclipseView->hasUserRequestedAnimation() && eclipseView->cellEdgeResult()->hasResult())
        {
            double  min, max;
            QString cellEdgeName = eclipseView->cellEdgeResult()->resultVariableUiShortName();
            eclipseView->cellEdgeResult()->minMaxCellEdgeValues(min, max);
            infoText += QString("<b>Cell Edge Property:</b> %1 ").arg(cellEdgeName);
            infoText += QString("<table border=0 cellspacing=5 >"
                                "<tr> <td>Min</td> <td></td> <td></td> <td></td> <td>Max</td> </tr>"
                                "<tr> <td>%1</td>  <td></td> <td></td> <td></td> <td> %2</td></tr>"
                                "</table>")
                            .arg(min)
                            .arg(max);
        }

        if (showVolumeWeightedMean && histData.weightedMean != HUGE_VAL)
        {
            infoText += QString("<b>Mobile Volume Weighted Mean:</b> %1").arg(histData.weightedMean);
        }
    }
    return infoText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dOverlayInfoConfig::resultInfoText(const HistogramData& histData, RimGeoMechView* geoMechView)
{
    QString infoText;

    if (geoMechView)
    {
        RimGeoMechCase*     geoMechCase = geoMechView->geoMechCase();
        RigGeoMechCaseData* caseData    = geoMechCase ? geoMechCase->geoMechData() : nullptr;
        bool                isResultsInfoRelevant =
            caseData && geoMechView->hasUserRequestedAnimation() && geoMechView->cellResultResultDefinition()->hasResult();

        if (isResultsInfoRelevant)
        {
            QString resultPos;
            QString fieldName     = geoMechView->cellResultResultDefinition()->resultFieldUiName();
            QString compName      = geoMechView->cellResultResultDefinition()->resultComponentUiName();
            QString diffResString = geoMechView->cellResultResultDefinition()->diffResultUiName();
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

                case RIG_ELEMENT:
                    resultPos = "Element";
                    break;
                default:
                    break;
            }
            if (compName == "")
            {
                infoText += QString("<b>Cell result:</b> %1, %2<br>").arg(resultPos).arg(fieldName);
            }
            else
            {
                infoText += QString("<b>Cell result:</b> %1, %2, %3<br>").arg(resultPos).arg(fieldName).arg(compName);
            }

            if (!diffResString.isEmpty())
            {
                infoText += QString("%1<br>").arg(diffResString);
            }
            infoText += QString("<br><b>Statistics:</b> ") + m_statisticsTimeRange().uiText() + " and " +
                        m_statisticsCellRange().uiText();
            infoText += QString("<table border=0 cellspacing=5 >"
                                "<tr> <td>Min</td> <td>P90</td> <td>Mean</td> <td>P10</td> <td>Max</td> <td>Sum</td> </tr>"
                                "<tr> <td>%1</td>  <td> %2</td> <td> %3</td>  <td> %4</td> <td> %5</td> <td> %6</td> </tr>"
                                "</table>")
                            .arg(histData.min)
                            .arg(histData.p10)
                            .arg(histData.mean)
                            .arg(histData.p90)
                            .arg(histData.max)
                            .arg(histData.sum);
        }
        else
        {
            infoText += QString("<b>No valid result selected</b>");
        }
    }
    return infoText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::showStatisticsInfoDialog(bool raise)
{
    if (m_viewDef)
    {
        // Show dialog before setting data due to text edit auto height setting
        m_gridStatisticsDialog->resize(600, 800);
        m_gridStatisticsDialog->show();

        m_gridStatisticsDialog->setLabel("Grid statistics");
        m_gridStatisticsDialog->updateFromRimView(m_viewDef);

        if (raise)
        {
            m_gridStatisticsDialog->raise();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::update3DInfo()
{
    this->updateUiIconFromToggleField();

    if (!m_viewDef) return;
    if (!m_viewDef->viewer()) return;

    if (!this->m_active())
    {
        m_viewDef->viewer()->showInfoText(false);
        m_viewDef->viewer()->showHistogram(false);
        m_viewDef->viewer()->showAnimationProgress(false);
        m_viewDef->viewer()->showVersionInfo(false);

        update3DInfoIn2dViews();
        return;
    }

    m_viewDef->viewer()->showInfoText(m_showCaseInfo() || m_showResultInfo());
    m_viewDef->viewer()->showHistogram(false);
    m_viewDef->viewer()->showAnimationProgress(m_showAnimProgress());
    m_viewDef->viewer()->showVersionInfo(m_showVersionInfo());

    m_isVisCellStatUpToDate = false;

    if (hasInvalidStatisticsCombination())
    {
        displayPropertyFilteredStatisticsMessage(true);
        m_statisticsTimeRange = CURRENT_TIMESTEP;
    }

    RimEclipseView* reservoirView = dynamic_cast<RimEclipseView*>(m_viewDef.p());
    if (reservoirView)
    {
        updateEclipse3DInfo(reservoirView);

        // Update statistics dialog
        m_gridStatisticsDialog->updateFromRimView(reservoirView);
    }

    RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>(m_viewDef.p());
    if (geoMechView)
    {
        m_showVolumeWeightedMean = false;

        updateGeoMech3DInfo(geoMechView);

        // Update statistics dialog
        m_gridStatisticsDialog->updateFromRimView(geoMechView);
    }

    update3DInfoIn2dViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* Rim3dOverlayInfoConfig::objectToggleField()
{
    return &m_active;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* visGroup = uiOrdering.addNewGroup("Visibility");

    RimEclipseView*           eclipseView = dynamic_cast<RimEclipseView*>(m_viewDef.p());
    RimEclipseContourMapView* contourMap  = dynamic_cast<RimEclipseContourMapView*>(eclipseView);
    RimGeoMechView*           geoMechView = dynamic_cast<RimGeoMechView*>(m_viewDef.p());

    visGroup->add(&m_showAnimProgress);
    visGroup->add(&m_showCaseInfo);
    visGroup->add(&m_showResultInfo);
    if (!geoMechView && !contourMap)
    {
        visGroup->add(&m_showVolumeWeightedMean);
    }

    if (!contourMap)
    {
        visGroup->add(&m_showHistogram);
    }

    visGroup->add(&m_showVersionInfo);

    if (contourMap)
    {
        m_statisticsTimeRange = Rim3dOverlayInfoConfig::CURRENT_TIMESTEP;
        m_statisticsCellRange = Rim3dOverlayInfoConfig::VISIBLE_CELLS;
    }
    else
    {
        caf::PdmUiGroup* statGroup = uiOrdering.addNewGroup("Statistics Options");

        if (!eclipseView || !eclipseView->cellResult()->isFlowDiagOrInjectionFlooding())
        {
            statGroup->add(&m_statisticsTimeRange);
        }
        statGroup->add(&m_statisticsCellRange);
    }
    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::setReservoirView(RimGridView* ownerReservoirView)
{
    m_viewDef = ownerReservoirView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::updateEclipse3DInfo(RimEclipseView* eclipseView)
{
    HistogramData histData;

    if (m_showHistogram() || m_showResultInfo())
    {
        histData = histogramData();
    }

    QString infoText;

    if (m_showCaseInfo())
    {
        infoText = caseInfoText();
    }

    if (m_showResultInfo())
    {
        infoText += resultInfoText(histData);
    }

    if (!infoText.isEmpty())
    {
        eclipseView->viewer()->setInfoText(infoText);
    }

    if (m_showHistogram())
    {
        bool isResultsInfoRelevant = eclipseView->hasUserRequestedAnimation() && eclipseView->cellResult()->hasResult();

        if (isResultsInfoRelevant && histData.histogram)
        {
            eclipseView->viewer()->showHistogram(true);
            eclipseView->viewer()->setHistogram(histData.min, histData.max, *histData.histogram);
            eclipseView->viewer()->setHistogramPercentiles(histData.p10, histData.p90, histData.mean);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::updateGeoMech3DInfo(RimGeoMechView* geoMechView)
{
    HistogramData histData;

    if (m_showResultInfo() || m_showHistogram())
    {
        histData = histogramData(geoMechView);
    }

    // Compose text

    QString infoText;

    if (m_showCaseInfo())
    {
        infoText = caseInfoText(geoMechView);
    }

    if (m_showResultInfo())
    {
        infoText += resultInfoText(histData, geoMechView);
    }

    if (!infoText.isEmpty())
    {
        geoMechView->viewer()->setInfoText(infoText);
    }

    // Populate histogram

    if (m_showHistogram())
    {
        RimGeoMechCase*     geoMechCase = geoMechView->geoMechCase();
        RigGeoMechCaseData* caseData    = geoMechCase ? geoMechCase->geoMechData() : nullptr;
        bool                isResultsInfoRelevant =
            caseData && geoMechView->hasUserRequestedAnimation() && geoMechView->cellResultResultDefinition()->hasResult();

        if (isResultsInfoRelevant)
        {
            geoMechView->viewer()->showHistogram(true);
            geoMechView->viewer()->setHistogram(histData.min, histData.max, *histData.histogram);
            geoMechView->viewer()->setHistogramPercentiles(histData.p10, histData.p90, histData.mean);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::update3DInfoIn2dViews() const
{
    RimCase* rimCase;
    firstAncestorOrThisOfType(rimCase);
    if (rimCase)
    {
        for (Rim2dIntersectionView* view : rimCase->intersectionViewCollection()->views())
        {
            view->update3dInfo();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dOverlayInfoConfig::timeStepText(RimEclipseView* eclipseView)
{
    int                    currTimeStepIndex = eclipseView->currentTimeStep();
    std::vector<QDateTime> timeSteps         = eclipseView->currentGridCellResults()->allTimeStepDatesFromEclipseReader();

    QString dateTimeString;
    if (currTimeStepIndex >= 0 && currTimeStepIndex < (int)timeSteps.size())
    {
        QString dateFormat = RimTools::createTimeFormatStringFromDates(timeSteps);

        QString dateString = RiaQDateTimeTools::toStringUsingApplicationLocale(timeSteps[currTimeStepIndex], dateFormat);

        dateTimeString = QString("Time Step: %1/%2  %3")
                             .arg(QString::number(currTimeStepIndex), QString::number(timeSteps.size() - 1), dateString);
    }

    return QString("<p><b><center>-- %1 --</center></b>").arg(dateTimeString) +
           QString("<center>------------------------------------------------</center>");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString Rim3dOverlayInfoConfig::timeStepText(RimGeoMechView* geoMechView)
{
    int currTimeStepIndex = geoMechView->currentTimeStep();

    QStringList timeSteps;
    if (geoMechView->geoMechCase()) timeSteps = geoMechView->geoMechCase()->timeStepStrings();

    QString dateTimeString;
    if (currTimeStepIndex >= 0 && currTimeStepIndex < timeSteps.size())
    {
        dateTimeString =
            QString("Time Step: %1/%2  %3")
                .arg(QString::number(currTimeStepIndex), QString::number(timeSteps.size() - 1), timeSteps[currTimeStepIndex]);
    }

    return QString("<p><b><center>-- %1 --</center></b>").arg(dateTimeString) +
           QString("<center>------------------------------------------------</center>");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::updateVisCellStatsIfNeeded()
{
    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_viewDef.p());
    RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>(m_viewDef.p());

    if (!m_isVisCellStatUpToDate)
    {
        cvf::ref<RigStatisticsCalculator> calc;
        if (geoMechView)
        {
            RigFemResultAddress resAddress = geoMechView->cellResultResultDefinition()->resultAddress();
            calc                           = new RigFemNativeVisibleCellsStatCalc(
                geoMechView->geoMechCase()->geoMechData(), resAddress, geoMechView->currentTotalCellVisibility().p());
        }
        else if (eclipseView)
        {
            if (eclipseView->cellResult()->isFlowDiagOrInjectionFlooding())
            {
                RigFlowDiagResultAddress resAddr    = eclipseView->cellResult()->flowDiagResAddress();
                RigFlowDiagResults*      fldResults = eclipseView->cellResult()->flowDiagSolution()->flowDiagResults();
                calc = new RigFlowDiagVisibleCellsStatCalc(fldResults, resAddr, eclipseView->currentTotalCellVisibility().p());
            }
            else
            {
                RigEclipseResultAddress scalarIndex = eclipseView->cellResult()->eclipseResultAddress();
                calc                                = new RigEclipseNativeVisibleCellsStatCalc(
                    eclipseView->currentGridCellResults(), scalarIndex, eclipseView->currentTotalCellVisibility().p());
            }
        }

        m_visibleCellStatistics = new RigStatisticsDataCache(calc.p());
        m_isVisCellStatUpToDate = true;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
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
        QMessageBox::information(
            m_viewDef->viewer()->layoutWidget(),
            QString("ResInsight"),
            QString("Statistics not available<br>"
                    "<br>"
                    "Statistics calculations of <b>Visible Cells</b> for <b>All Time Steps</b> is not supported<br>"
                    "when you have an active Property filter on a time varying result.<br>") +
                switchString);
        isShowing = false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Rim3dOverlayInfoConfig::hasInvalidStatisticsCombination()
{
    if (m_viewDef->propertyFilterCollection() && m_viewDef->propertyFilterCollection()->hasActiveDynamicFilters() &&
        m_statisticsCellRange() == VISIBLE_CELLS && m_statisticsTimeRange() == ALL_TIMESTEPS)
    {
        RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_viewDef.p());
        if (!(eclipseView && eclipseView->cellResult()
                                 ->isFlowDiagOrInjectionFlooding())) // If isFlowDiagOrInjFlooding then skip this check as
                                                                     // ALL_TIMESTEPS is overridden to CURRENT behind the scenes
        {
            return true;
        }
    }

    return false;
}
