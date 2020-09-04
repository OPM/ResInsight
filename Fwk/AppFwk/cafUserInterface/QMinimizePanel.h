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

#pragma once

#include <QFrame>
#include <QIcon>

class QLabel;
class QPushButton;

//==================================================================================================
//
//
//
//==================================================================================================
class QMinimizePanel : public QFrame
{
    Q_OBJECT
    Q_PROPERTY( QString background READ getBackground WRITE setBackground DESIGNABLE true )
    Q_PROPERTY( QString titleBackground READ getTitleBackground WRITE setTitleBackground DESIGNABLE true )
    Q_PROPERTY( QString border READ getBorder WRITE setBorder DESIGNABLE true )
    Q_PROPERTY( QString expandIconPath READ getExpandIconPath WRITE setExpandIconPath DESIGNABLE true )
    Q_PROPERTY( QString collapseIconPath READ getCollapseIconPath WRITE setCollapseIconPath DESIGNABLE true )
    Q_PROPERTY( QString iconSize READ getIconSize WRITE setIconSize DESIGNABLE true )
public:
    explicit QMinimizePanel( QWidget* parent = nullptr );
    explicit QMinimizePanel( const QString& title, QWidget* parent = nullptr );
    ~QMinimizePanel() override;

    QFrame* contentFrame();
    void    setTitle( const QString& title );
    QString title() const;
    void    enableFrame( bool showFrame );
    bool    isExpanded() const;

    QString getBackground() const;
    void    setBackground( QString background );

    QString getTitleBackground() const;
    void    setTitleBackground( QString background );

    QString getBorder() const;
    void    setBorder( QString border );

    QString getExpandIconPath() const;
    void    setExpandIconPath( QString path );

    QString getCollapseIconPath() const;
    void    setCollapseIconPath( QString path );

    QString getIconSize() const;
    void    setIconSize( QString size );

public slots:
    void setExpanded( bool isExpanded );
    void toggleExpanded();

signals:
    void expandedChanged( bool isExpanded );

protected:
    QFrame*      m_titleFrame;
    QLabel*      m_titleLabel;
    QPushButton* m_collapseButton;
    QFrame*      m_contentFrame;

private:
    void initialize( const QString& title );

    QString titleFrameStyleSheet();
    QString contentFrameStyleSheet();

    QString m_background;
    QString m_titleBackground;
    QString m_border;
    QString m_expandIconPath;
    QString m_collapseIconPath;
    QString m_iconSize;
    QIcon   m_expandIcon;
    QIcon   m_collapseIcon;
};
