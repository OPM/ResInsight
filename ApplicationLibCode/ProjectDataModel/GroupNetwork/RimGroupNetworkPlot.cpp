/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimGroupNetworkPlot.h"

#include "RimGroupNetworkData.h"
#include "RimGroupNetworkDataCollection.h"
#include "RimGroupNetworkNode.h"

#include "RiaPreferences.h"
#include "Summary/RiaSummaryPlotTools.h"

#include "RifSummaryReaderInterface.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleStatistics.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryPlot.h"

#include "RiuGroupNetworkWidget.h"
#include "RiuPlotMainWindowTools.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafFontTools.h"

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <set>

namespace caf
{

template <>
void AppEnum<RifGroupNetworkSummaryData::EdgeMetric>::setUp()
{
    using Metric = RifGroupNetworkSummaryData::EdgeMetric;
    addItem( Metric::OIL_RATE, "OIL_RATE", "Oil Rate" );
    addItem( Metric::GAS_RATE, "GAS_RATE", "Gas Rate" );
    addItem( Metric::WATER_RATE, "WATER_RATE", "Water Rate" );
    addItem( Metric::WATER_INJ_RATE, "WATER_INJ_RATE", "Water Injection Rate" );
    addItem( Metric::GAS_INJ_RATE, "GAS_INJ_RATE", "Gas Injection Rate" );
    setDefault( Metric::OIL_RATE );
}

template <>
void AppEnum<RifGroupNetworkSummaryData::NodeMetric>::setUp()
{
    using Metric = RifGroupNetworkSummaryData::NodeMetric;
    addItem( Metric::PRESSURE, "PRESSURE", "Pressure" );
    addItem( Metric::BHP, "BHP", "Bottom Hole Pressure" );
    addItem( Metric::CONTROL_MODE, "CONTROL_MODE", "Well Control Mode" );
    setDefault( Metric::PRESSURE );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimGroupNetworkPlot, "RimGroupNetworkPlot" );

namespace
{
QString dateText( std::time_t date )
{
    return RimGroupNetworkData::dateText( date );
}

QString valueWithUnit( std::optional<double> value, const std::string& unit )
{
    if ( !value ) return {};
    QString text = RimGroupNetworkGraph::formatValue( *value );
    if ( !unit.empty() ) text += " " + QString::fromStdString( unit );
    return text;
}

// The colours used for the edge values in the group tree plot of webviz
QColor edgeMetricColor( RifGroupNetworkSummaryData::EdgeMetric metric )
{
    using Metric = RifGroupNetworkSummaryData::EdgeMetric;
    switch ( metric )
    {
        case Metric::OIL_RATE:
            return QColor( "#60be6c" );
        case Metric::GAS_RATE:
            return QColor( "#c5221c" );
        case Metric::WATER_RATE:
            return QColor( "#0d1b9e" );
        case Metric::WATER_INJ_RATE:
            return QColor( "#00c3ff" );
        case Metric::GAS_INJ_RATE:
            return QColor( "#d6397a" );
    }
    return QColor( "#5c5c5c" );
}

const std::vector<RifGroupNetworkSummaryData::EdgeMetric> allEdgeMetrics = { RifGroupNetworkSummaryData::EdgeMetric::OIL_RATE,
                                                                             RifGroupNetworkSummaryData::EdgeMetric::GAS_RATE,
                                                                             RifGroupNetworkSummaryData::EdgeMetric::WATER_RATE,
                                                                             RifGroupNetworkSummaryData::EdgeMetric::WATER_INJ_RATE,
                                                                             RifGroupNetworkSummaryData::EdgeMetric::GAS_INJ_RATE };

const std::vector<RifGroupNetworkSummaryData::NodeMetric> allNodeMetrics = { RifGroupNetworkSummaryData::NodeMetric::PRESSURE,
                                                                             RifGroupNetworkSummaryData::NodeMetric::BHP,
                                                                             RifGroupNetworkSummaryData::NodeMetric::CONTROL_MODE };
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGroupNetworkPlot::RimGroupNetworkPlot()
    : networkChanged( this )
{
    CAF_PDM_InitObject( "Production Network", ":/summary/components/images/production-network.svg" );
    setDeletable( true );

    CAF_PDM_InitField( &m_name, "Name", QString( "Production Network" ), "Name" );
    CAF_PDM_InitFieldNoDefault( &m_data, "Data", "Data Source" );
    CAF_PDM_InitFieldNoDefault( &m_ensembleValue, "EnsembleValue", "Show" );
    CAF_PDM_InitFieldNoDefault( &m_realization, "Realization", "Realization" );
    CAF_PDM_InitFieldNoDefault( &m_edgeMetric, "EdgeMetric", "Edge Value" );
    CAF_PDM_InitFieldNoDefault( &m_nodeMetric, "NodeMetric", "Node Value" );
    CAF_PDM_InitField( &m_showProducers, "ShowProducers", true, "Producers" );
    CAF_PDM_InitField( &m_showInjectors, "ShowInjectors", true, "Injectors" );
    CAF_PDM_InitField( &m_showOther, "ShowOther", true, "Other" );
    CAF_PDM_InitField( &m_timeStepIndex, "TimeStepIndex", 0, "Time Step" );
    CAF_PDM_InitFieldNoDefault( &m_collapsedNodes, "CollapsedNodes", "Collapsed Nodes" );
    m_collapsedNodes.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_selectedNode, "SelectedNode", "Selected Node" );
    m_selectedNode = new RimGroupNetworkNode();
    m_selectedNode.uiCapability()->setUiHidden( true );
    m_selectedNode.xmlCapability()->disableIO();

    dockAsPlotWindow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGroupNetworkPlot::~RimGroupNetworkPlot()
{
    removeWindowFromDock();
    deleteViewWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimGroupNetworkPlot::id() const
{
    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimGroupNetworkPlot::viewWidget()
{
    return m_viewWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlot::zoomAll()
{
    if ( m_viewWidget ) m_viewWidget->zoomAll();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimGroupNetworkPlot::fontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlot::updateFonts()
{
}

//--------------------------------------------------------------------------------------------------
/// The window title shows the statistic or the realization for an ensemble
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlot::updateWindowTitle()
{
    QString title = windowTitle();
    if ( isEnsemble() ) title += " - " + ensembleValueText();
    updateWindowTitleIcon( title );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimGroupNetworkPlot::createViewWidget( QWidget* mainWindowParent )
{
    m_viewWidget = new RiuGroupNetworkWidget( mainWindowParent );
    m_viewWidget->setNetwork( this );

    QObject::connect( m_viewWidget,
                      &RiuGroupNetworkWidget::nodeSelected,
                      m_viewWidget,
                      [this]( const QString& nodeName ) { selectNode( nodeName ); } );

    return m_viewWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlot::deleteViewWidget()
{
    if ( m_viewWidget )
    {
        m_viewWidget->hide();
        m_viewWidget->setParent( nullptr );
        delete m_viewWidget;
        m_viewWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlot::onLoadDataAndUpdate()
{
    updateDockWindowVisibility();
    updateWindowTitle();
    networkChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlot::assignIdIfNecessary()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGroupNetworkPlot::name() const
{
    return m_name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlot::setDataSource( RimGroupNetworkData* data )
{
    disconnectFromData( m_data() );
    m_data = data;
    if ( data ) m_name = data->name();
    connectToData();
    resetTimeStep();
    updateWindowTitle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGroupNetworkGraph::Node> RimGroupNetworkPlot::graphNodes() const
{
    const RigGroupNetwork::TreeRevision* tree = currentTree();
    if ( !tree ) return {};

    const size_t timeStep = currentSummaryTimeStep();

    const auto* data      = summaryData();
    const auto  edgeUnit  = data ? data->edgeUnit( m_edgeMetric() ) : std::string();
    const auto  nodeUnit  = data ? data->nodeUnit( m_nodeMetric() ) : std::string();
    const auto  edgeColor = edgeMetricColor( m_edgeMetric() );

    if ( data && data->isStatistic() )
    {
        // Compute the statistics of all nodes in one operation, as the realizations are then read in parallel
        std::vector<RifGroupNetworkSummaryData::NodeId> nodeIds;
        nodeIds.reserve( tree->edges.size() );
        for ( const RigGroupNetwork::Edge& edge : tree->edges )
        {
            nodeIds.emplace_back( edge.child, edge.isWell() );
        }
        data->prefetch( nodeIds, { m_edgeMetric() }, { m_nodeMetric() } );
    }

    std::vector<RimGroupNetworkGraph::Node> nodes;
    for ( const RigGroupNetwork::Edge& edge : tree->edges )
    {
        RimGroupNetworkGraph::Node node;
        node.name      = QString::fromStdString( edge.child );
        node.parent    = QString::fromStdString( edge.parent );
        node.isWell    = edge.isWell();
        node.edgeColor = edgeColor;

        if ( data )
        {
            const auto category = data->category( edge.child, edge.isWell() );
            node.isProducer     = category.isProducer;
            node.isInjector     = category.isInjector;

            node.edgeValue = data->edgeValue( edge.child, edge.isWell(), m_edgeMetric(), timeStep );
            node.edgeText  = valueWithUnit( node.edgeValue, edgeUnit );

            const auto nodeValue = data->nodeValue( edge.child, edge.isWell(), m_nodeMetric(), timeStep );
            node.valueUnit       = QString::fromStdString( nodeUnit );
            node.valueNumber     = nodeValue ? RimGroupNetworkGraph::formatValue( *nodeValue ) : QString();
            node.valueText       = valueWithUnit( nodeValue, nodeUnit );
        }

        node.toolTip = toolTip( data, edge, node );
        nodes.push_back( node );
    }

    RimGroupNetworkGraph::Filter filter;
    filter.showProducers = m_showProducers();
    filter.showInjectors = m_showInjectors();
    filter.showOther     = m_showOther();
    filter.collapsedNodes.insert( m_collapsedNodes().begin(), m_collapsedNodes().end() );

    return RimGroupNetworkGraph::visibleNodes( nodes, filter );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGroupNetworkPlot::statusText() const
{
    if ( !m_data ) return "No data source selected";

    QString text = m_data->statusText( currentTree() );
    if ( isEnsemble() ) text += QString( ". Showing %1" ).arg( ensembleValueText() );
    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimGroupNetworkPlot::timeStepTexts() const
{
    QStringList texts;
    for ( std::time_t date : timeSteps() )
    {
        texts.append( dateText( date ) );
    }
    return texts;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimGroupNetworkPlot::timeStepIndex() const
{
    const int count = static_cast<int>( timeSteps().size() );
    return count == 0 ? 0 : std::clamp( m_timeStepIndex(), 0, count - 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlot::setTimeStepIndex( int index )
{
    const int count = static_cast<int>( timeSteps().size() );
    index           = count == 0 ? 0 : std::clamp( index, 0, count - 1 );
    if ( index == m_timeStepIndex() ) return;

    m_timeStepIndex = index;
    m_timeStepIndex.uiCapability()->updateConnectedEditors();
    networkChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGroupNetworkPlot::isCollapsed( const QString& nodeName ) const
{
    const auto& collapsed = m_collapsedNodes();
    return std::find( collapsed.begin(), collapsed.end(), nodeName ) != collapsed.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlot::setCollapsed( const QString& nodeName, bool collapsed )
{
    std::vector<QString> nodes = m_collapsedNodes();
    nodes.erase( std::remove( nodes.begin(), nodes.end(), nodeName ), nodes.end() );
    if ( collapsed ) nodes.push_back( nodeName );
    m_collapsedNodes = nodes;
    networkChanged.send();
}

//--------------------------------------------------------------------------------------------------
/// Collapse all nodes below the root, so only the root and its children are shown
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlot::collapseAll()
{
    std::vector<QString> nodes;
    if ( const RigGroupNetwork::TreeRevision* tree = currentTree() )
    {
        std::set<std::string> parents;
        for ( const RigGroupNetwork::Edge& edge : tree->edges )
        {
            parents.insert( edge.parent );
        }
        for ( const RigGroupNetwork::Edge& edge : tree->edges )
        {
            if ( !edge.isRoot() && parents.count( edge.child ) > 0 ) nodes.push_back( QString::fromStdString( edge.child ) );
        }
    }
    m_collapsedNodes = nodes;
    networkChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlot::expandAll()
{
    m_collapsedNodes = std::vector<QString>();
    networkChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlot::selectNode( const QString& nodeName )
{
    const RigGroupNetwork::Edge* edge = nodeName.isEmpty() ? nullptr : findEdge( nodeName );
    if ( !edge )
    {
        m_selectedNode->clear();
        m_selectedNode->updateConnectedEditors();
        m_selectedNode.uiCapability()->updateConnectedEditors();
        return;
    }

    QString     nodeType = edge->isWell() ? "Well" : "Group";
    const auto* data     = summaryData();
    if ( data )
    {
        const auto category = data->category( edge->child, edge->isWell() );
        if ( category.isProducer && category.isInjector )
            nodeType += " (producer and injector)";
        else if ( category.isProducer )
            nodeType += " (producer)";
        else if ( category.isInjector )
            nodeType += " (injector)";
    }

    m_selectedNode->setNodeName( nodeName );
    m_selectedNode->setDescription( nodeType,
                                    QString::fromStdString( edge->parent ),
                                    QString::fromStdString( RigGroupNetwork::keywordText( edge->keyword ) ) );
    m_selectedNode->setValues( propertiesText( *edge ), valueTexts( data, *edge ) );
    m_selectedNode->updateConnectedEditors();
    m_selectedNode.uiCapability()->updateConnectedEditors();

    RiuPlotMainWindowTools::selectAsCurrentItem( m_selectedNode() );
}

//--------------------------------------------------------------------------------------------------
/// Plot the rates and pressures of a node. For an ensemble, the realizations are plotted with the ensemble statistics. The
/// realization curves are hidden if a statistic is shown in the network.
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlot::createSummaryPlot( const QString& nodeName )
{
    const RigGroupNetwork::Edge* edge = findEdge( nodeName );
    const auto*                  data = summaryData();
    if ( !edge || !data || !m_data ) return;

    RimSummaryCase* referenceCase = isEnsemble() ? m_data->referenceRealization() : m_data->summaryCase();
    if ( !referenceCase || !referenceCase->summaryReader() ) return;

    // The vectors of the shown realization, or of any realization for a statistic
    std::set<RifEclipseSummaryAddress> available;
    if ( isEnsemble() && ensembleValue() != RimGroupNetworkData::EnsembleValue::REALIZATION )
    {
        for ( auto* realization : m_data->realizations() )
        {
            if ( realization && realization->summaryReader() )
            {
                const auto& realizationAddresses = realization->summaryReader()->allResultAddresses();
                available.insert( realizationAddresses.begin(), realizationAddresses.end() );
            }
        }
    }
    else
    {
        RimSummaryCase* shownCase = isEnsemble() && m_realization() ? m_realization() : referenceCase;
        if ( !shownCase->summaryReader() ) return;
        available = shownCase->summaryReader()->allResultAddresses();
    }

    std::set<RifEclipseSummaryAddress> addresses;
    for ( auto metric : allEdgeMetrics )
    {
        addresses.insert( data->edgeAddress( edge->child, edge->isWell(), metric ) );
    }
    for ( auto metric : { RifGroupNetworkSummaryData::NodeMetric::PRESSURE, RifGroupNetworkSummaryData::NodeMetric::BHP } )
    {
        addresses.insert( data->nodeAddress( edge->child, edge->isWell(), metric ) );
    }

    std::set<RifEclipseSummaryAddress> plotAddresses;
    for ( const auto& address : addresses )
    {
        if ( address.isValid() && available.count( address ) > 0 ) plotAddresses.insert( address );
    }
    if ( plotAddresses.empty() ) return;

    RimSummaryPlot* plot = nullptr;
    if ( isEnsemble() )
    {
        RimSummaryEnsemble* ensemble = m_data->summaryEnsemble();
        if ( !ensemble ) return;

        if ( ensembleValue() == RimGroupNetworkData::EnsembleValue::REALIZATION )
        {
            RimSummaryCase* realization = m_realization() ? m_realization() : referenceCase;
            plot                        = RiaSummaryPlotTools::createPlot( plotAddresses, { realization }, { ensemble } );
        }
        else
        {
            plot             = RiaSummaryPlotTools::createPlot( plotAddresses, {}, { ensemble } );
            const auto value = ensembleValue();
            for ( auto* curveSet : plot->curveSets() )
            {
                auto* statistics = curveSet->statisticsOptions();
                statistics->setHideEnsembleCurves( true );
                statistics->setShownStatistics( value == RimGroupNetworkData::EnsembleValue::MEAN,
                                                value == RimGroupNetworkData::EnsembleValue::P10,
                                                value == RimGroupNetworkData::EnsembleValue::P50,
                                                value == RimGroupNetworkData::EnsembleValue::P90,
                                                value == RimGroupNetworkData::EnsembleValue::MINIMUM   ? "0"
                                                : value == RimGroupNetworkData::EnsembleValue::MAXIMUM ? "100"
                                                                                                       : "" );
            }
        }
    }
    else
    {
        plot = RiaSummaryPlotTools::createPlot( plotAddresses, { referenceCase }, {} );
    }

    RiaSummaryPlotTools::createAndAppendSingleSummaryMultiPlot( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimGroupNetworkPlot::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimViewWindow::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_showWindow ) return;

    if ( changedField == &m_data )
    {
        auto* previousData = dynamic_cast<RimGroupNetworkData*>( oldValue.value<caf::PdmPointer<caf::PdmObjectHandle>>().rawPtr() );
        disconnectFromData( previousData );
        connectToData();
        resetTimeStep();
        updateWindowTitle();
    }
    else if ( changedField == &m_ensembleValue || changedField == &m_realization )
    {
        // Keep the shown date, as the time steps of the realizations and the statistics may differ
        std::optional<std::time_t> shownDate;
        if ( m_data )
        {
            const auto& previousSteps = m_data->timeSteps( m_shownEnsembleValue, m_shownRealization.p() );
            if ( !previousSteps.empty() ) shownDate = previousSteps[std::clamp<size_t>( m_timeStepIndex(), 0, previousSteps.size() - 1 )];
        }

        updateShownSelection();
        if ( shownDate )
            showClosestTimeStep( *shownDate );
        else
            resetTimeStep();

        updateWindowTitle();
        updateConnectedEditors();
    }
    else if ( changedField == &m_name )
    {
        updateWindowTitle();
        return;
    }

    networkChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGroupNetworkPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_data )
    {
        if ( auto* project = RimProject::current() )
        {
            if ( auto* oilField = project->activeOilField() )
            {
                if ( RimGroupNetworkDataCollection* collection = oilField->groupNetworkCollection() )
                {
                    for ( RimGroupNetworkData* data : collection->items() )
                    {
                        options.push_back( caf::PdmOptionItemInfo( data->name(), data ) );
                    }
                }
            }
        }
    }
    else if ( fieldNeedingOptions == &m_realization )
    {
        if ( m_data )
        {
            for ( RimSummaryCase* realization : m_data->realizations() )
            {
                options.push_back( caf::PdmOptionItemInfo( realization->displayCaseName(), realization ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_timeStepIndex )
    {
        const QStringList texts = timeStepTexts();
        for ( int i = 0; i < texts.size(); ++i )
        {
            options.push_back( caf::PdmOptionItemInfo( texts[i], i ) );
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_name );
    uiOrdering.add( &m_data );

    caf::PdmUiGroup* valueGroup = uiOrdering.addNewGroup( "Values" );
    if ( isEnsemble() )
    {
        valueGroup->add( &m_ensembleValue );
        if ( m_ensembleValue() == RimGroupNetworkData::EnsembleValue::REALIZATION ) valueGroup->add( &m_realization );
    }
    valueGroup->add( &m_timeStepIndex );
    valueGroup->add( &m_edgeMetric );
    valueGroup->add( &m_nodeMetric );

    caf::PdmUiGroup* filterGroup = uiOrdering.addNewGroup( "Show Nodes" );
    filterGroup->add( &m_showProducers );
    filterGroup->add( &m_showInjectors );
    filterGroup->add( &m_showOther );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlot::initAfterRead()
{
    connectToData();
    updateShownSelection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlot::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicNewGroupNetworkPlotFeature";
    menuBuilder.addSeparator();
    menuBuilder << "RicDeleteItemFeature";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlot::connectToData()
{
    if ( m_data ) m_data->dataChanged.connect( this, &RimGroupNetworkPlot::onDataChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlot::disconnectFromData( RimGroupNetworkData* data )
{
    if ( data ) data->dataChanged.disconnect( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlot::onDataChanged( const caf::SignalEmitter* emitter )
{
    resetTimeStep();
    updateWindowTitle();
    updateConnectedEditors();
    networkChanged.send();
}

//--------------------------------------------------------------------------------------------------
/// Show the last time step of the data
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlot::resetTimeStep()
{
    if ( m_realization() )
    {
        const auto realizations = m_data ? m_data->realizations() : std::vector<RimSummaryCase*>();
        if ( std::find( realizations.begin(), realizations.end(), m_realization() ) == realizations.end() ) m_realization = nullptr;
    }

    updateShownSelection();
    const auto& steps = timeSteps();
    m_timeStepIndex   = steps.empty() ? 0 : static_cast<int>( steps.size() ) - 1;
}

//--------------------------------------------------------------------------------------------------
/// Show the time step closest to the given date
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlot::showClosestTimeStep( std::time_t date )
{
    const auto& steps = timeSteps();
    if ( steps.empty() )
    {
        m_timeStepIndex = 0;
        return;
    }

    const auto it    = std::lower_bound( steps.begin(), steps.end(), date );
    size_t     index = std::min( static_cast<size_t>( it - steps.begin() ), steps.size() - 1 );
    if ( index > 0 &&
         std::llabs( static_cast<long long>( steps[index - 1] - date ) ) <= std::llabs( static_cast<long long>( steps[index] - date ) ) )
    {
        index--;
    }
    m_timeStepIndex = static_cast<int>( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGroupNetworkPlot::updateShownSelection()
{
    m_shownEnsembleValue = ensembleValue();
    m_shownRealization   = m_realization();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RifGroupNetworkSummaryData* RimGroupNetworkPlot::summaryData() const
{
    return m_data ? m_data->summaryData( ensembleValue(), m_realization() ) : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGroupNetworkPlot::isEnsemble() const
{
    return m_data && m_data->isEnsemble();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGroupNetworkData::EnsembleValue RimGroupNetworkPlot::ensembleValue() const
{
    if ( !isEnsemble() ) return RimGroupNetworkData::EnsembleValue::REALIZATION;
    return m_ensembleValue();
}

//--------------------------------------------------------------------------------------------------
/// The statistic, or the name of the realization
//--------------------------------------------------------------------------------------------------
QString RimGroupNetworkPlot::ensembleValueText() const
{
    if ( ensembleValue() != RimGroupNetworkData::EnsembleValue::REALIZATION ) return m_ensembleValue().uiText();

    RimSummaryCase* realization = m_realization() ? m_realization() : ( m_data ? m_data->referenceRealization() : nullptr );
    return realization ? realization->displayCaseName() : "Realization";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigGroupNetwork::TreeRevision* RimGroupNetworkPlot::currentTree() const
{
    if ( !m_data ) return nullptr;
    return m_data->treeAtDate( currentDate() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigGroupNetwork::Edge* RimGroupNetworkPlot::findEdge( const QString& nodeName ) const
{
    const RigGroupNetwork::TreeRevision* tree = currentTree();
    if ( !tree ) return nullptr;

    const std::string name = nodeName.toStdString();
    for ( const RigGroupNetwork::Edge& edge : tree->edges )
    {
        if ( edge.child == name ) return &edge;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::time_t RimGroupNetworkPlot::currentDate() const
{
    const auto& steps = timeSteps();
    if ( steps.empty() ) return 0;
    return steps[timeStepIndex()];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimGroupNetworkPlot::currentSummaryTimeStep() const
{
    return static_cast<size_t>( timeStepIndex() );
}

//--------------------------------------------------------------------------------------------------
/// The summary time steps, or the dates of the network definitions if there is no summary data
//--------------------------------------------------------------------------------------------------
const std::vector<std::time_t>& RimGroupNetworkPlot::timeSteps() const
{
    static const std::vector<std::time_t> noTimeSteps;
    if ( !m_data ) return noTimeSteps;
    return m_data->timeSteps( ensembleValue(), m_realization() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGroupNetworkPlot::valueTexts( const RifGroupNetworkSummaryData* data, const RigGroupNetwork::Edge& edge ) const
{
    if ( !data ) return isEnsemble() ? "No summary ensemble" : "No summary case";

    if ( data->isStatistic() )
    {
        std::vector<RifGroupNetworkSummaryData::NodeMetric> nodeMetrics;
        for ( auto metric : allNodeMetrics )
        {
            if ( !RifGroupNetworkSummaryData::isDiscrete( metric ) ) nodeMetrics.push_back( metric );
        }
        data->prefetch( { { edge.child, edge.isWell() } }, allEdgeMetrics, nodeMetrics );
    }

    const size_t timeStep = currentSummaryTimeStep();
    QStringList  lines;
    if ( isEnsemble() )
        lines.append( QString( "%1 values at %2" ).arg( ensembleValueText() ).arg( dateText( currentDate() ) ) );
    else
        lines.append( QString( "Values at %1" ).arg( dateText( currentDate() ) ) );

    for ( auto metric : allEdgeMetrics )
    {
        const auto value = data->edgeValue( edge.child, edge.isWell(), metric, timeStep );
        if ( !value ) continue;
        lines.append( QString( "%1 (%2): %3" )
                          .arg( QString::fromStdString( RifGroupNetworkSummaryData::edgeMetricText( metric ) ) )
                          .arg( QString::fromStdString( data->edgeAddress( edge.child, edge.isWell(), metric ).vectorName() ) )
                          .arg( valueWithUnit( value, data->edgeUnit( metric ) ) ) );
    }
    for ( auto metric : allNodeMetrics )
    {
        const auto value = data->nodeValue( edge.child, edge.isWell(), metric, timeStep );
        if ( !value ) continue;
        lines.append( QString( "%1 (%2): %3" )
                          .arg( QString::fromStdString( RifGroupNetworkSummaryData::nodeMetricText( metric ) ) )
                          .arg( QString::fromStdString( data->nodeAddress( edge.child, edge.isWell(), metric ).vectorName() ) )
                          .arg( valueWithUnit( value, data->nodeUnit( metric ) ) ) );
    }
    if ( lines.size() == 1 ) lines.append( "No summary vectors found for the node" );
    return lines.join( "\n" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGroupNetworkPlot::toolTip( const RifGroupNetworkSummaryData* data,
                                      const RigGroupNetwork::Edge&      edge,
                                      const RimGroupNetworkGraph::Node& node ) const
{
    QStringList lines;
    lines.append( QString( "<b>%1</b> (%2)" ).arg( node.name.toHtmlEscaped(), edge.isWell() ? "well" : "group" ) );
    if ( !edge.isRoot() ) lines.append( QString( "Parent: %1" ).arg( node.parent.toHtmlEscaped() ) );
    lines.append( QString( "Defined by: %1" ).arg( QString::fromStdString( RigGroupNetwork::keywordText( edge.keyword ) ).toHtmlEscaped() ) );

    const QString properties = propertiesText( edge );
    if ( !properties.isEmpty() ) lines.append( properties );

    if ( data )
    {
        QString edgeName = QString::fromStdString( RifGroupNetworkSummaryData::edgeMetricText( m_edgeMetric() ) );
        QString nodeName = QString::fromStdString( RifGroupNetworkSummaryData::nodeMetricText( m_nodeMetric() ) );
        if ( data->isStatistic() )
        {
            const QString statistic = m_ensembleValue().uiText();
            edgeName += QString( " (%1)" ).arg( statistic );
            nodeName += QString( " (%1)" ).arg( statistic );
        }
        lines.append( QString( "%1: %2" ).arg( edgeName.toHtmlEscaped(), node.edgeText.isEmpty() ? "-" : node.edgeText ) );
        lines.append( QString( "%1: %2" ).arg( nodeName.toHtmlEscaped(), node.valueText.isEmpty() ? "-" : node.valueText ) );
    }
    return lines.join( "<br>" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGroupNetworkPlot::propertiesText( const RigGroupNetwork::Edge& edge )
{
    QStringList texts;
    for ( const auto& [key, value] : edge.properties )
    {
        texts.append( QString( "%1: %2" ).arg( QString::fromStdString( key ).toHtmlEscaped() ).arg( value ) );
    }
    return texts.join( ", " );
}
