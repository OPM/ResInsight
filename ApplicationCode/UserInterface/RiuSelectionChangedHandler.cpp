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
#include "RigCaseData.h"
#include "RigFemTimeHistoryResultAccessor.h"
#include "RigGeoMechCaseData.h"
#include "RigTimeHistoryResultAccessor.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechView.h"
#include "RimProject.h"

#include "RiuFemResultTextBuilder.h"
#include "RiuMainWindow.h"
#include "RiuResultTextBuilder.h"
#include "RiuSelectionManager.h"
#include "RiuResultQwtPlot.h"

#include <QStatusBar>

#include <assert.h>

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

    updateResultInfo(NULL);

    scheduleUpdateForAllVisibleViews();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::handleItemAppended(const RiuSelectionItem* item) const
{
    addCurveFromSelectionItem(item);

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

    if (eclipseView->cellResult()->hasDynamicResult() &&
        eclipseView->eclipseCase() &&
        eclipseView->eclipseCase()->reservoirData())
    {
        RifReaderInterface::PorosityModelResultType porosityModel = RigCaseCellResultsData::convertFromProjectModelPorosityModel(eclipseView->cellResult()->porosityModel());

        size_t scalarIndexWithMaxTimeStepCount = cvf::UNDEFINED_SIZE_T;
        eclipseView->eclipseCase()->reservoirData()->results(porosityModel)->maxTimeStepCount(&scalarIndexWithMaxTimeStepCount);
        std::vector<QDateTime> timeStepDates = eclipseView->eclipseCase()->reservoirData()->results(porosityModel)->timeStepDates(scalarIndexWithMaxTimeStepCount);

        RigTimeHistoryResultAccessor timeHistResultAccessor(eclipseView->eclipseCase()->reservoirData(),
            eclipseSelectionItem->m_gridIndex, eclipseSelectionItem->m_cellIndex,
            eclipseView->cellResult()->scalarResultIndex(), porosityModel);

        QString curveName = eclipseView->eclipseCase()->caseUserDescription();
        curveName += ", ";
        curveName += eclipseView->cellResult()->resultVariable();
        curveName += ", ";
        curveName += QString("Grid index %1").arg(eclipseSelectionItem->m_gridIndex);
        curveName += ", ";
        curveName += timeHistResultAccessor.topologyText();

        std::vector<double> timeHistoryValues = timeHistResultAccessor.timeHistoryValues();
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
        geoMechView->cellResult() &&
        geoMechView->cellResult()->hasResult() &&
        geoMechView->geoMechCase() &&
        geoMechView->geoMechCase()->geoMechData())
    {
        RigFemTimeHistoryResultAccessor timeHistResultAccessor(geoMechView->geoMechCase()->geoMechData(), geoMechView->cellResult->resultAddress(),
            geomSelectionItem->m_gridIndex, geomSelectionItem->m_cellIndex, geomSelectionItem->m_localIntersectionPoint);

        QString curveName;
        curveName.append(geoMechView->geoMechCase()->caseUserDescription() + ", ");

        caf::AppEnum<RigFemResultPosEnum> resPosAppEnum = geoMechView->cellResult()->resultPositionType();
        curveName.append(resPosAppEnum.uiText() + ", ");
        curveName.append(geoMechView->cellResult()->resultFieldUiName()+ ", ") ;
        curveName.append(geoMechView->cellResult()->resultComponentUiName() + ":\n");
        curveName.append(timeHistResultAccessor.topologyText());

        std::vector<double> timeHistoryValues = timeHistResultAccessor.timeHistoryValues();
        std::vector<double> frameTimes;
        for (size_t i = 0; i < timeHistoryValues.size(); i++)
        {
            frameTimes.push_back(i);
        }

        CVF_ASSERT(frameTimes.size() == timeHistoryValues.size());
        
        RiuMainWindow::instance()->resultPlot()->addCurve(curveName, geomSelectionItem->m_color, frameTimes, timeHistoryValues);
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
void RiuSelectionChangedHandler::scheduleUpdateForAllVisibleViews() const
{
    RiuMainWindow* mainWnd = RiuMainWindow::instance();

    RimProject* proj = RiaApplication::instance()->project();
    if (proj)
    {
        std::vector<RimView*> visibleViews;
        proj->allVisibleViews(visibleViews);

        for (size_t i = 0; i < visibleViews.size(); i++)
        {
            visibleViews[i]->createOverlayDisplayModelAndRedraw();
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

            RiuResultTextBuilder textBuilder(eclipseView, eclipseSelectionItem->m_gridIndex, eclipseSelectionItem->m_cellIndex, eclipseView->currentTimeStep());
            textBuilder.setFace(eclipseSelectionItem->m_face);
            textBuilder.setNncIndex(eclipseSelectionItem->m_nncIndex);
            textBuilder.setIntersectionPoint(eclipseSelectionItem->m_localIntersectionPoint);

            resultInfo = textBuilder.mainResultText();

            pickInfo = textBuilder.topologyText(", ");
        }
        else if (itemAdded->type() == RiuSelectionItem::GEOMECH_SELECTION_OBJECT)
        {
            const RiuGeoMechSelectionItem* geomSelectionItem = static_cast<const RiuGeoMechSelectionItem*>(itemAdded);

            RimGeoMechView* geomView = geomSelectionItem->m_view.p();
            RiuFemResultTextBuilder textBuilder(geomView, (int)geomSelectionItem->m_gridIndex, (int)geomSelectionItem->m_cellIndex, geomView->currentTimeStep());
            textBuilder.setIntersectionPoint(geomSelectionItem->m_localIntersectionPoint);

            resultInfo = textBuilder.mainResultText();

            pickInfo = textBuilder.topologyText(", ");
        }
    }

    RiuMainWindow* mainWnd = RiuMainWindow::instance();
    mainWnd->statusBar()->showMessage(pickInfo);
    mainWnd->setResultInfo(resultInfo);
}
