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
#include "cvfCollection.h"
#include "cvfObject.h"
#include "cvfOpenGL.h"
#include "cvfRect.h"
#include "cvfRenderingScissor.h"
#include "cvfVector3.h"

#include "cafOpenGLWidget.h"

namespace cvf
{
class Camera;
class FramebufferObject;
class HitItemCollection;
class Model;
class OverlayImage;
class OverlayItem;
class RenderSequence;
class Rendering;
class Scene;
class Texture;
class TextureImage;
class RayIntersectSpec;
} // namespace cvf

namespace caf
{
class FrameAnimationControl;
class NavigationPolicy;
class Viewer;
class PointOfInterestVisualizer;
} // namespace caf

class QInputEvent;
#include <QPointer>

namespace caf
{
class GlobalViewerDynUniformSet;
class ScissorChanger;

class Viewer : public caf::OpenGLWidget
{
    Q_OBJECT
public:
    Viewer( const QGLFormat& format, QWidget* parent );
    ~Viewer() override;

    QWidget*     layoutWidget() { return m_layoutWidget; } // Use this when putting it into something
    cvf::Camera* mainCamera();
    cvf::Camera* comparisonMainCamera();

    void             setComparisonViewEyePointOffset( const cvf::Vec3d& offset );
    const cvf::Vec3d comparisonViewEyePointOffset();

    void       setComparisonViewVisibleNormalizedRect( const cvf::Rectf& visibleRect );
    cvf::Rectf comparisonViewVisibleNormalizedRect() const;
    bool       isComparisonViewActive() const;

    // Set the main scene : the scene active when the animation is not active. (Stopped)
    void        setMainScene( cvf::Scene* scene, bool isForComparisonView = false );
    cvf::Scene* mainScene( bool isForComparisonView = false );
    cvf::Scene* currentScene( bool isForComparisonView = false ); // The scene currently rendered

    // Frame scenes for animation control
    void        addFrame( cvf::Scene* scene, bool isForComparisonView = false );
    size_t      frameCount() const { return std::max( m_frameScenes.size(), m_comparisonFrameScenes.size() ); }
    cvf::Scene* frame( size_t frameIndex, bool isForComparisonView = false );
    void        removeAllFrames( bool isForComparisonView = false );
    int         currentFrameIndex() const;

    // Static models to be shown in all frames
    void addStaticModelOnce( cvf::Model* model, bool isForComparisonView = false );
    void removeStaticModel( cvf::Model* model );
    void removeAllStaticModels();

    // Part enable/ disable mask
    void setEnableMask( unsigned int mask, bool isForComparisonView = false );

    // Recursively traverse all the scenes managed by the viewer and make sure all cached values are up-to-date
    // Use when changing the contents inside the objects in the scene.
    // As of yet: Only updates bounding boxes. Other things might be added later.
    void updateCachedValuesInScene();

    bool                        isAnimationActive( bool isForComparisonView = false );
    caf::FrameAnimationControl* animationControl() { return m_animationControl; }
    void                        setReleaseOGLResourcesEachFrame( bool releaseOGLResourcesEachFrame )
    {
        m_releaseOGLResourcesEachFrame = releaseOGLResourcesEachFrame;
    }

    // Set the navigation policy
    void                         setNavigationPolicy( caf::NavigationPolicy* navigationPolicy );
    const caf::NavigationPolicy* getNavigationPolicy() const;
    void                         enableNavigationPolicy( bool enable );
    void                         setView( const cvf::Vec3d& alongDirection, const cvf::Vec3d& upDirection );
    void                         zoomAll();
    void                         enableParallelProjection( bool enable );
    void                         setParallelProjectionHeadLightDirection( const cvf::Vec3f& direction );
    void                         setPointOfInterestVisualizer( PointOfInterestVisualizer* poiVisualizer );

    // Interface for navigation policies
    void updateParallelProjectionHeightFromMoveZoom( const cvf::Vec3d& pointOfInterest );
    void updateParallelProjectionCameraPosFromPointOfInterestMove( const cvf::Vec3d& pointOfInterest );

    virtual void navigationPolicyUpdate();

    // Min max near far plane.
    void setDefaultPerspectiveNearPlaneDistance( double dist );
    void setMaxClipPlaneDistance( double dist );

    // Test whether it is any point in doing navigation etc.
    bool canRender() const;

    cvf::ref<cvf::RayIntersectSpec>
                                    rayIntersectSpecFromWindowCoordinates( int winPosX, int winPosY, bool isForComparisonView );
    cvf::ref<cvf::RayIntersectSpec> rayIntersectSpecFromWindowCoordinates( int winPosX, int winPosY );
    bool rayPick( int winPosX, int winPosY, cvf::HitItemCollection* pickedPoints, cvf::Vec3d* rayGlobalOrigin = nullptr );

    bool isMousePosWithinComparisonView( int winPosX, int winPosY );

    cvf::OverlayItem* overlayItem( int winPosX, int winPosY );

    // QPainter based drawing on top of the OpenGL graphics

    bool            isOverlayPaintingEnabled() const;
    void            enableOverlayPainting( bool val );
    cvf::Rendering* overlayItemsRendering();

    // Performance information for debugging etc.
    void enablePerfInfoHud( bool enable );
    bool isPerfInfoHudEnabled();

    void enableForcedImmediateMode( bool enable );

    // Find out whether the system supports shaders
    static bool isShadersSupported();

    QImage snapshotImage();

    static void copyCameraView( cvf::Camera* srcCamera, cvf::Camera* dstCamera );

    void setCurrentComparisonFrame( int frameIndex );
    void setComparisonViewToFollowAnimation( bool isToFollow );

public slots:
    virtual void slotSetCurrentFrame( int frameIndex );
    virtual void slotEndAnimation();

public:
    QSize sizeHint() const override;

protected:
    // Method to override if painting directly on the OpenGl Canvas is needed.
    virtual void paintOverlayItems( QPainter* painter ){};

    // Overridable methods to setup the render system
    virtual void optimizeClippingPlanes();

    bool calculateNearFarPlanes( const cvf::Rendering* rendering,
                                 const cvf::Vec3d&     navPointOfinterest,
                                 double*               farPlaneDist,
                                 double*               nearPlaneDist );

    // Standard overrides. Not for overriding
    void resizeGL( int width, int height ) override;
    void paintEvent( QPaintEvent* event ) override;

    // Support the navigation policy concept
    bool event( QEvent* e ) override;

    cvf::ref<caf::NavigationPolicy> m_navigationPolicy;
    bool                            m_navigationPolicyEnabled;

    // Actual rendering objects
    cvf::ref<cvf::RenderSequence> m_renderingSequence;
    cvf::ref<cvf::Scene>          m_mainScene;
    cvf::ref<cvf::Camera>         m_mainCamera;
    cvf::ref<cvf::Rendering>      m_mainRendering;

    double m_defaultPerspectiveNearPlaneDistance;
    double m_maxClipPlaneDistance; //< Max far plane distance and max negative near plane distance in orthographic
                                   // projection
    double m_cameraFieldOfViewYDeg;

private:
    void setupMainRendering();
    void setupRenderingSequence();

    void appendAllStaticModelsToFrame( cvf::Scene* scene, bool isForComparisonView = false );
    void appendModelToAllFrames( cvf::Model* model, bool isForComparisonView = false );
    void removeModelFromAllFrames( cvf::Model* model );

    void updateCamera( int width, int height );
    void releaseOGlResourcesForCurrentFrame();
    void debugShowRenderingSequencePartNames();

    int clampFrameIndex( int frameIndex ) const;

    bool              m_showPerfInfoHud;
    size_t            m_paintCounter;
    bool              m_releaseOGLResourcesEachFrame;
    QPointer<QWidget> m_layoutWidget;

    bool                        m_isOverlayPaintingEnabled;
    cvf::ref<cvf::TextureImage> m_overlayTextureImage;
    cvf::ref<cvf::OverlayImage> m_overlayImage;
    QImage                      m_overlayPaintingQImage;
    void                        updateOverlayImagePresence();

    // System to make sure we share OpenGL resources
    static Viewer*                  sharedWidget();
    static cvf::OpenGLContextGroup* contextGroup();

    static std::list<Viewer*>                sm_viewers;
    static cvf::ref<cvf::OpenGLContextGroup> sm_openGLContextGroup;

    caf::FrameAnimationControl* m_animationControl;

    cvf::Collection<cvf::Scene> m_frameScenes;
    cvf::Collection<cvf::Model> m_staticModels;

    cvf::ref<cvf::Rendering> m_comparisonMainRendering;
    cvf::ref<cvf::Camera>    m_comparisonMainCamera;

    cvf::ref<cvf::Scene>        m_comparisonMainScene;
    cvf::Collection<cvf::Scene> m_comparisonFrameScenes;
    cvf::Collection<cvf::Model> m_comparisonStaticModels;
    bool                        m_isComparisonFollowingAnimation;
    bool                        m_isComparisonViewActiveFlag;
    void                        updateComparisonViewActiveFlag();

    cvf::Vec3d                      m_comparisonViewOffset;
    cvf::ref<cvf::RenderingScissor> m_comparisonRenderingScissor;
    cvf::Rectf                      m_comparisonWindowNormalizedRect;

    // Poi visualization
    cvf::ref<PointOfInterestVisualizer> m_poiVisualizationManager;

    // Parallel projection light modification

    cvf::ref<GlobalViewerDynUniformSet> m_globalUniformSet;
    cvf::Vec3f                          m_parallelProjectionLightDirection;

    // Offscreen render objects
    cvf::ref<cvf::FramebufferObject> m_offscreenFbo;
    cvf::ref<cvf::Texture>           m_offscreenTexture;
    int                              m_offscreenViewportWidth;
    int                              m_offscreenViewportHeight;
    cvf::ref<cvf::Rendering>         m_quadRendering;

    cvf::ref<cvf::Rendering> m_overlayItemsRendering;
};

} // End namespace caf
