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

#include <QWidget>

class QFrame;
class QLabel;
class QPushButton;

//==================================================================================================
//
//
//
//==================================================================================================
class QMinimizePanel : public QWidget
{
    Q_OBJECT
public:
    explicit QMinimizePanel(QWidget* parent=0);
    explicit QMinimizePanel(const QString &title, QWidget* parent=0);
    ~QMinimizePanel();

    QFrame*         contentFrame() { return m_contentFrame; }  
    void            setTitle (const QString& title);
    QString         title() const;

    virtual QSize   sizeHint() const override;

public slots:
    void            setExpanded(bool isExpanded);
    void            toggleExpanded();

signals:
    void            expandedChanged(bool isExpanded);

public:
    virtual QSize   minimumSizeHint() const override;

protected:
    QFrame*         m_titleFrame;
    QLabel*         m_titleLabel;
    QPushButton*    m_collapseButton;
    QFrame*         m_contentFrame;

    virtual void    resizeEvent(QResizeEvent *) override;
    virtual bool    event(QEvent* event) override; // To catch QEvent::LayoutRequest

private:
    void            initialize(const QString &title);
};
