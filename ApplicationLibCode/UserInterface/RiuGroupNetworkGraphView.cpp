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

#include "RiuGroupNetworkGraphView.h"

#include "RiuGroupNetworkLayout.h"

#include <QContextMenuEvent>
#include <QFont>
#include <QFontMetrics>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QIcon>
#include <QMenu>
#include <QMouseEvent>
#include <QPainterPath>
#include <QPen>
#include <QWheelEvent>

namespace
{
// The appearance of the group tree plot in webviz: circular nodes with the node value inside, and edges
// with a width relative to the largest edge value
constexpr qreal nodeRadius         = 15.0;
constexpr qreal networkColumnStep  = 290.0;
constexpr qreal rowStep            = 70.0;
constexpr qreal minEdgeWidth       = 2.0;
constexpr qreal maxEdgeWidth       = 100.0;
constexpr qreal edgeOpacity        = 0.3;
constexpr qreal labelOffset        = 21.0;
constexpr qreal nodeBorderLeaf     = 1.0;
constexpr qreal nodeBorderChildren = 2.5;

const QColor nodeBorderColor( "#60be6c" );
const QColor nodeBackground( Qt::white );
const QColor selectedColor( 30, 120, 220 );
const QColor labelColor( Qt::black );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QFont sansFont( int pixelSize )
{
    QFont font( "sans-serif" );
    font.setPixelSize( pixelSize );
    return font;
}
} // namespace

namespace RiuGroupNetworkGraphViewItems
{
//==================================================================================================
///
//==================================================================================================
class NetworkNode : public QGraphicsEllipseItem
{
public:
    explicit NetworkNode( const RimGroupNetworkGraph::Node& node );

    const QString& name() const { return m_name; }
    QPointF        center() const;
    void           applyStyle();

protected:
    QVariant itemChange( GraphicsItemChange change, const QVariant& value ) override;

private:
    QString m_name;
    bool    m_hasChildren;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
NetworkNode::NetworkNode( const RimGroupNetworkGraph::Node& node )
    : m_name( node.name )
    , m_hasChildren( node.childCount > 0 || node.collapsed )
{
    setRect( -nodeRadius, -nodeRadius, 2 * nodeRadius, 2 * nodeRadius );
    setFlags( ItemIsSelectable );
    setZValue( 1 );
    setToolTip( node.toolTip );

    // The name is to the right of a leaf node, and to the left of a node with children
    const bool isLeaf = !m_hasChildren;
    auto*      title  = new QGraphicsSimpleTextItem( this );
    title->setFont( sansFont( 10 ) );
    title->setBrush( labelColor );
    title->setText( node.name );
    title->setAcceptedMouseButtons( Qt::NoButton );
    const QRectF titleBounds = title->boundingRect();
    title->setPos( isLeaf ? labelOffset : -labelOffset - titleBounds.width(), -titleBounds.height() / 2 );

    if ( !node.valueNumber.isEmpty() )
    {
        auto* value = new QGraphicsSimpleTextItem( this );
        value->setFont( sansFont( 10 ) );
        value->setBrush( labelColor );
        value->setText( node.valueNumber );
        value->setAcceptedMouseButtons( Qt::NoButton );
        value->setPos( -value->boundingRect().width() / 2, -value->boundingRect().height() );

        auto* unit = new QGraphicsSimpleTextItem( this );
        unit->setFont( sansFont( 9 ) );
        unit->setBrush( labelColor );
        unit->setText( node.valueUnit );
        unit->setAcceptedMouseButtons( Qt::NoButton );
        unit->setPos( -unit->boundingRect().width() / 2, 0 );
    }

    // Children hidden by collapsing the node
    if ( node.collapsed )
    {
        auto* hidden = new QGraphicsSimpleTextItem( this );
        hidden->setFont( sansFont( 10 ) );
        hidden->setBrush( labelColor );
        hidden->setText( QString( "+ %1 %2" ).arg( node.hiddenNodeCount ).arg( node.hiddenNodeCount > 1 ? "children" : "child" ) );
        hidden->setAcceptedMouseButtons( Qt::NoButton );
        hidden->setPos( labelOffset, -hidden->boundingRect().height() / 2 );
    }

    applyStyle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QPointF NetworkNode::center() const
{
    return scenePos();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void NetworkNode::applyStyle()
{
    const qreal borderWidth = m_hasChildren ? nodeBorderChildren : nodeBorderLeaf;
    setPen( QPen( isSelected() ? selectedColor : nodeBorderColor, borderWidth ) );
    setBrush( nodeBackground );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant NetworkNode::itemChange( GraphicsItemChange change, const QVariant& value )
{
    if ( change == ItemSelectedHasChanged ) applyStyle();
    return QGraphicsEllipseItem::itemChange( change, value );
}
} // namespace RiuGroupNetworkGraphViewItems

using RiuGroupNetworkGraphViewItems::NetworkNode;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuGroupNetworkGraphView::RiuGroupNetworkGraphView( QWidget* parent )
    : QGraphicsView( parent )
    , m_scene( new QGraphicsScene( this ) )
{
    setScene( m_scene );
    setRenderHint( QPainter::Antialiasing );
    setDragMode( QGraphicsView::ScrollHandDrag );
    viewport()->unsetCursor();
    setTransformationAnchor( QGraphicsView::AnchorUnderMouse );
    setFocusPolicy( Qt::StrongFocus );

    connect( m_scene, &QGraphicsScene::selectionChanged, this, &RiuGroupNetworkGraphView::onSelectionChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuGroupNetworkGraphView::~RiuGroupNetworkGraphView() = default;

//--------------------------------------------------------------------------------------------------
/// Rebuilds the graph. The zoom and scroll position are kept if the nodes are the same as before, so
/// stepping in time only changes the values.
//--------------------------------------------------------------------------------------------------
void RiuGroupNetworkGraphView::setNodes( const std::vector<RimGroupNetworkGraph::Node>& nodes )
{
    m_updating = true;
    m_scene->clear();
    m_nodes.clear();

    std::vector<std::pair<QString, QString>> nodeParents;
    QStringList                              structure;
    for ( const auto& node : nodes )
    {
        nodeParents.emplace_back( node.name, node.parent );
        structure.append( node.name + "/" + node.parent + ( node.collapsed ? "+" : "" ) );
    }
    const auto positions = RiuGroupNetworkLayout::computePositions( nodeParents );

    for ( const auto& node : nodes )
    {
        if ( !positions.contains( node.name ) ) continue;

        const auto position = positions.value( node.name );
        auto*      item     = new NetworkNode( node );
        item->setPos( position.column * networkColumnStep, position.row * rowStep );
        m_scene->addItem( item );
        m_nodes.insert( node.name, item );
    }

    QFont labelFont = sansFont( 10 );
    for ( const auto& node : nodes )
    {
        NetworkNode* child  = m_nodes.value( node.name );
        NetworkNode* parent = m_nodes.value( node.parent );
        if ( !child || !parent ) continue;

        const QPointF start = parent->center();
        const QPointF end   = child->center();
        const qreal   midX  = ( start.x() + end.x() ) / 2.0;

        QPainterPath path( start );
        path.cubicTo( QPointF( midX, start.y() ), QPointF( midX, end.y() ), end );

        const bool  hasFlow = node.edgeValue && *node.edgeValue != 0.0;
        const qreal width   = hasFlow ? minEdgeWidth + ( maxEdgeWidth - minEdgeWidth ) * node.edgeWidthFraction : minEdgeWidth;

        QColor color = node.edgeColor.isValid() ? node.edgeColor : QColor( "#5c5c5c" );
        color.setAlphaF( static_cast<float>( edgeOpacity ) );
        QPen pen( color, width );
        pen.setCapStyle( Qt::FlatCap );
        if ( !hasFlow ) pen.setDashPattern( { 5.0 / width, 5.0 / width } );

        auto* edge = m_scene->addPath( path, pen );
        edge->setZValue( 0 );
        edge->setToolTip( QString( "%1 to %2<br>%3" )
                              .arg( node.name.toHtmlEscaped(),
                                    node.parent.toHtmlEscaped(),
                                    node.edgeText.isEmpty() ? "No value" : node.edgeText.toHtmlEscaped() ) );

        if ( !node.edgeText.isEmpty() )
        {
            auto* label = m_scene->addSimpleText( node.edgeText, labelFont );
            label->setBrush( labelColor );
            const QRectF bounds = label->boundingRect();
            label->setTransformOriginPoint( bounds.center() );
            label->setPos( path.pointAtPercent( 0.5 ) - bounds.center() );
            label->setRotation( -path.angleAtPercent( 0.5 ) );
            label->setZValue( 2 );
        }
    }

    const QRectF bounds = m_scene->itemsBoundingRect();
    m_scene->setSceneRect( bounds.adjusted( -40, -40, 40, 40 ) );

    if ( NetworkNode* selected = m_nodes.value( m_selectedNode ) )
    {
        selected->setSelected( true );
    }
    else if ( !m_selectedNode.isEmpty() )
    {
        m_selectedNode.clear();
        emit nodeSelected( QString() );
    }
    m_updating = false;

    if ( structure != m_structure )
    {
        m_structure   = structure;
        m_fitOnResize = true;
        fitGraph();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupNetworkGraphView::clear()
{
    m_updating = true;
    m_scene->clear();
    m_nodes.clear();
    m_structure.clear();
    m_selectedNode.clear();
    m_updating = false;
}

//--------------------------------------------------------------------------------------------------
/// Fits the graph in the view, but does not magnify small graphs beyond the natural size
//--------------------------------------------------------------------------------------------------
void RiuGroupNetworkGraphView::fitGraph()
{
    if ( m_scene->sceneRect().isEmpty() ) return;

    fitInView( m_scene->sceneRect(), Qt::KeepAspectRatio );
    if ( transform().m11() > 1.0 ) resetTransform();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuGroupNetworkGraphView::selectedNode() const
{
    return m_selectedNode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupNetworkGraphView::resizeEvent( QResizeEvent* event )
{
    QGraphicsView::resizeEvent( event );
    if ( m_fitOnResize ) fitGraph();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupNetworkGraphView::mousePressEvent( QMouseEvent* event )
{
    if ( event->button() == Qt::LeftButton ) m_fitOnResize = false;
    QGraphicsView::mousePressEvent( event );

    // The hand drag mode sets a hand cursor, use the default cursor instead
    viewport()->unsetCursor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupNetworkGraphView::mouseReleaseEvent( QMouseEvent* event )
{
    QGraphicsView::mouseReleaseEvent( event );
    viewport()->unsetCursor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupNetworkGraphView::mouseDoubleClickEvent( QMouseEvent* event )
{
    if ( NetworkNode* node = nodeAt( event->pos() ) )
    {
        emit toggleCollapseRequested( node->name() );
        event->accept();
        return;
    }
    QGraphicsView::mouseDoubleClickEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupNetworkGraphView::wheelEvent( QWheelEvent* event )
{
    if ( event->modifiers() & Qt::ControlModifier )
    {
        m_fitOnResize      = false;
        const qreal factor = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
        scale( factor, factor );
        event->accept();
        return;
    }
    QGraphicsView::wheelEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupNetworkGraphView::contextMenuEvent( QContextMenuEvent* event )
{
    QMenu menu( this );
    if ( NetworkNode* node = nodeAt( event->pos() ) )
    {
        const QString name = node->name();
        m_scene->clearSelection();
        node->setSelected( true );

        menu.addAction( QIcon( ":/SummaryPlotLight16x16.png" ),
                        "Create Summary Plot",
                        this,
                        [this, name]() { emit createPlotRequested( name ); } );
        menu.addAction( QIcon( ":/ToggleOnOff16x16.png" ), "Collapse/Expand", this, [this, name]() { emit toggleCollapseRequested( name ); } );
        menu.addSeparator();
    }
    menu.addAction( QIcon( ":/Plus.svg" ), "Expand All", this, [this]() { emit expandAllRequested(); } );
    menu.addAction( QIcon( ":/Minus.svg" ), "Collapse All", this, [this]() { emit collapseAllRequested(); } );
    menu.addAction( QIcon( ":/ZoomAll.svg" ),
                    "Fit to View",
                    this,
                    [this]()
                    {
                        m_fitOnResize = true;
                        fitGraph();
                    } );
    menu.exec( event->globalPos() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupNetworkGraphView::onSelectionChanged()
{
    if ( m_updating ) return;

    QString name;
    for ( QGraphicsItem* item : m_scene->selectedItems() )
    {
        if ( auto* node = dynamic_cast<NetworkNode*>( item ) ) name = node->name();
    }
    if ( name == m_selectedNode ) return;

    m_selectedNode = name;
    emit nodeSelected( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
NetworkNode* RiuGroupNetworkGraphView::nodeAt( const QPoint& viewPos ) const
{
    for ( QGraphicsItem* item = itemAt( viewPos ); item; item = item->parentItem() )
    {
        if ( auto* node = dynamic_cast<NetworkNode*>( item ) ) return node;
    }
    return nullptr;
}
