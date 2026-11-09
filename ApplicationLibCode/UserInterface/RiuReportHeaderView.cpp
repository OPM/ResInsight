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

#include "RiuReportHeaderView.h"

#include "RiuReportTableModel.h"

#include <QFontMetrics>
#include <QPainter>

namespace
{
constexpr int horizontalPadding = 10;
constexpr int verticalPadding   = 5;

const QString ascendingArrow  = QString( " " ) + QChar( 0x25B2 );
const QString descendingArrow = QString( " " ) + QChar( 0x25BC );
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuReportHeaderView::RiuReportHeaderView( const Colors& colors, QWidget* parent )
    : QHeaderView( Qt::Horizontal, parent )
    , m_colors( colors )
{
    setSectionsClickable( true );
    setSortIndicatorShown( true );
    setSortIndicator( -1, Qt::AscendingOrder );
#if QT_VERSION >= QT_VERSION_CHECK( 6, 1, 0 )
    // Clicking a sorted column a third time restores the original order
    setSortIndicatorClearable( true );
#endif
    setHighlightSections( false );
    setDefaultAlignment( Qt::AlignLeft | Qt::AlignVCenter );
    setTextElideMode( Qt::ElideRight );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuReportHeaderView::paintSection( QPainter* painter, const QRect& rect, int logicalIndex ) const
{
    if ( !rect.isValid() || !model() ) return;

    painter->save();
    painter->fillRect( rect, m_colors.background );
    painter->setPen( m_colors.border );
    painter->drawLine( rect.bottomLeft(), rect.bottomRight() );

    const QString       title     = model()->headerData( logicalIndex, Qt::Horizontal, Qt::DisplayRole ).toString();
    const QString       unit      = model()->headerData( logicalIndex, Qt::Horizontal, RiuReportTableModel::UnitRole ).toString();
    const auto          alignment = model()->headerData( logicalIndex, Qt::Horizontal, Qt::TextAlignmentRole ).value<Qt::Alignment>();
    const Qt::Alignment horizontalAlignment = ( alignment & Qt::AlignRight ) ? Qt::AlignRight : Qt::AlignLeft;

    const QRect contentRect = rect.adjusted( horizontalPadding, verticalPadding, -horizontalPadding, -verticalPadding );

    const QFont        titleFnt = titleFont();
    const QFontMetrics titleMetrics( titleFnt );
    QRect              titleRect = contentRect;
    if ( anyColumnHasUnit() ) titleRect.setHeight( titleMetrics.height() );

    QString arrow;
    if ( isSortIndicatorShown() && sortIndicatorSection() == logicalIndex )
    {
        arrow = sortIndicatorOrder() == Qt::AscendingOrder ? ascendingArrow : descendingArrow;
    }
    const int arrowWidth = titleMetrics.horizontalAdvance( arrow );

    painter->setFont( titleFnt );
    painter->setPen( m_colors.text );
    const QString elidedTitle = titleMetrics.elidedText( title, Qt::ElideRight, std::max( 0, titleRect.width() - arrowWidth ) );
    painter->drawText( titleRect, horizontalAlignment | Qt::AlignVCenter, elidedTitle + arrow );

    if ( !unit.isEmpty() )
    {
        const QFont        unitFnt = unitFont();
        const QFontMetrics unitMetrics( unitFnt );
        QRect              unitRect( contentRect.left(), titleRect.bottom() + 1, contentRect.width(), unitMetrics.height() );

        painter->setFont( unitFnt );
        painter->setPen( m_colors.mutedText );
        painter->drawText( unitRect, horizontalAlignment | Qt::AlignVCenter, unitMetrics.elidedText( unit, Qt::ElideRight, unitRect.width() ) );
    }

    painter->restore();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuReportHeaderView::sectionSizeFromContents( int logicalIndex ) const
{
    if ( !model() ) return QHeaderView::sectionSizeFromContents( logicalIndex );

    const QString title = model()->headerData( logicalIndex, Qt::Horizontal, Qt::DisplayRole ).toString();
    const QString unit  = model()->headerData( logicalIndex, Qt::Horizontal, RiuReportTableModel::UnitRole ).toString();

    const QFontMetrics titleMetrics( titleFont() );
    const QFontMetrics unitMetrics( unitFont() );

    // Reserve space for the sort arrow so sorting does not elide the title
    const int titleWidth = titleMetrics.horizontalAdvance( title ) + titleMetrics.horizontalAdvance( descendingArrow );
    const int width      = std::max( titleWidth, unitMetrics.horizontalAdvance( unit ) );
    int       height     = titleMetrics.height();
    if ( anyColumnHasUnit() ) height += unitMetrics.height() + 1;

    return { width + 2 * horizontalPadding, height + 2 * verticalPadding };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QFont RiuReportHeaderView::titleFont() const
{
    QFont fnt = font();
    fnt.setBold( true );
    return fnt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QFont RiuReportHeaderView::unitFont() const
{
    QFont fnt = font();
    if ( fnt.pointSizeF() > 0 )
        fnt.setPointSizeF( fnt.pointSizeF() * 0.85 );
    else if ( fnt.pixelSize() > 0 )
        fnt.setPixelSize( std::max( 1, static_cast<int>( fnt.pixelSize() * 0.85 ) ) );
    return fnt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuReportHeaderView::anyColumnHasUnit() const
{
    if ( !model() ) return false;

    for ( int i = 0; i < model()->columnCount(); i++ )
    {
        if ( !model()->headerData( i, Qt::Horizontal, RiuReportTableModel::UnitRole ).toString().isEmpty() ) return true;
    }
    return false;
}
