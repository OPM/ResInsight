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

// Needed for moc file
#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfDrawableGeo.h"
#include "cvfTimer.h"

#include "cvfuTestSnippet.h"

#include "cvfqtOpenGLWidget.h"

class QTimer;


//==================================================================================================
//
// 
//
//==================================================================================================
class QSRSnippetWidget : public cvfqt::OpenGLWidget
{
    Q_OBJECT

public:
    QSRSnippetWidget(cvfu::TestSnippet* snippet, cvf::OpenGLContextGroup* contextGroup, const QGLFormat& format, QWidget* parent);
    ~QSRSnippetWidget();

    cvfu::TestSnippet*      snippet();

    void                            setRenderMode(cvf::DrawableGeo::RenderMode mode);
    cvf::DrawableGeo::RenderMode    renderMode() const;
    void                            enableForcedImmediateMode(bool enable);
    bool                            isForcedImmediateModeEnabled() const;

    void    enablePerfInfoHUD(bool drawHUD);
    bool    isPerfInfoHudEnabled() const;

    void    enableViewFrustumCulling(bool enable);
    bool    isViewFrustumCullingEnabled() const;
    void    enablePixelSizeCulling(bool enable);
    bool    isPixelSizeCullingEnabled() const;
    void    setPixelSizeCullingThreshold(double threshold);
    double  pixelSizeCullingThreshold() const;

    void    enableItemCountUpdates(bool enable);
    bool    isItemCountUpdatesEnabled() const;
    void    setRenderDrawablesDisabled(bool disable);
    bool    isRenderDrawablesDisabled() const;
    void    setApplyEffectsDisabled(bool disable);
    bool    isApplyEffectsDisabled() const;
    
    void    convertDrawablesToShort();
    void    showModelStatistics();
    void    enableMultisampleWhenDrawing(bool enable);

private:
    void    initializeGL();
    void    paintEvent(QPaintEvent *event);
    void    resizeGL(int width, int height);

    void    mousePressEvent(QMouseEvent* event);
    void    mouseMoveEvent(QMouseEvent* event);
    void    mouseReleaseEvent(QMouseEvent* event);
    void    keyPressEvent(QKeyEvent* event);

	void	drawHud(QPainter* painter, cvf::OpenGLContext* oglContext) const;

private slots:
    void    slotAnimationUpdateTimer();

private:
    cvf::ref<cvfu::TestSnippet>     m_snippet;                      // The test snip
    cvf::Timer                      m_currAnimationTime;            // Keeps track of current animation time.
    QTimer*                         m_animationUpdateTimer;         // Qt timer for triggering animation updates
    bool                            m_drawHUD;                      // Whether to draw the performance info HUD
    cvf::DrawableGeo::RenderMode    m_lastSetRenderMode;            // The last render mode that has been set
    bool                            m_enableMultisampleWhenDrawing;
};


