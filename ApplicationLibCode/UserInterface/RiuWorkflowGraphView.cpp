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
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
//  at <http://www.gnu.org/licenses/gpl.html> for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RiuWorkflowGraphView.h"

#include <QFontMetrics>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QJsonArray>
#include <QJsonValue>
#include <QMap>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QResizeEvent>
#include <QSet>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
constexpr qreal nodeWidth  = 240.0;
constexpr qreal portStep   = 28.0;
constexpr qreal portTop    = 51.0;
constexpr qreal columnStep = 370.0;
constexpr qreal rowGap     = 55.0;

QStringList fieldPorts( const QJsonArray& fields )
{
    QStringList keys;
    for ( const QJsonValue& field : fields )
    {
        if ( field.isString() && !field.toString().isEmpty() ) keys.append( "field:" + field.toString() );
    }
    keys.removeDuplicates();
    keys.sort();
    return keys;
}

QString inputPortKey( const QJsonObject& edge )
{
    const QString field = edge.value( "input" ).toString();
    return field.isEmpty() ? "model" : "field:" + field;
}

QString outputPortKey( const QJsonObject& edge )
{
    const QString field = edge.value( "output" ).toString();
    return field.isEmpty() ? "model" : "field:" + field;
}

class GraphEdge;

class GraphNode : public QGraphicsRectItem
{
public:
    GraphNode( const QString& name, QStringList inputs, QStringList outputs );

    qreal   height() const;
    QPointF portPosition( const QString& key, bool output ) const;
    void    addEdge( GraphEdge* edge );

protected:
    QVariant itemChange( GraphicsItemChange change, const QVariant& value ) override;

private:
    void addPorts( const QStringList& keys, bool output );

    QMap<QString, QGraphicsEllipseItem*> m_inputs;
    QMap<QString, QGraphicsEllipseItem*> m_outputs;
    std::vector<GraphEdge*>              m_edges;
};

class GraphEdge : public QGraphicsPathItem
{
public:
    GraphEdge( GraphNode* from, QString outputKey, GraphNode* to, QString inputKey );

    void updatePath();

private:
    GraphNode*            m_from;
    GraphNode*            m_to;
    QString               m_outputKey;
    QString               m_inputKey;
    QGraphicsPolygonItem* m_arrow;
};

GraphNode::GraphNode( const QString& name, QStringList inputs, QStringList outputs )
{
    setRect( 0, 0, nodeWidth, portTop + portStep * std::max( { qsizetype( 1 ), inputs.size(), outputs.size() } ) + 12 );
    setPen( QPen( QColor( 60, 85, 115 ) ) );
    setBrush( QColor( 226, 238, 250 ) );
    setFlags( ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges );
    setZValue( 1 );
    setToolTip( name );

    QFont titleFont;
    titleFont.setBold( true );
    auto* title = new QGraphicsTextItem( QFontMetrics( titleFont ).elidedText( name, Qt::ElideRight, nodeWidth - 20 ), this );
    title->setFont( titleFont );
    title->setToolTip( name );
    title->setPos( 10, 4 );
    title->setAcceptedMouseButtons( Qt::NoButton );

    addPorts( inputs, false );
    addPorts( outputs, true );
}

qreal GraphNode::height() const
{
    return rect().height();
}

void GraphNode::addPorts( const QStringList& keys, bool output )
{
    auto& ports = output ? m_outputs : m_inputs;
    for ( int row = 0; row < keys.size(); ++row )
    {
        const QString& key   = keys[row];
        const qreal    x     = output ? nodeWidth : 0.0;
        const qreal    y     = portTop + row * portStep;
        auto*          port  = new QGraphicsEllipseItem( x - 5, y - 5, 10, 10, this );
        const QString  label = key == "model" ? ( output ? "output" : "input" ) : key.mid( 6 );
        port->setBrush( output ? QColor( 50, 115, 165 ) : QColor( 75, 145, 95 ) );
        port->setPen( Qt::NoPen );
        port->setToolTip( label );
        port->setAcceptedMouseButtons( Qt::NoButton );
        ports.insert( key, port );

        QFont font;
        font.setPointSize( 9 );
        const QString display = QFontMetrics( font ).elidedText( label, Qt::ElideRight, nodeWidth / 2 - 18 );
        auto*         text    = new QGraphicsTextItem( display, this );
        text->setFont( font );
        text->setToolTip( label );
        text->setDefaultTextColor( QColor( 45, 65, 80 ) );
        text->setPos( output ? nodeWidth - text->boundingRect().width() - 12 : 10, y - 13 );
        text->setAcceptedMouseButtons( Qt::NoButton );
    }
}

QPointF GraphNode::portPosition( const QString& key, bool output ) const
{
    const auto& ports = output ? m_outputs : m_inputs;
    auto*       port  = ports.value( key, nullptr );
    return port ? port->mapToScene( port->rect().center() ) : scenePos();
}

void GraphNode::addEdge( GraphEdge* edge )
{
    m_edges.push_back( edge );
}

QVariant GraphNode::itemChange( GraphicsItemChange change, const QVariant& value )
{
    if ( change == ItemPositionHasChanged )
    {
        for ( GraphEdge* edge : m_edges )
            edge->updatePath();
        if ( scene() ) scene()->setSceneRect( scene()->sceneRect().united( scene()->itemsBoundingRect().adjusted( -50, -50, 50, 50 ) ) );
    }
    return QGraphicsRectItem::itemChange( change, value );
}

GraphEdge::GraphEdge( GraphNode* from, QString outputKey, GraphNode* to, QString inputKey )
    : m_from( from )
    , m_to( to )
    , m_outputKey( std::move( outputKey ) )
    , m_inputKey( std::move( inputKey ) )
    , m_arrow( new QGraphicsPolygonItem( this ) )
{
    setPen( QPen( QColor( 75, 95, 115 ), 1.7 ) );
    setZValue( -1 );
    m_arrow->setPen( Qt::NoPen );
    m_arrow->setBrush( QColor( 75, 95, 115 ) );
    m_from->addEdge( this );
    m_to->addEdge( this );
    updatePath();
}

void GraphEdge::updatePath()
{
    const QPointF start = m_from->portPosition( m_outputKey, true );
    const QPointF end   = m_to->portPosition( m_inputKey, false );
    const qreal   bend  = std::max( 35.0, std::abs( end.x() - start.x() ) / 2.0 );
    QPainterPath  path( start );
    path.cubicTo( start + QPointF( bend, 0 ), end - QPointF( bend, 0 ), end );
    setPath( path );
    m_arrow->setPolygon( QPolygonF{ end, end + QPointF( -9, -5 ), end + QPointF( -9, 5 ) } );
}
} // namespace

RiuWorkflowGraphView::RiuWorkflowGraphView( QWidget* parent )
    : QGraphicsView( parent )
    , m_scene( new QGraphicsScene( this ) )
{
    setScene( m_scene );
    setRenderHint( QPainter::Antialiasing );
    setDragMode( QGraphicsView::ScrollHandDrag );
    setTransformationAnchor( QGraphicsView::AnchorUnderMouse );
}

void RiuWorkflowGraphView::showGraph( const QJsonObject& graph, const QString& error )
{
    m_scene->clear();
    m_fitOnResize = true;

    if ( !error.isEmpty() || graph.value( "tasks" ).toArray().isEmpty() )
    {
        m_scene->addText( error.isEmpty() ? "No tasks in this workflow" : error );
        fitInView( m_scene->itemsBoundingRect().adjusted( -30, -30, 30, 30 ), Qt::KeepAspectRatio );
        return;
    }

    const QJsonArray           tasks = graph.value( "tasks" ).toArray();
    const QJsonArray           edges = graph.value( "edges" ).toArray();
    QMap<QString, int>         ranks;
    QMap<QString, QStringList> inputs;
    QMap<QString, QStringList> outputs;
    QMap<QString, QStringList> declaredOutputs;
    for ( const QJsonValue& task : tasks )
    {
        const QJsonObject data = task.toObject();
        const QString     name = data.value( "name" ).toString();
        if ( name.isEmpty() || ranks.contains( name ) ) continue;
        ranks.insert( name, 0 );
        inputs.insert( name, fieldPorts( data.value( "inputs" ).toArray() ) );
        declaredOutputs.insert( name, fieldPorts( data.value( "outputs" ).toArray() ) );
    }

    // The Python helper provides tasks in topological order.
    for ( const QJsonValue& task : tasks )
    {
        const QString name = task.toObject().value( "name" ).toString();
        if ( !ranks.contains( name ) ) continue;
        for ( const QJsonValue& value : edges )
        {
            const QJsonObject edge = value.toObject();
            if ( edge.value( "to" ).toString() == name && ranks.contains( edge.value( "from" ).toString() ) )
            {
                ranks[name] = std::max( ranks[name], ranks.value( edge.value( "from" ).toString() ) + 1 );
            }
        }
    }

    for ( const QJsonValue& value : edges )
    {
        const QJsonObject edge = value.toObject();
        const QString     from = edge.value( "from" ).toString();
        const QString     to   = edge.value( "to" ).toString();
        if ( !ranks.contains( from ) || !ranks.contains( to ) || from == to ) continue;
        inputs[to].append( inputPortKey( edge ) );
        outputs[from].append( outputPortKey( edge ) );
    }

    QMap<int, QStringList>    layers;
    QMap<QString, GraphNode*> nodes;
    for ( auto it = ranks.cbegin(); it != ranks.cend(); ++it )
    {
        layers[it.value()].append( it.key() );
        QStringList in  = inputs.value( it.key() );
        QStringList out = outputs.value( it.key() );
        // Non-terminal tasks only need ports used by outgoing connections.
        // Keep declared fields visible on terminal tasks as workflow results.
        if ( out.isEmpty() ) out = declaredOutputs.value( it.key() );
        if ( out.isEmpty() ) out.append( "model" );
        in.removeDuplicates();
        out.removeDuplicates();
        in.sort();
        out.sort();
        auto* node = new GraphNode( it.key(), in, out );
        m_scene->addItem( node );
        nodes.insert( it.key(), node );
    }

    qreal totalHeight = 0;
    for ( const QStringList& layer : layers )
    {
        qreal height = 0;
        for ( const QString& name : layer )
            height += nodes.value( name )->height() + rowGap;
        totalHeight = std::max( totalHeight, height - rowGap );
    }
    for ( auto it = layers.cbegin(); it != layers.cend(); ++it )
    {
        qreal layerHeight = 0;
        for ( const QString& name : it.value() )
            layerHeight += nodes.value( name )->height() + rowGap;
        qreal y = ( totalHeight - layerHeight + rowGap ) / 2.0;
        for ( const QString& name : it.value() )
        {
            auto* node = nodes.value( name );
            node->setPos( it.key() * columnStep, y );
            y += node->height() + rowGap;
        }
    }

    for ( const QJsonValue& value : edges )
    {
        const QJsonObject edge = value.toObject();
        const QString     from = edge.value( "from" ).toString();
        const QString     to   = edge.value( "to" ).toString();
        if ( !nodes.contains( from ) || !nodes.contains( to ) || from == to ) continue;
        m_scene->addItem( new GraphEdge( nodes.value( from ), outputPortKey( edge ), nodes.value( to ), inputPortKey( edge ) ) );
    }

    m_scene->setSceneRect( m_scene->itemsBoundingRect().adjusted( -50, -50, 50, 50 ) );
    fitInView( m_scene->sceneRect(), Qt::KeepAspectRatio );
}

void RiuWorkflowGraphView::resizeEvent( QResizeEvent* event )
{
    QGraphicsView::resizeEvent( event );
    if ( m_fitOnResize && !m_scene->sceneRect().isEmpty() ) fitInView( m_scene->sceneRect(), Qt::KeepAspectRatio );
}

void RiuWorkflowGraphView::mousePressEvent( QMouseEvent* event )
{
    if ( event->button() == Qt::LeftButton ) m_fitOnResize = false;
    QGraphicsView::mousePressEvent( event );
}

void RiuWorkflowGraphView::wheelEvent( QWheelEvent* event )
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
