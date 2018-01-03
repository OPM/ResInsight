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

#include "QMinimizePanel.h"

#include <QApplication>
#include <QFrame>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

static const struct {
    unsigned int 	 width;
    unsigned int 	 height;
    unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
    unsigned char	 pixel_data[7 * 10 * 4 + 1];
} expandDownArrow = {
    7, 10, 4,
    "QRY\317445A\0\0\0\0\0\0\0\0\0\0\0\0""445AJJN\317OQW\256OPW\317445#\0\0\0"
    "\0""445#IJP\317HIN\256445#MOT\317LMS\317445#IJP\317GHM\317445#\0\0\0\0""4"
    "45#DEK\317??C\317BBG\317445#\0\0\0\0\0\0\0\0\0\0\0\0""445#>?B\317445#\0\0"
    "\0\0\0\0\0\0LNT\317445A\0\0\0\0\0\0\0\0\0\0\0\0""445ACEI\317JKR\256IJP\317"
    "445#\0\0\0\0""445#DEH\317BCJ\256445#GHO\317EGK\317445#DEH\317BDI\317445#"
    "\0\0\0\0""445#@AE\317??C\317??B\317445#\0\0\0\0\0\0\0\0\0\0\0\0""445#<<?"
    "\317445#\0\0\0\0\0\0\0\0",
};

QIcon createExpandDownIcon()
{
    QImage img(expandDownArrow.pixel_data,expandDownArrow.width, expandDownArrow.height, QImage::Format_ARGB32 );
    QPixmap pxMap;
    pxMap = QPixmap::fromImage(img);

    return QIcon(pxMap);
}

static const QIcon& expandDownIcon()
{
    static QIcon expandDownIcon(createExpandDownIcon());
    return expandDownIcon;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

static const struct {
    unsigned int 	 width;
    unsigned int 	 height;
    unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
    unsigned char	 pixel_data[7 * 10 * 4 + 1];
} expandUpArrow = {
    7, 10, 4,
    "\0\0\0\0\0\0\0\0""445#<<?\317445#\0\0\0\0\0\0\0\0\0\0\0\0""445#@AE\317??"
    "C\317??B\317445#\0\0\0\0""445#GHO\317EGK\317445#DEH\317BDI\317445#JKR\256"
    "IJP\317445#\0\0\0\0""445#DEH\317BCJ\256LNT\317445A\0\0\0\0\0\0\0\0\0\0\0"
    "\0""445ACEI\317\0\0\0\0\0\0\0\0""445#>?B\317445#\0\0\0\0\0\0\0\0\0\0\0\0"
    """445#DEK\317??C\317BBG\317445#\0\0\0\0""445#MOT\317LMS\317445#IJP\317GH"
    "M\317445#OQW\256OPW\317445#\0\0\0\0""445#IJP\317HIN\256QRY\317445A\0\0\0"
    "\0\0\0\0\0\0\0\0\0""445AJJN\317",
};

QIcon createExpandUpIcon()
{
    QImage img(expandUpArrow.pixel_data,expandUpArrow.width, expandUpArrow.height, QImage::Format_ARGB32 );
    QPixmap pxMap;
    pxMap = QPixmap::fromImage(img);

    return QIcon(pxMap);
}

static const QIcon& expandUpIcon()
{
    static QIcon expandUpIcon(createExpandUpIcon());
    return expandUpIcon;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QMinimizePanel::QMinimizePanel(QWidget* parent/*=0*/)
{
    this->initialize("");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QMinimizePanel::QMinimizePanel(const QString &title, QWidget* parent/*=0*/)
{
    this->initialize(title);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMinimizePanel::initialize(const QString &title)
{
    m_titleFrame = new QFrame(this);
    m_titleFrame->setFrameStyle(QFrame::Box | QFrame::Plain);
    m_titleFrame->setAutoFillBackground(true);

    m_titleLabel = new QLabel(title, m_titleFrame);
    QPalette titleLabelPalette = m_titleLabel->palette();
    titleLabelPalette.setBrush(QPalette::Foreground, titleLabelPalette.windowText());

    {
        QLinearGradient titleGrad(QPointF(0, 0), QPointF(0, 1));
        titleGrad.setCoordinateMode(QGradient::StretchToDeviceMode);
        titleGrad.setColorAt(0, QColor(255, 255, 255, 20));
        titleGrad.setColorAt(1, QColor(0, 0, 0, 30));

        QPalette titleFramePalette = m_titleFrame->palette();
        titleFramePalette.setBrush(QPalette::Window, titleGrad);
        titleFramePalette.setBrush(QPalette::Foreground, titleFramePalette.dark());
        m_titleFrame->setPalette(titleFramePalette);
    }

    m_titleLabel->setPalette(titleLabelPalette);

    m_collapseButton = new QPushButton( m_titleFrame);
    m_collapseButton->setFlat(true);
    m_collapseButton->setIcon(expandUpIcon());
    m_collapseButton->setDefault(false);
    m_collapseButton->setAutoDefault(false);

    m_contentFrame = new QFrame(this);
    m_contentFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    m_contentFrame->setAutoFillBackground(true);

    QPalette contentFramePalette = m_contentFrame->palette();
    contentFramePalette.setBrush(QPalette::Window, QColor(255,250,250,85));
    m_contentFrame->setPalette(contentFramePalette);

    connect(m_collapseButton, SIGNAL(clicked()),this, SLOT(toggleExpanded()) );
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
void QMinimizePanel::setTitle(const QString& title)
{
    m_titleLabel->setText(title);
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
QSize QMinimizePanel::sizeHint() const
{
    QSize labelSize = m_titleLabel->sizeHint();
    QSize titleBarHint = labelSize + QSize(4 + labelSize.height() + 8 - 2 + 1, 8);

    if (!m_contentFrame->isHidden())
    {
        QSize titleBarMin(0, labelSize.height() + 8);
        QSize contentsMin(m_contentFrame->sizeHint());
        QSize total = contentsMin.expandedTo(titleBarMin);
        total.rheight() += titleBarMin.height();
        
        return total;
    }
    else
    {
        return titleBarHint;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMinimizePanel::setExpanded(bool isExpanded)
{
    if (m_contentFrame->isHidden() != isExpanded) return;

    m_contentFrame->setVisible(isExpanded);
    isExpanded ? m_collapseButton->setIcon(expandUpIcon()) : m_collapseButton->setIcon(expandDownIcon());
    this->QWidget::updateGeometry();
    
    emit expandedChanged(isExpanded);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMinimizePanel::toggleExpanded()
{
    setExpanded(m_contentFrame->isHidden());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSize QMinimizePanel::minimumSizeHint() const
{
    QSize labelSize =  m_titleLabel->sizeHint();
    QSize titleBarHint = labelSize + QSize(4 + labelSize.height() + 8 - 2 + 1, 8);

    if (!m_contentFrame->isHidden())
    {
        QSize titleBarMin(0, labelSize.height() + 8  );
        QSize contentsMin(m_contentFrame->minimumSizeHint());
        QSize total = contentsMin.expandedTo(titleBarMin);
        total.rheight() +=  titleBarMin.height();
        
        return total;
    }
    else
    {
        return titleBarHint;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QMinimizePanel::resizeEvent(QResizeEvent *resizeEv )
{
    QWidget::updateGeometry();

    int width = resizeEv->size().width();
    int heigth = resizeEv->size().height();
    int labelHeight = m_titleLabel->sizeHint().height();
    int titleHeight = labelHeight + 8;

    int buttonSize = titleHeight - 2;

    m_titleFrame->setGeometry(0,0,width, titleHeight);
    m_titleLabel->setGeometry( 4, titleHeight - labelHeight - 4, width - 4 - buttonSize - 1, labelHeight);
    m_collapseButton->setGeometry(width - buttonSize - 1, 1, buttonSize, buttonSize);
    
    m_contentFrame->setGeometry(0, titleHeight-1, width, heigth - (titleHeight-1));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool QMinimizePanel::event(QEvent* event)
{
    if (event->type() == QEvent::LayoutRequest)
    {
        this->QWidget::updateGeometry();
    }

   return this->QWidget::event(event);    
}
