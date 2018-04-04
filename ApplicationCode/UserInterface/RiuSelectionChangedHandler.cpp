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
#include "RiuFemTimeHistoryResultAccessor.h"

#include "Rim2dIntersectionView.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimIntersection.h"
#include "RimProject.h"

#include "RiuFemResultTextBuilder.h"
#include "RiuMainWindow.h"
#include "RiuMohrsCirclePlot.h"
#include "RiuPvtPlotPanel.h"
#include "RiuPvtPlotUpdater.h"
#include "RiuRelativePermeabilityPlotPanel.h"
#include "RiuRelativePermeabilityPlotUpdater.h"
#include "RiuResultQwtPlot.h"
#include "RiuResultTextBuilder.h"
#include "RiuSelectionManager.h"

#include <QStatusBar>

#include <assert.h>



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

    RiuRelativePermeabilityPlotUpdater* relPermPlotUpdater = RiuMainWindow::instance()->relativePermeabilityPlotPanel()->plotUpdater();
    relPermPlotUpdater->updateOnSelectionChanged(nullptr);

    RiuPvtPlotUpdater* pvtPlotUpdater = RiuMainWindow::instance()->pvtPlotPanel()->plotUpdater();
    pvtPlotUpdater->updateOnSelectionChanged(nullptr);

    RiuMohrsCirclePlot* mohrsCirclePlot = RiuMainWindow::instance()->mohrsCirclePlot();
    if (mohrsCirclePlot) mohrsCirclePlot->clearPlot();

    updateResultInfo(nullptr);

    scheduleUpdateForAllVisibleViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::handleItemAppended(const RiuSelectionItem* item) const
{
    addCurveFromSelectionItem(item);

    RiuRelativePermeabilityPlotUpdater* relPermUpdater = RiuMainWindow::instance()->relativePermeabilityPlotPanel()->plotUpdater();
    relPermUpdater->updateOnSelectionChanged(item);

    RiuPvtPlotUpdater* pvtPlotUpdater = RiuMainWindow::instance()->pvtPlotPanel()->plotUpdater();
    pvtPlotUpdater->updateOnSelectionChanged(item);

    RiuMohrsCirclePlot* mohrsCirclePlot = RiuMainWindow::instance()->mohrsCirclePlot();
    if (mohrsCirclePlot) mohrsCirclePlot->appendSelection(item);

    updateResultInfo(item);

    scheduleUpdateForAllVisibleViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::handleSetSelectedItem(const RiuSelectionItem* item) const
{
    RiuMainWindow::instance()->resultPlot()->deleteAllCurves();

    RiuMohrsCirclePlot* mohrsCirclePlot = RiuMainWindow::instance()->mohrsCirclePlot();
    if (mohrsCirclePlot) mohrsCirclePlot->clearPlot();

    handleItemAppended(item);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::addCurveFromSelectionItem(const RiuEclipseSelectionItem* eclipseSelectionItem) const
{
    RimEclipseView* eclipseView = eclipseSelectionItem->m_view.p();

    if (eclipseView->cellResult()->isFlowDiagOrInjectionFlooding() && eclipseView->cellResult()->resultVariable() != RIG_NUM_FLOODED_PV)
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

        RiuMainWindow::instance()->resultPlot()->addCurve(eclipseView->eclipseCase(), curveName, eclipseSelectionItem->m_color, timeStepDates, timeHistoryValues);
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
            RiuMainWindow::instance()->resultPlot()->addCurve(geoMechView->geoMechCase(), curveName, geomSelectionItem->m_color, dates, timeHistoryValues);
        }
        else
        {
            std::vector<double> dummyStepTimes;
            for (size_t i = 0; i < timeHistoryValues.size(); i++)
            {
                dummyStepTimes.push_back(i);
            }

            RiuMainWindow::instance()->resultPlot()->addCurve(geoMechView->geoMechCase(), curveName, geomSelectionItem->m_color, dummyStepTimes, timeHistoryValues);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::addCurveFromSelectionItem(const Riu2dIntersectionSelectionItem* selectionItem) const
{
    if (selectionItem->eclipseSelectionItem())
    {
        addCurveFromSelectionItem(selectionItem->eclipseSelectionItem());
    }
    else if (selectionItem->geoMechSelectionItem())
    {
        addCurveFromSelectionItem(selectionItem->geoMechSelectionItem());
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
    else if (itemAdded->type() == RiuSelectionItem::INTERSECTION_SELECTION_OBJECT)
    {
        const Riu2dIntersectionSelectionItem* _2dSelectionItem = static_cast<const Riu2dIntersectionSelectionItem*>(itemAdded);

        addCurveFromSelectionItem(_2dSelectionItem);
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
        std::vector<Rim3dView*> visibleViews;
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

    RiuSelectionItem* selItem = const_cast<RiuSelectionItem*>(itemAdded);
    if (selItem != nullptr)
    {
        Rim2dIntersectionView* intersectionView = nullptr;

        if (selItem->type() == RiuSelectionItem::INTERSECTION_SELECTION_OBJECT)
        {
            const Riu2dIntersectionSelectionItem* wrapperSelItem = dynamic_cast<Riu2dIntersectionSelectionItem*>(selItem);
            intersectionView = wrapperSelItem->view();
            if (wrapperSelItem && wrapperSelItem->eclipseSelectionItem()) selItem = wrapperSelItem->eclipseSelectionItem();
            else if (wrapperSelItem && wrapperSelItem->geoMechSelectionItem()) selItem = wrapperSelItem->geoMechSelectionItem();
        }

        if (selItem->type() == RiuSelectionItem::ECLIPSE_SELECTION_OBJECT)
        {
            const RiuEclipseSelectionItem* eclipseSelectionItem = static_cast<const RiuEclipseSelectionItem*>(selItem);

            RimEclipseView* eclipseView = eclipseSelectionItem->m_view.p();

            RiuResultTextBuilder textBuilder(eclipseView, eclipseSelectionItem->m_gridIndex, eclipseSelectionItem->m_gridLocalCellIndex, eclipseView->currentTimeStep());
            textBuilder.setFace(eclipseSelectionItem->m_face);
            textBuilder.setNncIndex(eclipseSelectionItem->m_nncIndex);
            textBuilder.setIntersectionPoint(eclipseSelectionItem->m_localIntersectionPoint);
            textBuilder.set2dIntersectionView(intersectionView);

            resultInfo = textBuilder.mainResultText();

            pickInfo = textBuilder.geometrySelectionText(", ");
        }
        else if (selItem->type() == RiuSelectionItem::GEOMECH_SELECTION_OBJECT)
        {
            const RiuGeoMechSelectionItem* geomSelectionItem = static_cast<const RiuGeoMechSelectionItem*>(selItem);

            RimGeoMechView* geomView = geomSelectionItem->m_view.p();
            RiuFemResultTextBuilder textBuilder(geomView, (int)geomSelectionItem->m_gridIndex, (int)geomSelectionItem->m_cellIndex, geomView->currentTimeStep());
            textBuilder.setIntersectionPoint(geomSelectionItem->m_localIntersectionPoint);
            textBuilder.setFace(geomSelectionItem->m_elementFace);
            textBuilder.set2dIntersectionView(intersectionView);
            if (geomSelectionItem->m_hasIntersectionTriangle) textBuilder.setIntersectionTriangle(geomSelectionItem->m_intersectionTriangle);

            resultInfo = textBuilder.mainResultText();

            pickInfo = textBuilder.geometrySelectionText(", ");
        }
    }

    RiuMainWindow* mainWnd = RiuMainWindow::instance();
    mainWnd->statusBar()->showMessage(pickInfo);
    mainWnd->setResultInfo(resultInfo);
}
