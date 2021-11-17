/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimWellDistributionPlotCollection.h"
#include "RimEclipseResultCase.h"
#include "RimFlowDiagSolution.h"
#include "RimPlot.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellDistributionPlot.h"

#include "RigEclipseCaseData.h"
#include "RigTofWellDistributionCalculator.h"

#include "RiaColorTools.h"

#include "RiuMultiPlotPage.h"
#include "RiuQwtPlotTools.h"

#include "qwt_legend.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"

#include <QGridLayout>
#include <QTextBrowser>
#include <QWidget>

//#include "cvfBase.h"
//#include "cvfTrace.h"
//#include "cvfDebugTimer.h"

//==================================================================================================
//
//
//
//==================================================================================================

CAF_PDM_SOURCE_INIT( RimWellDistributionPlotCollection, "WellDistributionPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellDistributionPlotCollection::RimWellDistributionPlotCollection()
{
    // cvf::Trace::show("RimWellDistributionPlotCollection::RimWellDistributionPlotCollection()");

    CAF_PDM_InitObject( "Cumulative Phase Distribution Plot", ":/CumulativePhaseDist16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_case, "Case", "Case" );
    CAF_PDM_InitField( &m_timeStepIndex, "TimeStepIndex", -1, "Time Step" );
    CAF_PDM_InitField( &m_wellName, "WellName", QString( "None" ), "Well" );
    CAF_PDM_InitField( &m_groupSmallContributions, "GroupSmallContributions", true, "Group Small Contributions" );
    CAF_PDM_InitField( &m_smallContributionsRelativeThreshold,
                       "SmallContributionsRelativeThreshold",
                       0.005,
                       "Relative Threshold [0, 1]",
                       "",
                       "",
                       "" );

    CAF_PDM_InitField( &m_maximumTof, "MaximumTOF", 20.0, "Maximum Time of Flight [0, 200]" );

    CAF_PDM_InitFieldNoDefault( &m_plots, "Plots", "" );
    m_plots.uiCapability()->setUiTreeHidden( true );
    m_plots.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitField( &m_showOil, "ShowOil", true, "Show Oil" );
    CAF_PDM_InitField( &m_showGas, "ShowGas", true, "Show Gas" );
    CAF_PDM_InitField( &m_showWater, "ShowWater", true, "Show Water" );

    CAF_PDM_InitField( &m_plotWindowTitle,
                       "PlotDescription",
                       QString( "Cumulative Phase Distribution Plots" ),
                       "Name",
                       "",
                       "",
                       "" );

    m_showWindow = false;

    setAsPlotMdiWindow();

    addPlot( new RimWellDistributionPlot( RiaDefines::PhaseType::OIL_PHASE ) );
    addPlot( new RimWellDistributionPlot( RiaDefines::PhaseType::GAS_PHASE ) );
    addPlot( new RimWellDistributionPlot( RiaDefines::PhaseType::WATER_PHASE ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellDistributionPlotCollection::~RimWellDistributionPlotCollection()
{
    removeMdiWindowFromMdiArea();
    m_plots.deleteAllChildObjects();

    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlotCollection::setData( RimEclipseResultCase* eclipseCase, QString wellName, int timeStepIndex )
{
    m_case          = eclipseCase;
    m_wellName      = wellName;
    m_timeStepIndex = timeStepIndex;

    applyPlotParametersToContainedPlots();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimWellDistributionPlotCollection::viewWidget()
{
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellDistributionPlotCollection::description() const
{
    return m_plotWindowTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimWellDistributionPlotCollection::snapshotWindowContent()
{
    QImage image;

    if ( m_viewer )
    {
        QPixmap pix( m_viewer->size() );
        m_viewer->renderTo( &pix );
        image = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlotCollection::zoomAll()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellDistributionPlotCollection::userDescriptionField()
{
    return &m_plotWindowTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlotCollection::onLoadDataAndUpdate()
{
    // cvf::Trace::show("RimWellDistributionPlotCollection::onLoadDataAndUpdate()");
    updateMdiWindowVisibility();
    updatePlots();
    updateLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimWellDistributionPlotCollection::createViewWidget( QWidget* mainWindowParent )
{
    m_viewer = new RiuMultiPlotPage( this, mainWindowParent );
    m_viewer->setPlotTitle( m_plotWindowTitle );
    recreatePlotWidgets();

    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlotCollection::deleteViewWidget()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlotCollection::doRenderWindowContent( QPaintDevice* paintDevice )
{
    if ( m_viewer )
    {
        m_viewer->renderTo( paintDevice );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlotCollection::addPlot( RimQwtPlot* plot )
{
    if ( plot )
    {
        size_t index = m_plots.size();
        m_plots.insert( index, plot );

        if ( m_viewer )
        {
            plot->createPlotWidget();
            m_viewer->insertPlot( plot->viewer(), index );
        }
        plot->setShowWindow( true );
        plot->setLegendsVisible( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlotCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_case );
    uiOrdering.add( &m_timeStepIndex );
    uiOrdering.add( &m_wellName );
    uiOrdering.add( &m_groupSmallContributions );
    uiOrdering.add( &m_smallContributionsRelativeThreshold );
    uiOrdering.add( &m_maximumTof );

    uiOrdering.add( &m_showOil );
    uiOrdering.add( &m_showGas );
    uiOrdering.add( &m_showWater );

    m_smallContributionsRelativeThreshold.uiCapability()->setUiReadOnly( m_groupSmallContributions == false );

    // RimMultiPlotWindow::defineUiOrdering(uiConfigName, uiOrdering);
    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimWellDistributionPlotCollection::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                              bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options = RimPlotWindow::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );

    if ( fieldNeedingOptions == &m_case )
    {
        RimProject* ownerProj = nullptr;
        firstAncestorOrThisOfType( ownerProj );
        if ( ownerProj )
        {
            std::vector<RimEclipseResultCase*> caseArr;
            ownerProj->descendantsIncludingThisOfType( caseArr );
            for ( RimEclipseResultCase* c : caseArr )
            {
                options.push_back( caf::PdmOptionItemInfo( c->caseUserDescription(), c, true, c->uiIconProvider() ) );
            }
        }
    }

    else if ( fieldNeedingOptions == &m_timeStepIndex )
    {
        RimTools::timeStepsForCase( m_case, &options );

        if ( options.size() == 0 )
        {
            options.push_back( caf::PdmOptionItemInfo( "None", -1 ) );
        }
    }

    else if ( fieldNeedingOptions == &m_wellName )
    {
        if ( m_case && m_case->eclipseCaseData() )
        {
            caf::IconProvider       simWellIcon( ":/Well.svg" );
            const std::set<QString> sortedWellNameSet = m_case->eclipseCaseData()->findSortedWellNames();
            for ( const QString& name : sortedWellNameSet )
            {
                options.push_back( caf::PdmOptionItemInfo( name, name, true, simWellIcon ) );
            }
        }

        if ( options.size() == 0 )
        {
            options.push_back( caf::PdmOptionItemInfo( "None", QVariant() ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlotCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                          const QVariant&            oldValue,
                                                          const QVariant&            newValue )
{
    if ( changedField == &m_case )
    {
        fixupDependentFieldsAfterCaseChange();
    }

    bool shouldRecalculatePlotData = false;
    if ( changedField == &m_case || changedField == &m_timeStepIndex || changedField == &m_wellName ||
         changedField == &m_groupSmallContributions || changedField == &m_smallContributionsRelativeThreshold ||
         changedField == &m_maximumTof || changedField == &m_showOil || changedField == &m_showGas ||
         changedField == &m_showWater || changedField == &m_showWindow )
    {
        applyPlotParametersToContainedPlots();
        shouldRecalculatePlotData = true;
    }

    RimPlotWindow::fieldChangedByUi( changedField, oldValue, newValue );

    if ( shouldRecalculatePlotData )
    {
        loadDataAndUpdate();
        updateLayout();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlotCollection::applyPlotParametersToContainedPlots()
{
    const size_t numPlots = m_plots.size();
    for ( size_t i = 0; i < numPlots; i++ )
    {
        // Dirty usage of dyn_cast, but type is lost when adding the plots to our base class
        RimWellDistributionPlot* aPlot = dynamic_cast<RimWellDistributionPlot*>( m_plots[i] );
        if ( aPlot )
        {
            if ( aPlot->phase() == RiaDefines::PhaseType::OIL_PHASE )
            {
                aPlot->setShowWindow( m_showOil );
            }
            else if ( aPlot->phase() == RiaDefines::PhaseType::GAS_PHASE )
            {
                aPlot->setShowWindow( m_showGas );
            }
            else if ( aPlot->phase() == RiaDefines::PhaseType::WATER_PHASE )
            {
                aPlot->setShowWindow( m_showWater );
            }

            aPlot->setDataSourceParameters( m_case, m_timeStepIndex, m_wellName );
            aPlot->setPlotOptions( m_groupSmallContributions, m_smallContributionsRelativeThreshold, m_maximumTof );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlotCollection::updatePlots()
{
    if ( m_showWindow )
    {
        for ( RimQwtPlot* plot : m_plots() )
        {
            plot->loadDataAndUpdate();
            plot->updateZoomInParentPlot();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlotCollection::cleanupBeforeClose()
{
    auto plotVector = m_plots.childObjects();
    for ( size_t tIdx = 0; tIdx < plotVector.size(); ++tIdx )
    {
        plotVector[tIdx]->detachAllCurves();
    }

    if ( m_viewer )
    {
        m_viewer->setParent( nullptr );
        delete m_viewer;
        m_viewer = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlotCollection::recreatePlotWidgets()
{
    CVF_ASSERT( m_viewer );

    for ( auto plot : m_plots() )
    {
        plot->createPlotWidget();
        m_viewer->addPlot( plot->viewer() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellDistributionPlotCollection::fixupDependentFieldsAfterCaseChange()
{
    int     newTimeStepIndex = -1;
    QString newWellName;

    if ( m_case )
    {
        const int timeStepCount = m_case->timeStepStrings().size();
        if ( timeStepCount > 0 )
        {
            newTimeStepIndex = timeStepCount - 1;
        }

        const std::set<QString> sortedWellNameSet = m_case->eclipseCaseData()->findSortedWellNames();
        if ( sortedWellNameSet.size() > 0 )
        {
            newWellName = *sortedWellNameSet.begin();
        }
    }

    m_timeStepIndex = newTimeStepIndex;
    m_wellName      = newWellName;
}
