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

#include "cafPopupMenuButton.h"

#include <QAction>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>

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
    m_animPauseAction        = new QAction(QIcon(":/cafAnimControl/Pause.png"),          tr("Pause"), this);
    m_animPlayAction         = new QAction(QIcon(":/cafAnimControl/Play.png"),           tr("Play"), this);
    m_animPlayPauseButton    = new QToolButton(this);
    m_animPlayPauseButton->setIcon(m_animPlayAction->icon());
    m_animPlayPauseButton->setToolTip(m_animPlayAction->toolTip());
    QObject::connect(m_animPlayPauseButton, SIGNAL(clicked()), this, SLOT(playPauseChanged()));

    m_animStepForwardAction  = new QAction(QIcon(":/cafAnimControl/StepFwd.png"),        tr("Step Forward"), this);
    m_animSkipToEndAction    = new QAction(QIcon(":/cafAnimControl/SkipToEnd.png"),      tr("Skip to End"), this);
    
    m_animRepeatFromStartAction = new QAction(QIcon(":/cafAnimControl/RepeatFromStart.png"),      tr("Repeat From start"), this);
    m_animRepeatFromStartAction->setCheckable(true);
   
    m_animSpeedButton = new PopupMenuButton(this);
    m_animSpeedButton->setIcon(QIcon(":/cafAnimControl/Speed.png"));
    m_animSpeedButton->setToolTip("Adjust Animation Speed");

    m_frameRateSlowLabel = new QLabel(this);
    m_frameRateSlowLabel->setPixmap(QPixmap(":/cafAnimControl/SlowHorizontal.png"));
    m_frameRateSlowLabel->setToolTip(tr("Slow"));

    m_frameRateFastLabel = new QLabel(this);
    m_frameRateFastLabel->setPixmap(QPixmap(":/cafAnimControl/FastHorizontal.png"));
    m_frameRateFastLabel->setToolTip(tr("Fast"));
    m_frameRateFastLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_frameRateSlider = new QSlider(Qt::Horizontal, this);
    m_frameRateSlider->setToolTip(tr("Animation speed"));
    m_frameRateSlider->setMinimumWidth(100);
        
    m_animSpeedButton->addWidget(m_frameRateSlowLabel);
    m_animSpeedButton->addWidget(m_frameRateSlider);
    m_animSpeedButton->addWidget(m_frameRateFastLabel);
    
    m_timestepCombo = new QComboBox(this);
    m_timestepCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_timestepCombo->setToolTip(tr("Current Time Step"));

    QAction* separator1 = new QAction(this);
    separator1->setSeparator(true);
    QAction* separator2 = new QAction(this);
    separator2->setSeparator(true);
    QAction* separator3 = new QAction(this);
    separator3->setSeparator(true);
    
    // Add actions and widgets to animation toolbar
    addAction(m_animSkipToStartAction);
    addAction(m_animStepBackwardAction);
    addWidget(m_animPlayPauseButton);
    addAction(m_animStepForwardAction);
    addAction(m_animSkipToEndAction);

    addAction(separator1);

    addAction(m_animRepeatFromStartAction );

    addAction(separator2);
    
    addWidget(m_animSpeedButton);

    addAction(separator3);

    addWidget(m_timestepCombo);

    updateAnimationButtons();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void AnimationToolBar::updateAnimationButtons()
{
    bool isPlaying = m_activeAnimationControl && m_activeAnimationControl->isActive();
    
    if (isPlaying)
    {
        m_animPlayPauseButton->setIcon(m_animPauseAction->icon());
        m_animPlayPauseButton->setToolTip(m_animPauseAction->toolTip());
    }
    else
    {
        m_animPlayPauseButton->setIcon(m_animPlayAction->icon());
        m_animPlayPauseButton->setToolTip(m_animPlayAction->toolTip());
    }

    bool isAtStart = m_timestepCombo->count() == 0 || m_timestepCombo->currentIndex() == 0;
    bool isAtEnd   = m_timestepCombo->count() > 0 && m_timestepCombo->currentIndex() == m_timestepCombo->count() - 1;

    // Going backwards actions disabled when we're stopped at the start
    m_animSkipToStartAction->setEnabled(isPlaying || !isAtStart);
    m_animStepBackwardAction->setEnabled(isPlaying || !isAtStart);

    bool isRepeat = m_activeAnimationControl &&
                    m_activeAnimationControl->isRepeatingFromStart();

    // Going forwards actions disabled when we're stopped at the end
    m_animStepForwardAction->setEnabled(isPlaying || !isAtEnd);
    m_animSkipToEndAction->setEnabled(isPlaying || !isAtEnd);
    // ... however we allow playing if we have repeat on
    m_animPlayPauseButton->setEnabled(isPlaying || isRepeat || !isAtEnd);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AnimationToolBar::connectAnimationControl(caf::FrameAnimationControl* animationControl)
{
    // Animation action connections
    if (m_activeAnimationControl)
    {
        m_activeAnimationControl->disconnect(this, SLOT(slotUpdateAnimationGuiFromFrameIndex(int)));
        m_activeAnimationControl->disconnect(this, SLOT(slotUpdateTimestepList(int)));
    }

    m_activeAnimationControl = animationControl;

    m_animSkipToStartAction->disconnect();
    m_animStepBackwardAction->disconnect();
    m_animPauseAction->disconnect();  
    m_animPlayAction->disconnect();
    m_animStepForwardAction->disconnect();
    m_animSkipToEndAction->disconnect();

    m_animRepeatFromStartAction->disconnect();

    m_timestepCombo->disconnect();
    m_frameRateSlider->disconnect();

    if (animationControl)
    {
        connect(m_animSkipToStartAction,    SIGNAL(triggered()),        animationControl, SLOT(slotSkipToStart()));
        connect(m_animStepBackwardAction,   SIGNAL(triggered()),        animationControl, SLOT(slotStepBackward()));
        connect(m_animPauseAction,          SIGNAL(triggered()),        animationControl, SLOT(slotPause()));
        connect(m_animPlayAction,           SIGNAL(triggered()),        animationControl, SLOT(slotPlayFwd()));
        connect(m_animStepForwardAction,    SIGNAL(triggered()),        animationControl, SLOT(slotStepForward()));
        connect(m_animSkipToEndAction,      SIGNAL(triggered()),        animationControl, SLOT(slotSkipToEnd()));

        m_animRepeatFromStartAction->setChecked(animationControl->isRepeatingFromStart());

        connect(m_animRepeatFromStartAction,SIGNAL(triggered(bool)),    animationControl, SLOT(slotRepeatFromStart(bool)));
        
        connect(m_timestepCombo,            SIGNAL(currentIndexChanged(int)), animationControl, SLOT(setCurrentFrame(int)));
        connect(m_frameRateSlider,          SIGNAL(valueChanged(int)),  this,             SLOT(slotFrameRateSliderChanged(int)));

        connect(animationControl,           SIGNAL(changeFrame(int)),       this,         SLOT(slotUpdateAnimationGuiFromFrameIndex(int)));
        connect(animationControl,           SIGNAL(frameCountChanged(int)), this,         SLOT(slotUpdateTimestepList(int)));

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
    updateAnimationButtons();
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
    updateAnimationButtons();
    m_timestepCombo->blockSignals(false);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void AnimationToolBar::playPauseChanged()
{
    if (m_activeAnimationControl->isActive())
    {
        m_animPauseAction->trigger();
        updateAnimationButtons();
    }
    else
    {
        m_animPlayAction->trigger();
        updateAnimationButtons();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void AnimationToolBar::slotUpdateAnimationGuiFromFrameIndex(int value)
{
    // Update only the combo box index, but do not set current frame 
    // Disconnect the signal temporarily when updating UI

    disconnect(m_timestepCombo, SIGNAL(currentIndexChanged(int)), m_activeAnimationControl, SLOT(setCurrentFrame(int)));
    m_timestepCombo->setCurrentIndex(value);
    updateAnimationButtons();
    connect(m_timestepCombo, SIGNAL(currentIndexChanged(int)), m_activeAnimationControl, SLOT(setCurrentFrame(int)));
}

} // End namespace caf
