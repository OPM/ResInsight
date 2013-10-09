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

#include "cvfObject.h"
#include "cvfMatrix4.h"
#include "cvfFrustum.h"

namespace cvf {

class BoundingBox;
class Viewport;
class Ray;


//==================================================================================================
//
// A camera defines the view point, view direction and the up vector
//
//==================================================================================================
class Camera : public Object
{
public:
    enum ProjectionType
    {
        PERSPECTIVE,            ///< Perspective projection
        ORTHO                   ///< Orthographic (parallel) projection
    };

public:
    Camera();
    ~Camera();

    void            setViewMatrix(const Mat4d& mat);
    void            setFromLookAt(const Vec3d& eye, const Vec3d& vrp, const Vec3d& up);
    void            toLookAt(Vec3d* eye, Vec3d* vrp, Vec3d* up) const;

    Vec3d           position() const;   
    Vec3d           direction() const;   
    Vec3d           up() const;   
    Vec3d           right() const;   

    ProjectionType  projection() const;
    void            setProjectionAsPerspective(double fieldOfViewYDeg, double nearPlane, double farPlane);
    void            setProjectionAsOrtho(double height, double nearPlane, double farPlane);
    void            setProjectionAsUnitOrtho();
    void            setProjectionAsPixelExact2D();

    void            fitView(const BoundingBox& boundingBox, const Vec3d& dir, const Vec3d& up, double coverageFactor = 0.9);
    void            fitViewOrtho(const BoundingBox& boundingBox, double eyeDist, const Vec3d& dir, const Vec3d& up, double coverageFactor = 0.9);
    static Vec3d    computeFitViewEyePosition(const BoundingBox& boundingBox, const Vec3d& dir, const Vec3d& up, double coverageFactor, double fieldOfViewYDeg, double aspectRatio);

    void            setClipPlanesFromBoundingBox(const BoundingBox& boundingBox, double minNearPlaneDistance = 0.01);

    const Mat4d&    viewMatrix() const; 
    const Mat4d&    invertedViewMatrix() const; 
    const Mat4d&    projectionMatrix() const; 

    double          fieldOfViewYDeg() const;
    double          nearPlane() const;
    double          farPlane()const;

    double          aspectRatio() const;
    double          frontPlaneFrustumHeight() const;
    double          frontPlanePixelHeight() const;
    double          distanceWhereObjectProjectsToPixelExtent(double objectExtentWorld, double objectExtentPixels) const;

    void            setViewport(int x, int y, uint width, uint height);
    Viewport*       viewport();
    const Viewport* viewport() const;

    ref<Ray>        rayFromWindowCoordinates(int x, int y) const;
    ref<Plane>      planeFromLineWindowCoordinates(Vec2i coordStart, Vec2i coordEnd) const;
    bool            unproject(const Vec3d& coord, Vec3d* out) const;
    bool            project(const Vec3d& point, Vec3d* out) const;

    double          computeProjectedBoundingBoxPixelArea(const BoundingBox& box) const;
    double          computeProjectedBoundingSpherePixelArea(const Vec3d& center, double radius) const;
    bool            isProjectedBoundingBoxLessThanThreshold(const BoundingBox& box, double pixelThreshold) const;

    Frustum         frustum() const;

    void            applyOpenGL() const;

private:
    void            updateCachedValues();

    Frustum         computeViewFrustum() const;

    static Mat4d    createFrustumMatrix(double left, double right, double bottom, double top, double zNear, double zFar);
    static Mat4d    createPerspectiveMatrix(double fovy, double aspect_ratio, double znear, double zfar);
    static Mat4d    createOrthoMatrix(double left, double right, double bottom, double top, double near, double far);
    static Mat4d    createLookAtMatrix(Vec3d eye, Vec3d vrp, Vec3d up);

private:
    Mat4d           m_viewMatrix;
    Mat4d           m_cachedInvertedViewMatrix;         // Cached inverted version of the m_viewMatrix
    Mat4d           m_projectionMatrix;
    Mat4d           m_cachedProjectionMultViewMatrix;   // Caching projMat*viewMat, as this is used in many tight loops (eps. computeBoundingBoxPixelSize)
    ProjectionType  m_projectionType;

    double          m_fieldOfViewYDeg;                  // Stored for perspective projection
    double          m_frontPlaneFrustumHeight;          // Height of view frustum in the front plane
    double          m_nearPlane;
    double          m_farPlane;

    ref<Viewport>   m_viewport;
    double          m_cachedFrontPlanePixelHeight;      // Height of a pixel in the front clipping plane given in world coordinates
    Frustum         m_cachedViewFrustum;
};

}
