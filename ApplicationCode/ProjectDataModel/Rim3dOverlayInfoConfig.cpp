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
#include "RigCaseData.h"
#include "RimEclipseCase.h"
#include "RimCellEdgeColors.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimFaultCollection.h"
#include "RimEclipseFaultColors.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimEclipseView.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseWellCollection.h"
#include "RiuViewer.h"
#include "RimGeoMechView.h"
#include "RimView.h"
#include "RimGeoMechCase.h"
#include "RigGeoMechCaseData.h"
#include "RigFemPartCollection.h"
#include "RimGeoMechCellColors.h"
#include "RigFemResultAddress.h"
#include "RigFemPartResultsCollection.h"

#include "RigStatisticsDataCache.h"

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

    CAF_PDM_InitField(&showInfoText,        "ShowInfoText",         true,   "Info Text",   "", "", "");
    CAF_PDM_InitField(&showAnimProgress,    "ShowAnimProgress",     true,   "Animation progress",   "", "", "");
    CAF_PDM_InitField(&showHistogram,       "ShowHistogram",        true,   "Histogram",   "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_statisticsTimeRange, "StatisticsTimeRange", "Statistics Time Range", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_statisticsCellRange, "StatisticsCellRange", "Statistics Cell Range", "", "", "");
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

    m_viewDef->viewer()->showInfoText(showInfoText());
    m_viewDef->viewer()->showHistogram(false);
    m_viewDef->viewer()->showAnimationProgress(showAnimProgress());

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
void Rim3dOverlayInfoConfig::setReservoirView(RimView* ownerReservoirView)
{
    m_viewDef = ownerReservoirView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::updateEclipse3DInfo(RimEclipseView * reservoirView)
{
    if (showInfoText())
    {
        QString caseName;
        QString totCellCount;
        QString activeCellCountText;
        QString fractureActiveCellCount;
        QString iSize, jSize, kSize;
        QString zScale;
        QString propName;
        QString cellEdgeName;
        QString faultCellResultMapping;


        if (reservoirView->eclipseCase() && reservoirView->eclipseCase()->reservoirData() && reservoirView->eclipseCase()->reservoirData()->mainGrid())
        {
            caseName = reservoirView->eclipseCase()->caseUserDescription();
            totCellCount = QString::number(reservoirView->eclipseCase()->reservoirData()->mainGrid()->cells().size());
            size_t mxActCellCount = reservoirView->eclipseCase()->reservoirData()->activeCellInfo(RifReaderInterface::MATRIX_RESULTS)->reservoirActiveCellCount();
            size_t frActCellCount = reservoirView->eclipseCase()->reservoirData()->activeCellInfo(RifReaderInterface::FRACTURE_RESULTS)->reservoirActiveCellCount();
            if (frActCellCount > 0)  activeCellCountText += "Matrix : ";
            activeCellCountText += QString::number(mxActCellCount);
            if (frActCellCount > 0)  activeCellCountText += " Fracture : " + QString::number(frActCellCount);

            iSize = QString::number(reservoirView->eclipseCase()->reservoirData()->mainGrid()->cellCountI());
            jSize = QString::number(reservoirView->eclipseCase()->reservoirData()->mainGrid()->cellCountJ());
            kSize = QString::number(reservoirView->eclipseCase()->reservoirData()->mainGrid()->cellCountK());

            zScale = QString::number(reservoirView->scaleZ());

            propName = reservoirView->cellResult()->resultVariable();
            cellEdgeName = reservoirView->cellEdgeResult()->resultVariable();
        }

        QString infoText = QString(
            "<p><b><center>-- %1 --</center></b><p>  "
            "<b>Cell count. Total:</b> %2 <b>Active:</b> %3 <br>"
            "<b>Main Grid I,J,K:</b> %4, %5, %6 <b>Z-Scale:</b> %7<br>").arg(caseName, totCellCount, activeCellCountText, iSize, jSize, kSize, zScale);

        if (reservoirView->cellResult()->isTernarySaturationSelected())
        {
            infoText += QString("<b>Cell Property:</b> %1 ").arg(propName);
        }

        if (reservoirView->hasUserRequestedAnimation() && reservoirView->cellResult()->hasResult())
        {
            infoText += QString("<b>Cell Property:</b> %1 ").arg(propName);
            // Wait until regression tests confirm new statisticks is ok  infoText += QString("<br>Statistics for: ") + m_statisticsTimeRange().uiText() + " and " + m_statisticsCellRange().uiText();

            if (m_statisticsCellRange == ALL_CELLS)
            {
                double min, max;
                double p10, p90;
                double mean;
                
                size_t scalarIndex = reservoirView->cellResult()->scalarResultIndex();

                if (m_statisticsTimeRange == ALL_TIMESTEPS)
                {
                    reservoirView->currentGridCellResults()->cellResults()->minMaxCellScalarValues(scalarIndex, min, max);
                    reservoirView->currentGridCellResults()->cellResults()->p10p90CellScalarValues(scalarIndex, p10, p90);
                    reservoirView->currentGridCellResults()->cellResults()->meanCellScalarValues(scalarIndex, mean);
                }
                else if (m_statisticsTimeRange == CURRENT_TIMESTEP)
                {
                    int timeStepIdx = reservoirView->currentTimeStep();
                    reservoirView->currentGridCellResults()->cellResults()->minMaxCellScalarValues(scalarIndex, timeStepIdx, min, max);
                    //reservoirView->currentGridCellResults()->cellResults()->p10p90CellScalarValues(scalarIndex, timeStepIdx, p10, p90);
                    //reservoirView->currentGridCellResults()->cellResults()->meanCellScalarValues(scalarIndex,  timeStepIdx, mean);
                    p10 = HUGE_VAL;
                    p90 = HUGE_VAL;
                    mean = HUGE_VAL;
                }

                infoText += QString("<table border=0 cellspacing=5 >"
                                    "<tr> <td>Min</td> <td>P10</td> <td>Mean</td> <td>P90</td> <td>Max</td> </tr>"
                                    "<tr> <td>%1</td>  <td> %2</td> <td> %3</td>  <td> %4</td> <td> %5 </td></tr>"
                                    "</table>").arg(min).arg(p10).arg(mean).arg(p90).arg(max);
            }

            if (reservoirView->faultResultSettings()->hasValidCustomResult())
            {
                QString faultMapping;
                bool isShowingGrid = reservoirView->faultCollection()->isGridVisualizationMode();
                if (!isShowingGrid)
                {
                    if (reservoirView->faultCollection()->faultResult() == RimFaultCollection::FAULT_BACK_FACE_CULLING)
                    {
                        faultMapping = "Cells behind fault";
                    }
                    else if (reservoirView->faultCollection()->faultResult() == RimFaultCollection::FAULT_FRONT_FACE_CULLING)
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

                infoText += QString("<b>Fault Property:</b> %1 <br>").arg(reservoirView->faultResultSettings()->customFaultResult()->resultVariable());
            }
        }
        else
        {
            infoText += "<br>";
        }


        if (reservoirView->hasUserRequestedAnimation() && reservoirView->cellEdgeResult()->hasResult())
        {
            double min, max;
            reservoirView->cellEdgeResult()->minMaxCellEdgeValues(min, max);
            infoText += QString("<b>Cell Edge Property:</b> %1 <blockquote>Min: %2 Max: %3 </blockquote>").arg(cellEdgeName).arg(min).arg(max);

        }

        if (reservoirView->cellResult()->hasDynamicResult()
            || reservoirView->propertyFilterCollection()->hasActiveDynamicFilters()
            || reservoirView->wellCollection()->hasVisibleWellPipes()
            || reservoirView->cellResult()->isTernarySaturationSelected())
        {
            int currentTimeStep = reservoirView->currentTimeStep();
            QDateTime date = reservoirView->currentGridCellResults()->cellResults()->timeStepDate(0, currentTimeStep);
            infoText += QString("<b>Time Step:</b> %1    <b>Time:</b> %2").arg(currentTimeStep).arg(date.toString("dd.MMM yyyy"));
        }

        reservoirView->viewer()->setInfoText(infoText);
    }

    if (showHistogram())
    {
        if (reservoirView->hasUserRequestedAnimation() && reservoirView->cellResult()->hasResult())
        {
            if (m_statisticsCellRange == ALL_CELLS && m_statisticsTimeRange == ALL_TIMESTEPS)
            {
            double min, max;
            double p10, p90;
            double mean;

            size_t scalarIndex = reservoirView->cellResult()->scalarResultIndex();
            reservoirView->currentGridCellResults()->cellResults()->minMaxCellScalarValues(scalarIndex, min, max);
            reservoirView->currentGridCellResults()->cellResults()->p10p90CellScalarValues(scalarIndex, p10, p90);
            reservoirView->currentGridCellResults()->cellResults()->meanCellScalarValues(scalarIndex, mean);

            reservoirView->viewer()->showHistogram(true);
            reservoirView->viewer()->setHistogram(min, max, reservoirView->currentGridCellResults()->cellResults()->cellScalarValuesHistogram(scalarIndex));
            reservoirView->viewer()->setHistogramPercentiles(p10, p90, mean);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Rim3dOverlayInfoConfig::updateGeoMech3DInfo(RimGeoMechView * geoMechView)
{
    if (showInfoText())
    {
        QString infoText;

        RimGeoMechCase* geoMechCase = geoMechView->geoMechCase();
        RigGeoMechCaseData* caseData = geoMechCase ? geoMechCase->geoMechData() : NULL;
        RigFemPartCollection* femParts = caseData ? caseData->femParts() : NULL;

        if (femParts)
        {
            QString caseName = geoMechCase->caseUserDescription();
            QString cellCount = QString("%1").arg(femParts->totalElementCount());
            QString zScale = QString::number(geoMechView->scaleZ());
            
            infoText = QString(
            "<p><b><center>-- %1 --</center></b><p>"
            "<b>Cell count:</b> %2 <b>Z-Scale:</b> %3<br>").arg(caseName, cellCount, zScale);

            if (geoMechView->hasUserRequestedAnimation() && geoMechView->cellResult()->hasResult())
            {
                QString resultPos;
                QString fieldName = geoMechView->cellResult()->resultFieldUiName();
                QString compName = geoMechView->cellResult()->resultComponentUiName();

                if (!fieldName.isEmpty())
                {
                    switch (geoMechView->cellResult()->resultPositionType())
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

                    double min = 0, max = 0;
                    double p10 = 0, p90 = 0;
                    double mean = 0;
                    
                    RigFemResultAddress resAddress = geoMechView->cellResult()->resultAddress();
                    caseData->femPartResults()->meanScalarValue(resAddress, &mean);
                    caseData->femPartResults()->minMaxScalarValues(resAddress,&min, &max);
                    caseData->femPartResults()->p10p90ScalarValues(resAddress, &p10, &p90);

                    infoText += QString("<table border=0 cellspacing=5 >"
                                        "<tr> <td>Min</td> <td>P10</td> <td>Mean</td> <td>P90</td> <td>Max</td> </tr>"
                                        "<tr> <td>%1</td>  <td> %2</td> <td> %3</td>  <td> %4</td> <td> %5</td> </tr>"
                                        "</table>").arg(min).arg(p10).arg(mean).arg(p90).arg(max);
                }
                else
                {
                    infoText += QString("<br>");
                }

                int currentTimeStep = geoMechView->currentTimeStep();
                QString stepName = QString::fromStdString(caseData->femPartResults()->stepNames()[currentTimeStep]);
                infoText += QString("<b>Time Step:</b> %1    <b>Time:</b> %2").arg(currentTimeStep).arg(stepName);
            }
        }

        geoMechView->viewer()->setInfoText(infoText);
    }

    if (showHistogram())
    {
        if (geoMechView->hasUserRequestedAnimation() && geoMechView->cellResult()->hasResult())
        {
            geoMechView->viewer()->showHistogram(true);

            // ToDo: Implement statistics for geomech data

            RimGeoMechCase* geoMechCase = geoMechView->geoMechCase();
            RigGeoMechCaseData* caseData = geoMechCase ? geoMechCase->geoMechData() : NULL;

            if (caseData)
            {
                double min = 0, max = 0;
                double p10 = 0, p90 = 0;
                double mean = 0;

                RigFemResultAddress resAddress = geoMechView->cellResult()->resultAddress();
                caseData->femPartResults()->meanScalarValue(resAddress, &mean);
                caseData->femPartResults()->minMaxScalarValues(resAddress, &min, &max);
                caseData->femPartResults()->p10p90ScalarValues(resAddress, &p10, &p90);
                geoMechView->viewer()->setHistogram(min, max, caseData->femPartResults()->scalarValuesHistogram(resAddress));
                geoMechView->viewer()->setHistogramPercentiles(p10, p90, mean);
            }
        }
    }
}
