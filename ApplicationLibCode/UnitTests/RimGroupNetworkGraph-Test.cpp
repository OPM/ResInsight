#include "gtest/gtest.h"

#include "RimGroupNetworkGraph.h"

#include <QStringList>

namespace
{
RimGroupNetworkGraph::Node makeNode( const QString& name, const QString& parent, bool isWell, bool isProducer, bool isInjector )
{
    RimGroupNetworkGraph::Node node;
    node.name       = name;
    node.parent     = parent;
    node.isWell     = isWell;
    node.isProducer = isProducer;
    node.isInjector = isInjector;
    return node;
}

// FIELD
//   PROD_GRP: P1, P2
//   INJ_GRP: I1
//   EMPTY_GRP
std::vector<RimGroupNetworkGraph::Node> testNetwork()
{
    return { makeNode( "FIELD", "", false, true, true ),
             makeNode( "PROD_GRP", "FIELD", false, true, false ),
             makeNode( "P1", "PROD_GRP", true, true, false ),
             makeNode( "P2", "PROD_GRP", true, true, false ),
             makeNode( "INJ_GRP", "FIELD", false, false, true ),
             makeNode( "I1", "INJ_GRP", true, false, true ),
             makeNode( "EMPTY_GRP", "FIELD", false, false, false ) };
}

QStringList names( const std::vector<RimGroupNetworkGraph::Node>& nodes )
{
    QStringList result;
    for ( const auto& node : nodes )
    {
        result.append( node.name );
    }
    return result;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimGroupNetworkGraph, TypeFilterKeepsPathToShownNodes )
{
    RimGroupNetworkGraph::Filter filter;
    filter.showProducers = false;
    filter.showOther     = false;

    const auto nodes = RimGroupNetworkGraph::visibleNodes( testNetwork(), filter );

    // The root is always kept, and INJ_GRP is kept as the parent of an injector
    EXPECT_EQ( QStringList( { "FIELD", "INJ_GRP", "I1" } ), names( nodes ) );
    EXPECT_EQ( 1, nodes[0].childCount );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimGroupNetworkGraph, CollapsedNodeHidesDescendants )
{
    RimGroupNetworkGraph::Filter filter;
    filter.collapsedNodes = { "PROD_GRP", "I1", "UNKNOWN" };

    const auto nodes = RimGroupNetworkGraph::visibleNodes( testNetwork(), filter );

    EXPECT_EQ( QStringList( { "FIELD", "PROD_GRP", "INJ_GRP", "I1", "EMPTY_GRP" } ), names( nodes ) );

    EXPECT_TRUE( nodes[1].collapsed );
    EXPECT_EQ( 2, nodes[1].hiddenNodeCount );

    // A leaf has nothing to collapse
    EXPECT_FALSE( nodes[3].collapsed );
    EXPECT_EQ( 0, nodes[3].hiddenNodeCount );

    filter.collapsedNodes    = { "FIELD" };
    const auto collapsedRoot = RimGroupNetworkGraph::visibleNodes( testNetwork(), filter );
    ASSERT_EQ( 1u, collapsedRoot.size() );
    EXPECT_EQ( 6, collapsedRoot[0].hiddenNodeCount );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimGroupNetworkGraph, EdgeWidthRelativeToLargestVisibleEdge )
{
    auto input         = testNetwork();
    input[1].edgeValue = 400.0;
    input[2].edgeValue = 100.0;
    input[3].edgeValue = 300.0;
    input[5].edgeValue = -1000.0; // Hidden below

    RimGroupNetworkGraph::Filter filter;
    filter.collapsedNodes = { "INJ_GRP" };

    const auto nodes = RimGroupNetworkGraph::visibleNodes( input, filter );
    ASSERT_EQ( 6u, nodes.size() );

    EXPECT_DOUBLE_EQ( 0.0, nodes[0].edgeWidthFraction );
    EXPECT_DOUBLE_EQ( 1.0, nodes[1].edgeWidthFraction );
    EXPECT_DOUBLE_EQ( 0.25, nodes[2].edgeWidthFraction );
    EXPECT_DOUBLE_EQ( 0.75, nodes[3].edgeWidthFraction );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RimGroupNetworkGraph, ChildrenBeforeParentsAndCycles )
{
    std::vector<RimGroupNetworkGraph::Node> input = { makeNode( "P1", "PROD_GRP", true, true, false ),
                                                      makeNode( "PROD_GRP", "FIELD", false, true, false ),
                                                      makeNode( "A", "B", false, false, false ),
                                                      makeNode( "FIELD", "", false, true, false ),
                                                      makeNode( "B", "A", false, false, false ) };

    RimGroupNetworkGraph::Filter filter;
    filter.showOther = false;

    const auto nodes = RimGroupNetworkGraph::visibleNodes( input, filter );

    EXPECT_EQ( QStringList( { "FIELD", "PROD_GRP", "P1" } ), names( nodes ) );
    EXPECT_EQ( 1, nodes[0].childCount );
    EXPECT_EQ( 1, nodes[1].childCount );

    filter.collapsedNodes    = { "FIELD" };
    const auto collapsedRoot = RimGroupNetworkGraph::visibleNodes( input, filter );
    ASSERT_EQ( 1u, collapsedRoot.size() );
    EXPECT_EQ( 2, collapsedRoot[0].hiddenNodeCount );
}
