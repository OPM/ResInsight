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

#include "cafFrameAnimationControl.h"

#include <QTimer>

namespace caf
{
//==================================================================================================
///
/// \class RIAnimationControl
///
/// Animation control class
///
//==================================================================================================

// Default timeout 100 ms, 10 FPS
static const int TIMEOUT_DEFAULT = 100;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FrameAnimationControl::FrameAnimationControl( QObject* parent )
    : QObject( parent )
{
    m_timer = new QTimer( this );
    connect( m_timer, SIGNAL( timeout() ), SLOT( slotTimerTriggered() ) );

    setDefault();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::setDefault()
{
    setCurrentFrame( 0 );
    setNumFrames( 0 );
    setTimeout( TIMEOUT_DEFAULT );
    setForward( true );
    setRepeatFromStart( false );
    setRepeatFwdBwd( false );
    //    m_lastTimeStamp = 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::start()
{
    m_timer->start( m_timeout );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::stop()
{
    m_timer->stop();
    emit endAnimation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::pause()
{
    m_timer->stop();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::stepForward()
{
    if ( m_currentFrame < m_numFrames - 1 )
    {
        m_timer->stop();
        setCurrentFrame( m_currentFrame + 1 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::stepBackward()
{
    if ( m_currentFrame >= 1 )
    {
        m_timer->stop();
        setCurrentFrame( m_currentFrame - 1 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool FrameAnimationControl::isActive() const
{
    return m_timer->isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::setCurrentFrame( int frameIndex )
{
    if ( frameIndex >= 0 )
    {
        m_currentFrame = frameIndex;
        emit changeFrame( m_currentFrame );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int FrameAnimationControl::currentFrame() const
{
    return m_currentFrame;
}

//--------------------------------------------------------------------------------------------------
/// Set current frame without emitting signal
/// Used when views are linked and need to update current frame without emitting a signal
/// Emitting a signal will cause infinite recursion
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::setCurrentFrameOnly( int frameIndex )
{
    if ( frameIndex >= 0 )
    {
        m_currentFrame = frameIndex;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::setNumFrames( int numFrames )
{
    m_numFrames = numFrames < 0 ? 0 : numFrames;

    emit frameCountChanged( m_numFrames );

    if ( m_currentFrame >= numFrames ) m_currentFrame = 0; // Should we emit frameChanged ?
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int FrameAnimationControl::numFrames() const
{
    return m_numFrames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::setTimeout( int milliSeconds )
{
    m_timeout = milliSeconds < 0 ? 0 : milliSeconds;
    if ( isActive() ) start();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int FrameAnimationControl::timeout() const
{
    return m_timeout;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::setForward( bool forward )
{
    m_forward = forward;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool FrameAnimationControl::forward() const
{
    return m_forward;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::setRepeatFromStart( bool turnRepeatOn )
{
    m_repeatFromStart = turnRepeatOn;
    if ( turnRepeatOn ) m_repeatFwdBwd = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool FrameAnimationControl::isRepeatingFromStart() const
{
    return m_repeatFromStart;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::setRepeatFwdBwd( bool turnRepeatOn )
{
    m_repeatFwdBwd = turnRepeatOn;
    if ( turnRepeatOn ) m_repeatFromStart = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool FrameAnimationControl::isRepeatingFwdBwd() const
{
    return m_repeatFwdBwd;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::slotPlayFwd()
{
    setForward( true );
    start();
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::slotPlayBwd()
{
    setForward( false );
    start();
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::slotStop()
{
    stop();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::slotPause()
{
    pause();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::slotStepForward()
{
    stepForward();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::slotStepBackward()
{
    stepBackward();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::slotTimerTriggered()
{
    // Update current frame according to settings
    if ( m_forward )
    {
        if ( m_currentFrame + 1 >= m_numFrames )
        {
            if ( m_repeatFromStart )
            {
                m_currentFrame = 0;
            }
            else if ( m_repeatFwdBwd )
            {
                setForward( false );
                m_currentFrame--;
            }
            else
            {
                m_timer->stop();
                m_currentFrame = m_numFrames - 1;
            }
        }
        else
        {
            m_currentFrame++;
        }
    }
    else
    {
        if ( m_currentFrame - 1 < 0 )
        {
            if ( m_repeatFromStart )
            {
                m_currentFrame = m_numFrames - 1;
            }
            else if ( m_repeatFwdBwd )
            {
                setForward( true );
                m_currentFrame++; // Ends up as 1 (second frame) makes 2 1 0 1 2 and not 2 1 0 0 1 2
            }
            else
            {
                m_timer->stop();
                m_currentFrame = 0;
            }
        }
        else
        {
            m_currentFrame--;
        }
    }

    // Emit signal with updated frame index
    emit changeFrame( m_currentFrame );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::skipToEnd()
{
    m_timer->stop();
    setCurrentFrame( m_numFrames - 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::skipToStart()
{
    m_timer->stop();
    setCurrentFrame( 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::slotSkipToEnd()
{
    skipToEnd();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::slotSkipToStart()
{
    skipToStart();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::slotRepeatFromStart( bool turnRepeatOn )
{
    setRepeatFromStart( turnRepeatOn );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FrameAnimationControl::slotRepeatFwdBwd( bool turnRepeatOn )
{
    setRepeatFwdBwd( turnRepeatOn );
}

} // End namespace caf
