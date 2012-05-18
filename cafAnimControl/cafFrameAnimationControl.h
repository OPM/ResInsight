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

#include <QObject>

class QTimer;


namespace caf
{

//==================================================================================================
/// 
//==================================================================================================
class FrameAnimationControl : public QObject
{
    Q_OBJECT

public:
    FrameAnimationControl(QObject* parent = 0);

    void    setNumFrames(int numFrames);
    int     numFrames() const;
    int     currentFrame() const;

    bool    isActive() const;

    void    setTimeout(int milliSeconds);
    int     timeout() const;

    bool    isRepeatingFromStart() const;
    bool    isRepeatingFwdBwd() const;

public slots:
    void    slotPlayFwd();
    void    slotPlayBwd();
    void    slotStop();
    void    slotPause();
    void    slotStepForward();
    void    slotStepBackward();
    void    setCurrentFrame(int frameIndex);
    void    slotSkipToEnd();
    void    slotSkipToStart();
    void    slotRepeatFromStart(bool turnRepeatOn);
    void    slotRepeatFwdBwd(bool turnRepeatOn);

signals:
    void    changeFrame(int frameIndex);
    void    endAnimation();
    void    frameCountChanged(int frameCount);

private slots:
    void    slotTimerTriggered();

private:
    void    start();
    void    stop();
    void    pause();
    void    stepForward();
    void    stepBackward();
    void    skipToEnd();
    void    skipToStart();

    void    setForward(bool forward);
    bool    forward() const;

    void    setRepeatFromStart(bool repeat);

    void    setRepeatFwdBwd(bool repeat);

    void    setDefault();

private:
    QTimer* m_timer;
    int     m_numFrames;
    int     m_currentFrame;
    int     m_timeout;
    int     m_lastTimeStamp;
    bool    m_forward;
    bool    m_repeatOn;
    bool    m_repeatFromStart;
    bool    m_repeatFwdBwd;
};


} // End namespace caf
