//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
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

#include <QPointer>
#include <QToolBar>

#include "cafFrameAnimationControl.h"

class QComboBox;
class QLabel;
class QLineEdit;
class QSlider;
class QToolButton;

namespace caf
{
class PopupMenuButton;

//==================================================================================================
///
//==================================================================================================
class AnimationToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit AnimationToolBar( QWidget* parent = nullptr );
    AnimationToolBar( const QString& title, QWidget* parent = nullptr );

    void connectAnimationControl( caf::FrameAnimationControl* animationControl );

    void setTimeStepStrings( const QStringList& timeStepStrings );

    void setFrameRate( double frameRate );
    void setSlowFrameRate( float frameRate );
    void setFastFrameRate( float frameRate );

    void setCurrentTimeStepIndex( int index );

public slots:
    void slotUpdateTimestepList( int frameCount );
    void playPauseChanged();

private slots:
    void slotFrameRateSliderChanged( int value );
    void slotUpdateAnimationGuiFromFrameIndex( int value );

private:
    void init();
    void updateAnimationButtons();

private:
    QAction*     m_animSkipToStartAction;
    QAction*     m_animStepBackwardAction;
    QToolButton* m_animPlayPauseButton;
    QAction*     m_animPauseAction;
    QAction*     m_animPlayAction;
    QAction*     m_animStepForwardAction;
    QAction*     m_animSkipToEndAction;

    QAction* m_animRepeatFromStartAction;

    PopupMenuButton* m_animSpeedButton;
    QLabel*          m_frameRateFastLabel;
    QLabel*          m_frameRateSlowLabel;
    QSlider*         m_frameRateSlider;

    QComboBox* m_timestepCombo;

    QPointer<caf::FrameAnimationControl> m_activeAnimationControl;

    float m_slowFrameRate;
    float m_fastFrameRate;
    bool  m_hasAutoTimeStepStrings;
};

} // End namespace caf
