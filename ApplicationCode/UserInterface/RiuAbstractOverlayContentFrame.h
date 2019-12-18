/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
#pragma once

#include <QFrame>
#include <QPointer>
#include <QString>

class QLabel;

class RiuAbstractOverlayContentFrame : public QFrame
{
    Q_OBJECT
public:
    RiuAbstractOverlayContentFrame( QWidget* parent = nullptr );
    ~RiuAbstractOverlayContentFrame();

    virtual void renderTo( QPainter* painter, const QRect& targetRect ) = 0;
};

class RiuTextOverlayContentFrame : public RiuAbstractOverlayContentFrame
{
    Q_OBJECT
public:
    RiuTextOverlayContentFrame( QWidget* parent = nullptr );

    void setText( const QString& text );
    void renderTo( QPainter* painter, const QRect& targetRect ) override;

private:
    QPointer<QLabel> m_textLabel;
};
