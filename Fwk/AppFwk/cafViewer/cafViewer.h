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

#include "cvfAssert.h"
#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfCollection.h"
#include "cvfVector3.h"

#include "cvfOpenGL.h"
#include "cafOpenGLWidget.h"


namespace cvf {
    class Camera;
    class Scene;
    class Rendering;
    class RenderSequence;
    class OverlayScalarMapperLegend;
    class HitItemCollection;
    class OverlayImage;
    class TextureImage;
}

namespace caf {
    class FrameAnimationControl;
    class Viewer;
    class NavigationPolicy;
}

class QInputEvent;
#include <QPointer>


namespace caf
{


class Viewer : public caf::OpenGLWidget
{
    Q_OBJECT
public:
    Viewer(const QGLFormat& format, QWidget* parent);
    ~Viewer();

    QWidget*                layoutWidget() { return m_layoutWidget; } // Use this when putting it into something
    cvf::Camera*            mainCamera();

    // Set the main scene : the scene active when the animation is not active. (Stopped)
    void                    setMainScene(cvf::Scene* scene);
    cvf::Scene*             mainScene();
    cvf::Scene*             currentScene(); // The scene currently rendered

    // Frame scenes for animation control
    void                    addFrame(cvf::Scene* scene);
    size_t                  frameCount()                                                       { return m_frameScenes.size(); }
    cvf::Scene*             frame(size_t frameIndex); 
    void                    removeAllFrames();

    // Recursively traverse all the scenes managed by the viewer and make sure all cached values are up-to-date
    // Use when changing the contents inside the objects in the scene.
    // As of yet: Only updates bounding boxes. Other things might be added later.
    void                    updateCachedValuesInScene();

    bool                    isAnimationActive();
    caf::FrameAnimationControl* 
                            animationControl()                                             { return m_animationControl;}
    void                    setReleaseOGLResourcesEachFrame(bool releaseOGLResourcesEachFrame) { m_releaseOGLResourcesEachFrame = releaseOGLResourcesEachFrame; }

    // Set the navigation policy
    void                    setNavigationPolicy(caf::NavigationPolicy* navigationPolicy);
    void                    enableNavigationPolicy(bool enable); 
    void                    setView( const cvf::Vec3d& alongDirection, const cvf::Vec3d& upDirection );
    void                    zoomAll();

    void                    setMinNearPlaneDistance(double dist);
    void                    setMaxFarPlaneDistance(double dist);

    // Test whether it is any point in doing navigation etc.
    bool                    canRender() const;

    bool                    rayPick(int winPosX, int winPosY, cvf::HitItemCollection* pickedPoints) ;

    // QPainter based drawing on top of the OpenGL graphics

    bool                    isOverlyPaintingEnabled() const;
    void                    enableOverlyPainting(bool val);

    // Performance information for debugging etc.
    void	                enablePerfInfoHud(bool enable);
    bool	                isPerfInfoHudEnabled();

    void                    enableForcedImmediateMode(bool enable);

    // Find out whether the system supports shaders
    static bool             isShadersSupported();

public slots:
    virtual void            slotSetCurrentFrame(int frameIndex);
    virtual void            slotEndAnimation();
    int                     currentFrameIndex();

public:
    virtual QSize           sizeHint() const;

protected:
    // Method to override if painting directly on the OpenGl Canvas is needed.
    virtual void            paintOverlayItems(QPainter* painter) {};

    // Overridable methods to setup the render system
    virtual void            setupMainRendering();
    virtual void            setupRenderingSequence();
    virtual void            optimizeClippingPlanes();

    // Standard overrides. Not for overriding
    virtual void            resizeGL(int width, int height);
    virtual void            paintEvent(QPaintEvent* event);

    // Support the navigation policy concept
    virtual bool                        event( QEvent* e );
    cvf::ref<caf::NavigationPolicy>     m_navigationPolicy;
    bool                                m_navigationPolicyEnabled;

    // Actual rendering objects
    cvf::ref<cvf::RenderSequence>       m_renderingSequence;
    cvf::ref<cvf::Scene>                m_mainScene;
    cvf::ref<cvf::Camera>               m_mainCamera;
    cvf::ref<cvf::Rendering>            m_mainRendering;

    double                              m_minNearPlaneDistance;
    double                              m_maxFarPlaneDistance;

private:
    void                                updateCamera(int width, int height);

    void                                releaseOGlResourcesForCurrentFrame();
    void                                debugShowRenderingSequencePartNames();

    bool                                m_showPerfInfoHud;
    size_t                              m_paintCounter;
    bool                                m_releaseOGLResourcesEachFrame;
    QPointer<QWidget>                   m_layoutWidget;

    bool                                m_isOverlyPaintingEnabled;
    cvf::ref<cvf::TextureImage>         m_overlayTextureImage;
    cvf::ref<cvf::OverlayImage>         m_overlayImage;
    QImage                              m_overlayPaintingQImage;
    void                                updateOverlayImagePresence();

    // System to make sure we share OpenGL resources
    static Viewer*                      sharedWidget();
    static cvf::OpenGLContextGroup*     contextGroup();
    static std::list<Viewer*>           sm_viewers;
    static cvf::ref<cvf::OpenGLContextGroup> 
                                        sm_openGLContextGroup;

    caf::FrameAnimationControl*         m_animationControl;
    cvf::Collection<cvf::Scene>         m_frameScenes;
};

} // End namespace caf
