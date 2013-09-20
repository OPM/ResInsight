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


#include "cafAnimationToolBar.h"

#include <QAction>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>


namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
AnimationToolBar::AnimationToolBar(QWidget *parent /*= 0*/)
    : QToolBar (parent)
{
    setObjectName("AnimationToolBar");
    init();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
AnimationToolBar::AnimationToolBar(const QString &title, QWidget *parent /*= 0*/)
    : QToolBar(title, parent)
{
    setObjectName(title);
    init();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AnimationToolBar::init()
{
    m_hasAutoTimeStepStrings = true;
    m_slowFrameRate = 0.25;
    m_fastFrameRate = 20;

    // Create actions and widgets
    m_animSkipToStartAction  = new QAction(QIcon(":/cafAnimControl/SkipToStart.png"),    tr("Skip to Start"), this);
    m_animStepBackwardAction = new QAction(QIcon(":/cafAnimControl/StepBwd.png"),        tr("Step Backward"), this);
    m_animPlayBwdAction      = new QAction(QIcon(":/cafAnimControl/PlayBwd.png"),        tr("Play Backwards"), this);
    m_animStopAction         = new QAction(QIcon(":/cafAnimControl/Stop.png"),           tr("Stop"), this);
    m_animPauseAction        = new QAction(QIcon(":/cafAnimControl/Pause.png"),          tr("Pause"), this);
    m_animPlayAction         = new QAction(QIcon(":/cafAnimControl/Play.png"),           tr("Play"), this);
    m_animStepForwardAction  = new QAction(QIcon(":/cafAnimControl/StepFwd.png"),        tr("Step Forward"), this);
    m_animSkipToEndAction    = new QAction(QIcon(":/cafAnimControl/SkipToEnd.png"),      tr("Skip to End"), this);
    
    m_animRepeatFromStartAction = new QAction(QIcon(":/cafAnimControl/RepeatFromStart.png"),      tr("Repeat From start"), this);
    m_animRepeatFromStartAction->setCheckable(true);
    m_animRepeatFwdBwdAction    = new QAction(QIcon(":/cafAnimControl/RepeatFwdBwd.png"),      tr("Repeat Forward/Backward"), this);
    m_animRepeatFwdBwdAction->setCheckable(true);
   
    m_timestepCombo = new QComboBox(this);
    m_timestepCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_timestepCombo->setToolTip(tr("Current Time Step"));

    QLabel* slowLabel = new QLabel ( this);
    slowLabel->setPixmap(QPixmap(":/cafAnimControl/Slow.png"));
    slowLabel->setToolTip(tr("Slow"));
    QLabel* fastLabel = new QLabel(this);
    fastLabel->setPixmap(QPixmap(":/cafAnimControl/Fast.png"));
    fastLabel->setAlignment(Qt::AlignRight);
    fastLabel->setToolTip(tr("Fast"));

    m_frameRateSlider = new QSlider(Qt::Horizontal, this);
    m_frameRateSlider->setMaximumWidth(75);
    m_frameRateSlider->setToolTip(tr("Animation speed"));

    QAction* separator1 = new QAction(this);
    separator1->setSeparator(true);
    QAction* separator2 = new QAction(this);
    separator2->setSeparator(true);
    QAction* separator3 = new QAction(this);
    separator3->setSeparator(true);
    
    // Add actions and widgets to animation toolbar
    addAction(m_animSkipToStartAction);
    addAction(m_animStepBackwardAction);
    addAction(m_animPlayBwdAction         );
    //addAction(m_animStopAction);
    addAction(m_animPauseAction);
    addAction(m_animPlayAction);
    addAction(m_animStepForwardAction);
    addAction(m_animSkipToEndAction);

    addAction(separator1);

    addAction(m_animRepeatFromStartAction );
    addAction(m_animRepeatFwdBwdAction    );

    addAction(separator2);

    addWidget(slowLabel);
    addWidget(m_frameRateSlider);
    addWidget(fastLabel);

    addAction(separator3);

    addWidget(m_timestepCombo);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AnimationToolBar::connectAnimationControl(caf::FrameAnimationControl* animationControl)
{
    // Animation action connections
    if (m_activeAnimationControl)
    {
        m_activeAnimationControl->disconnect(m_timestepCombo, SLOT(setCurrentIndex(int)));
    }

    m_activeAnimationControl = animationControl;

    m_animSkipToStartAction->disconnect();
    m_animStepBackwardAction->disconnect();
    m_animStopAction->disconnect();  
    m_animPauseAction->disconnect();  
    m_animPlayAction->disconnect();
    m_animStepForwardAction->disconnect();
    m_animSkipToEndAction->disconnect();

    m_animPlayBwdAction        ->disconnect();
    m_animRepeatFromStartAction->disconnect();
    m_animRepeatFwdBwdAction   ->disconnect();

    m_timestepCombo->disconnect();
    m_frameRateSlider->disconnect();

    if (animationControl)
    {
        connect(m_animSkipToStartAction,    SIGNAL(triggered()), animationControl, SLOT(slotSkipToStart()));
        connect(m_animStepBackwardAction,   SIGNAL(triggered()), animationControl, SLOT(slotStepBackward()));
        connect(m_animStopAction,           SIGNAL(triggered()), animationControl, SLOT(slotStop()));
        connect(m_animPauseAction,          SIGNAL(triggered()), animationControl, SLOT(slotPause()));
        connect(m_animPlayAction,           SIGNAL(triggered()), animationControl, SLOT(slotPlayFwd()));
        connect(m_animStepForwardAction,    SIGNAL(triggered()), animationControl, SLOT(slotStepForward()));
        connect(m_animSkipToEndAction,      SIGNAL(triggered()), animationControl, SLOT(slotSkipToEnd()));

        connect(m_animPlayBwdAction        ,SIGNAL(triggered()), animationControl, SLOT(slotPlayBwd()));
        m_animRepeatFromStartAction->setChecked(animationControl->isRepeatingFromStart());
        m_animRepeatFwdBwdAction->setChecked(animationControl->isRepeatingFwdBwd());
        connect(m_animRepeatFromStartAction,SIGNAL(triggered(bool)), animationControl, SLOT(slotRepeatFromStart(bool)));
        connect(m_animRepeatFwdBwdAction   ,SIGNAL(triggered(bool)), animationControl, SLOT(slotRepeatFwdBwd(bool)));

        connect(m_animRepeatFromStartAction,SIGNAL(triggered(bool)), this, SLOT(slotFromStartModeToggled(bool)));
        connect(m_animRepeatFwdBwdAction   ,SIGNAL(triggered(bool)), this, SLOT(slotFwdBwdModeToggled(bool)));
        
        connect(m_timestepCombo,            SIGNAL(currentIndexChanged(int)), animationControl, SLOT(setCurrentFrame(int)));
        connect(m_frameRateSlider,          SIGNAL(valueChanged(int)), this, SLOT(slotFrameRateSliderChanged(int)));

        connect(animationControl, SIGNAL(changeFrame(int)), SLOT(slotUpdateComboBoxIndex(int)));
        connect(animationControl, SIGNAL(frameCountChanged(int)), this, SLOT(slotUpdateTimestepList(int)));
        int timeout = animationControl->timeout();
        double initialFrameRate = 1000;
        if (timeout > 0) initialFrameRate = 1000.0/timeout;
        setFrameRate(initialFrameRate);
        
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AnimationToolBar::setFrameRate(double frameRate)
{
    float sliderRange = m_frameRateSlider->maximum() - m_frameRateSlider->minimum();

    float frameRateRange = m_fastFrameRate - m_slowFrameRate;
    float normalizedSliderPosition = (frameRate - m_slowFrameRate )/ frameRateRange ;
    float sliderTickValue = sliderRange* normalizedSliderPosition ;

    m_frameRateSlider->blockSignals(true);
    m_frameRateSlider->setValue(static_cast<int>(sliderTickValue));
    m_frameRateSlider->blockSignals(false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AnimationToolBar::setTimeStepStrings(const QStringList& timeStepStrings)
{
    if (timeStepStrings.empty())
    {
        m_hasAutoTimeStepStrings = true;
    }
    else
    {
        m_hasAutoTimeStepStrings = false;
    }

    m_timestepCombo->blockSignals(true);
    m_timestepCombo->clear();

    m_timestepCombo->addItems(timeStepStrings);

    m_timestepCombo->blockSignals(false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AnimationToolBar::setCurrentTimeStepIndex(int index)
{
    m_timestepCombo->blockSignals(true);
    m_timestepCombo->setCurrentIndex(index);
    m_timestepCombo->blockSignals(false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AnimationToolBar::slotFrameRateSliderChanged(int sliderTickValue)
{
    float sliderRange = m_frameRateSlider->maximum() - m_frameRateSlider->minimum();
    float normalizedSliderPosition = sliderTickValue / sliderRange;

    float frameRateRange = m_fastFrameRate - m_slowFrameRate;
    float newFrameRate = m_slowFrameRate + frameRateRange * normalizedSliderPosition;

    if (newFrameRate > m_fastFrameRate)
    {
        newFrameRate = m_fastFrameRate;
    }

    if (newFrameRate < m_slowFrameRate)
    {
        newFrameRate = m_slowFrameRate;
    }

    setFrameRate(newFrameRate);


    if(!m_activeAnimationControl.isNull()) m_activeAnimationControl->setTimeout((int)(1.0/newFrameRate * 1000));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AnimationToolBar::setSlowFrameRate(float frameRate)
{
    m_slowFrameRate = frameRate;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AnimationToolBar::setFastFrameRate(float frameRate)
{
    m_fastFrameRate = frameRate;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AnimationToolBar::slotUpdateTimestepList(int frameCount)
{
    QStringList timeStepNames;
    for (int vIdx = 0; vIdx < frameCount; ++vIdx)
    {
        timeStepNames.append(QString().setNum(vIdx));
    }
    m_timestepCombo->blockSignals(true);

    m_timestepCombo->clear();
    m_timestepCombo->addItems(timeStepNames);
    m_timestepCombo->blockSignals(false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AnimationToolBar::slotFromStartModeToggled(bool on)
{
    if (on) 
    {
        m_animRepeatFwdBwdAction->blockSignals(true);
        m_animRepeatFwdBwdAction->setChecked(false);
        m_animRepeatFwdBwdAction->blockSignals(false);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AnimationToolBar::slotFwdBwdModeToggled(bool on)
{
    if (on) 
    {
        m_animRepeatFromStartAction->blockSignals(true);
        m_animRepeatFromStartAction->setChecked(false);
        m_animRepeatFromStartAction->blockSignals(false);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AnimationToolBar::slotUpdateComboBoxIndex(int value)
{
    // Update only the combo box index, but do not set current frame 
    // Disconnect the signal temporarily when updating UI

    disconnect(m_timestepCombo, SIGNAL(currentIndexChanged(int)), m_activeAnimationControl, SLOT(setCurrentFrame(int)));
    m_timestepCombo->setCurrentIndex(value);
    connect(m_timestepCombo, SIGNAL(currentIndexChanged(int)), m_activeAnimationControl, SLOT(setCurrentFrame(int)));
}

} // End namespace caf
