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
constexpr qreal nodeWidth   = 240.0;
constexpr qreal configWidth = 190.0;
constexpr qreal portStep    = 40.0;
constexpr qreal portTop     = 51.0;
constexpr qreal columnStep  = 650.0;
constexpr qreal configGap   = 110.0;
constexpr qreal rowGap      = 55.0;

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
    GraphNode( const QString& name, QStringList inputs, QStringList outputs, bool isConfig = false );

    qreal   height() const;
    QPointF portPosition( const QString& key, bool output ) const;
    void    addEdge( GraphEdge* edge );
    QString name() const;
    bool    isConfig() const;
    void    setConfigValue( const QString& fieldName, const QString& value );
    void    setTaskState( const QString& state, const QString& error = {} );

protected:
    QVariant itemChange( GraphicsItemChange change, const QVariant& value ) override;

private:
    void addPorts( const QStringList& keys, bool output );

    QString                              m_name;
    bool                                 m_isConfig;
    qreal                                m_width;
    QMap<QString, QGraphicsEllipseItem*> m_inputs;
    QMap<QString, QGraphicsEllipseItem*> m_outputs;
    QMap<QString, QGraphicsTextItem*>    m_configValues;
    std::vector<GraphEdge*>              m_edges;
    QGraphicsTextItem*                   m_stateLabel = nullptr;
};

class GraphEdge : public QGraphicsPathItem
{
public:
    GraphEdge( GraphNode* from, QString outputKey, GraphNode* to, QString inputKey, bool isConfig = false );

    void updatePath();

private:
    GraphNode*            m_from;
    GraphNode*            m_to;
    QString               m_outputKey;
    QString               m_inputKey;
    QGraphicsPolygonItem* m_arrow;
};

GraphNode::GraphNode( const QString& name, QStringList inputs, QStringList outputs, bool isConfig )
    : m_name( name )
    , m_isConfig( isConfig )
    , m_width( isConfig ? configWidth : nodeWidth )
{
    setRect( 0, 0, m_width, portTop + portStep * std::max( { qsizetype( 1 ), inputs.size(), outputs.size() } ) + 12 );
    setPen( QPen( isConfig ? QColor( 140, 105, 45 ) : QColor( 60, 85, 115 ) ) );
    setBrush( isConfig ? QColor( 253, 244, 218 ) : QColor( 226, 238, 250 ) );
    setFlags( ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges );
    setZValue( 1 );
    setToolTip( name );

    QFont titleFont;
    titleFont.setBold( true );
    const QString titleText = isConfig ? "Config: " + name : name;
    auto*         title =
        new QGraphicsTextItem( QFontMetrics( titleFont ).elidedText( titleText, Qt::ElideRight, m_width - ( isConfig ? 20 : 100 ) ), this );
    title->setFont( titleFont );
    title->setToolTip( titleText );
    title->setPos( 10, 4 );
    title->setAcceptedMouseButtons( Qt::NoButton );

    if ( !isConfig )
    {
        m_stateLabel = new QGraphicsTextItem( this );
        QFont font;
        font.setPointSize( 8 );
        font.setBold( true );
        m_stateLabel->setFont( font );
        m_stateLabel->setPos( m_width - 91, 5 );
        m_stateLabel->setAcceptedMouseButtons( Qt::NoButton );
    }

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
        const qreal    x     = output ? m_width : 0.0;
        const qreal    y     = portTop + row * portStep;
        auto*          port  = new QGraphicsEllipseItem( x - 5, y - 5, 10, 10, this );
        const QString  label = key == "model" ? ( output ? "output" : "input" ) : key.mid( 6 );
        port->setBrush( m_isConfig ? QColor( 170, 110, 35 ) : ( output ? QColor( 50, 115, 165 ) : QColor( 75, 145, 95 ) ) );
        port->setPen( Qt::NoPen );
        port->setToolTip( label );
        port->setAcceptedMouseButtons( Qt::NoButton );
        ports.insert( key, port );

        QFont font;
        font.setPointSize( 9 );
        const QString display = QFontMetrics( font ).elidedText( label, Qt::ElideRight, m_isConfig ? m_width - 24 : m_width / 2 - 18 );
        auto*         text    = new QGraphicsTextItem( display, this );
        text->setFont( font );
        text->setToolTip( label );
        text->setDefaultTextColor( QColor( 45, 65, 80 ) );
        text->setPos( output ? m_width - text->boundingRect().width() - 12 : 10, y - ( m_isConfig || !output ? 20 : 13 ) );
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

QString GraphNode::name() const
{
    return m_name;
}

bool GraphNode::isConfig() const
{
    return m_isConfig;
}

void GraphNode::setTaskState( const QString& state, const QString& error )
{
    if ( m_isConfig ) return;

    QColor border( 60, 85, 115 );
    QColor background( 226, 238, 250 );
    if ( state == "running" )
    {
        border     = QColor( 158, 112, 16 );
        background = QColor( 255, 241, 189 );
    }
    else if ( state == "completed" )
    {
        border     = QColor( 42, 120, 65 );
        background = QColor( 215, 242, 222 );
    }
    else if ( state == "failed" )
    {
        border     = QColor( 165, 50, 50 );
        background = QColor( 253, 225, 225 );
    }
    else if ( state == "interrupted" )
    {
        border     = QColor( 140, 85, 50 );
        background = QColor( 245, 228, 210 );
    }
    setPen( QPen( border, state.isEmpty() ? 1 : 2 ) );
    setBrush( background );
    m_stateLabel->setDefaultTextColor( border );
    m_stateLabel->setPlainText( state.isEmpty() ? "" : state.at( 0 ).toUpper() + state.mid( 1 ) );
    setToolTip( error.isEmpty() ? m_name : m_name + "\n" + error );
}

void GraphNode::setConfigValue( const QString& fieldName, const QString& value )
{
    if ( !m_isConfig ) return;
    auto* port = m_outputs.value( "field:" + fieldName, nullptr );
    if ( !port ) return;

    auto* text = m_configValues.value( fieldName, nullptr );
    if ( !text )
    {
        text = new QGraphicsTextItem( this );
        QFont font;
        font.setPointSize( 8 );
        text->setFont( font );
        text->setDefaultTextColor( QColor( 125, 80, 30 ) );
        text->setPos( 10, port->rect().center().y() - 2 );
        text->setAcceptedMouseButtons( Qt::NoButton );
        m_configValues.insert( fieldName, text );
    }

    text->setPlainText( QFontMetrics( text->font() ).elidedText( value, Qt::ElideRight, m_width - 24 ) );
    text->setToolTip( fieldName + ": " + value );
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

GraphEdge::GraphEdge( GraphNode* from, QString outputKey, GraphNode* to, QString inputKey, bool isConfig )
    : m_from( from )
    , m_to( to )
    , m_outputKey( std::move( outputKey ) )
    , m_inputKey( std::move( inputKey ) )
    , m_arrow( new QGraphicsPolygonItem( this ) )
{
    QPen pen( isConfig ? QColor( 170, 110, 35 ) : QColor( 75, 95, 115 ), 1.7 );
    if ( isConfig ) pen.setStyle( Qt::DashLine );
    setPen( pen );
    setZValue( -1 );
    m_arrow->setPen( Qt::NoPen );
    m_arrow->setBrush( pen.color() );
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
    m_runStatusLabel = nullptr;
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
    QMap<QString, QStringList> configFields;
    for ( const QJsonValue& task : tasks )
    {
        const QJsonObject data = task.toObject();
        const QString     name = data.value( "name" ).toString();
        if ( name.isEmpty() || ranks.contains( name ) ) continue;
        ranks.insert( name, 0 );
        inputs.insert( name, fieldPorts( data.value( "inputs" ).toArray() ) );
        declaredOutputs.insert( name, fieldPorts( data.value( "outputs" ).toArray() ) );
        QStringList configured;
        for ( const QJsonValue& field : data.value( "config_fields" ).toArray() )
        {
            const QString fieldName = field.toObject().value( "name" ).toString();
            if ( !fieldName.isEmpty() ) configured.append( "field:" + fieldName );
        }
        configured.removeDuplicates();
        configured.sort();
        configFields.insert( name, configured );
        inputs[name].append( configured );
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
    QMap<QString, GraphNode*> configNodes;
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
        if ( !configFields.value( it.key() ).isEmpty() )
        {
            auto* configNode = new GraphNode( it.key(), {}, configFields.value( it.key() ), true );
            m_scene->addItem( configNode );
            configNodes.insert( it.key(), configNode );
        }
    }

    auto slotHeight = [&nodes, &configNodes]( const QString& name )
    {
        qreal height = nodes.value( name )->height();
        if ( auto* config = configNodes.value( name, nullptr ) ) height = std::max( height, config->height() );
        return height;
    };

    qreal totalHeight = 0;
    for ( const QStringList& layer : layers )
    {
        qreal height = 0;
        for ( const QString& name : layer )
            height += slotHeight( name ) + rowGap;
        totalHeight = std::max( totalHeight, height - rowGap );
    }
    for ( auto it = layers.cbegin(); it != layers.cend(); ++it )
    {
        qreal layerHeight = 0;
        for ( const QString& name : it.value() )
            layerHeight += slotHeight( name ) + rowGap;
        qreal y = ( totalHeight - layerHeight + rowGap ) / 2.0;
        for ( const QString& name : it.value() )
        {
            auto*       node = nodes.value( name );
            const qreal x    = it.key() * columnStep + configWidth + configGap;
            node->setPos( x, y + ( slotHeight( name ) - node->height() ) / 2.0 );
            if ( auto* config = configNodes.value( name, nullptr ) )
            {
                config->setPos( x - configWidth - configGap, y + ( slotHeight( name ) - config->height() ) / 2.0 );
            }
            y += slotHeight( name ) + rowGap;
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

    for ( auto it = configNodes.cbegin(); it != configNodes.cend(); ++it )
    {
        for ( const QString& key : configFields.value( it.key() ) )
        {
            m_scene->addItem( new GraphEdge( it.value(), key, nodes.value( it.key() ), key, true ) );
        }
    }

    m_runStatusLabel = m_scene->addText( {} );
    m_runStatusLabel->setDefaultTextColor( QColor( 45, 65, 80 ) );
    m_runStatusLabel->setPos( 0, -45 );
    m_scene->setSceneRect( m_scene->itemsBoundingRect().adjusted( -50, -50, 50, 50 ) );
    fitInView( m_scene->sceneRect(), Qt::KeepAspectRatio );
}

void RiuWorkflowGraphView::setTaskInputValue( const QString& taskName, const QString& fieldName, const QString& value )
{
    for ( QGraphicsItem* item : m_scene->items() )
    {
        auto* node = dynamic_cast<GraphNode*>( item );
        if ( node && node->isConfig() && node->name() == taskName )
        {
            node->setConfigValue( fieldName, value );
            return;
        }
    }
}

void RiuWorkflowGraphView::resetTaskStates()
{
    for ( QGraphicsItem* item : m_scene->items() )
    {
        if ( auto* node = dynamic_cast<GraphNode*>( item ) ) node->setTaskState( {} );
    }
}

void RiuWorkflowGraphView::setTaskState( const QString& taskName, const QString& state, const QString& error )
{
    for ( QGraphicsItem* item : m_scene->items() )
    {
        auto* node = dynamic_cast<GraphNode*>( item );
        if ( node && !node->isConfig() && node->name() == taskName )
        {
            node->setTaskState( state, error );
            return;
        }
    }
}

void RiuWorkflowGraphView::setRunStatus( const QString& status )
{
    if ( !m_runStatusLabel ) return;
    m_runStatusLabel->setPlainText( QFontMetrics( m_runStatusLabel->font() ).elidedText( status, Qt::ElideRight, 580 ) );
    m_runStatusLabel->setToolTip( status );
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
