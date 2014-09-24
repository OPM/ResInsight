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
#include "RimCase.h"
#include "RimCellEdgeResultSlot.h"
#include "RimCellPropertyFilterCollection.h"
#include "RimFaultCollection.h"
#include "RimFaultResultSlot.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimReservoirView.h"
#include "RimResultSlot.h"
#include "RimWellCollection.h"
#include "RiuViewer.h"

CAF_PDM_SOURCE_INIT(Rim3dOverlayInfoConfig, "View3dOverlayInfoConfig");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Rim3dOverlayInfoConfig::Rim3dOverlayInfoConfig() 
{
    CAF_PDM_InitObject("Overlay 3D info", ":/InfoBox16x16.png", "", "");

    CAF_PDM_InitField(&active,              "Active",               true,   "Active",   "", "", "");
    active.setUiHidden(true);

    CAF_PDM_InitField(&showInfoText,        "ShowInfoText",         true,   "Info Text",   "", "", "");
    CAF_PDM_InitField(&showAnimProgress,    "ShowAnimProgress",     true,   "Animation progress",   "", "", "");
    CAF_PDM_InitField(&showHistogram,       "ShowHistogram",        true,   "Histogram",   "", "", "");
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

    if (m_reservoirView && m_reservoirView->viewer())
    {
        m_reservoirView->viewer()->update();
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

    if (!m_reservoirView) return;
    if (!m_reservoirView->viewer()) return;
    
    if (!this->active())
    {
        m_reservoirView->viewer()->showInfoText(false);
        m_reservoirView->viewer()->showHistogram(false);
        m_reservoirView->viewer()->showAnimationProgress(false);
        
        return;
    }

    m_reservoirView->viewer()->showInfoText(showInfoText());
    m_reservoirView->viewer()->showHistogram(false);
    m_reservoirView->viewer()->showAnimationProgress(showAnimProgress());

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


        if (m_reservoirView->eclipseCase() && m_reservoirView->eclipseCase()->reservoirData() && m_reservoirView->eclipseCase()->reservoirData()->mainGrid())
        {
            caseName = m_reservoirView->eclipseCase()->caseUserDescription();
            totCellCount = QString::number(m_reservoirView->eclipseCase()->reservoirData()->mainGrid()->cells().size());
            size_t mxActCellCount = m_reservoirView->eclipseCase()->reservoirData()->activeCellInfo(RifReaderInterface::MATRIX_RESULTS)->reservoirActiveCellCount();
            size_t frActCellCount = m_reservoirView->eclipseCase()->reservoirData()->activeCellInfo(RifReaderInterface::FRACTURE_RESULTS)->reservoirActiveCellCount();
            if (frActCellCount > 0)  activeCellCountText += "Matrix : ";
            activeCellCountText += QString::number(mxActCellCount);
            if (frActCellCount > 0)  activeCellCountText += " Fracture : " + QString::number(frActCellCount);

            iSize = QString::number(m_reservoirView->eclipseCase()->reservoirData()->mainGrid()->cellCountI());
            jSize = QString::number(m_reservoirView->eclipseCase()->reservoirData()->mainGrid()->cellCountJ());
            kSize = QString::number(m_reservoirView->eclipseCase()->reservoirData()->mainGrid()->cellCountK());

            zScale = QString::number(m_reservoirView->scaleZ());

            propName = m_reservoirView->cellResult()->resultVariable();
            cellEdgeName = m_reservoirView->cellEdgeResult()->resultVariable();
        }

        QString infoText = QString(
            "<p><b><center>-- %1 --</center></b><p>  "
            "<b>Cell count. Total:</b> %2 <b>Active:</b> %3 <br>" 
            "<b>Main Grid I,J,K:</b> %4, %5, %6 <b>Z-Scale:</b> %7<br>").arg(caseName, totCellCount, activeCellCountText, iSize, jSize, kSize, zScale);

        if (m_reservoirView->cellResult()->isTernarySaturationSelected())
        {
            infoText += QString("<b>Cell Property:</b> %1 ").arg(propName);
        }

        if (m_reservoirView->animationMode() && m_reservoirView->cellResult()->hasResult())
        {
            infoText += QString("<b>Cell Property:</b> %1 ").arg(propName);

            double min, max;
            double p10, p90;
            double mean;
            size_t scalarIndex = m_reservoirView->cellResult()->scalarResultIndex();
            m_reservoirView->currentGridCellResults()->cellResults()->minMaxCellScalarValues(scalarIndex, min, max);
            m_reservoirView->currentGridCellResults()->cellResults()->p10p90CellScalarValues(scalarIndex, p10, p90);
            m_reservoirView->currentGridCellResults()->cellResults()->meanCellScalarValues(scalarIndex, mean);

            //infoText += QString("<blockquote><b>Min:</b> %1   <b>P10:</b> %2   <b>Mean:</b> %3   <b>P90:</b> %4   <b>Max:</b> %5 </blockquote>").arg(min).arg(p10).arg(mean).arg(p90).arg(max);
            //infoText += QString("<blockquote><pre>Min: %1   P10: %2   Mean: %3 \n  P90: %4   Max: %5 </pre></blockquote>").arg(min).arg(p10).arg(mean).arg(p90).arg(max);
            infoText += QString("<table border=0 cellspacing=5 ><tr><td>Min</td><td>P10</td> <td>Mean</td> <td>P90</td> <td>Max</td> </tr>" 
                                       "<tr><td>%1</td><td> %2</td><td> %3</td><td> %4</td><td> %5 </td></tr></table>").arg(min).arg(p10).arg(mean).arg(p90).arg(max);

            if (m_reservoirView->faultResultSettings()->hasValidCustomResult())
            {
                QString faultMapping;
                bool isShowingGrid = m_reservoirView->faultCollection()->isGridVisualizationMode();
                if (!isShowingGrid)
                {
                    if (m_reservoirView->faultCollection()->faultResult() == RimFaultCollection::FAULT_BACK_FACE_CULLING)
                    {
                        faultMapping = "Cells behind fault";
                    }
                    else if (m_reservoirView->faultCollection()->faultResult() == RimFaultCollection::FAULT_FRONT_FACE_CULLING)
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

                infoText += QString("<b>Fault Property:</b> %1 <br>").arg(m_reservoirView->faultResultSettings()->customFaultResult()->resultVariable());
            }
        }
        else
        {
            infoText += "<br>";
        }


        if (m_reservoirView->animationMode() && m_reservoirView->cellEdgeResult()->hasResult())
        {
            double min, max;
            m_reservoirView->cellEdgeResult()->minMaxCellEdgeValues(min, max);
            infoText += QString("<b>Cell Edge Property:</b> %1 <blockquote>Min: %2 Max: %3 </blockquote>").arg(cellEdgeName).arg(min).arg(max);

        }

        if (   m_reservoirView->cellResult()->hasDynamicResult() 
            || m_reservoirView->propertyFilterCollection()->hasActiveDynamicFilters() 
            || m_reservoirView->wellCollection()->hasVisibleWellPipes()
            || m_reservoirView->cellResult()->isTernarySaturationSelected())
        {
            int currentTimeStep = m_reservoirView->currentTimeStep();
            QDateTime date = m_reservoirView->currentGridCellResults()->cellResults()->timeStepDate(0, currentTimeStep);
            infoText += QString("<b>Time Step:</b> %1    <b>Time:</b> %2").arg(currentTimeStep).arg(date.toString("dd.MMM yyyy"));
        }

        m_reservoirView->viewer()->setInfoText(infoText);
    }

    if (showHistogram())
    {
        if (m_reservoirView->animationMode() && m_reservoirView->cellResult()->hasResult())
        {
            double min, max;
            double p10, p90;
            double mean;

            size_t scalarIndex = m_reservoirView->cellResult()->scalarResultIndex();
            m_reservoirView->currentGridCellResults()->cellResults()->minMaxCellScalarValues(scalarIndex, min, max);
            m_reservoirView->currentGridCellResults()->cellResults()->p10p90CellScalarValues(scalarIndex, p10, p90);
            m_reservoirView->currentGridCellResults()->cellResults()->meanCellScalarValues(scalarIndex, mean);

            m_reservoirView->viewer()->showHistogram(true);
            m_reservoirView->viewer()->setHistogram(min, max, m_reservoirView->currentGridCellResults()->cellResults()->cellScalarValuesHistogram(scalarIndex));
            m_reservoirView->viewer()->setHistogramPercentiles(p10, p90, mean);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* Rim3dOverlayInfoConfig::objectToggleField()
{
    return &active;
}
