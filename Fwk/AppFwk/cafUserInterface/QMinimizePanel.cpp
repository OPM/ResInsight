//##################################################################################################
//
//   QMinimizePanel
//   Copyright (C) 2017 Ceetron Solutions AS
//
//   This class may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#pragma warning( disable : 4125 )

#include "QMinimizePanel.h"

#include <QApplication>
#include <QDebug>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QVBoxLayout>

#include <algorithm>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

static const struct
{
    unsigned int  width;
    unsigned int  height;
    unsigned int  bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
    unsigned char pixel_data[7 * 10 * 4 + 1];
} expandDownArrow = {
    7,
    10,
    4,
    "QRY\317445A\0\0\0\0\0\0\0\0\0\0\0\0"
    "445AJJN\317OQW\256OPW\317445#\0\0\0"
    "\0"
    "445#IJP\317HIN\256445#MOT\317LMS\317445#IJP\317GHM\317445#\0\0\0\0"
    "4"
    "45#DEK\317??C\317BBG\317445#\0\0\0\0\0\0\0\0\0\0\0\0"
    "445#>?B\317445#\0\0"
    "\0\0\0\0\0\0LNT\317445A\0\0\0\0\0\0\0\0\0\0\0\0"
    "445ACEI\317JKR\256IJP\317"
    "445#\0\0\0\0"
    "445#DEH\317BCJ\256445#GHO\317EGK\317445#DEH\317BDI\317445#"
    "\0\0\0\0"
    "445#@AE\317??C\317??B\317445#\0\0\0\0\0\0\0\0\0\0\0\0"
    "445#<<?"
    "\317445#\0\0\0\0\0\0\0\0",
};

QIcon createExpandDownIcon()
{
    QImage  img( expandDownArrow.pixel_data, expandDownArrow.width, expandDownArrow.height, QImage::Format_ARGB32 );
    QPixmap pxMap;
    pxMap = QPixmap::fromImage( img );

    return QIcon( pxMap );
}

static const QIcon& expandDownIcon()
{
    static QIcon expandDownIcon( createExpandDownIcon() );
    return expandDownIcon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

static const struct
{
    unsigned int  width;
    unsigned int  height;
    unsigned int  bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
    unsigned char pixel_data[7 * 10 * 4 + 1];
} expandUpArrow = {
    7,
    10,
    4,
    "\0\0\0\0\0\0\0\0"
    "445#<<?\317445#\0\0\0\0\0\0\0\0\0\0\0\0"
    "445#@AE\317??"
    "C\317??B\317445#\0\0\0\0"
    "445#GHO\317EGK\317445#DEH\317BDI\317445#JKR\256"
    "IJP\317445#\0\0\0\0"
    "445#DEH\317BCJ\256LNT\317445A\0\0\0\0\0\0\0\0\0\0\0"
    "\0"
    "445ACEI\317\0\0\0\0\0\0\0\0"
    "445#>?B\317445#\0\0\0\0\0\0\0\0\0\0\0\0"
    ""
    "445#DEK\317??C\317BBG\317445#\0\0\0\0"
    "445#MOT\317LMS\317445#IJP\317GH"
    "M\317445#OQW\256OPW\317445#\0\0\0\0"
    "445#IJP\317HIN\256QRY\317445A\0\0\0"
    "\0\0\0\0\0\0\0\0\0"
    "445AJJN\317",
};

QIcon createExpandUpIcon()
{
    QImage  img( expandUpArrow.pixel_data, expandUpArrow.width, expandUpArrow.height, QImage::Format_ARGB32 );
    QPixmap pxMap;
    pxMap = QPixmap::fromImage( img );

    return QIcon( pxMap );
}

static const QIcon& expandUpIcon()
{
    static QIcon expandUpIcon( createExpandUpIcon() );
    return expandUpIcon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMinimizePanel::QMinimizePanel( QWidget* parent /*=0*/ )
    : QFrame( parent )
{
    this->initialize( "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMinimizePanel::QMinimizePanel( const QString& title, QWidget* parent /*=0*/ )
    : QFrame( parent )
{
    this->initialize( title );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMinimizePanel::~QMinimizePanel()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QFrame* QMinimizePanel::contentFrame()
{
    return m_contentFrame;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QMinimizePanel::setTitle( const QString& title )
{
    m_titleLabel->setText( title );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString QMinimizePanel::title() const
{
    return m_titleLabel->text();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QMinimizePanel::enableFrame( bool showFrame )
{
    if ( showFrame )
    {
        this->setFrameStyle( QFrame::StyledPanel | QFrame::Plain );
        m_titleFrame->show();
        m_titleLabel->show();
        m_collapseButton->show();
        m_contentFrame->setObjectName( "FramedGroupContent" );
    }
    else
    {
        this->setFrameStyle( QFrame::NoFrame );
        m_titleFrame->hide();
        m_titleLabel->hide();
        m_collapseButton->hide();
        m_contentFrame->setObjectName( "UnframedGroupContent" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool QMinimizePanel::isExpanded() const
{
    return !m_contentFrame->isHidden();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QMinimizePanel::setExpanded( bool isExpanded )
{
    if ( m_contentFrame->isHidden() != isExpanded ) return;

    m_contentFrame->setVisible( isExpanded );
    if ( isExpanded )
    {
        m_collapseButton->setIcon( expandUpIcon() );
        this->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    }
    else
    {
        m_collapseButton->setIcon( expandDownIcon() );
        this->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
    }

    this->QWidget::updateGeometry();

    emit expandedChanged( isExpanded );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QMinimizePanel::toggleExpanded()
{
    setExpanded( m_contentFrame->isHidden() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void QMinimizePanel::initialize( const QString& title )
{
    this->setFrameStyle( QFrame::StyledPanel | QFrame::Plain );
    QVBoxLayout* fullLayout = new QVBoxLayout( this );

    fullLayout->setContentsMargins( 0, 0, 0, 0 );
    fullLayout->setSpacing( 0 );
    { // Title
        m_titleFrame = new QFrame();
        fullLayout->addWidget( m_titleFrame, 0 );
        fullLayout->setStretch( 0, 0 );
        m_titleFrame->setObjectName( "GroupTitleFrame" );
        m_titleFrame->setStyleSheet( titleFrameStyleSheet() );

        QHBoxLayout* titleLayout = new QHBoxLayout();
        titleLayout->setContentsMargins( 4, 2, 0, 2 );
        m_titleFrame->setLayout( titleLayout );
        m_titleFrame->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
        {
            m_titleLabel               = new QLabel( title );
            QPalette titleLabelPalette = m_titleLabel->palette();
            titleLabelPalette.setBrush( QPalette::Foreground, titleLabelPalette.windowText() );
            m_titleLabel->setPalette( titleLabelPalette );
            titleLayout->addWidget( m_titleLabel, 1, Qt::AlignLeft );
        }
        {
            m_collapseButton = new QPushButton();
            m_collapseButton->setFlat( true );
            m_collapseButton->setIcon( expandUpIcon() );
            m_collapseButton->setDefault( false );
            m_collapseButton->setAutoDefault( false );
            m_collapseButton->setIconSize( QSize( 16, 16 ) );
            m_collapseButton->setMaximumSize( QSize( 16, 16 ) );
            titleLayout->addWidget( m_collapseButton, 0, Qt::AlignRight );
        }
    }
    {
        m_contentFrame = new QFrame();
        m_contentFrame->setStyleSheet( contentFrameStyleSheet() );
        m_contentFrame->setObjectName( "GroupContentFrame" );
        fullLayout->addWidget( m_contentFrame, 1 );
        fullLayout->setStretch( 1, 1 );
    }

    connect( m_collapseButton, SIGNAL( clicked() ), this, SLOT( toggleExpanded() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString QMinimizePanel::titleFrameStyleSheet()
{
    return QString( "QFrame#GroupTitleFrame "
                    "{"
                    "  border-top: none; border-left: none; border-right: none; border-bottom: none;"
                    "  background: qlineargradient(spread:pad, x1:0 y1:0, x2:0 y2:1,"
                    "                              stop:0 rgba(150, 150, 150, 20), stop:1 rgba(0, 0, 0, 50));"
                    "}" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString QMinimizePanel::contentFrameStyleSheet()
{
    return QString( "QFrame#FramedGroupContent"
                    "{"
                    "  border-top: 1px solid darkgray; border-left: none; border-right: none; border-bottom: none; "
                    "  background: rgba(255, 250, 250, 85)"
                    "}"
                    "QFrame#UnframedGroupContent"
                    "{"
                    "  border-top: none; border-left: none; border-right: none; border-bottom: none; "
                    "}" );
}
