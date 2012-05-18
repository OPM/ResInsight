//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#pragma once

#include <QToolBar>
#include <QPointer>

#include "cafFrameAnimationControl.h"

class QComboBox;
class QLabel;
class QLineEdit;
class QSlider;

namespace caf
{

//==================================================================================================
/// 
//==================================================================================================
class AnimationToolBar : public QToolBar
{
    Q_OBJECT

public:
    AnimationToolBar(QWidget *parent = 0);
    AnimationToolBar(const QString &title, QWidget *parent = 0);

    void connectAnimationControl(caf::FrameAnimationControl* animationControl);

    void setTimeStepStrings(const QStringList& timeStepStrings);

    void setFrameRate(double frameRate);
    void setSlowFrameRate(float frameRate);
    void setFastFrameRate(float frameRate);

    void setCurrentTimeStepIndex(int index);

public slots:
    void slotUpdateTimestepList(int frameCount);

private slots:
    void slotFrameRateSliderChanged(int value);
    void slotFromStartModeToggled(bool on);
    void slotFwdBwdModeToggled(bool on);

private:
    void init();

private:
    QAction*    m_animSkipToStartAction;
    QAction*    m_animStepBackwardAction;
    QAction*    m_animPlayBwdAction;
    QAction*    m_animStopAction;  
    QAction*    m_animPauseAction;  
    QAction*    m_animPlayAction;  
    QAction*    m_animStepForwardAction;  
    QAction*    m_animSkipToEndAction;

    QAction*    m_animRepeatFromStartAction;
    QAction*    m_animRepeatFwdBwdAction;

    QSlider*    m_frameRateSlider;

    QComboBox*  m_timestepCombo;
    
    QPointer<caf::FrameAnimationControl> m_activeAnimationControl;

    float       m_slowFrameRate;
    float       m_fastFrameRate;
    bool        m_hasAutoTimeStepStrings;
};


} // End namespace caf
