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

#include "RiuWorkflowGraphLayout.h"

#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QFontMetrics>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QIcon>
#include <QJsonValue>
#include <QKeyEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QPen>
#include <QResizeEvent>
#include <QToolTip>
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
constexpr qreal subtitleGap = 16.0;
constexpr qreal stackOffset = 5.0;
constexpr qreal columnStep  = 650.0;
constexpr qreal configGap   = 110.0;
constexpr qreal rowGap      = 55.0;
constexpr qreal portRadius  = 5.0;
constexpr qreal portHitDist = 14.0;
constexpr int   iconSize    = 12;

const QColor taskBorder( 60, 85, 115 );
const QColor taskBackground( 226, 238, 250 );
const QColor unknownBackground( 232, 232, 232 );
const QColor configBorder( 140, 105, 45 );
const QColor configBackground( 253, 244, 218 );
const QColor configPortColor( 170, 110, 35 );
const QColor inputPortColor( 75, 145, 95 );
const QColor outputPortColor( 50, 115, 165 );
const QColor missingPortColor( 190, 60, 60 );
const QColor validTargetColor( 40, 170, 70 );
const QColor invalidTargetColor( 175, 175, 175 );
const QColor errorColor( 190, 45, 45 );
const QColor warningColor( 200, 130, 20 );
const QColor edgeColor( 75, 95, 115 );
const QColor selectedEdgeColor( 30, 120, 220 );
const QColor mapColor( 95, 80, 160 );

QString keyForField( const QString& field )
{
    return field.isEmpty() ? "model" : "field:" + field;
}

QString fieldForKey( const QString& key )
{
    return key == "model" ? QString() : key.mid( 6 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
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
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
namespace RiuWorkflowGraphViewItems
{
class GraphEdge;

//==================================================================================================
/// How a port is drawn. Hollow: optional and not used. Missing: required and not provided.
//==================================================================================================
struct PortInfo
{
    enum class Style
    {
        Normal,
        Hollow,
        Missing
    };

    QString key;
    QString label;
    QString toolTip;
    QString typeName{};
    QString iconResource{};
    Style   style   = Style::Normal;
    bool    collect = false;
};

//==================================================================================================
///
//==================================================================================================
class PortItem : public QGraphicsEllipseItem
{
public:
    enum class Highlight
    {
        None,
        Valid,
        Invalid
    };

    PortItem( GraphNode* node, const PortInfo& info, bool output, bool config, const QPointF& center );

    GraphNode* node() const { return m_node; }
    QString    key() const { return m_key; }
    QString    field() const { return fieldForKey( m_key ); }
    bool       isOutput() const { return m_output; }
    bool       isConfig() const { return m_config; }
    void       setHighlight( Highlight highlight, const QString& reason = {} );

private:
    GraphNode* m_node;
    QString    m_key;
    bool       m_output;
    bool       m_config;
    QPointF    m_center;
    QBrush     m_brush;
    QPen       m_pen;
    QString    m_toolTip;
};

//==================================================================================================
///
//==================================================================================================
class GraphNode : public QGraphicsRectItem
{
public:
    GraphNode( const QString&               name,
               const std::vector<PortInfo>& inputs,
               const std::vector<PortInfo>& outputs,
               bool                         isConfig = false,
               const QString&               subtitle = {} );

    qreal                  height() const;
    QPointF                portPosition( const QString& key, bool output ) const;
    void                   addEdge( GraphEdge* edge );
    QString                name() const;
    bool                   isConfig() const;
    std::vector<PortItem*> ports() const;
    void                   setConfigValue( const QString& fieldName, const QString& value );
    void                   setTaskState( const QString& state, const QString& error = {} );
    void                   setItemState( const QString& item, const QString& state, const QString& error = {} );
    void                   setSubtitleToolTip( const QString& toolTip );
    void                   setTitle( const QString& title, const QString& toolTip );
    void                   setIssues( const QJsonArray& issues );
    void                   setUnknownType( bool unknown );

protected:
    QVariant itemChange( GraphicsItemChange change, const QVariant& value ) override;

private:
    void addPorts( const std::vector<PortInfo>& ports, bool output );
    void applyStyle();

    QString                           m_name;
    bool                              m_isConfig;
    qreal                             m_width;
    QMap<QString, PortItem*>          m_inputs;
    QMap<QString, PortItem*>          m_outputs;
    QMap<QString, QGraphicsTextItem*> m_configValues;
    std::vector<GraphEdge*>           m_edges;
    QGraphicsTextItem*                m_title      = nullptr;
    QGraphicsTextItem*                m_stateLabel = nullptr;
    QGraphicsTextItem*                m_issueLabel = nullptr;
    QGraphicsTextItem*                m_subtitle   = nullptr;
    std::vector<QGraphicsRectItem*>   m_stack;
    qreal                             m_portTop = portTop;
    QMap<QString, QString>            m_itemStates;
    QMap<QString, QString>            m_itemErrors;
    QString                           m_state;
    QString                           m_stateError;
    QString                           m_baseToolTip;
    QString                           m_issueText;
    int                               m_errorCount   = 0;
    int                               m_warningCount = 0;
    bool                              m_unknownType  = false;
};

//==================================================================================================
///
//==================================================================================================
class GraphEdge : public QGraphicsPathItem
{
public:
    GraphEdge( GraphNode* from, QString outputKey, GraphNode* to, QString inputKey, bool isConfig, bool selectable );

    void         updatePath();
    QPainterPath shape() const override;
    bool         isConfig() const { return m_isConfig; }
    GraphNode*   fromNode() const { return m_from; }
    GraphNode*   toNode() const { return m_to; }
    QString      outputField() const { return fieldForKey( m_outputKey ); }
    QString      inputField() const { return fieldForKey( m_inputKey ); }
    void         setLabel( const QString& label, const QString& toolTip );

protected:
    QVariant itemChange( GraphicsItemChange change, const QVariant& value ) override;

private:
    void applyPen();

    GraphNode*            m_from;
    GraphNode*            m_to;
    QString               m_outputKey;
    QString               m_inputKey;
    bool                  m_isConfig;
    QGraphicsPolygonItem* m_arrow;
    QGraphicsTextItem*    m_label = nullptr;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PortItem::PortItem( GraphNode* node, const PortInfo& info, bool output, bool config, const QPointF& center )
    : QGraphicsEllipseItem( center.x() - portRadius, center.y() - portRadius, 2 * portRadius, 2 * portRadius, node )
    , m_node( node )
    , m_key( info.key )
    , m_output( output )
    , m_config( config )
    , m_center( center )
    , m_toolTip( info.toolTip.isEmpty() ? info.label : info.toolTip )
{
    const QColor color = config ? configPortColor : ( output ? outputPortColor : inputPortColor );
    switch ( info.style )
    {
        case PortInfo::Style::Normal:
            m_brush = color;
            m_pen   = Qt::NoPen;
            break;
        case PortInfo::Style::Hollow:
            m_brush = QColor( 255, 255, 255 );
            m_pen   = QPen( color, 1.5 );
            break;
        case PortInfo::Style::Missing:
            m_brush = QColor( 255, 255, 255 );
            m_pen   = QPen( missingPortColor, 2.0 );
            break;
    }
    setHighlight( Highlight::None );
    setAcceptedMouseButtons( Qt::NoButton );
    setZValue( 2 );

    // An outer ring marks inputs that collect several connections
    if ( info.collect )
    {
        const qreal ring  = portRadius + 3.0;
        auto*       outer = new QGraphicsEllipseItem( center.x() - ring, center.y() - ring, 2 * ring, 2 * ring, node );
        outer->setPen( QPen( color, 1.2 ) );
        outer->setBrush( Qt::NoBrush );
        outer->setAcceptedMouseButtons( Qt::NoButton );
        outer->setZValue( 1.5 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PortItem::setHighlight( Highlight highlight, const QString& reason )
{
    const qreal radius = highlight == Highlight::Valid ? portRadius * 1.8 : portRadius;
    setRect( m_center.x() - radius, m_center.y() - radius, 2 * radius, 2 * radius );
    switch ( highlight )
    {
        case Highlight::None:
            setBrush( m_brush );
            setPen( m_pen );
            setToolTip( m_toolTip );
            break;
        case Highlight::Valid:
            setBrush( validTargetColor );
            setPen( QPen( validTargetColor.darker( 130 ), 1.5 ) );
            setToolTip( m_toolTip );
            break;
        case Highlight::Invalid:
            setBrush( invalidTargetColor );
            setPen( Qt::NoPen );
            setToolTip( reason.isEmpty() ? m_toolTip : m_toolTip + "\n\nCannot connect: " + reason );
            break;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
GraphNode::GraphNode( const QString&               name,
                      const std::vector<PortInfo>& inputs,
                      const std::vector<PortInfo>& outputs,
                      bool                         isConfig,
                      const QString&               subtitle )
    : m_name( name )
    , m_isConfig( isConfig )
    , m_width( isConfig ? configWidth : nodeWidth )
    , m_portTop( subtitle.isEmpty() ? portTop : portTop + subtitleGap )
    , m_baseToolTip( name )
{
    const qsizetype rows = std::max( { qsizetype( 1 ), qsizetype( inputs.size() ), qsizetype( outputs.size() ) } );
    setRect( 0, 0, m_width, m_portTop + portStep * rows + 12 );
    setFlags( ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges );
    setZValue( 1 );

    m_title = new QGraphicsTextItem( this );
    QFont titleFont;
    titleFont.setBold( true );
    m_title->setFont( titleFont );
    m_title->setPos( 10, 4 );
    m_title->setAcceptedMouseButtons( Qt::NoButton );

    if ( !isConfig )
    {
        m_stateLabel = new QGraphicsTextItem( this );
        QFont font;
        font.setPointSize( 8 );
        font.setBold( true );
        m_stateLabel->setFont( font );
        m_stateLabel->setPos( m_width - 91, 5 );
        m_stateLabel->setAcceptedMouseButtons( Qt::NoButton );

        m_issueLabel = new QGraphicsTextItem( this );
        m_issueLabel->setFont( font );
        m_issueLabel->setPos( 8, rect().height() - 2 );
        m_issueLabel->setAcceptedMouseButtons( Qt::NoButton );
    }

    // A mapped task runs once per item: drawn as a stack of boxes with a line saying what it maps over
    if ( !subtitle.isEmpty() )
    {
        for ( int depth = 2; depth >= 1; --depth )
        {
            auto* card = new QGraphicsRectItem( rect().translated( depth * stackOffset, depth * stackOffset ), this );
            card->setFlag( ItemStacksBehindParent );
            card->setAcceptedMouseButtons( Qt::NoButton );
            m_stack.push_back( card );
        }

        m_subtitle = new QGraphicsTextItem( this );
        QFont font;
        font.setPointSize( 8 );
        font.setItalic( true );
        m_subtitle->setFont( font );
        m_subtitle->setDefaultTextColor( mapColor );
        m_subtitle->setPlainText( QFontMetrics( font ).elidedText( subtitle, Qt::ElideRight, m_width - 20 ) );
        m_subtitle->setPos( 10, 24 );
        m_subtitle->setAcceptedMouseButtons( Qt::NoButton );
    }

    addPorts( inputs, false );
    addPorts( outputs, true );
    setTitle( isConfig ? "Config: " + name : name, {} );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
qreal GraphNode::height() const
{
    return rect().height();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void GraphNode::setTitle( const QString& title, const QString& toolTip )
{
    m_title->setPlainText( QFontMetrics( m_title->font() ).elidedText( title, Qt::ElideRight, m_width - ( m_isConfig ? 20 : 100 ) ) );
    m_title->setToolTip( title );
    if ( !toolTip.isEmpty() ) m_baseToolTip = toolTip;
    applyStyle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void GraphNode::addPorts( const std::vector<PortInfo>& ports, bool output )
{
    auto& portMap = output ? m_outputs : m_inputs;
    for ( size_t row = 0; row < ports.size(); ++row )
    {
        const PortInfo& info = ports[row];
        const qreal     x    = output ? m_width : 0.0;
        const qreal     y    = m_portTop + row * portStep;
        auto*           port = new PortItem( this, info, output, m_isConfig, QPointF( x, y ) );
        portMap.insert( info.key, port );

        const QString toolTip   = info.toolTip.isEmpty() ? info.label : info.toolTip;
        const qreal   textWidth = m_isConfig ? m_width - 24 : m_width / 2 - 18;
        const bool    showType  = !info.typeName.isEmpty();
        auto          addText   = [&]( const QString& content, const QFont& font, const QColor& color, qreal top, qreal indent = 0.0 )
        {
            auto* text = new QGraphicsTextItem( QFontMetrics( font ).elidedText( content, Qt::ElideRight, textWidth - indent ), this );
            text->setFont( font );
            text->setToolTip( toolTip );
            text->setDefaultTextColor( color );
            text->setPos( output ? m_width - text->boundingRect().width() - 12 : 10 + indent, top );
            text->setAcceptedMouseButtons( Qt::NoButton );
            return text;
        };

        QFont labelFont;
        labelFont.setPointSize( 9 );
        const QColor labelColor = info.style == PortInfo::Style::Missing ? missingPortColor : QColor( 45, 65, 80 );
        addText( info.label, labelFont, labelColor, y - ( m_isConfig || !output || showType ? 20 : 13 ) );

        // The type goes on a second, smaller line below the label
        if ( showType )
        {
            QFont typeFont;
            typeFont.setPointSize( 7 );
            typeFont.setItalic( true );
            const QIcon icon( info.iconResource );
            const bool  showIcon = !info.iconResource.isEmpty() && !icon.isNull();
            auto*       typeText = addText( info.typeName, typeFont, QColor( 110, 120, 135 ), y - 3, showIcon ? iconSize + 2 : 0.0 );

            // The icon of the ResInsight object goes in front of the type
            if ( showIcon )
            {
                const QRectF textRect = typeText->mapRectToParent( typeText->boundingRect() );
                const qreal  iconX    = output ? textRect.left() - iconSize - 1 : 10 + 3;
                auto*        pixmap   = new QGraphicsPixmapItem( icon.pixmap( iconSize, iconSize ), this );
                pixmap->setPos( iconX, textRect.center().y() - iconSize / 2.0 );
                pixmap->setTransformationMode( Qt::SmoothTransformation );
                pixmap->setToolTip( toolTip );
                pixmap->setAcceptedMouseButtons( Qt::NoButton );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QPointF GraphNode::portPosition( const QString& key, bool output ) const
{
    const auto& ports = output ? m_outputs : m_inputs;
    auto*       port  = ports.value( key, nullptr );
    return port ? port->mapToScene( port->rect().center() ) : scenePos();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void GraphNode::addEdge( GraphEdge* edge )
{
    m_edges.push_back( edge );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString GraphNode::name() const
{
    return m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool GraphNode::isConfig() const
{
    return m_isConfig;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<PortItem*> GraphNode::ports() const
{
    std::vector<PortItem*> result;
    for ( PortItem* port : m_inputs )
        result.push_back( port );
    for ( PortItem* port : m_outputs )
        result.push_back( port );
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void GraphNode::setTaskState( const QString& state, const QString& error )
{
    if ( m_isConfig ) return;
    m_state      = state;
    m_stateError = error;
    if ( state.isEmpty() )
    {
        m_itemStates.clear();
        m_itemErrors.clear();
    }
    applyStyle();
}

//--------------------------------------------------------------------------------------------------
/// The state of one item of a mapped task
//--------------------------------------------------------------------------------------------------
void GraphNode::setItemState( const QString& item, const QString& state, const QString& error )
{
    if ( m_isConfig ) return;
    m_itemStates[item] = state;
    if ( error.isEmpty() )
        m_itemErrors.remove( item );
    else
        m_itemErrors[item] = error;
    applyStyle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void GraphNode::setSubtitleToolTip( const QString& toolTip )
{
    if ( m_subtitle ) m_subtitle->setToolTip( toolTip );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void GraphNode::setIssues( const QJsonArray& issues )
{
    if ( m_isConfig ) return;

    m_errorCount   = 0;
    m_warningCount = 0;
    QStringList lines;
    for ( const QJsonValue& value : issues )
    {
        const QJsonObject issue   = value.toObject();
        const bool        isError = issue.value( "severity" ).toString() != "warning";
        ( isError ? m_errorCount : m_warningCount )++;
        lines.append( QString( "%1 %2" ).arg( isError ? "Error:" : "Warning:", issue.value( "message" ).toString() ) );
    }
    m_issueText = lines.join( "\n" );
    applyStyle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void GraphNode::setUnknownType( bool unknown )
{
    m_unknownType = unknown;
    applyStyle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void GraphNode::applyStyle()
{
    if ( m_isConfig )
    {
        setPen( QPen( configBorder ) );
        setBrush( configBackground );
        setToolTip( m_baseToolTip );
        return;
    }

    QColor border     = taskBorder;
    QColor background = m_unknownType ? unknownBackground : taskBackground;
    qreal  width      = 1.0;
    auto   style      = Qt::SolidLine;
    if ( m_state == "running" )
    {
        border     = QColor( 158, 112, 16 );
        background = QColor( 255, 241, 189 );
    }
    else if ( m_state == "completed" )
    {
        border     = QColor( 42, 120, 65 );
        background = QColor( 215, 242, 222 );
    }
    else if ( m_state == "failed" )
    {
        border     = QColor( 165, 50, 50 );
        background = QColor( 253, 225, 225 );
    }
    else if ( m_state == "interrupted" )
    {
        border     = QColor( 140, 85, 50 );
        background = QColor( 245, 228, 210 );
    }
    else if ( m_errorCount > 0 || m_warningCount > 0 )
    {
        border = m_errorCount > 0 ? errorColor : warningColor;
        style  = Qt::DashLine;
    }
    if ( !m_state.isEmpty() || m_errorCount > 0 || m_warningCount > 0 ) width = 2.0;
    if ( isSelected() ) width += 1.0;

    QPen pen( border, width );
    pen.setStyle( style );
    setPen( pen );
    setBrush( background );
    for ( QGraphicsRectItem* card : m_stack )
    {
        card->setPen( QPen( border, 1.0 ) );
        card->setBrush( background.darker( 104 ) );
    }

    // Item progress of a mapped task, such as "Running 3✓ 1✗"
    int done   = 0;
    int failed = 0;
    for ( const QString& itemState : m_itemStates )
    {
        if ( itemState == "completed" ) ++done;
        if ( itemState == "failed" ) ++failed;
    }
    if ( m_stateLabel )
    {
        QString text = m_state.isEmpty() ? "" : m_state.at( 0 ).toUpper() + m_state.mid( 1 );
        if ( !m_itemStates.isEmpty() )
        {
            text += QString( " %1/%2" ).arg( done ).arg( m_itemStates.size() );
            if ( failed > 0 ) text += QString::fromUtf8( " ✗%1" ).arg( failed );
        }
        m_stateLabel->setDefaultTextColor( border );
        m_stateLabel->setPlainText( text );
        m_stateLabel->setPos( m_width - m_stateLabel->boundingRect().width() - 6, 5 );
    }

    if ( m_issueLabel )
    {
        QStringList badge;
        if ( m_errorCount > 0 ) badge << QString( "%1 error%2" ).arg( m_errorCount ).arg( m_errorCount > 1 ? "s" : "" );
        if ( m_warningCount > 0 ) badge << QString( "%1 warning%2" ).arg( m_warningCount ).arg( m_warningCount > 1 ? "s" : "" );
        m_issueLabel->setDefaultTextColor( m_errorCount > 0 ? errorColor : warningColor );
        m_issueLabel->setPlainText( badge.isEmpty() ? "" : QString::fromUtf8( "\u26a0 " ) + badge.join( ", " ) );
        m_issueLabel->setToolTip( m_issueText );
    }

    QStringList toolTip{ m_baseToolTip };
    if ( !m_stateError.isEmpty() ) toolTip << m_stateError;
    if ( !m_itemStates.isEmpty() )
    {
        QStringList items;
        for ( auto it = m_itemStates.cbegin(); it != m_itemStates.cend(); ++it )
        {
            QString line = QString( "%1: %2" ).arg( it.key(), it.value() );
            if ( m_itemErrors.contains( it.key() ) ) line += " - " + m_itemErrors.value( it.key() );
            items << line;
        }
        toolTip << "Items:\n" + items.join( "\n" );
    }
    if ( !m_issueText.isEmpty() ) toolTip << m_issueText;
    setToolTip( toolTip.join( "\n\n" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant GraphNode::itemChange( GraphicsItemChange change, const QVariant& value )
{
    if ( change == ItemPositionHasChanged )
    {
        for ( GraphEdge* edge : m_edges )
            edge->updatePath();
        if ( scene() ) scene()->setSceneRect( scene()->sceneRect().united( scene()->itemsBoundingRect().adjusted( -50, -50, 50, 50 ) ) );
    }
    else if ( change == ItemSelectedHasChanged && m_title )
    {
        applyStyle();
    }
    return QGraphicsRectItem::itemChange( change, value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
GraphEdge::GraphEdge( GraphNode* from, QString outputKey, GraphNode* to, QString inputKey, bool isConfig, bool selectable )
    : m_from( from )
    , m_to( to )
    , m_outputKey( std::move( outputKey ) )
    , m_inputKey( std::move( inputKey ) )
    , m_isConfig( isConfig )
    , m_arrow( new QGraphicsPolygonItem( this ) )
{
    setZValue( -1 );
    if ( selectable )
    {
        setFlag( ItemIsSelectable );
        setToolTip( QString( "%1.%2 \u2192 %3.%4" )
                        .arg( from->name(),
                              outputField().isEmpty() ? "output" : outputField(),
                              to->name(),
                              inputField().isEmpty() ? "input" : inputField() ) );
    }
    m_arrow->setPen( Qt::NoPen );
    applyPen();
    m_from->addEdge( this );
    m_to->addEdge( this );
    updatePath();
}

//--------------------------------------------------------------------------------------------------
/// A label near the target, such as the index or key of a collected member
//--------------------------------------------------------------------------------------------------
void GraphEdge::setLabel( const QString& label, const QString& toolTip )
{
    if ( label.isEmpty() ) return;
    if ( !m_label )
    {
        m_label = new QGraphicsTextItem( this );
        QFont font;
        font.setPointSize( 8 );
        m_label->setFont( font );
        m_label->setDefaultTextColor( edgeColor );
        m_label->setAcceptedMouseButtons( Qt::NoButton );
    }
    m_label->setPlainText( label );
    if ( !toolTip.isEmpty() ) m_label->setToolTip( toolTip );
    updatePath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void GraphEdge::applyPen()
{
    const QColor color = isSelected() ? selectedEdgeColor : ( m_isConfig ? configPortColor : edgeColor );
    QPen         pen( color, isSelected() ? 3.0 : 1.7 );
    if ( m_isConfig ) pen.setStyle( Qt::DashLine );
    setPen( pen );
    m_arrow->setBrush( color );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void GraphEdge::updatePath()
{
    const QPointF start = m_from->portPosition( m_outputKey, true );
    const QPointF end   = m_to->portPosition( m_inputKey, false );
    const qreal   bend  = std::max( 35.0, std::abs( end.x() - start.x() ) / 2.0 );
    QPainterPath  path( start );
    path.cubicTo( start + QPointF( bend, 0 ), end - QPointF( bend, 0 ), end );
    setPath( path );
    m_arrow->setPolygon( QPolygonF{ end, end + QPointF( -9, -5 ), end + QPointF( -9, 5 ) } );

    // Place the label on the curve shortly before the target
    if ( m_label )
    {
        const QPointF anchor = path.pointAtPercent( 0.85 );
        const QRectF  bounds = m_label->boundingRect();
        m_label->setPos( anchor.x() - bounds.width() / 2.0, anchor.y() - bounds.height() );
    }
}

//--------------------------------------------------------------------------------------------------
/// A wide stroke, so the thin curve is easy to click
//--------------------------------------------------------------------------------------------------
QPainterPath GraphEdge::shape() const
{
    QPainterPathStroker stroker;
    stroker.setWidth( 12 );
    return stroker.createStroke( path() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant GraphEdge::itemChange( GraphicsItemChange change, const QVariant& value )
{
    if ( change == ItemSelectedHasChanged ) applyPen();
    return QGraphicsPathItem::itemChange( change, value );
}
} // namespace RiuWorkflowGraphViewItems

using namespace RiuWorkflowGraphViewItems;

namespace
{
//--------------------------------------------------------------------------------------------------
/// Ports of a task in edit mode: every input and output the task type declares
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<PortInfo>, std::vector<PortInfo>> editModePorts( const QJsonObject& task, const QJsonArray& edges )
{
    const QString name = task.value( "name" ).toString();

    std::vector<PortInfo> inputs;
    bool                  hasWholeInput = false;
    for ( const QJsonValue& value : task.value( "input_ports" ).toArray() )
    {
        const QJsonObject port     = value.toObject();
        const QString     field    = port.value( "name" ).toString();
        const bool        required = port.value( "required" ).toBool();
        const bool provided = port.value( "wired" ).toBool() || port.value( "configured" ).toBool() || port.value( "covered" ).toBool();

        // The whole input model is only shown when it is wired, or when it is the only way into the task
        if ( field.isEmpty() )
        {
            if ( !port.value( "wired" ).toBool() && !task.value( "whole_input" ).toBool() ) continue;
            hasWholeInput = true;
        }

        PortInfo info;
        info.key          = keyForField( field );
        info.label        = field.isEmpty() ? "input" : field;
        info.typeName     = port.value( "type" ).toString();
        info.iconResource = port.value( "icon" ).toString();
        QStringList toolTip{ QString( "%1: %2" ).arg( info.label, port.value( "type" ).toString( "any" ) ) };
        if ( !port.value( "description" ).toString().isEmpty() ) toolTip << port.value( "description" ).toString();
        toolTip << ( required ? "Required" : "Optional" );
        if ( port.value( "configured" ).toBool() ) toolTip << "Set in the job configuration";
        if ( port.value( "covered" ).toBool() ) toolTip << "Provided by a whole-output connection";
        info.toolTip = toolTip.join( "\n" );
        if ( !port.value( "collect" ).toString().isEmpty() )
        {
            info.collect = true;
            toolTip << ( port.value( "collect" ).toString() == "dict" ? "Connect several outputs to collect them, each under its own key"
                                                                      : "Connect several outputs to collect them into a list" );
            info.toolTip = toolTip.join( "\n" );
        }
        if ( port.value( "map_over" ).toBool() )
        {
            info.toolTip = QString( "%1: %2\n%3\nThe task runs once per item; set the items in the job configuration" )
                               .arg( info.label, info.typeName, port.value( "description" ).toString() );
        }
        if ( !provided && !field.isEmpty() ) info.style = required ? PortInfo::Style::Missing : PortInfo::Style::Hollow;
        inputs.push_back( info );
    }
    for ( const QJsonValue& value : edges )
    {
        const QJsonObject edge = value.toObject();
        if ( edge.value( "to" ).toString() == name && edge.value( "input" ).toString().isEmpty() && !hasWholeInput )
        {
            inputs.insert( inputs.begin(),
                           PortInfo{ .key          = "model",
                                     .label        = "input",
                                     .toolTip      = {},
                                     .typeName     = task.value( "input_types" ).toObject().value( "" ).toString(),
                                     .iconResource = task.value( "input_icons" ).toObject().value( "" ).toString() } );
            hasWholeInput = true;
        }
    }

    std::vector<PortInfo> outputs;
    for ( const QJsonValue& value : task.value( "output_ports" ).toArray() )
    {
        const QJsonObject port  = value.toObject();
        const QString     field = port.value( "name" ).toString();
        PortInfo          info;
        info.key          = keyForField( field );
        info.label        = field.isEmpty() ? "output" : field;
        info.typeName     = port.value( "type" ).toString();
        info.iconResource = port.value( "icon" ).toString();
        QStringList toolTip{ QString( "%1: %2" ).arg( info.label, port.value( "type" ).toString( "any" ) ) };
        if ( !port.value( "description" ).toString().isEmpty() ) toolTip << port.value( "description" ).toString();
        info.toolTip = toolTip.join( "\n" );
        outputs.push_back( info );
    }
    if ( outputs.empty() )
        outputs.push_back( PortInfo{ .key          = "model",
                                     .label        = "output",
                                     .toolTip      = {},
                                     .typeName     = task.value( "output_types" ).toObject().value( "" ).toString(),
                                     .iconResource = task.value( "output_icons" ).toObject().value( "" ).toString() } );
    return { inputs, outputs };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<PortInfo> portInfos( const QStringList& keys, bool output, const QJsonObject& types, const QJsonObject& icons )
{
    std::vector<PortInfo> result;
    for ( const QString& key : keys )
    {
        const QString typeName = types.value( fieldForKey( key ) ).toString();
        const QString label    = key == "model" ? ( output ? "output" : "input" ) : fieldForKey( key );
        result.push_back( PortInfo{ .key          = key,
                                    .label        = label,
                                    .toolTip      = typeName.isEmpty() ? label : QString( "%1: %2" ).arg( label, typeName ),
                                    .typeName     = typeName,
                                    .iconResource = icons.value( fieldForKey( key ) ).toString() } );
    }
    return result;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWorkflowGraphView::RiuWorkflowGraphView( QWidget* parent )
    : QGraphicsView( parent )
    , m_scene( new QGraphicsScene( this ) )
{
    setScene( m_scene );
    setRenderHint( QPainter::Antialiasing );
    setDragMode( QGraphicsView::ScrollHandDrag );
    setTransformationAnchor( QGraphicsView::AnchorUnderMouse );
    setAcceptDrops( true );
    setFocusPolicy( Qt::StrongFocus );

    connect( m_scene, &QGraphicsScene::selectionChanged, this, &RiuWorkflowGraphView::onSelectionChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWorkflowGraphView::~RiuWorkflowGraphView() = default;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::setEditable( bool editable )
{
    m_editable = editable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuWorkflowGraphView::isEditable() const
{
    return m_editable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::setConnectionValidator( const ConnectionValidator& validator )
{
    m_validator = validator;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::setPendingNodePosition( const QString& taskName, const QPointF& scenePos )
{
    m_pendingPositions[taskName] = scenePos;
}

//--------------------------------------------------------------------------------------------------
/// Keep the position of a task the user has placed when it is renamed
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::renameNodePosition( const QString& oldName, const QString& newName )
{
    rememberPositions();
    for ( const QString& prefix : { QString( "task:" ), QString( "config:" ) } )
    {
        if ( m_positions.contains( prefix + oldName ) ) m_positions[prefix + newName] = m_positions.take( prefix + oldName );
    }
    if ( m_lastSelectedTask == oldName ) m_lastSelectedTask = newName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::rememberPositions()
{
    for ( auto it = m_taskNodes.cbegin(); it != m_taskNodes.cend(); ++it )
        m_positions["task:" + it.key()] = it.value()->pos();
    for ( auto it = m_configNodes.cbegin(); it != m_configNodes.cend(); ++it )
        m_positions["config:" + it.key()] = it.value()->pos();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::fitGraph()
{
    m_fitOnResize = true;
    if ( !m_scene->sceneRect().isEmpty() ) fitInView( m_scene->sceneRect(), Qt::KeepAspectRatio );
}

//--------------------------------------------------------------------------------------------------
/// Rebuild the scene. With ViewportPolicy::Keep, tasks keep their positions and the view keeps
/// its zoom and scroll position, so edits do not make the graph jump.
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::showGraph( const QJsonObject& graph, const QString& error, ViewportPolicy policy )
{
    finishConnectionDrag( false );

    const QTransform oldTransform = transform();
    const QPointF    oldCenter    = mapToScene( viewport()->rect().center() );
    const bool       keepViewport = policy == ViewportPolicy::Keep && !m_taskNodes.isEmpty() && !m_fitOnResize;
    const QString    selectedTask = m_lastSelectedTask;
    if ( policy == ViewportPolicy::Keep )
        rememberPositions();
    else
        m_positions.clear();

    m_rebuilding = true;
    m_taskNodes.clear();
    m_configNodes.clear();
    m_runStatusLabel = nullptr;
    m_scene->clear();
    if ( policy == ViewportPolicy::Fit ) m_fitOnResize = true;

    if ( !error.isEmpty() || graph.value( "tasks" ).toArray().isEmpty() )
    {
        QString message = error;
        if ( message.isEmpty() )
            message = m_editable ? "No tasks yet.\nDrag a task from the palette, or right-click to add one." : "No tasks in this workflow";
        m_scene->addText( message );
        m_scene->setSceneRect( m_scene->itemsBoundingRect().adjusted( -30, -30, 30, 30 ) );
        fitInView( m_scene->sceneRect(), Qt::KeepAspectRatio );
        m_rebuilding = false;
        m_pendingPositions.clear();
        return;
    }

    const bool       editMode = m_editable && graph.value( "editable" ).toBool();
    const QJsonArray tasks    = graph.value( "tasks" ).toArray();
    const QJsonArray edges    = graph.value( "edges" ).toArray();
    const QString    result   = graph.value( "result_task" ).toString();

    QStringList                              taskNames;
    std::vector<std::pair<QString, QString>> layoutEdges;
    QMap<QString, QJsonObject>               taskData;
    QMap<QString, QStringList>               configFields;
    for ( const QJsonValue& task : tasks )
    {
        const QJsonObject data = task.toObject();
        const QString     name = data.value( "name" ).toString();
        if ( name.isEmpty() || taskData.contains( name ) ) continue;
        taskNames.append( name );
        taskData.insert( name, data );

        QStringList configured;
        for ( const QJsonValue& field : data.value( "config_fields" ).toArray() )
        {
            const QString fieldName = field.toObject().value( "name" ).toString();
            if ( !fieldName.isEmpty() ) configured.append( "field:" + fieldName );
        }
        configured.removeDuplicates();
        configured.sort();
        configFields.insert( name, configured );
    }
    for ( const QJsonValue& value : edges )
    {
        const QJsonObject edge = value.toObject();
        layoutEdges.emplace_back( edge.value( "from" ).toString(), edge.value( "to" ).toString() );
    }

    // Read-only graphs only show the ports in use, plus the declared outputs of end tasks
    QMap<QString, QStringList> usedInputs;
    QMap<QString, QStringList> usedOutputs;
    for ( const QJsonValue& value : edges )
    {
        const QJsonObject edge = value.toObject();
        const QString     from = edge.value( "from" ).toString();
        const QString     to   = edge.value( "to" ).toString();
        if ( !taskData.contains( from ) || !taskData.contains( to ) || from == to ) continue;
        usedInputs[to].append( keyForField( edge.value( "input" ).toString() ) );
        usedOutputs[from].append( keyForField( edge.value( "output" ).toString() ) );
    }

    for ( const QString& name : taskNames )
    {
        const QJsonObject data = taskData.value( name );

        std::vector<PortInfo> inputs;
        std::vector<PortInfo> outputs;
        if ( editMode )
        {
            std::tie( inputs, outputs ) = editModePorts( data, edges );
        }
        else
        {
            QStringList in = fieldPorts( data.value( "inputs" ).toArray() ) + usedInputs.value( name );
            in.append( configFields.value( name ) );
            QStringList out = usedOutputs.value( name );
            if ( out.isEmpty() ) out = fieldPorts( data.value( "outputs" ).toArray() );
            if ( out.isEmpty() ) out.append( "model" );
            in.removeDuplicates();
            out.removeDuplicates();
            in.sort();
            out.sort();
            inputs  = portInfos( in, false, data.value( "input_types" ).toObject(), data.value( "input_icons" ).toObject() );
            outputs = portInfos( out, true, data.value( "output_types" ).toObject(), data.value( "output_icons" ).toObject() );
        }

        const QJsonObject map = data.value( "map" ).toObject();
        QString           subtitle;
        if ( !map.isEmpty() ) subtitle = QString::fromUtf8( "⟳ map over %1" ).arg( map.value( "over" ).toString() );
        auto* node = new GraphNode( name, inputs, outputs, false, subtitle );
        if ( !map.isEmpty() )
        {
            QStringList mapToolTip{
                QString( "Runs once per item of '%1' (%2)" ).arg( map.value( "over" ).toString(), map.value( "type" ).toString() ) };
            if ( !map.value( "key_as" ).toString().isEmpty() ) mapToolTip << QString( "Key → %1" ).arg( map.value( "key_as" ).toString() );
            if ( !map.value( "value_as" ).toString().isEmpty() )
                mapToolTip << QString( "Value → %1" ).arg( map.value( "value_as" ).toString() );
            mapToolTip << ( map.value( "error_mode" ).toString() == "collect_all"
                                ? "Every item runs; the task fails afterwards if any item failed"
                                : "The first failed item stops the task" );
            mapToolTip << "The output is a dict with one entry per item";
            node->setSubtitleToolTip( mapToolTip.join( "\n" ) );
        }
        const QString taskId      = data.value( "task_id" ).toString();
        const QString description = data.value( "description" ).toString();
        QStringList   toolTip{ name };
        if ( !taskId.isEmpty() ) toolTip << taskId;
        if ( !description.isEmpty() ) toolTip << description;
        if ( name == result ) toolTip << "Result task: its output is the result of the workflow";
        if ( data.contains( "known_type" ) && !data.value( "known_type" ).toBool() )
            toolTip << "Unknown task type: the package providing it is not installed";
        node->setTitle( name == result ? QString::fromUtf8( "\u2605 " ) + name : name, toolTip.join( "\n" ) );
        node->setUnknownType( data.contains( "known_type" ) && !data.value( "known_type" ).toBool() );
        node->setIssues( data.value( "issues" ).toArray() );
        m_scene->addItem( node );
        m_taskNodes.insert( name, node );

        if ( !configFields.value( name ).isEmpty() )
        {
            auto* configNode = new GraphNode( name,
                                              {},
                                              portInfos( configFields.value( name ),
                                                         true,
                                                         data.value( "input_types" ).toObject(),
                                                         data.value( "input_icons" ).toObject() ),
                                              true );
            m_scene->addItem( configNode );
            m_configNodes.insert( name, configNode );
        }
    }

    auto slotHeight = [this]( const QString& name )
    {
        qreal height = m_taskNodes.value( name )->height();
        if ( auto* config = m_configNodes.value( name, nullptr ) ) height = std::max( height, config->height() );
        return height;
    };

    const auto layout      = RiuWorkflowGraphLayout::computeLayers( taskNames, layoutEdges );
    qreal      totalHeight = 0;
    for ( const QStringList& layer : layout.layers )
    {
        qreal height = 0;
        for ( const QString& name : layer )
            height += slotHeight( name ) + rowGap;
        totalHeight = std::max( totalHeight, height - rowGap );
    }
    for ( size_t column = 0; column < layout.layers.size(); ++column )
    {
        const QStringList& layer       = layout.layers[column];
        qreal              layerHeight = 0;
        for ( const QString& name : layer )
            layerHeight += slotHeight( name ) + rowGap;
        qreal y = ( totalHeight - layerHeight + rowGap ) / 2.0;
        for ( const QString& name : layer )
        {
            auto*       node = m_taskNodes.value( name );
            const qreal x    = column * columnStep + configWidth + configGap;
            node->setPos( x, y + ( slotHeight( name ) - node->height() ) / 2.0 );
            if ( auto* config = m_configNodes.value( name, nullptr ) )
            {
                config->setPos( x - configWidth - configGap, y + ( slotHeight( name ) - config->height() ) / 2.0 );
            }
            y += slotHeight( name ) + rowGap;
        }
    }

    // Positions the user (or a drop) has chosen override the layout
    for ( auto it = m_taskNodes.cbegin(); it != m_taskNodes.cend(); ++it )
    {
        GraphNode*    config     = m_configNodes.value( it.key(), nullptr );
        const QPointF layoutPos  = it.value()->pos();
        QPointF       position   = layoutPos;
        bool          positioned = false;
        if ( m_pendingPositions.contains( it.key() ) )
        {
            position   = m_pendingPositions.value( it.key() );
            positioned = true;
        }
        else if ( m_positions.contains( "task:" + it.key() ) )
        {
            position   = m_positions.value( "task:" + it.key() );
            positioned = true;
        }
        it.value()->setPos( position );

        if ( config )
        {
            if ( m_positions.contains( "config:" + it.key() ) && !m_pendingPositions.contains( it.key() ) )
                config->setPos( m_positions.value( "config:" + it.key() ) );
            else if ( positioned )
                config->setPos( position + QPointF( -configWidth - configGap, 0 ) );
        }
    }
    m_pendingPositions.clear();

    const bool selectableEdges = editMode;
    for ( const QJsonValue& value : edges )
    {
        const QJsonObject edge = value.toObject();
        const QString     from = edge.value( "from" ).toString();
        const QString     to   = edge.value( "to" ).toString();
        if ( !m_taskNodes.contains( from ) || !m_taskNodes.contains( to ) || from == to ) continue;
        auto* graphEdge = new GraphEdge( m_taskNodes.value( from ),
                                         keyForField( edge.value( "output" ).toString() ),
                                         m_taskNodes.value( to ),
                                         keyForField( edge.value( "input" ).toString() ),
                                         false,
                                         selectableEdges );
        if ( edge.contains( "collect" ) )
        {
            const bool keyed = edge.value( "collect" ).toString() == "dict";
            graphEdge->setLabel( edge.value( "label" ).toString(),
                                 keyed ? QString( "Collected under the key '%1'" ).arg( edge.value( "label" ).toString() )
                                       : QString( "Collected as item %1 of the list" ).arg( edge.value( "label" ).toString() ) );
        }
        m_scene->addItem( graphEdge );
    }

    for ( auto it = m_configNodes.cbegin(); it != m_configNodes.cend(); ++it )
    {
        for ( const QString& key : configFields.value( it.key() ) )
        {
            m_scene->addItem( new GraphEdge( it.value(), key, m_taskNodes.value( it.key() ), key, true, false ) );
        }
    }

    m_runStatusLabel = m_scene->addText( {} );
    m_runStatusLabel->setDefaultTextColor( QColor( 45, 65, 80 ) );
    m_runStatusLabel->setPos( 0, -45 );
    m_scene->setSceneRect( m_scene->itemsBoundingRect().adjusted( -50, -50, 50, 50 ) );

    if ( keepViewport )
    {
        setTransform( oldTransform );
        centerOn( oldCenter );
    }
    else
    {
        fitInView( m_scene->sceneRect(), Qt::KeepAspectRatio );
    }

    m_lastSelectedTask.clear();
    if ( auto* node = m_taskNodes.value( selectedTask, nullptr ) )
    {
        node->setSelected( true );
        m_lastSelectedTask = selectedTask;
    }
    m_rebuilding = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::setTaskIssues( const QString& taskName, const QJsonArray& issues )
{
    if ( auto* node = m_taskNodes.value( taskName, nullptr ) ) node->setIssues( issues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::selectTask( const QString& taskName )
{
    auto* node = m_taskNodes.value( taskName, nullptr );
    if ( !node ) return;

    m_rebuilding = true;
    m_scene->clearSelection();
    node->setSelected( true );
    m_lastSelectedTask = taskName;
    m_rebuilding       = false;
    ensureVisible( node );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWorkflowGraphView::ItemRef RiuWorkflowGraphView::ItemRef::forTask( const QString& taskName )
{
    ItemRef item;
    item.kind = Kind::Task;
    item.task = taskName;
    return item;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWorkflowGraphView::ItemRef
    RiuWorkflowGraphView::ItemRef::forEdge( const QString& from, const QString& output, const QString& to, const QString& input )
{
    ItemRef item;
    item.kind   = Kind::Edge;
    item.from   = from;
    item.output = output;
    item.to     = to;
    item.input  = input;
    return item;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWorkflowGraphView::ItemRef RiuWorkflowGraphView::selectedItem() const
{
    for ( QGraphicsItem* item : m_scene->selectedItems() )
    {
        if ( auto* node = dynamic_cast<GraphNode*>( item ) ) return ItemRef::forTask( node->name() );
        if ( auto* edge = dynamic_cast<GraphEdge*>( item ); edge && !edge->isConfig() )
        {
            return ItemRef::forEdge( edge->fromNode()->name(), edge->outputField(), edge->toNode()->name(), edge->inputField() );
        }
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::onSelectionChanged()
{
    if ( m_rebuilding ) return;

    const ItemRef selected = selectedItem();
    if ( selected.kind != ItemRef::Kind::Task || selected.task == m_lastSelectedTask ) return;

    m_lastSelectedTask = selected.task;
    emit nodeSelected( selected.task );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::setTaskInputValue( const QString& taskName, const QString& fieldName, const QString& value )
{
    if ( auto* node = m_configNodes.value( taskName, nullptr ) ) node->setConfigValue( fieldName, value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::resetTaskStates()
{
    for ( GraphNode* node : m_taskNodes )
        node->setTaskState( {} );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::setTaskState( const QString& taskName, const QString& state, const QString& error )
{
    if ( auto* node = m_taskNodes.value( taskName, nullptr ) ) node->setTaskState( state, error );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::setTaskItemState( const QString& taskName, const QString& item, const QString& state, const QString& error )
{
    if ( auto* node = m_taskNodes.value( taskName, nullptr ) ) node->setItemState( item, state, error );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::setRunStatus( const QString& status )
{
    if ( !m_runStatusLabel ) return;
    m_runStatusLabel->setPlainText( QFontMetrics( m_runStatusLabel->font() ).elidedText( status, Qt::ElideRight, 580 ) );
    m_runStatusLabel->setToolTip( status );
}

//--------------------------------------------------------------------------------------------------
/// The task port nearest to the cursor, within a few pixels
//--------------------------------------------------------------------------------------------------
PortItem* RiuWorkflowGraphView::portAt( const QPoint& viewPos ) const
{
    const QPointF scenePos = mapToScene( viewPos );
    const qreal   distance = portHitDist / std::max( 0.05, transform().m11() );
    PortItem*     nearest  = nullptr;
    qreal         best     = distance;
    for ( QGraphicsItem* item : m_scene->items( QRectF( scenePos - QPointF( distance, distance ), QSizeF( 2 * distance, 2 * distance ) ) ) )
    {
        auto* port = dynamic_cast<PortItem*>( item );
        if ( !port || port->isConfig() ) continue;
        const QPointF delta = port->mapToScene( port->rect().center() ) - scenePos;
        const qreal   d     = std::hypot( delta.x(), delta.y() );
        if ( d <= best )
        {
            best    = d;
            nearest = port;
        }
    }
    return nearest;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWorkflowGraphView::ItemRef RiuWorkflowGraphView::itemRefAt( const QPoint& viewPos ) const
{
    for ( QGraphicsItem* item : items( viewPos ) )
    {
        QGraphicsItem* current = item;
        while ( current )
        {
            if ( auto* node = dynamic_cast<GraphNode*>( current ) ) return ItemRef::forTask( node->name() );
            if ( auto* edge = dynamic_cast<GraphEdge*>( current ) )
            {
                if ( edge->isConfig() ) break;
                return ItemRef::forEdge( edge->fromNode()->name(), edge->outputField(), edge->toNode()->name(), edge->inputField() );
            }
            current = current->parentItem();
        }
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
/// Highlight the ports the dragged port can connect to
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::startConnectionDrag( PortItem* port )
{
    m_dragPort = port;
    m_dragInvalidReasons.clear();

    for ( GraphNode* node : m_taskNodes )
    {
        for ( PortItem* candidate : node->ports() )
        {
            if ( candidate->isOutput() == port->isOutput() ) continue;

            PortItem* outputPort = port->isOutput() ? port : candidate;
            PortItem* inputPort  = port->isOutput() ? candidate : port;

            std::expected<void, QString> valid;
            if ( outputPort->node() == inputPort->node() )
                valid = std::unexpected( QString( "A task cannot be connected to itself" ) );
            else if ( m_validator )
                valid = m_validator( outputPort->node()->name(), outputPort->field(), inputPort->node()->name(), inputPort->field() );

            if ( valid )
                candidate->setHighlight( PortItem::Highlight::Valid );
            else
            {
                candidate->setHighlight( PortItem::Highlight::Invalid, valid.error() );
                m_dragInvalidReasons.insert( candidate, valid.error() );
            }
        }
    }

    m_dragPath = m_scene->addPath( QPainterPath(), QPen( validTargetColor.darker( 120 ), 2.0, Qt::DashLine ) );
    m_dragPath->setZValue( 3 );
    updateConnectionDrag( port->mapToScene( port->rect().center() ) );
    setCursor( Qt::CrossCursor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::updateConnectionDrag( const QPointF& scenePos )
{
    if ( !m_dragPort || !m_dragPath ) return;

    m_dragTarget = nullptr;
    QPointF end  = scenePos;
    if ( PortItem* target = portAt( mapFromScene( scenePos ) );
         target && target->isOutput() != m_dragPort->isOutput() && target->node() != m_dragPort->node() )
    {
        if ( !m_dragInvalidReasons.contains( target ) )
        {
            m_dragTarget = target;
            end          = target->mapToScene( target->rect().center() );
        }
        else
        {
            QToolTip::showText( mapToGlobal( mapFromScene( scenePos ) ), m_dragInvalidReasons.value( target ), this );
        }
    }

    const QPointF start = m_dragPort->mapToScene( m_dragPort->rect().center() );
    QPointF       from  = m_dragPort->isOutput() ? start : end;
    QPointF       to    = m_dragPort->isOutput() ? end : start;
    const qreal   bend  = std::max( 35.0, std::abs( to.x() - from.x() ) / 2.0 );
    QPainterPath  path( from );
    path.cubicTo( from + QPointF( bend, 0 ), to - QPointF( bend, 0 ), to );
    m_dragPath->setPath( path );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::finishConnectionDrag( bool connect )
{
    if ( !m_dragPort ) return;

    PortItem* source = m_dragPort;
    PortItem* target = m_dragTarget;
    m_dragPort       = nullptr;
    m_dragTarget     = nullptr;
    m_dragInvalidReasons.clear();
    if ( m_dragPath )
    {
        m_scene->removeItem( m_dragPath );
        delete m_dragPath;
        m_dragPath = nullptr;
    }
    for ( GraphNode* node : m_taskNodes )
    {
        for ( PortItem* port : node->ports() )
            port->setHighlight( PortItem::Highlight::None );
    }
    unsetCursor();
    QToolTip::hideText();

    if ( !connect || !target ) return;

    PortItem* outputPort = source->isOutput() ? source : target;
    PortItem* inputPort  = source->isOutput() ? target : source;
    emit      connectRequested( outputPort->node()->name(), outputPort->field(), inputPort->node()->name(), inputPort->field() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::resizeEvent( QResizeEvent* event )
{
    QGraphicsView::resizeEvent( event );
    if ( m_fitOnResize && !m_scene->sceneRect().isEmpty() ) fitInView( m_scene->sceneRect(), Qt::KeepAspectRatio );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::mousePressEvent( QMouseEvent* event )
{
    if ( event->button() == Qt::LeftButton ) m_fitOnResize = false;

    if ( m_editable && event->button() == Qt::LeftButton )
    {
        if ( PortItem* port = portAt( event->pos() ) )
        {
            startConnectionDrag( port );
            event->accept();
            return;
        }
    }
    QGraphicsView::mousePressEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::mouseMoveEvent( QMouseEvent* event )
{
    if ( m_dragPort )
    {
        updateConnectionDrag( mapToScene( event->pos() ) );
        event->accept();
        return;
    }
    QGraphicsView::mouseMoveEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::mouseReleaseEvent( QMouseEvent* event )
{
    if ( m_dragPort )
    {
        updateConnectionDrag( mapToScene( event->pos() ) );
        finishConnectionDrag( true );
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::keyPressEvent( QKeyEvent* event )
{
    if ( event->key() == Qt::Key_Escape && m_dragPort )
    {
        finishConnectionDrag( false );
        event->accept();
        return;
    }

    if ( m_editable )
    {
        const ItemRef selected = selectedItem();
        if ( ( event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace ) && selected.kind != ItemRef::Kind::None )
        {
            emit deleteRequested( selected );
            event->accept();
            return;
        }
        if ( event->key() == Qt::Key_F2 && selected.kind == ItemRef::Kind::Task )
        {
            emit renameRequested( selected.task );
            event->accept();
            return;
        }
    }
    QGraphicsView::keyPressEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::contextMenuEvent( QContextMenuEvent* event )
{
    const ItemRef item = itemRefAt( event->pos() );

    m_rebuilding = true;
    m_scene->clearSelection();
    for ( QGraphicsItem* graphicsItem : items( event->pos() ) )
    {
        QGraphicsItem* current = graphicsItem;
        while ( current && !( current->flags() & QGraphicsItem::ItemIsSelectable ) )
            current = current->parentItem();
        if ( current )
        {
            current->setSelected( true );
            break;
        }
    }
    m_rebuilding = false;
    onSelectionChanged();

    emit contextMenuRequested( item, event->globalPos(), mapToScene( event->pos() ) );
    event->accept();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::dragEnterEvent( QDragEnterEvent* event )
{
    if ( m_editable && event->mimeData()->hasFormat( taskMimeType ) )
        event->acceptProposedAction();
    else
        event->ignore();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::dragMoveEvent( QDragMoveEvent* event )
{
    if ( m_editable && event->mimeData()->hasFormat( taskMimeType ) )
        event->acceptProposedAction();
    else
        event->ignore();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWorkflowGraphView::dropEvent( QDropEvent* event )
{
    if ( !m_editable || !event->mimeData()->hasFormat( taskMimeType ) )
    {
        event->ignore();
        return;
    }

    const QString taskId = QString::fromUtf8( event->mimeData()->data( taskMimeType ) );
    event->acceptProposedAction();
    m_fitOnResize = false;
    emit addTaskRequested( taskId, mapToScene( event->position().toPoint() ) );
}
