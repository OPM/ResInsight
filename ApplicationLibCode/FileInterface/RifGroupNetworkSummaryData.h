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

#include "RifEclipseSummaryAddress.h"
#include "RifGroupNetworkEnsembleStatistics.h"
#include "RigGroupNetwork.h"

#include <ctime>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

class RifSummaryReaderInterface;

//==================================================================================================
/// Binds the nodes of a group network to summary vectors, as the flow network assembler of webviz.
/// The edge of a node is the flow from the node to its parent, so edge values are the rates of the
/// node. Missing vectors give no value.
///
/// The values are taken from a summary case, or from a statistic of an ensemble. For a statistic, the
/// reader is a reference realization, used for the units and the node categories. Discrete values,
/// as the control mode, are not available for a statistic.
//==================================================================================================
class RifGroupNetworkSummaryData
{
public:
    enum class EdgeMetric
    {
        OIL_RATE,
        GAS_RATE,
        WATER_RATE,
        WATER_INJ_RATE,
        GAS_INJ_RATE
    };

    enum class NodeMetric
    {
        PRESSURE,
        BHP,
        CONTROL_MODE
    };

    struct NodeCategory
    {
        bool isProducer = false;
        bool isInjector = false;

        bool isOther() const;
    };

    // Returns the current reader, or nullptr. The reader is owned by the summary case, which can replace or delete it,
    // so no reader pointer is kept between calls. The cached values are discarded when the reader is replaced.
    using ReaderProvider = std::function<RifSummaryReaderInterface*()>;

    using Statistic = RifGroupNetworkEnsembleStatistics::Statistic;

    // A node, given by the name and if it is a well
    using NodeId = std::pair<std::string, bool>;

    RifGroupNetworkSummaryData( ReaderProvider readerProvider, RigGroupNetwork::TreeType treeType );
    RifGroupNetworkSummaryData( ReaderProvider                                           referenceReaderProvider,
                                RigGroupNetwork::TreeType                                treeType,
                                std::shared_ptr<const RifGroupNetworkEnsembleStatistics> statistics,
                                Statistic                                                statistic );

    bool                     isStatistic() const;
    std::optional<Statistic> statistic() const;

    const std::vector<std::time_t>& timeSteps() const;

    // Computes the values of the metrics for the nodes in one operation. Only required for a statistic, where it reads the
    // realizations in parallel. The values are otherwise computed on first access.
    void prefetch( const std::vector<NodeId>& nodes, const std::vector<EdgeMetric>& edgeMetrics, const std::vector<NodeMetric>& nodeMetrics ) const;

    std::optional<double> edgeValue( const std::string& nodeName, bool isWell, EdgeMetric metric, size_t timeStepIndex ) const;
    std::optional<double> nodeValue( const std::string& nodeName, bool isWell, NodeMetric metric, size_t timeStepIndex ) const;
    std::string           edgeUnit( EdgeMetric metric ) const;
    std::string           nodeUnit( NodeMetric metric ) const;

    // Classification over the whole period. Wells use WSTAT if present, otherwise rates.
    NodeCategory category( const std::string& nodeName, bool isWell ) const;

    RifEclipseSummaryAddress edgeAddress( const std::string& nodeName, bool isWell, EdgeMetric metric ) const;
    RifEclipseSummaryAddress nodeAddress( const std::string& nodeName, bool isWell, NodeMetric metric ) const;

    static std::string edgeMetricText( EdgeMetric metric );
    static std::string nodeMetricText( NodeMetric metric );

    // Discrete values can not be combined into a statistic
    static bool isDiscrete( NodeMetric metric );

private:
    const RifSummaryReaderInterface* reader() const;
    const std::vector<double>*       values( const RifEclipseSummaryAddress& address ) const;
    std::optional<double>            valueAt( const RifEclipseSummaryAddress& address, size_t timeStepIndex ) const;
    std::optional<double>            statisticValueAt( const RifEclipseSummaryAddress& address, size_t timeStepIndex ) const;
    bool                             hasPositiveValue( const RifEclipseSummaryAddress& address ) const;

    static RifEclipseSummaryAddress address( const std::string& vectorName, const std::string& nodeName, bool isWell );

private:
    ReaderProvider            m_readerProvider;
    RigGroupNetwork::TreeType m_treeType;

    std::shared_ptr<const RifGroupNetworkEnsembleStatistics> m_statistics;
    Statistic                                                m_statistic = Statistic::MEAN;

    mutable int                                                                    m_readerSerialNumber = -1;
    mutable std::vector<std::time_t>                                               m_timeSteps;
    mutable std::map<RifEclipseSummaryAddress, std::optional<std::vector<double>>> m_cache;
    mutable std::map<NodeMetric, std::string>                                      m_nodeUnits;
};
