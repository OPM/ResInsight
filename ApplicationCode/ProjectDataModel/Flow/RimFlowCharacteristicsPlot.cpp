/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimFlowCharacteristicsPlot.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RifCsvDataTableFormatter.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigFlowDiagResults.h"

#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimFaultInViewCollection.h"
#include "RimFlowDiagSolution.h"
#include "RimProject.h"

#include "RicEclipsePropertyFilterFeatureImpl.h"
#include "RicSelectOrCreateViewFeatureImpl.h"

#include "RiuFlowCharacteristicsPlot.h"
#include "RiuMainWindow.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafUtils.h"

#include <QDateTime>

#include <cmath> // Needed for HUGE_VAL on Linux

namespace caf
{
template <>
void AppEnum<RimFlowCharacteristicsPlot::TimeSelectionType>::setUp()
{
    addItem( RimFlowCharacteristicsPlot::ALL_AVAILABLE, "ALL_AVAILABLE", "All With Calculated Flow Diagnostics" );
    addItem( RimFlowCharacteristicsPlot::SELECTED, "SELECTED", "Selected" );
    setDefault( RimFlowCharacteristicsPlot::SELECTED );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimFlowCharacteristicsPlot, "FlowCharacteristicsPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFlowCharacteristicsPlot::RimFlowCharacteristicsPlot()
{
    CAF_PDM_InitObject( "Flow Characteristics", ":/FlowCharPlot16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_case, "FlowCase", "Case", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_flowDiagSolution, "FlowDiagSolution", "Flow Diag Solution", "", "", "" );
    m_flowDiagSolution.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_timeStepSelectionType, "TimeSelectionType", "Time Steps", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_selectedTimeSteps, "SelectedTimeSteps", "", "", "", "" );
    m_selectedTimeSteps.uiCapability()->setUiHidden( true );
    CAF_PDM_InitFieldNoDefault( &m_selectedTimeStepsUi, "SelectedTimeStepsUi", "", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_applyTimeSteps, "ApplyTimeSteps", "", "", "", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_applyTimeSteps );

    CAF_PDM_InitField( &m_maxPvFraction,
                       "CellPVThreshold",
                       0.1,
                       "Aquifer Cell Threshold",
                       "",
                       "Exclude Aquifer Effects by adding a Cell Pore Volume Threshold as Fraction of Total Pore "
                       "Volume.",
                       "" );

    CAF_PDM_InitField( &m_showLegend, "ShowLegend", true, "Legend", "", "", "" );

    // Region group
    CAF_PDM_InitFieldNoDefault( &m_cellFilter, "CellFilter", "Cell Filter", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_cellFilterView, "CellFilterView", "View", "", "", "" );
    CAF_PDM_InitField( &m_tracerFilter, "TracerFilter", QString(), "Tracer Filter", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_selectedTracerNames, "SelectedTracerNames", " ", "", "", "" );
    m_selectedTracerNames.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );
    CAF_PDM_InitFieldNoDefault( &m_showRegion, "ShowRegion", "", "", "", "" );
    caf::PdmUiPushButtonEditor::configureEditorForField( &m_showRegion );

    CAF_PDM_InitField( &m_minCommunication, "MinCommunication", 0.0, "Min Communication", "", "", "" );
    CAF_PDM_InitField( &m_maxTof, "MaxTof", 146000, "Max Time of Flight [days]", "", "", "" );

    this->m_showWindow = false;
    setAsPlotMdiWindow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFlowCharacteristicsPlot::~RimFlowCharacteristicsPlot()
{
    removeMdiWindowFromMdiArea();

    if ( m_flowCharPlotWidget )
    {
        m_flowCharPlotWidget->deleteLater();
        m_flowCharPlotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// TODO: implement properly
//--------------------------------------------------------------------------------------------------
int RimFlowCharacteristicsPlot::id() const
{
    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFlowCharacteristicsPlot::setFromFlowSolution( RimFlowDiagSolution* flowSolution )
{
    if ( !flowSolution )
    {
        m_case           = nullptr;
        m_cellFilterView = nullptr;
    }
    else
    {
        RimEclipseResultCase* eclCase;
        flowSolution->firstAncestorOrThisOfType( eclCase );
        m_case = eclCase;
        if ( !eclCase->reservoirViews.empty() )
        {
            m_cellFilterView = eclCase->reservoirViews()[0];
        }
    }

    m_flowDiagSolution = flowSolution;
    m_showWindow       = true;
    m_timeStepToFlowResultMap.clear();
    m_currentlyPlottedTimeSteps.clear();

    onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFlowCharacteristicsPlot::deleteViewWidget()
{
    if ( m_flowCharPlotWidget )
    {
        m_flowCharPlotWidget->deleteLater();
        m_flowCharPlotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFlowCharacteristicsPlot::updateCurrentTimeStep()
{
    if ( m_timeStepSelectionType() != ALL_AVAILABLE ) return;
    if ( !m_flowDiagSolution() ) return;

    RigFlowDiagResults* flowResult          = m_flowDiagSolution->flowDiagResults();
    std::vector<int>    calculatedTimesteps = flowResult->calculatedTimeSteps( RigFlowDiagResultAddress::PHASE_ALL );

    if ( m_currentlyPlottedTimeSteps == calculatedTimesteps ) return;

    this->onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFlowCharacteristicsPlot::setTimeSteps( const std::vector<int>& timeSteps )
{
    m_selectedTimeSteps = timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFlowCharacteristicsPlot::setInjectorsAndProducers( const std::vector<QString>& injectors,
                                                           const std::vector<QString>& producers )
{
    std::vector<QString> allTracers;

    allTracers = producers;
    allTracers.insert( allTracers.end(), injectors.begin(), injectors.end() );

    if ( producers.empty() && !injectors.empty() )
    {
        m_cellFilter = RigFlowDiagResults::CELLS_FLOODED;
    }
    else if ( !producers.empty() && injectors.empty() )
    {
        m_cellFilter = RigFlowDiagResults::CELLS_DRAINED;
    }
    else if ( !producers.empty() && !injectors.empty() )
    {
        m_cellFilter = RigFlowDiagResults::CELLS_COMMUNICATION;
    }

    m_selectedTracerNames = allTracers;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFlowCharacteristicsPlot::setMinimumCommunication( double minimumCommunication )
{
    m_minCommunication = minimumCommunication;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFlowCharacteristicsPlot::setAquiferCellThreshold( double aquiferCellThreshold )
{
    m_maxPvFraction = aquiferCellThreshold;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimFlowCharacteristicsPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                       bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_case )
    {
        RimProject* proj = nullptr;
        this->firstAncestorOrThisOfType( proj );
        if ( proj )
        {
            std::vector<RimEclipseResultCase*> cases;
            proj->descendantsIncludingThisOfType( cases );
            for ( RimEclipseResultCase* c : cases )
            {
                if ( c->defaultFlowDiagSolution() )
                {
                    options.push_back( caf::PdmOptionItemInfo( c->caseUserDescription(), c, false, c->uiIconProvider() ) );
                }
            }
        }
    }
    else if ( fieldNeedingOptions == &m_cellFilterView )
    {
        if ( m_case )
        {
            for ( RimEclipseView* view : m_case()->reservoirViews() )
            {
                options.push_back( caf::PdmOptionItemInfo( view->name(), view, false, view->uiIconProvider() ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_flowDiagSolution )
    {
        if ( m_case )
        {
            std::vector<RimFlowDiagSolution*> flowSols = m_case->flowDiagSolutions();

            options.push_back( caf::PdmOptionItemInfo( "None", nullptr ) );
            for ( RimFlowDiagSolution* flowSol : flowSols )
            {
                options.push_back(
                    caf::PdmOptionItemInfo( flowSol->userDescription(), flowSol, false, flowSol->uiIconProvider() ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_selectedTimeStepsUi )
    {
        if ( m_flowDiagSolution && m_case )
        {
            QStringList      timeStepDates       = m_case->timeStepStrings();
            std::vector<int> calculatedTimeSteps = m_flowDiagSolution()->flowDiagResults()->calculatedTimeSteps(
                RigFlowDiagResultAddress::PHASE_ALL );
            for ( int tsIdx = 0; tsIdx < timeStepDates.size(); ++tsIdx )
            {
                auto    it       = std::find( calculatedTimeSteps.begin(), calculatedTimeSteps.end(), tsIdx );
                QString itemText = timeStepDates[tsIdx];
                if ( it != calculatedTimeSteps.end() )
                {
                    itemText = itemText + " *";
                }
                options.push_back( caf::PdmOptionItemInfo( itemText, tsIdx ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_selectedTracerNames )
    {
        if ( m_flowDiagSolution )
        {
            std::vector<QString>                     tracerNames = m_flowDiagSolution->tracerNames();
            std::vector<std::pair<QString, QString>> sortedTracerNames;
            for ( QString tracerName : tracerNames )
            {
                if ( !caf::Utils::isStringMatch( m_tracerFilter, tracerName ) ) continue;

                RimFlowDiagSolution::TracerStatusType tracerStatus = m_flowDiagSolution->tracerStatusOverall( tracerName );
                if ( tracerStatus == RimFlowDiagSolution::CLOSED ) continue;

                if ( m_cellFilter() == RigFlowDiagResults::CELLS_FLOODED )
                {
                    if ( tracerStatus == RimFlowDiagSolution::INJECTOR || tracerStatus == RimFlowDiagSolution::VARYING )
                    {
                        sortedTracerNames.push_back( std::make_pair( tracerName, tracerName ) );
                    }
                }
                else if ( m_cellFilter() == RigFlowDiagResults::CELLS_DRAINED )
                {
                    if ( tracerStatus == RimFlowDiagSolution::PRODUCER || tracerStatus == RimFlowDiagSolution::VARYING )
                    {
                        sortedTracerNames.push_back( std::make_pair( tracerName, tracerName ) );
                    }
                }
                else if ( m_cellFilter() == RigFlowDiagResults::CELLS_COMMUNICATION )
                {
                    QString prefix;
                    switch ( tracerStatus )
                    {
                        case RimFlowDiagSolution::INJECTOR:
                            prefix = "I   : ";
                            break;
                        case RimFlowDiagSolution::PRODUCER:
                            prefix = "P  : ";
                            break;
                        case RimFlowDiagSolution::VARYING:
                            prefix = "I/P: ";
                            break;
                        case RimFlowDiagSolution::UNDEFINED:
                            prefix = "U  : ";
                            break;
                    }
                    sortedTracerNames.push_back( std::make_pair( prefix + tracerName, tracerName ) );
                }
            }

            std::sort( sortedTracerNames.begin(),
                       sortedTracerNames.end(),
                       []( const std::pair<QString, QString>& a, const std::pair<QString, QString>& b ) -> bool {
                           return a.first < b.first;
                       } );

            for ( auto& tracer : sortedTracerNames )
            {
                options.push_back( caf::PdmOptionItemInfo( tracer.first, tracer.second ) );
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFlowCharacteristicsPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    {
        // Ensure a case is selected if one is available
        RimProject* proj = nullptr;
        this->firstAncestorOrThisOfType( proj );
        if ( proj )
        {
            std::vector<RimEclipseResultCase*> cases;
            proj->descendantsIncludingThisOfType( cases );
            RimEclipseResultCase* defaultCase = nullptr;
            for ( RimEclipseResultCase* c : cases )
            {
                if ( c->defaultFlowDiagSolution() )
                {
                    if ( !defaultCase ) defaultCase = c; // Select first
                }
            }
            if ( !m_case() && defaultCase )
            {
                m_case             = defaultCase;
                m_flowDiagSolution = m_case->defaultFlowDiagSolution();
                if ( !m_case()->reservoirViews.empty() )
                {
                    m_cellFilterView = m_case()->reservoirViews()[0];
                }
            }
        }
    }

    uiOrdering.add( &m_case );

    {
        caf::PdmUiGroup* timeStepsGroup = uiOrdering.addNewGroup( "Time Steps" );

        timeStepsGroup->add( &m_timeStepSelectionType );

        if ( m_timeStepSelectionType == SELECTED )
        {
            timeStepsGroup->add( &m_selectedTimeStepsUi );
            timeStepsGroup->add( &m_applyTimeSteps );
        }
    }

    {
        caf::PdmUiGroup* regionGroup = uiOrdering.addNewGroup( "Region" );
        regionGroup->add( &m_cellFilter );
        if ( m_cellFilter() == RigFlowDiagResults::CELLS_COMMUNICATION ||
             m_cellFilter() == RigFlowDiagResults::CELLS_DRAINED || m_cellFilter() == RigFlowDiagResults::CELLS_FLOODED )
        {
            regionGroup->add( &m_tracerFilter );
            regionGroup->add( &m_selectedTracerNames );
            regionGroup->add( &m_showRegion );
        }
        else if ( m_cellFilter() == RigFlowDiagResults::CELLS_VISIBLE )
        {
            regionGroup->add( &m_cellFilterView );
        }

        if ( m_cellFilter() == RigFlowDiagResults::CELLS_COMMUNICATION )
        {
            regionGroup->add( &m_minCommunication );
        }
        else if ( m_cellFilter() == RigFlowDiagResults::CELLS_DRAINED ||
                  m_cellFilter() == RigFlowDiagResults::CELLS_FLOODED )
        {
            regionGroup->add( &m_maxTof );
        }
    }

    {
        caf::PdmUiGroup* optionsGroup = uiOrdering.addNewGroup( "Options" );
        optionsGroup->add( &m_flowDiagSolution );

        optionsGroup->add( &m_showLegend );
        optionsGroup->add( &m_maxPvFraction );
    }

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFlowCharacteristicsPlot::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                        QString                    uiConfigName,
                                                        caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_applyTimeSteps )
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_buttonText = "Apply";
        }
    }
    else if ( field == &m_showRegion )
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_buttonText = "Show Region";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimFlowCharacteristicsPlot::viewWidget()
{
    return m_flowCharPlotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFlowCharacteristicsPlot::zoomAll()
{
    if ( m_flowCharPlotWidget ) m_flowCharPlotWidget->zoomAll();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFlowCharacteristicsPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                   const QVariant&            oldValue,
                                                   const QVariant&            newValue )
{
    RimViewWindow::fieldChangedByUi( changedField, oldValue, newValue );

    if ( &m_case == changedField )
    {
        m_flowDiagSolution = m_case->defaultFlowDiagSolution();
        m_currentlyPlottedTimeSteps.clear();
        if ( !m_case()->reservoirViews.empty() )
        {
            m_cellFilterView = m_case()->reservoirViews()[0];
        }
    }
    else if ( &m_applyTimeSteps == changedField )
    {
        if ( m_flowDiagSolution )
        {
            // Compute any missing time steps from selected
            for ( int tsIdx : m_selectedTimeStepsUi() )
            {
                m_flowDiagSolution()->flowDiagResults()->maxAbsPairFlux( tsIdx );
            }
            m_selectedTimeSteps = m_selectedTimeStepsUi;
        }
        m_applyTimeSteps = false;
    }
    else if ( &m_showRegion == changedField )
    {
        if ( m_case )
        {
            if ( m_cellFilter() != RigFlowDiagResults::CELLS_ACTIVE )
            {
                RimEclipseView* view =
                    RicSelectOrCreateViewFeatureImpl::showViewSelection( m_case,
                                                                         "FlowCharacteristicsLastUsedView",
                                                                         "RegionView",
                                                                         "Show Region in View" );

                if ( view != nullptr )
                {
                    view->faultCollection()->showFaultCollection = false;
                    view->cellResult()->setResultType( RiaDefines::FLOW_DIAGNOSTICS );
                    view->cellResult()->setFlowDiagTracerSelectionType( RimEclipseResultDefinition::FLOW_TR_BY_SELECTION );
                    view->cellResult()->setSelectedTracers( m_selectedTracerNames );

                    if ( m_cellFilter() == RigFlowDiagResults::CELLS_COMMUNICATION )
                    {
                        view->cellResult()->setResultVariable( RIG_FLD_COMMUNICATION_RESNAME );
                    }
                    else
                    {
                        view->cellResult()->setResultVariable( RIG_FLD_TOF_RESNAME );
                    }

                    int timeStep = 0;
                    if ( m_timeStepSelectionType() == ALL_AVAILABLE )
                    {
                        if ( m_flowDiagSolution )
                        {
                            std::vector<int> timeSteps = m_flowDiagSolution()->flowDiagResults()->calculatedTimeSteps(
                                RigFlowDiagResultAddress::PHASE_ALL );
                            if ( !timeSteps.empty() )
                            {
                                timeStep = timeSteps[0];
                            }
                        }
                    }
                    else
                    {
                        if ( !m_selectedTimeStepsUi().empty() )
                        {
                            timeStep = m_selectedTimeStepsUi()[0];
                        }
                    }

                    // Ensure selected time step has computed results
                    m_flowDiagSolution()->flowDiagResults()->maxAbsPairFlux( timeStep );

                    view->setCurrentTimeStep( timeStep );

                    for ( RimEclipsePropertyFilter* f : view->eclipsePropertyFilterCollection()->propertyFilters() )
                    {
                        f->isActive = false;
                    }
                    RicEclipsePropertyFilterFeatureImpl::addPropertyFilter( view->eclipsePropertyFilterCollection() );

                    view->loadDataAndUpdate();
                    m_case->updateConnectedEditors();

                    RicSelectOrCreateViewFeatureImpl::focusView( view );
                }
            }
        }
    }
    else if ( changedField == &m_cellFilter )
    {
        m_selectedTracerNames = std::vector<QString>();
    }

    // All fields update plot

    this->onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimFlowCharacteristicsPlot::snapshotWindowContent()
{
    QImage image;

    if ( m_flowCharPlotWidget )
    {
        QPixmap pix = m_flowCharPlotWidget->grab();
        image       = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFlowCharacteristicsPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    if ( m_flowDiagSolution && m_flowCharPlotWidget )
    {
        RigFlowDiagResults* flowResult = m_flowDiagSolution->flowDiagResults();

        {
            std::vector<int> calculatedTimesteps = flowResult->calculatedTimeSteps( RigFlowDiagResultAddress::PHASE_ALL );

            if ( m_timeStepSelectionType == SELECTED )
            {
                for ( int tsIdx : m_selectedTimeSteps() )
                {
                    m_flowDiagSolution()->flowDiagResults()->maxAbsPairFlux( tsIdx );
                }
                calculatedTimesteps = m_selectedTimeSteps();
            }

            m_currentlyPlottedTimeSteps = calculatedTimesteps;
        }

        std::vector<QDateTime> timeStepDates   = m_case->timeStepDates();
        QStringList            timeStepStrings = m_case->timeStepStrings();
        std::vector<double>    lorenzVals( timeStepDates.size(), HUGE_VAL );

        m_flowCharPlotWidget->removeAllCurves();

        std::vector<QString> selectedTracerNames = m_selectedTracerNames();
        if ( m_cellFilter() == RigFlowDiagResults::CELLS_ACTIVE )
        {
            if ( m_flowDiagSolution )
            {
                selectedTracerNames = m_flowDiagSolution->tracerNames();
            }
        }

        std::map<int, RigFlowDiagSolverInterface::FlowCharacteristicsResultFrame> timeStepToFlowResultMap;

        for ( int timeStepIdx : m_currentlyPlottedTimeSteps )
        {
            if ( m_cellFilter() == RigFlowDiagResults::CELLS_VISIBLE )
            {
                cvf::UByteArray visibleCells;
                m_case()->eclipseCaseData()->activeCellInfo( RiaDefines::MATRIX_MODEL );

                if ( m_cellFilterView )
                {
                    m_cellFilterView()->calculateCurrentTotalCellVisibility( &visibleCells, timeStepIdx );
                }

                RigActiveCellInfo* activeCellInfo = m_case()->eclipseCaseData()->activeCellInfo(
                    RiaDefines::MATRIX_MODEL );
                std::vector<char> visibleActiveCells( activeCellInfo->reservoirActiveCellCount(), 0 );

                for ( size_t i = 0; i < visibleCells.size(); ++i )
                {
                    size_t cellIndex = activeCellInfo->cellResultIndex( i );
                    if ( cellIndex != cvf::UNDEFINED_SIZE_T )
                    {
                        visibleActiveCells[cellIndex] = visibleCells[i];
                    }
                }

                auto flowCharResults                 = flowResult->flowCharacteristicsResults( timeStepIdx,
                                                                               visibleActiveCells,
                                                                               m_maxPvFraction() );
                timeStepToFlowResultMap[timeStepIdx] = flowCharResults;
            }
            else
            {
                auto flowCharResults                 = flowResult->flowCharacteristicsResults( timeStepIdx,
                                                                               m_cellFilter(),
                                                                               selectedTracerNames,
                                                                               m_maxPvFraction(),
                                                                               m_minCommunication(),
                                                                               m_maxTof() );
                timeStepToFlowResultMap[timeStepIdx] = flowCharResults;
            }
            lorenzVals[timeStepIdx] = timeStepToFlowResultMap[timeStepIdx].m_lorenzCoefficient;
        }

        m_timeStepToFlowResultMap = timeStepToFlowResultMap;

        m_flowCharPlotWidget->setLorenzCurve( timeStepStrings, timeStepDates, lorenzVals );

        for ( int timeStepIdx : m_currentlyPlottedTimeSteps )
        {
            const auto& flowCharResults = timeStepToFlowResultMap[timeStepIdx];

            m_flowCharPlotWidget->addFlowCapStorageCapCurve( timeStepDates[timeStepIdx],
                                                             flowCharResults.m_storageCapFlowCapCurve.first,
                                                             flowCharResults.m_storageCapFlowCapCurve.second );
            m_flowCharPlotWidget->addSweepEfficiencyCurve( timeStepDates[timeStepIdx],
                                                           flowCharResults.m_dimensionlessTimeSweepEfficiencyCurve.first,
                                                           flowCharResults.m_dimensionlessTimeSweepEfficiencyCurve.second );
        }

        m_flowCharPlotWidget->showLegend( m_showLegend() );
    }
}

//--------------------------------------------------------------------------------------------------
/// TODO: implement properly
//--------------------------------------------------------------------------------------------------
void RimFlowCharacteristicsPlot::assignIdIfNecessary() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFlowCharacteristicsPlot::viewGeometryUpdated()
{
    if ( m_cellFilter() == RigFlowDiagResults::CELLS_VISIBLE )
    {
        // Only need to reload data if cell filtering is based on visible cells in view.
        onLoadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double interpolate( const std::vector<double>& xData, const std::vector<double>& yData, double x, bool extrapolate )
{
    size_t itemCount = xData.size();

    size_t index = 0;
    if ( x >= xData[itemCount - 2] )
    {
        index = itemCount - 2;
    }
    else
    {
        while ( x > xData[index + 1] )
            index++;
    }
    double xLeft  = xData[index];
    double yLeft  = yData[index];
    double xRight = xData[index + 1];
    double yRight = yData[index + 1];

    if ( !extrapolate )
    {
        if ( x < xLeft ) yRight = yLeft;
        if ( x > xRight ) yLeft = yRight;
    }

    double dydx = ( yRight - yLeft ) / ( xRight - xLeft );

    return yLeft + dydx * ( x - xLeft );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFlowCharacteristicsPlot::curveDataAsText() const
{
    QString fieldSeparator = RiaApplication::instance()->preferences()->csvTextExportFieldSeparator;
    QString tableText;

    QTextStream              stream( &tableText );
    RifCsvDataTableFormatter formatter( stream, fieldSeparator );

    std::vector<RifTextDataTableColumn> header = {
        RifTextDataTableColumn( "Date" ),
        RifTextDataTableColumn( "StorageCapacity" ),
        RifTextDataTableColumn( "FlowCapacity" ),
        RifTextDataTableColumn( "SweepEfficiency" ),
        RifTextDataTableColumn( "DimensionlessTime" ),
        RifTextDataTableColumn( "LorentzCoefficient" ),
    };

    formatter.header( header );

    std::vector<QDateTime> timeStepDates = m_case->timeStepDates();

    std::vector<double> storageCapacitySamplingValues = { 0.08, 0.1, 0.2, 0.3, 0.4 };
    size_t              sampleCount                   = storageCapacitySamplingValues.size();

    for ( const auto& timeIndex : m_currentlyPlottedTimeSteps )
    {
        QString dateString = timeStepDates[timeIndex].toString( "yyyy-MM-dd" );

        auto a = m_timeStepToFlowResultMap.find( timeIndex );
        if ( a != m_timeStepToFlowResultMap.end() )
        {
            auto storageCapacityValues = a->second.m_storageCapFlowCapCurve.first;
            auto flowCapacityValues    = a->second.m_storageCapFlowCapCurve.second;

            bool                extrapolate = false;
            std::vector<double> flowCapacitySamplingValues;
            for ( const auto storageCapacity : storageCapacitySamplingValues )
            {
                {
                    double flowCapacity = interpolate( storageCapacityValues,
                                                       flowCapacityValues,
                                                       storageCapacity,
                                                       extrapolate );
                    flowCapacitySamplingValues.push_back( flowCapacity );
                }
            }

            auto dimensionLessTimeValues = a->second.m_dimensionlessTimeSweepEfficiencyCurve.first;
            auto sweepEffValues          = a->second.m_dimensionlessTimeSweepEfficiencyCurve.second;

            std::vector<double> dimensionLessTimeSamplingValues;
            std::vector<double> sweepEffSamplingValues;
            double              range = dimensionLessTimeValues.back() - dimensionLessTimeValues[0];
            double              step  = range / sampleCount;
            for ( size_t i = 0; i < sampleCount; i++ )
            {
                double dimensionLessTimeValue = i * step;
                dimensionLessTimeSamplingValues.push_back( dimensionLessTimeValue );
                double sweepEffValue = interpolate( dimensionLessTimeValues,
                                                    sweepEffValues,
                                                    dimensionLessTimeValue,
                                                    extrapolate );
                sweepEffSamplingValues.push_back( sweepEffValue );
            }

            auto lorentz = a->second.m_lorenzCoefficient;

            for ( size_t i = 0; i < sampleCount; i++ )
            {
                formatter.add( dateString );
                formatter.add( storageCapacitySamplingValues[i] );
                formatter.add( flowCapacitySamplingValues[i] );
                formatter.add( sweepEffSamplingValues[i] );
                formatter.add( dimensionLessTimeSamplingValues[i] );
                formatter.add( lorentz );
                formatter.rowCompleted();
            }
        }
    }

    formatter.tableCompleted();

    return tableText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimFlowCharacteristicsPlot::createViewWidget( QWidget* mainWindowParent )
{
    m_flowCharPlotWidget = new RiuFlowCharacteristicsPlot( this, mainWindowParent );
    return m_flowCharPlotWidget;
}
