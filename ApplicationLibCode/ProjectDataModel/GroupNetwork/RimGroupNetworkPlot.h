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

#pragma once

#include "RifGroupNetworkSummaryData.h"
#include "RigGroupNetwork.h"
#include "RimGroupNetworkGraph.h"
#include "RimViewWindow.h"

#include "cafAppEnum.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmPtrField.h"
#include "cafSignal.h"

#include <QPointer>

#include <ctime>
#include <vector>

class RimGroupNetworkData;
class RimGroupNetworkNode;
class RiuGroupNetworkWidget;

//==================================================================================================
/// A graph of the production network in a RimGroupNetworkData, with the view settings: values, time
/// step and the nodes to show. Each plot is shown in its own window in the plot main window.
//==================================================================================================
class RimGroupNetworkPlot : public RimViewWindow
{
    CAF_PDM_HEADER_INIT;

public:
    using EdgeMetricEnum = caf::AppEnum<RifGroupNetworkSummaryData::EdgeMetric>;
    using NodeMetricEnum = caf::AppEnum<RifGroupNetworkSummaryData::NodeMetric>;

    caf::Signal<> networkChanged;

public:
    RimGroupNetworkPlot();
    ~RimGroupNetworkPlot() override;

    QString name() const;
    void    setDataSource( RimGroupNetworkData* data );

    // RimViewWindow overrides
    int      id() const override;
    QWidget* viewWidget() override;
    void     zoomAll() override;

    int  fontSize() const override;
    void updateFonts() override;

    std::vector<RimGroupNetworkGraph::Node> graphNodes() const;
    QString                                 statusText() const;

    QStringList timeStepTexts() const;
    int         timeStepIndex() const;
    void        setTimeStepIndex( int index );

    bool isCollapsed( const QString& nodeName ) const;
    void setCollapsed( const QString& nodeName, bool collapsed );
    void collapseAll();
    void expandAll();

    // Shows the given node of the graph as the selected node in the project tree, or clears the
    // selected node if not found
    void selectNode( const QString& nodeName );
    void createSummaryPlot( const QString& nodeName );

protected:
    // RimViewWindow overrides
    QWidget* createViewWidget( QWidget* mainWindowParent = nullptr ) override;
    void     deleteViewWidget() override;
    void     onLoadDataAndUpdate() override;

private:
    void assignIdIfNecessary() override;

    caf::PdmFieldHandle* userDescriptionField() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          initAfterRead() override;
    void                          appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

    void connectToData();
    void disconnectFromData( RimGroupNetworkData* data );
    void onDataChanged( const caf::SignalEmitter* emitter );
    void resetTimeStep();

    const RigGroupNetwork::TreeRevision* currentTree() const;
    const RigGroupNetwork::Edge*         findEdge( const QString& nodeName ) const;
    std::time_t                          currentDate() const;
    size_t                               currentSummaryTimeStep() const;
    const std::vector<std::time_t>&      timeSteps() const;
    const RifGroupNetworkSummaryData*    summaryData() const;

    QString valueTexts( const RigGroupNetwork::Edge& edge ) const;
    QString toolTip( const RigGroupNetwork::Edge& edge, const RimGroupNetworkGraph::Node& node ) const;

    static QString propertiesText( const RigGroupNetwork::Edge& edge );

private:
    caf::PdmField<QString>                   m_name;
    caf::PdmPtrField<RimGroupNetworkData*>   m_data;
    caf::PdmField<EdgeMetricEnum>            m_edgeMetric;
    caf::PdmField<NodeMetricEnum>            m_nodeMetric;
    caf::PdmField<bool>                      m_showProducers;
    caf::PdmField<bool>                      m_showInjectors;
    caf::PdmField<bool>                      m_showOther;
    caf::PdmField<int>                       m_timeStepIndex;
    caf::PdmField<std::vector<QString>>      m_collapsedNodes;
    caf::PdmChildField<RimGroupNetworkNode*> m_selectedNode;

    QPointer<RiuGroupNetworkWidget> m_viewWidget;
};
