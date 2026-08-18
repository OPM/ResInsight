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
class QTimer;

class RiuAbstractOverlayContentFrame : public QFrame
{
    Q_OBJECT
public:
    RiuAbstractOverlayContentFrame( QWidget* parent = nullptr );
    ~RiuAbstractOverlayContentFrame() override;

    virtual void renderTo( QPainter* painter, const QRect& targetRect ) = 0;

protected:
    void updateFontSize();
};

class RiuTextOverlayContentFrame : public RiuAbstractOverlayContentFrame
{
    Q_OBJECT
public:
    RiuTextOverlayContentFrame( QWidget* parent = nullptr );

    void setText( const QString& text );
    void renderTo( QPainter* painter, const QRect& targetRect ) override;

private:
    void updateLabelFont();

private:
    QPointer<QLabel> m_textLabel;
};

//==================================================================================================
/// Says that something is going on, for work that finishes on its own and reports no progress along the
/// way. The animation is driven by a timer that only runs while the frame is visible, so a frame that has
/// been taken off a plot costs nothing.
//==================================================================================================
class RiuSpinnerOverlayContentFrame : public RiuAbstractOverlayContentFrame
{
    Q_OBJECT
public:
    RiuSpinnerOverlayContentFrame( QWidget* parent = nullptr );

    void setText( const QString& text );
    void renderTo( QPainter* painter, const QRect& targetRect ) override;

protected:
    void paintEvent( QPaintEvent* event ) override;
    void showEvent( QShowEvent* event ) override;
    void hideEvent( QHideEvent* event ) override;

private:
    void       drawSpinner( QPainter* painter, const QPoint& topLeft ) const;
    static int spinnerSize();
    static int spinnerMargin();
    void       updateLabelFont();

private:
    QPointer<QLabel> m_textLabel;
    QTimer*          m_animationTimer = nullptr;
    int              m_angleDegrees   = 0;
};
