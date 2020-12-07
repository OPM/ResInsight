#include "RiuCategoryLegendFrame.h"

#include "cafCategoryLegend.h"

#include "cvfqtUtils.h"

#include <QDebug>
#include <QPainter>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuCategoryLegendFrame::RiuCategoryLegendFrame( QWidget* parent, const QString& title, caf::CategoryMapper* categoryMapper )
    : RiuAbstractLegendFrame( parent, title )
    , m_categoryMapper( categoryMapper )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuCategoryLegendFrame::~RiuCategoryLegendFrame()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuCategoryLegendFrame::layoutInfo( LayoutInfo* layout ) const
{
    QFontMetrics fontMetrics( this->font() );
    QStringList  titleLines = m_title.split( "\n", QString::SkipEmptyParts );

    layout->charHeight        = fontMetrics.height();
    layout->charAscent        = fontMetrics.ascent();
    layout->lineSpacing       = fontMetrics.lineSpacing();
    layout->margins           = QMargins( 8, 8, 8, 8 );
    layout->tickTextLeadSpace = 5;

    int colorBarWidth  = 25;
    int colorBarHeight = layout->overallLegendSize.height() - layout->margins.top() - layout->margins.bottom() -
                         titleLines.size() * layout->lineSpacing - 1.0 * layout->lineSpacing;

    int colorBarStartY = layout->margins.top() + titleLines.size() * layout->lineSpacing + 1.0 * layout->lineSpacing;

    layout->colorBarRect = QRect( layout->margins.left(), colorBarStartY, colorBarWidth, colorBarHeight );

    layout->tickStartX = layout->margins.left();
    layout->tickMidX   = layout->margins.left() + layout->colorBarRect.width();
    layout->tickEndX   = layout->tickMidX + 5;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuCategoryLegendFrame::label( int index ) const
{
    return cvfqt::Utils::toQString( m_categoryMapper->textForCategoryIndex( index ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuCategoryLegendFrame::labelCount() const
{
    return (int)m_categoryMapper->categoryCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuCategoryLegendFrame::rectCount() const
{
    return (int)m_categoryMapper->categoryCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuCategoryLegendFrame::renderRect( QPainter* painter, const LayoutInfo& layout, int index ) const
{
    int           categoryIndex  = static_cast<int>( rectCount() - index - 1u );
    float         categoryHeight = static_cast<float>( layout.colorBarRect.height() ) / labelCount();
    float         pos            = ( categoryIndex + 0.5 ) * categoryHeight / layout.colorBarRect.height();
    float         domainValue    = m_categoryMapper->domainValue( pos );
    cvf::Color3ub clr            = m_categoryMapper->mapToColor( domainValue );
    // qDebug() << "Plot Legend: " << pos << " and " << domainValue << " = " << clr.r() << ", " << clr.g() << ", "
    //        << clr.b();
    QColor color( clr.r(), clr.g(), clr.b() );

    int yStart = layout.colorBarRect.top() + static_cast<int>( index * categoryHeight );
    int yEnd   = layout.colorBarRect.top() + static_cast<int>( ( index + 1 ) * categoryHeight );

    QRect rect( QPoint( layout.tickStartX, yStart ), QPoint( layout.tickMidX - 1, yEnd - 1 ) );
    painter->fillRect( rect, QBrush( color ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QRect RiuCategoryLegendFrame::labelRect( const LayoutInfo& layout, int index ) const
{
    float     categoryHeight = static_cast<float>( layout.colorBarRect.height() ) / labelCount();
    const int posX           = layout.tickEndX + layout.tickTextLeadSpace;
    int       posY           = static_cast<int>( layout.colorBarRect.bottom() - ( index + 1 ) * categoryHeight );
    QString   labelI         = this->label( index );
    int       width          = this->fontMetrics().boundingRect( labelI ).width();
    return QRect( posX, posY, width, categoryHeight );
}
