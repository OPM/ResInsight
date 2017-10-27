/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RiuSelectionChangedHandler.h"

#include "RiaApplication.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigTimeHistoryResultAccessor.h"
#include "RigGridBase.h"
#include "RigActiveCellInfo.h"
#include "RiuFemTimeHistoryResultAccessor.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimProject.h"
#include "RimEclipseResultCase.h"

#include "RiuFemResultTextBuilder.h"
#include "RiuMainWindow.h"
#include "RiuResultQwtPlot.h"
#include "RiuResultTextBuilder.h"
#include "RiuSelectionManager.h"
#include "RiuRelativePermeabilityPlotPanel.h"
#include "RiuPvtPlotPanel.h"

//#include "cvfTrace.h"

#include <QStatusBar>

#include <assert.h>



//==================================================================================================
//
//
//
//==================================================================================================
class CellLookupHelper
{
public:
    static size_t mapToActiveCellIndex(const RigEclipseCaseData* eclipseCaseData, size_t gridIndex, size_t gridLocalCellIndex)
    {
        const size_t gridCount = eclipseCaseData ? eclipseCaseData->gridCount() : 0;
        const RigGridBase* grid = gridIndex < gridCount ? eclipseCaseData->grid(gridIndex) : NULL;
        if (grid && gridLocalCellIndex < grid->cellCount())
        {
            // Note!!
            // Which type of porosity model to choose? Currently hard-code to MATRIX_MODEL
            const RigActiveCellInfo* activeCellInfo = eclipseCaseData->activeCellInfo(RiaDefines::MATRIX_MODEL);
            CVF_ASSERT(activeCellInfo);

            const size_t reservoirCellIndex = grid->reservoirCellIndex(gridLocalCellIndex);
            const size_t activeCellIndex = activeCellInfo->cellResultIndex(reservoirCellIndex);
            return activeCellIndex;
        }

        return cvf::UNDEFINED_SIZE_T;
    }

    static QString cellReferenceText(const RigEclipseCaseData* eclipseCaseData, size_t gridIndex, size_t gridLocalCellIndex)
    {
        const size_t gridCount = eclipseCaseData ? eclipseCaseData->gridCount() : 0;
        const RigGridBase* grid = gridIndex < gridCount ? eclipseCaseData->grid(gridIndex) : NULL;
        if (grid && gridLocalCellIndex < grid->cellCount())
        {
            size_t i = 0;
            size_t j = 0;
            size_t k = 0;
            if (grid->ijkFromCellIndex(gridLocalCellIndex, &i, &j, &k))
            {
                // Adjust to 1-based Eclipse indexing
                i++;
                j++;
                k++;

                QString retText = QString("Grid index %1, Cell : [%2, %3, %4]").arg(gridIndex).arg(i).arg(j).arg(k);
                return retText;
            }
        }

        return QString();
    }
};




//==================================================================================================
//
//
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuSelectionChangedHandler::RiuSelectionChangedHandler()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuSelectionChangedHandler::~RiuSelectionChangedHandler()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::handleSelectionDeleted() const
{
    RiuMainWindow::instance()->resultPlot()->deleteAllCurves();

    updateRelativePermeabilityPlot(NULL);
    updatePvtPlot(NULL);

    updateResultInfo(NULL);

    scheduleUpdateForAllVisibleViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::handleItemAppended(const RiuSelectionItem* item) const
{
    addCurveFromSelectionItem(item);

    updateRelativePermeabilityPlot(item);
    updatePvtPlot(item);

    updateResultInfo(item);

    scheduleUpdateForAllVisibleViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::handleSetSelectedItem(const RiuSelectionItem* item) const
{
    RiuMainWindow::instance()->resultPlot()->deleteAllCurves();

    handleItemAppended(item);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::addCurveFromSelectionItem(const RiuEclipseSelectionItem* eclipseSelectionItem) const
{
    RimEclipseView* eclipseView = eclipseSelectionItem->m_view.p();

    if (eclipseView->cellResult()->isFlowDiagOrInjectionFlooding())
    { 
        // NB! Do not read out data for flow results, as this can be a time consuming operation

        return;
    }
    else if (eclipseView->cellResult()->hasDynamicResult() 
             && !RiaDefines::isPerCellFaceResult(eclipseView->cellResult()->resultVariable())
             && eclipseView->eclipseCase() 
             && eclipseView->eclipseCase()->eclipseCaseData())
    {
        RiaDefines::PorosityModelType porosityModel = eclipseView->cellResult()->porosityModel();

        std::vector<QDateTime> timeStepDates = eclipseView->eclipseCase()->eclipseCaseData()->results(porosityModel)->timeStepDates();

        QString curveName = eclipseView->eclipseCase()->caseUserDescription();
        curveName += ", ";
        curveName += eclipseView->cellResult()->resultVariableUiShortName();
        curveName += ", ";
        curveName += QString("Grid index %1").arg(eclipseSelectionItem->m_gridIndex);
        curveName += ", ";
        curveName += RigTimeHistoryResultAccessor::geometrySelectionText(eclipseView->eclipseCase()->eclipseCaseData(), eclipseSelectionItem->m_gridIndex, eclipseSelectionItem->m_gridLocalCellIndex);


        std::vector<double> timeHistoryValues = RigTimeHistoryResultAccessor::timeHistoryValues(eclipseView->eclipseCase()->eclipseCaseData(), eclipseView->cellResult(), eclipseSelectionItem->m_gridIndex, eclipseSelectionItem->m_gridLocalCellIndex, timeStepDates.size());
        CVF_ASSERT(timeStepDates.size() == timeHistoryValues.size());

        RiuMainWindow::instance()->resultPlot()->addCurve(curveName, eclipseSelectionItem->m_color, timeStepDates, timeHistoryValues);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::addCurveFromSelectionItem(const RiuGeoMechSelectionItem* geomSelectionItem) const
{
    RimGeoMechView* geoMechView = geomSelectionItem->m_view.p();

    if (geoMechView &&
        geoMechView->cellResultResultDefinition() &&
        geoMechView->cellResultResultDefinition()->hasResult() &&
        geoMechView->geoMechCase() &&
        geoMechView->geoMechCase()->geoMechData())
    {
        std::unique_ptr<RiuFemTimeHistoryResultAccessor> timeHistResultAccessor;

        if ( geomSelectionItem->m_hasIntersectionTriangle )
        {
            timeHistResultAccessor = std::unique_ptr<RiuFemTimeHistoryResultAccessor>(
                new RiuFemTimeHistoryResultAccessor(geoMechView->geoMechCase()->geoMechData(),
                                                    geoMechView->cellResultResultDefinition()->resultAddress(),
                                                    geomSelectionItem->m_gridIndex,
                                                    static_cast<int>(geomSelectionItem->m_cellIndex),
                                                    geomSelectionItem->m_elementFace,
                                                    geomSelectionItem->m_localIntersectionPoint,
                                                    geomSelectionItem->m_intersectionTriangle));
        }
        else
        {
            timeHistResultAccessor = std::unique_ptr<RiuFemTimeHistoryResultAccessor>(
                new RiuFemTimeHistoryResultAccessor(geoMechView->geoMechCase()->geoMechData(),
                                                    geoMechView->cellResultResultDefinition()->resultAddress(),
                                                    geomSelectionItem->m_gridIndex,
                                                    static_cast<int>(geomSelectionItem->m_cellIndex),
                                                    geomSelectionItem->m_elementFace,
                                                    geomSelectionItem->m_localIntersectionPoint));
        }

        QString curveName;
        curveName.append(geoMechView->geoMechCase()->caseUserDescription() + ", ");

        caf::AppEnum<RigFemResultPosEnum> resPosAppEnum = geoMechView->cellResultResultDefinition()->resultPositionType();
        curveName.append(resPosAppEnum.uiText() + ", ");
        curveName.append(geoMechView->cellResultResultDefinition()->resultFieldUiName()+ ", ") ;
        curveName.append(geoMechView->cellResultResultDefinition()->resultComponentUiName() + " ");
        
        if ( resPosAppEnum == RIG_ELEMENT_NODAL_FACE )
        {
            if ( geomSelectionItem->m_elementFace >= 0 )
            {
                curveName.append(", " + caf::AppEnum<cvf::StructGridInterface::FaceType>::textFromIndex(geomSelectionItem->m_elementFace));
            }
            else
            {
                curveName.append(", from N[" + QString::number(timeHistResultAccessor->closestNodeId()) + "] transformed onto intersection");
            }
        }
        curveName.append("\n");

        curveName.append(timeHistResultAccessor->geometrySelectionText());

        std::vector<double> timeHistoryValues = timeHistResultAccessor->timeHistoryValues();

        std::vector<QDateTime> dates = geoMechView->geoMechCase()->timeStepDates();
        if (dates.size() == timeHistoryValues.size())
        {
            RiuMainWindow::instance()->resultPlot()->addCurve(curveName, geomSelectionItem->m_color, dates, timeHistoryValues);
        }
        else
        {
            std::vector<double> dummyStepTimes;
            for (size_t i = 0; i < timeHistoryValues.size(); i++)
            {
                dummyStepTimes.push_back(i);
            }

            RiuMainWindow::instance()->resultPlot()->addCurve(curveName, geomSelectionItem->m_color, dummyStepTimes, timeHistoryValues);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::addCurveFromSelectionItem(const RiuSelectionItem* itemAdded) const
{
    if (itemAdded->type() == RiuSelectionItem::ECLIPSE_SELECTION_OBJECT)
    {
        const RiuEclipseSelectionItem* eclipseSelectionItem = static_cast<const RiuEclipseSelectionItem*>(itemAdded);

        addCurveFromSelectionItem(eclipseSelectionItem);
    }
    else if (itemAdded->type() == RiuSelectionItem::GEOMECH_SELECTION_OBJECT)
    {
        const RiuGeoMechSelectionItem* geomSelectionItem = static_cast<const RiuGeoMechSelectionItem*>(itemAdded);

        addCurveFromSelectionItem(geomSelectionItem);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::updateRelativePermeabilityPlot(const RiuSelectionItem* selectionItem) const
{
    RiuRelativePermeabilityPlotPanel* relPermPlotPanel = RiuMainWindow::instance()->relativePermeabilityPlotPanel();
    if (!relPermPlotPanel)
    {
        return;
    }

    bool mustClearPlot = true;

    if (relPermPlotPanel->isVisible() && selectionItem && selectionItem->type() == RiuSelectionItem::ECLIPSE_SELECTION_OBJECT)
    {
        const RiuEclipseSelectionItem* eclipseSelectionItem = static_cast<const RiuEclipseSelectionItem*>(selectionItem);
        RimEclipseResultCase* eclipseResultCase = dynamic_cast<RimEclipseResultCase*>(eclipseSelectionItem->m_view->eclipseCase());
        if (eclipseResultCase && eclipseResultCase->flowDiagSolverInterface())
        {
            size_t activeCellIndex = CellLookupHelper::mapToActiveCellIndex(eclipseResultCase->eclipseCaseData(), eclipseSelectionItem->m_gridIndex, eclipseSelectionItem->m_gridLocalCellIndex);
            if (activeCellIndex != cvf::UNDEFINED_SIZE_T)
            {
                //cvf::Trace::show("Updating RelPerm plot for active cell index: %d", static_cast<int>(activeCellIndex));

                std::vector<RigFlowDiagSolverInterface::RelPermCurve> relPermCurveArr = eclipseResultCase->flowDiagSolverInterface()->calculateRelPermCurvesForActiveCell(activeCellIndex);
                QString cellRefText = CellLookupHelper::cellReferenceText(eclipseResultCase->eclipseCaseData(), eclipseSelectionItem->m_gridIndex, eclipseSelectionItem->m_gridLocalCellIndex);

                relPermPlotPanel->setPlotData(relPermCurveArr, cellRefText);
                mustClearPlot = false;
            }
        }
    }

    if (mustClearPlot)
    {
        relPermPlotPanel->clearPlot();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::updatePvtPlot(const RiuSelectionItem* selectionItem) const
{
    RiuPvtPlotPanel* pvtPlotPanel = RiuMainWindow::instance()->pvtPlotPanel();
    if (!pvtPlotPanel)
    {
        return;
    }

    bool mustClearPlot = true;

    if (pvtPlotPanel->isVisible() && selectionItem && selectionItem->type() == RiuSelectionItem::ECLIPSE_SELECTION_OBJECT)
    {
        const RiuEclipseSelectionItem* eclipseSelectionItem = static_cast<const RiuEclipseSelectionItem*>(selectionItem);
        RimEclipseResultCase* eclipseResultCase = dynamic_cast<RimEclipseResultCase*>(eclipseSelectionItem->m_view->eclipseCase());
        if (eclipseResultCase && eclipseResultCase->flowDiagSolverInterface())
        {
            size_t activeCellIndex = CellLookupHelper::mapToActiveCellIndex(eclipseResultCase->eclipseCaseData(), eclipseSelectionItem->m_gridIndex, eclipseSelectionItem->m_gridLocalCellIndex);
            if (activeCellIndex != cvf::UNDEFINED_SIZE_T)
            {
                //cvf::Trace::show("Update PVT plot for active cell index: %d", static_cast<int>(activeCellIndex));
                
                //std::vector<RigFlowDiagSolverInterface::PvtCurve> fvfCurveArr = eclipseResultCase->flowDiagSolverInterface()->calculatePvtFvfCurvesForActiveCell(activeCellIndex);
                //cvf::Trace::show("Got %d fvf curves", static_cast<int>(fvfCurveArr.size()));
                
                QString cellRefText = CellLookupHelper::cellReferenceText(eclipseResultCase->eclipseCaseData(), eclipseSelectionItem->m_gridIndex, eclipseSelectionItem->m_gridLocalCellIndex);

                pvtPlotPanel->setPlotData(cellRefText);
                mustClearPlot = false;
            }
        }
    }

    if (mustClearPlot)
    {
        pvtPlotPanel->clearPlot();
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::scheduleUpdateForAllVisibleViews() const
{
    RimProject* proj = RiaApplication::instance()->project();
    if (proj)
    {
        std::vector<RimView*> visibleViews;
        proj->allVisibleViews(visibleViews);

        for (size_t i = 0; i < visibleViews.size(); i++)
        {
            visibleViews[i]->createHighlightAndGridBoxDisplayModelWithRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::updateResultInfo(const RiuSelectionItem* itemAdded) const
{
    QString resultInfo;
    QString pickInfo;

    if (itemAdded != NULL)
    {
        if (itemAdded->type() == RiuSelectionItem::ECLIPSE_SELECTION_OBJECT)
        {
            const RiuEclipseSelectionItem* eclipseSelectionItem = static_cast<const RiuEclipseSelectionItem*>(itemAdded);

            RimEclipseView* eclipseView = eclipseSelectionItem->m_view.p();

            RiuResultTextBuilder textBuilder(eclipseView, eclipseSelectionItem->m_gridIndex, eclipseSelectionItem->m_gridLocalCellIndex, eclipseView->currentTimeStep());
            textBuilder.setFace(eclipseSelectionItem->m_face);
            textBuilder.setNncIndex(eclipseSelectionItem->m_nncIndex);
            textBuilder.setIntersectionPoint(eclipseSelectionItem->m_localIntersectionPoint);

            resultInfo = textBuilder.mainResultText();

            pickInfo = textBuilder.geometrySelectionText(", ");
        }
        else if (itemAdded->type() == RiuSelectionItem::GEOMECH_SELECTION_OBJECT)
        {
            const RiuGeoMechSelectionItem* geomSelectionItem = static_cast<const RiuGeoMechSelectionItem*>(itemAdded);

            RimGeoMechView* geomView = geomSelectionItem->m_view.p();
            RiuFemResultTextBuilder textBuilder(geomView, (int)geomSelectionItem->m_gridIndex, (int)geomSelectionItem->m_cellIndex, geomView->currentTimeStep());
            textBuilder.setIntersectionPoint(geomSelectionItem->m_localIntersectionPoint);
            textBuilder.setFace(geomSelectionItem->m_elementFace);
            if (geomSelectionItem->m_hasIntersectionTriangle) textBuilder.setIntersectionTriangle(geomSelectionItem->m_intersectionTriangle);

            resultInfo = textBuilder.mainResultText();

            pickInfo = textBuilder.geometrySelectionText(", ");
        }
    }

    RiuMainWindow* mainWnd = RiuMainWindow::instance();
    mainWnd->statusBar()->showMessage(pickInfo);
    mainWnd->setResultInfo(resultInfo);
}
