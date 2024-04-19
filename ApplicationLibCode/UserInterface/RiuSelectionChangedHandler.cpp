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

#include "RiaResultNames.h"

#include "RigCaseCellResultsData.h"
#include "RigDepthResultAccessor.h"
#include "RigEclipseCaseData.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigTimeHistoryResultAccessor.h"

#include "Rim2dIntersectionView.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimExtrudedCurveIntersection.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimProject.h"

#include "Riu3dSelectionManager.h"
#include "RiuDepthQwtPlot.h"
#include "RiuFemResultTextBuilder.h"
#include "RiuFemTimeHistoryResultAccessor.h"
#include "RiuMainWindow.h"
#include "RiuMohrsCirclePlot.h"
#include "RiuPvtPlotPanel.h"
#include "RiuPvtPlotUpdater.h"
#include "RiuRelativePermeabilityPlotPanel.h"
#include "RiuRelativePermeabilityPlotUpdater.h"
#include "RiuResultQwtPlot.h"
#include "RiuResultTextBuilder.h"
#include "RiuSeismicHistogramPanel.h"

#include <QStatusBar>

#include <cafDisplayCoordTransform.h>

#include <cassert>

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
    if ( !RiuMainWindow::instance() ) return;

    RiuMainWindow::instance()->resultPlot()->deleteAllCurves();
    RiuMainWindow::instance()->depthPlot()->deleteAllCurves();

    RiuRelativePermeabilityPlotUpdater* relPermPlotUpdater = RiuMainWindow::instance()->relativePermeabilityPlotPanel()->plotUpdater();
    relPermPlotUpdater->updateOnSelectionChanged( nullptr );

    RiuPvtPlotUpdater* pvtPlotUpdater = RiuMainWindow::instance()->pvtPlotPanel()->plotUpdater();
    pvtPlotUpdater->updateOnSelectionChanged( nullptr );

    RiuMohrsCirclePlot* mohrsCirclePlot = RiuMainWindow::instance()->mohrsCirclePlot();
    if ( mohrsCirclePlot ) mohrsCirclePlot->clearPlot();

    updateResultInfo( nullptr );

    scheduleUpdateForAllVisibleViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::handleItemAppended( const RiuSelectionItem* item ) const
{
    if ( !RiuMainWindow::instance() ) return;

    addResultCurveFromSelectionItem( item );

    addDepthCurveFromSelectionItem( item );

    RiuRelativePermeabilityPlotUpdater* relPermUpdater = RiuMainWindow::instance()->relativePermeabilityPlotPanel()->plotUpdater();
    relPermUpdater->updateOnSelectionChanged( item );

    RiuPvtPlotUpdater* pvtPlotUpdater = RiuMainWindow::instance()->pvtPlotPanel()->plotUpdater();
    pvtPlotUpdater->updateOnSelectionChanged( item );

    RiuMohrsCirclePlot* mohrsCirclePlot = RiuMainWindow::instance()->mohrsCirclePlot();
    if ( mohrsCirclePlot ) mohrsCirclePlot->appendSelection( item );

    updateResultInfo( item );

    scheduleUpdateForAllVisibleViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::handleSetSelectedItem( const RiuSelectionItem* item ) const
{
    if ( !RiuMainWindow::instance() ) return;

    RiuMainWindow::instance()->resultPlot()->deleteAllCurves();
    RiuMainWindow::instance()->depthPlot()->deleteAllCurves();

    RiuMohrsCirclePlot* mohrsCirclePlot = RiuMainWindow::instance()->mohrsCirclePlot();
    if ( mohrsCirclePlot ) mohrsCirclePlot->clearPlot();

    handleItemAppended( item );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::addResultCurveFromSelectionItem( const RiuEclipseSelectionItem* eclipseSelectionItem ) const
{
    RimEclipseResultDefinition* eclResDef = eclipseSelectionItem->m_resultDefinition;
    if ( !eclResDef ) return;

    if ( eclResDef->isFlowDiagOrInjectionFlooding() && eclResDef->resultVariable() != RIG_NUM_FLOODED_PV )
    {
        // NB! Do not read out data for flow results, as this can be a time consuming operation

        return;
    }
    else if ( eclResDef->hasDynamicResult() && !RiaResultNames::isPerCellFaceResult( eclResDef->resultVariable() ) &&
              eclResDef->eclipseCase() && eclResDef->eclipseCase()->eclipseCaseData() )
    {
        RiaDefines::PorosityModelType porosityModel = eclResDef->porosityModel();

        std::vector<QDateTime> timeStepDates = eclResDef->eclipseCase()->eclipseCaseData()->results( porosityModel )->timeStepDates();

        QString curveName = eclResDef->eclipseCase()->caseUserDescription();
        curveName += ", ";
        curveName += eclResDef->resultVariableUiShortName();
        curveName += ", ";
        curveName += QString( "Grid index %1" ).arg( eclipseSelectionItem->m_gridIndex );
        curveName += ", ";
        curveName += RigTimeHistoryResultAccessor::geometrySelectionText( eclResDef->eclipseCase()->eclipseCaseData(),
                                                                          eclipseSelectionItem->m_gridIndex,
                                                                          eclipseSelectionItem->m_gridLocalCellIndex );

        std::vector<double> timeHistoryValues = RigTimeHistoryResultAccessor::timeHistoryValues( eclResDef->eclipseCase()->eclipseCaseData(),
                                                                                                 eclResDef,
                                                                                                 eclipseSelectionItem->m_gridIndex,
                                                                                                 eclipseSelectionItem->m_gridLocalCellIndex,
                                                                                                 timeStepDates.size() );
        CVF_ASSERT( timeStepDates.size() == timeHistoryValues.size() );

        RiuMainWindow::instance()->resultPlot()->addCurve( eclResDef->eclipseCase(),
                                                           curveName,
                                                           eclipseSelectionItem->m_color,
                                                           timeStepDates,
                                                           timeHistoryValues );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::addResultCurveFromSelectionItem( const RiuGeoMechSelectionItem* geomSelectionItem ) const
{
    RimGeoMechResultDefinition* geomResDef = geomSelectionItem->m_resultDefinition;

    if ( geomResDef && geomResDef->hasResult() && geomResDef->geoMechCase() && geomResDef->geoMechCase()->geoMechData() )
    {
        std::unique_ptr<RiuFemTimeHistoryResultAccessor> timeHistResultAccessor;

        cvf::Vec3d intersectionPointInDomain =
            geomSelectionItem->m_view->displayCoordTransform()->translateToDomainCoord( geomSelectionItem->m_localIntersectionPointInDisplay );

        if ( geomSelectionItem->m_hasIntersectionTriangle )
        {
            timeHistResultAccessor = std::unique_ptr<RiuFemTimeHistoryResultAccessor>(
                new RiuFemTimeHistoryResultAccessor( geomResDef->geoMechCase()->geoMechData(),
                                                     geomResDef->resultAddress(),
                                                     geomSelectionItem->m_gridIndex,
                                                     static_cast<int>( geomSelectionItem->m_cellIndex ),
                                                     geomSelectionItem->m_elementFace,
                                                     intersectionPointInDomain,
                                                     geomSelectionItem->m_intersectionTriangle ) );
        }
        else
        {
            timeHistResultAccessor = std::unique_ptr<RiuFemTimeHistoryResultAccessor>(
                new RiuFemTimeHistoryResultAccessor( geomResDef->geoMechCase()->geoMechData(),
                                                     geomResDef->resultAddress(),
                                                     geomSelectionItem->m_gridIndex,
                                                     static_cast<int>( geomSelectionItem->m_cellIndex ),
                                                     geomSelectionItem->m_elementFace,
                                                     intersectionPointInDomain ) );
        }

        QString curveName;
        curveName.append( geomResDef->geoMechCase()->caseUserDescription() + ", " );

        caf::AppEnum<RigFemResultPosEnum> resPosAppEnum = geomResDef->resultPositionType();
        curveName.append( resPosAppEnum.uiText() + ", " );
        curveName.append( geomResDef->resultFieldUiName() + ", " );
        curveName.append( geomResDef->resultComponentUiName() + " " );

        if ( resPosAppEnum == RIG_ELEMENT_NODAL_FACE )
        {
            if ( geomSelectionItem->m_elementFace >= 0 )
            {
                curveName.append( ", " + caf::AppEnum<cvf::StructGridInterface::FaceType>::textFromIndex( geomSelectionItem->m_elementFace ) );
            }
            else
            {
                curveName.append( ", from N[" + QString::number( timeHistResultAccessor->closestNodeId() ) + "] transformed onto intersection" );
            }
        }
        curveName.append( "\n" );

        curveName.append( timeHistResultAccessor->geometrySelectionText() );

        std::vector<double> timeHistoryValues = timeHistResultAccessor->timeHistoryValues();

        std::vector<QDateTime> dates = geomResDef->geoMechCase()->timeStepDates();
        if ( dates.size() == timeHistoryValues.size() )
        {
            RiuMainWindow::instance()->resultPlot()->addCurve( geomResDef->geoMechCase(),
                                                               curveName,
                                                               geomSelectionItem->m_color,
                                                               dates,
                                                               timeHistoryValues );
        }
        else
        {
            std::vector<double> dummyStepTimes;
            for ( size_t i = 0; i < timeHistoryValues.size(); i++ )
            {
                dummyStepTimes.push_back( i );
            }

            RiuMainWindow::instance()->resultPlot()->addCurve( geomResDef->geoMechCase(),
                                                               curveName,
                                                               geomSelectionItem->m_color,
                                                               dummyStepTimes,
                                                               timeHistoryValues );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::addResultCurveFromSelectionItem( const Riu2dIntersectionSelectionItem* selectionItem ) const
{
    if ( selectionItem->eclipseSelectionItem() )
    {
        addResultCurveFromSelectionItem( selectionItem->eclipseSelectionItem() );
    }
    else if ( selectionItem->geoMechSelectionItem() )
    {
        addResultCurveFromSelectionItem( selectionItem->geoMechSelectionItem() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::addResultCurveFromSelectionItem( const RiuSelectionItem* itemAdded ) const
{
    if ( itemAdded->type() == RiuSelectionItem::ECLIPSE_SELECTION_OBJECT )
    {
        const RiuEclipseSelectionItem* eclipseSelectionItem = static_cast<const RiuEclipseSelectionItem*>( itemAdded );

        addResultCurveFromSelectionItem( eclipseSelectionItem );
    }
    else if ( itemAdded->type() == RiuSelectionItem::GEOMECH_SELECTION_OBJECT )
    {
        const RiuGeoMechSelectionItem* geomSelectionItem = static_cast<const RiuGeoMechSelectionItem*>( itemAdded );

        addResultCurveFromSelectionItem( geomSelectionItem );
    }
    else if ( itemAdded->type() == RiuSelectionItem::INTERSECTION_SELECTION_OBJECT )
    {
        const Riu2dIntersectionSelectionItem* _2dSelectionItem = static_cast<const Riu2dIntersectionSelectionItem*>( itemAdded );

        addResultCurveFromSelectionItem( _2dSelectionItem );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::scheduleUpdateForAllVisibleViews() const
{
    RimProject* proj = RimProject::current();
    if ( proj )
    {
        for ( Rim3dView* visibleView : proj->allVisibleViews() )
        {
            visibleView->createHighlightAndGridBoxDisplayModelAndRedraw();
            visibleView->createMeasurementDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::updateResultInfo( const RiuSelectionItem* itemAdded ) const
{
    QString resultInfo;
    QString pickInfo;

    RiuSelectionItem* selItem = const_cast<RiuSelectionItem*>( itemAdded );
    if ( selItem != nullptr )
    {
        Rim2dIntersectionView* intersectionView = nullptr;

        if ( selItem->type() == RiuSelectionItem::INTERSECTION_SELECTION_OBJECT )
        {
            const Riu2dIntersectionSelectionItem* wrapperSelItem = dynamic_cast<Riu2dIntersectionSelectionItem*>( selItem );
            if ( wrapperSelItem )
            {
                intersectionView = wrapperSelItem->view();
                if ( wrapperSelItem->eclipseSelectionItem() )
                {
                    selItem = wrapperSelItem->eclipseSelectionItem();
                }
                else if ( wrapperSelItem->geoMechSelectionItem() )
                {
                    selItem = wrapperSelItem->geoMechSelectionItem();
                }
            }
        }

        if ( selItem->type() == RiuSelectionItem::ECLIPSE_SELECTION_OBJECT )
        {
            const RiuEclipseSelectionItem* eclipseSelectionItem = static_cast<const RiuEclipseSelectionItem*>( selItem );

            RiuResultTextBuilder textBuilder( eclipseSelectionItem->m_view,
                                              eclipseSelectionItem->m_resultDefinition,
                                              eclipseSelectionItem->m_gridIndex,
                                              eclipseSelectionItem->m_gridLocalCellIndex,
                                              eclipseSelectionItem->m_timestepIdx );

            textBuilder.setFace( eclipseSelectionItem->m_face );
            textBuilder.setNncIndex( eclipseSelectionItem->m_nncIndex );
            textBuilder.setIntersectionPointInDisplay( eclipseSelectionItem->m_localIntersectionPointInDisplay );
            textBuilder.set2dIntersectionView( intersectionView );

            resultInfo = textBuilder.mainResultText();

            pickInfo = textBuilder.geometrySelectionText( ", " );
        }
        else if ( selItem->type() == RiuSelectionItem::GEOMECH_SELECTION_OBJECT )
        {
            const RiuGeoMechSelectionItem* geomSelectionItem = static_cast<const RiuGeoMechSelectionItem*>( selItem );

            RiuFemResultTextBuilder textBuilder( geomSelectionItem->m_view,
                                                 geomSelectionItem->m_resultDefinition,
                                                 (int)geomSelectionItem->m_gridIndex,
                                                 (int)geomSelectionItem->m_cellIndex,
                                                 (int)geomSelectionItem->m_timestepIdx,
                                                 geomSelectionItem->m_frameIdx );

            textBuilder.setIntersectionPointInDisplay( geomSelectionItem->m_localIntersectionPointInDisplay );
            textBuilder.setFace( geomSelectionItem->m_elementFace );
            textBuilder.set2dIntersectionView( intersectionView );

            if ( geomSelectionItem->m_hasIntersectionTriangle )
            {
                textBuilder.setIntersectionTriangle( geomSelectionItem->m_intersectionTriangle );
            }

            resultInfo = textBuilder.mainResultText();

            pickInfo = textBuilder.geometrySelectionText( ", " );
        }
    }

    RiuMainWindow* mainWnd = RiuMainWindow::instance();
    mainWnd->statusBar()->showMessage( pickInfo );
    mainWnd->setResultInfo( resultInfo );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuSelectionChangedHandler::addDepthCurveFromSelectionItem( const RiuSelectionItem* itemAdded ) const
{
    if ( itemAdded->type() != RiuSelectionItem::ECLIPSE_SELECTION_OBJECT ) return;

    const RiuEclipseSelectionItem* eclipseSelectionItem = static_cast<const RiuEclipseSelectionItem*>( itemAdded );
    if ( eclipseSelectionItem == nullptr ) return;

    int currentTimeStep = eclipseSelectionItem->m_view->currentTimeStep();

    RimEclipseResultDefinition* eclResDef = eclipseSelectionItem->m_resultDefinition;
    if ( !eclResDef ) return;

    if ( eclResDef->isFlowDiagOrInjectionFlooding() && eclResDef->resultVariable() != RIG_NUM_FLOODED_PV )
    {
        // NB! Do not read out data for flow results, as this can be a time consuming operation
        return;
    }
    else if ( eclResDef->hasResult() && !RiaResultNames::isPerCellFaceResult( eclResDef->resultVariable() ) && eclResDef->eclipseCase() &&
              eclResDef->eclipseCase()->eclipseCaseData() )
    {
        auto casedata = eclResDef->eclipseCase()->eclipseCaseData();

        QString curveName = eclResDef->resultVariableUiShortName();
        curveName += ", ";
        curveName += RigDepthResultAccessor::geometrySelectionText( casedata,
                                                                    eclipseSelectionItem->m_gridIndex,
                                                                    eclipseSelectionItem->m_gridLocalCellIndex );

        std::vector<double> resultValues = RigDepthResultAccessor::resultValues( casedata,
                                                                                 eclResDef,
                                                                                 static_cast<int>( eclipseSelectionItem->m_gridIndex ),
                                                                                 eclipseSelectionItem->m_gridLocalCellIndex,
                                                                                 currentTimeStep );

        std::vector<int> kValues = RigDepthResultAccessor::kValues( casedata, static_cast<int>( eclipseSelectionItem->m_gridIndex ) );

        std::vector<double> depthValues = RigDepthResultAccessor::depthValues( casedata,
                                                                               static_cast<int>( eclipseSelectionItem->m_gridLocalCellIndex ),
                                                                               static_cast<int>( eclipseSelectionItem->m_gridIndex ) );

        CVF_ASSERT( kValues.size() == resultValues.size() );

        RiuMainWindow::instance()->depthPlot()->addCurve( eclResDef->eclipseCase(),
                                                          curveName,
                                                          eclipseSelectionItem->m_color,
                                                          kValues,
                                                          depthValues,
                                                          resultValues );
    }
}
