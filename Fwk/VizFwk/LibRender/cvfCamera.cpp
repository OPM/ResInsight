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


#include "cvfBase.h"
#include "cvfCamera.h"
#include "cvfOpenGL.h"
#include "cvfRay.h"
#include "cvfViewport.h"
#include "cvfBoundingBox.h"
#include "cvfGeometryUtils.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::Camera
/// \ingroup Render
///
/// A Camera defines a view matrix (viewpoint) and a projection matrix
/// 
/// The camera stores and applies the view matrix and the projection matrix to OpenGL. A camera
/// has a reference to a Viewport that is associated with the camera. The camera needs the viewport
/// to be able to setup the projection matrix.
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///  
//--------------------------------------------------------------------------------------------------
Camera::Camera()
:   m_projectionType(PERSPECTIVE),
    m_fieldOfViewYDeg(40.0),
    m_frontPlaneFrustumHeight(0),
    m_nearPlane(0.05),
    m_farPlane(10000.0),
    m_cachedFrontPlanePixelHeight(0)
{
    m_viewport = new Viewport;

    // Default perspective projection. This will update the cached values
    setProjectionAsPerspective(m_fieldOfViewYDeg, m_nearPlane, m_farPlane);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Camera::~Camera()
{
    // Empty destructor to avoid errors with undefined types when cvf::ref's destructor gets called
}


//--------------------------------------------------------------------------------------------------
/// The Camera's view matrix. 
/// 
/// This is what you would pass to OpenGL with:
/// \code
/// glMatrixMode(GL_MODELVIEW);
/// glLoadMatrix(camera.viewMatrix().ptr());
/// \endcode
//--------------------------------------------------------------------------------------------------
void Camera::setViewMatrix(const Mat4d& mat) 
{ 
    m_viewMatrix = mat; 

    updateCachedValues();
}


//--------------------------------------------------------------------------------------------------
/// Set the view matrix from the standard OpenGL lookat specification. 
///
/// View direction will be (center - eye). Center is not stored in this class.
//--------------------------------------------------------------------------------------------------
void Camera::setFromLookAt(const Vec3d& eye, const Vec3d& center, const Vec3d& up)
{
    Mat4d invViewMatrix = createLookAtMatrix(eye, center, up);
    m_viewMatrix = invViewMatrix.getInverted();

    updateCachedValues();
}


//--------------------------------------------------------------------------------------------------
/// Get the eye point, vrp and up vector. Params can be NULL if not needed.
//--------------------------------------------------------------------------------------------------
void Camera::toLookAt(Vec3d* eye, Vec3d* vrp, Vec3d* up) const
{
    Vec3d vEye(0, 0, 0);
    Vec3d vVrp(0, 0, -1);
    Vec3d vUp(0, 1, 0);
    
    vEye.transformPoint(m_cachedInvertedViewMatrix);
    vVrp.transformPoint(m_cachedInvertedViewMatrix);
    vUp.transformVector(m_cachedInvertedViewMatrix);

    if (eye) *eye = vEye;
    if (vrp) *vrp = vVrp;
    if (up)  *up  = vUp;
}


//--------------------------------------------------------------------------------------------------
/// Retrieves the camera's position (eye point)
//--------------------------------------------------------------------------------------------------
Vec3d Camera::position() const
{
    Vec3d pos(0, 0, 0);
    pos.transformPoint(m_cachedInvertedViewMatrix);

    return pos;
}


//--------------------------------------------------------------------------------------------------
/// Get the camera's forward direction vector. Vector is normalized
//--------------------------------------------------------------------------------------------------
Vec3d Camera::direction() const
{
    Vec3d dir(0, 0, -1.0);
    dir.transformVector(m_cachedInvertedViewMatrix);

    return dir;
}


//--------------------------------------------------------------------------------------------------
/// Get the camera's up vector. Vector is normalized
//--------------------------------------------------------------------------------------------------
Vec3d Camera::up() const
{
    Vec3d up(0, 1.0, 0);
    up.transformVector(m_cachedInvertedViewMatrix);

    return up;
}


//--------------------------------------------------------------------------------------------------
/// Get the camera's right vector. Vector is normalized
//--------------------------------------------------------------------------------------------------
Vec3d Camera::right() const
{
    Vec3d right(1.0, 0, 0);
    right.transformVector(m_cachedInvertedViewMatrix);

    return right;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Camera::ProjectionType Camera::projection() const
{
    return m_projectionType;
}


//--------------------------------------------------------------------------------------------------
/// Setup a perspective projection. 
///
/// The fieldOfViewYDeg parameter is the total field of view angle (in degrees) in the Y direction.
/// Works similar to gluPerspective().
//--------------------------------------------------------------------------------------------------
void Camera::setProjectionAsPerspective(double fieldOfViewYDeg, double nearPlane, double farPlane)
{
    CVF_ASSERT(fieldOfViewYDeg > 0 && fieldOfViewYDeg < 180);
    CVF_ASSERT(nearPlane > 0);
    CVF_ASSERT(farPlane > nearPlane);

    m_projectionType = PERSPECTIVE;
    m_fieldOfViewYDeg = fieldOfViewYDeg;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    m_frontPlaneFrustumHeight = 2*m_nearPlane*Math::tan(Math::toRadians(m_fieldOfViewYDeg/2.0));

    m_projectionMatrix = createPerspectiveMatrix(Math::toRadians(m_fieldOfViewYDeg), aspectRatio(), m_nearPlane, m_farPlane);

    updateCachedValues();
}


//--------------------------------------------------------------------------------------------------
/// Setup an orthographic projection
///
/// This works similar to glOrtho().
//--------------------------------------------------------------------------------------------------
void Camera::setProjectionAsOrtho(double height, double nearPlane, double farPlane)
{
    CVF_ASSERT(height > 0);
    double width = height * aspectRatio();

    m_projectionType = ORTHO;
    m_fieldOfViewYDeg = UNDEFINED_DOUBLE;
    m_frontPlaneFrustumHeight = height;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;

    m_projectionMatrix = createOrthoMatrix(-width/2.0, width/2.0, -height/2.0, height/2.0, m_nearPlane, m_farPlane);

    updateCachedValues();
}


//--------------------------------------------------------------------------------------------------
/// Setup an orthographic projection
///
/// This works similar to glOrtho().
//--------------------------------------------------------------------------------------------------
void Camera::setProjectionAsUnitOrtho()
{
    m_projectionType = ORTHO;
    m_fieldOfViewYDeg = UNDEFINED_DOUBLE;
    m_frontPlaneFrustumHeight = 1;
    m_nearPlane = -1.0;
    m_farPlane = 1.0;

    m_projectionMatrix = createOrthoMatrix(0.0, 1.0, 0.0, 1.0, m_nearPlane, m_farPlane);

    updateCachedValues();
}


//--------------------------------------------------------------------------------------------------
/// Setup a pixel exact two-dimensional orthographic viewing region
///
/// Sets a 2D projection where each unit corresponds to a pixel.
//--------------------------------------------------------------------------------------------------
void Camera::setProjectionAsPixelExact2D()
{
    m_projectionType = ORTHO;
    m_fieldOfViewYDeg = UNDEFINED_DOUBLE;
    m_frontPlaneFrustumHeight = m_viewport->height();
    m_nearPlane = -1.0;
    m_farPlane = 1.0;

    m_projectionMatrix = createOrthoMatrix(0, m_viewport->width(), 0, m_viewport->height(), m_nearPlane, m_farPlane);

    updateCachedValues();
}


//--------------------------------------------------------------------------------------------------
/// Setup the view to contain the passed bounding box, with the camera looking from the given 
/// direction (dir) and with the given up vector (up).
///
/// The passed boundingBox should be the bounding box of the object/model you would like to fit
/// the view to. The relativeDistance parameter specifies the distance from the camera to the 
/// center of the bounding box
//--------------------------------------------------------------------------------------------------
void Camera::fitView(const BoundingBox& boundingBox, const Vec3d& dir, const Vec3d& up, double coverageFactor)
{
    // Use old view direction, but look towards model center
    Vec3d eye = computeFitViewEyePosition(boundingBox, dir, up, coverageFactor, m_fieldOfViewYDeg, viewport()->aspectRatio());

    // Will update cached values
    setFromLookAt(eye, boundingBox.center(), up);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Vec3d Camera::computeFitViewEyePosition(const BoundingBox& boundingBox, const Vec3d& dir, const Vec3d& up, double coverageFactor, double fieldOfViewYDeg, double aspectRatio) 
{
    cvf::Vec3d corners[8];
    boundingBox.cornerVertices(corners);

    cvf::Vec3d upNorm = up.getNormalized();
    cvf::Vec3d right = dir^up;
    right.normalize();
    cvf::Vec3d boxEyeNorm = (-dir).getNormalized();

    cvf::Plane planeTop;
    planeTop.setFromPointAndNormal(boundingBox.center(), up);

    cvf::Plane planeSide;
    planeSide.setFromPointAndNormal(boundingBox.center(), right);

    // fieldOfViewYDeg is the complete angle in degrees, get half in radians
    double fovY = Math::toRadians(fieldOfViewYDeg/2.0);
    double fovX = Math::atan(Math::tan(fovY)*aspectRatio);

    double dist = 0;

    for (size_t i = 0; i < 8; ++i)
    {
        cvf::Vec3d centerToCorner = corners[i] - boundingBox.center();

        // local horizontal plane
        cvf::Vec3d centerToCornerTop = planeTop.projectPoint(centerToCorner);
        double rightCoord = centerToCornerTop*right;
        double distRight = Math::abs(rightCoord/Math::tan(fovX));
        distRight += centerToCornerTop*boxEyeNorm;

        // local vertical plane
        cvf::Vec3d centerToCornerSide = planeSide.projectPoint(centerToCorner);
        double upCoord = centerToCornerSide*upNorm;
        double distUp    = Math::abs(upCoord/Math::tan(fovY));
        distUp += (centerToCornerSide*boxEyeNorm);

        // Adjust for the distance scale factor
        distRight /= coverageFactor;
        distUp /= coverageFactor;

        dist = CVF_MAX(dist, distRight);
        dist = CVF_MAX(dist, distUp);
    }

    // Avoid distances of 0 when model has no extent
    if (!(dist > 0))
    {
        dist = 1.0;
    }

    // Use old view direction, but look towards model center
    Vec3d eye = boundingBox.center()- dir.getNormalized()*dist;

    return eye;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Camera::fitViewOrtho(const BoundingBox& boundingBox, double eyeDist, const Vec3d& dir, const Vec3d& up, double adjustmentFactor)
{
    // Algorithm:
    // Project all points into the viewing plan. Find the distance along the right and up vector. 
    // Set the height of the frustum to this distance.
    cvf::Vec3d corners[8];
    boundingBox.cornerVertices(corners);

    cvf::BoundingBox projBox;
    cvf::Plane viewPlane;
    viewPlane.setFromPointAndNormal(boundingBox.center(), -dir);

    cvf::Vec3d upNorm = up.getNormalized();
    cvf::Vec3d right = up^dir;
    right.normalize();

    double rightMin = cvf::UNDEFINED_DOUBLE_THRESHOLD;
    double rightMax = -cvf::UNDEFINED_DOUBLE_THRESHOLD;
    double upMin = cvf::UNDEFINED_DOUBLE_THRESHOLD;
    double upMax = -cvf::UNDEFINED_DOUBLE_THRESHOLD;

    for (size_t i = 0; i < 8; ++i)
    {
        cvf::Vec3d cornerInPlane = viewPlane.projectPoint(corners[i]);
        cvf::Vec3d cornerVec = cornerInPlane-boundingBox.center();

        double rightCoord = cornerVec*right;
        rightMin = CVF_MIN(rightMin, rightCoord);
        rightMax = CVF_MAX(rightMax, rightCoord);

        double upCood = cornerVec*upNorm;
        upMin = CVF_MIN(upMin, upCood);
        upMax = CVF_MAX(upMax, upCood);
    }

    double deltaRight = rightMax - rightMin;
    double deltaUp = upMax - upMin;
    double newHeight = CVF_MAX(deltaUp, deltaRight/aspectRatio())/adjustmentFactor;

    setProjectionAsOrtho(newHeight, m_nearPlane, m_farPlane);

    Vec3d eye = boundingBox.center()- eyeDist*dir.getNormalized();
    setFromLookAt(eye, boundingBox.center(), up);
}


//--------------------------------------------------------------------------------------------------
/// Set the front and back clipping planes close to the given bounding box
//--------------------------------------------------------------------------------------------------
void Camera::setClipPlanesFromBoundingBox(const BoundingBox& boundingBox, double minNearPlaneDistance)
{
    CVF_ASSERT(minNearPlaneDistance > 0);

    Vec3d eye, vrp, up;
    toLookAt(&eye, &vrp, &up);

    Vec3d viewdir = (vrp - eye).getNormalized();
    double distEyeBoxCenterAlongViewDir = (boundingBox.center() - eye)*viewdir;

    double nearPlane = distEyeBoxCenterAlongViewDir - boundingBox.radius();
    double farPlane = distEyeBoxCenterAlongViewDir + boundingBox.radius();

    if (nearPlane < minNearPlaneDistance) nearPlane = minNearPlaneDistance;
    if (farPlane <= nearPlane) farPlane = nearPlane + 1.0;

    if (projection() == PERSPECTIVE)
    {
        setProjectionAsPerspective(m_fieldOfViewYDeg, nearPlane, farPlane);
    }
    else
    {
        setProjectionAsOrtho(m_frontPlaneFrustumHeight, nearPlane, farPlane);
    }
}


//--------------------------------------------------------------------------------------------------
/// Returns the current view matrix
//--------------------------------------------------------------------------------------------------
const Mat4d& Camera::viewMatrix() const
{ 
    return m_viewMatrix; 
}


//--------------------------------------------------------------------------------------------------
/// Returns the inverted current view matrix
/// 
/// The inverted view matrix is cached for performance reasons.
//--------------------------------------------------------------------------------------------------
const Mat4d& Camera::invertedViewMatrix() const
{
    return m_cachedInvertedViewMatrix;
}


//--------------------------------------------------------------------------------------------------
/// Returns the current projection matrix
//--------------------------------------------------------------------------------------------------
const Mat4d& Camera::projectionMatrix() const
{ 
    return m_projectionMatrix; 
}


//--------------------------------------------------------------------------------------------------
/// Returns the total field of view in Y direction in degrees.
/// 
/// If projection is orthographic, this function will return UNDEFINED_DOUBLE
//--------------------------------------------------------------------------------------------------
double Camera::fieldOfViewYDeg() const
{
    return m_fieldOfViewYDeg;
}


//--------------------------------------------------------------------------------------------------
/// Returns the near clipping plane
//--------------------------------------------------------------------------------------------------
double Camera::nearPlane() const
{
    return m_nearPlane;
}


//--------------------------------------------------------------------------------------------------
/// Returns the far clipping plane
//--------------------------------------------------------------------------------------------------
double Camera::farPlane()const
{
    return m_farPlane;
}


//--------------------------------------------------------------------------------------------------
/// Returns the aspect ratio (width / height) of the corresponding viewport. 
//--------------------------------------------------------------------------------------------------
double Camera::aspectRatio() const 
{
    CVF_ASSERT(m_viewport.notNull());

    return viewport()->aspectRatio();
}


//--------------------------------------------------------------------------------------------------
/// Specify the position and dimensions of the viewport, and update the projection matrix.
/// 
/// This method calls Viewport::set() and then updates the projection matrix, as this is depending
/// on the viewport dimensions.
/// This method is usually used as a response to a Resize message from the window system
/// 
/// The window coordinates (x,y) are in OpenGL style coordinates, which means a right handed 
/// coordinate system with the origin in the lower left corner of the window.
//--------------------------------------------------------------------------------------------------
void Camera::setViewport(int x, int y, uint width, uint height)
{
    CVF_ASSERT(m_viewport.notNull());
    m_viewport->set(x, y, width, height);

    // Update projection:
    if (m_projectionType == PERSPECTIVE)
    {
        setProjectionAsPerspective(m_fieldOfViewYDeg, m_nearPlane, m_farPlane);
    }
    else if (m_projectionType == ORTHO)
    {
        setProjectionAsOrtho(m_frontPlaneFrustumHeight, m_nearPlane, m_farPlane);
    }
}


//--------------------------------------------------------------------------------------------------
/// Get the viewport used by this camera.
//--------------------------------------------------------------------------------------------------
Viewport* Camera::viewport() 
{ 
    return m_viewport.p(); 
}


//--------------------------------------------------------------------------------------------------
/// Get the viewport used by this camera.
//--------------------------------------------------------------------------------------------------
const Viewport* Camera::viewport() const 
{ 
    return m_viewport.p(); 
}


//--------------------------------------------------------------------------------------------------
/// Create a Ray from window coordinates
/// 
    /// The window coordinates are in OpenGL style coordinates, which means a right handed 
    /// coordinate system with the origin in the lower left corner of the window.
//--------------------------------------------------------------------------------------------------
ref<Ray> Camera::rayFromWindowCoordinates(int x, int y) const
{
    Vec3d coord0(0, 0, 0);
    if (!unproject(Vec3d(static_cast<double>(x), static_cast<double>(y), 0), &coord0))
    {
        return NULL;
    }

    Vec3d coord1(0, 0, 0);
    if (!unproject(Vec3d(static_cast<double>(x), static_cast<double>(y), 1), &coord1))
    {
        return NULL;
    }

    Ray* ray = new Ray;
    ray->setOrigin(coord0);

    Vec3d dir = (coord1 - coord0).getNormalized();
    ray->setDirection(dir);

    return ray;
}


//--------------------------------------------------------------------------------------------------
/// Construct a plane from a line specified in window coordinates    
///
/// The plane will be constructed so that the specified line lies in the plane, and the plane 
/// normal will be pointing left when moving along the line from start to end.
/// 
/// The window coordinates are in OpenGL style coordinates, which means a right handed 
/// coordinate system with the origin in the lower left corner of the window.
//--------------------------------------------------------------------------------------------------
ref<Plane> Camera::planeFromLineWindowCoordinates(Vec2i coordStart, Vec2i coordEnd) const
{
    Vec3d s0(0, 0, 0);
    Vec3d e0(0, 0, 0);
    Vec3d e1(0, 0, 0);
    bool unprojOk = true;
    unprojOk &= unproject(Vec3d(coordStart.x(), coordStart.y(), 0), &s0);
    unprojOk &= unproject(Vec3d(coordEnd.x(),   coordEnd.y(),   0), &e0);
    unprojOk &= unproject(Vec3d(coordEnd.x(),   coordEnd.y(),   1), &e1);
    if (!unprojOk)
    {
        return NULL;
    }

    ref<Plane> plane = new Plane;
    if (!plane->setFromPoints(s0, e0, e1))
    {
        return NULL;
    }

    return plane;
}


//--------------------------------------------------------------------------------------------------
/// Computes the maximum pixel area that the projected bounding box will occupy with the current camera settings.
/// 
/// The current implementation gives the area of the 2D bounding box of the projected corners of the 
/// 3D bounding box, this not exact but at least >=
//--------------------------------------------------------------------------------------------------
double Camera::computeProjectedBoundingBoxPixelArea(const BoundingBox& box) const
{
    Vec3d corner[8];
    box.cornerVertices(corner);

    BoundingBox projBB;

    // project the 8 corners in the viewport
    int i;
    for (i = 0; i < 8; i++)
    {
        Vec3d v;
        if (project(corner[i], &v))
        {
            projBB.add(v);
        }
    }

    Vec3d extent = projBB.extent();

    double pixels = extent.x() * extent.y();
        
    return pixels;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double Camera::computeProjectedBoundingSpherePixelArea(const Vec3d& center, double radius) const
{
    if (m_cachedFrontPlanePixelHeight > 0)
    {
        Vec3d currentEye = position();

        double eyeDist = (currentEye - center).length();
        if (eyeDist > 0)
        {
            double radiusNearWorld = ((m_nearPlane*radius)/eyeDist);
            double pixels = radiusNearWorld/m_cachedFrontPlanePixelHeight;

            return PI_D*pixels*pixels;
        }
        else
        {
            // We should be returning a large value here, right?
            return std::numeric_limits<double>::max();
        }
    }
    else
    {
        return 0;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Camera::isProjectedBoundingBoxLessThanThreshold(const BoundingBox& box, double pixelThreshold) const
{
    Vec3d currentEye = position();

    double l2Dist = (currentEye - box.center()).lengthSquared();

    double thresholdDist = distanceWhereObjectProjectsToPixelExtent(2*box.radius(), pixelThreshold) + box.radius();

    if (l2Dist > thresholdDist*thresholdDist)
    {
        return true;
    }

    return false;
}


//--------------------------------------------------------------------------------------------------
/// Get height of the view frustum in the front plane in world coordinates
//--------------------------------------------------------------------------------------------------
double Camera::frontPlaneFrustumHeight() const
{
    return m_frontPlaneFrustumHeight;
}


//--------------------------------------------------------------------------------------------------
/// Get the height of a pixel in the front plane in world coordinates 
/// 
/// This value is cached for performance reasons.
//--------------------------------------------------------------------------------------------------
double Camera::frontPlanePixelHeight() const
{
    return m_cachedFrontPlanePixelHeight;
}


//--------------------------------------------------------------------------------------------------
/// Get the distance to where the object with the given extent projected onto the front clipping plane 
/// would have the specified pixel extent.
/// 
/// \param objectExtentWorld  The largest object extent in world coordinates
/// \param objectExtentPixels The requested pixel extent in the front clipping plane
//--------------------------------------------------------------------------------------------------
double Camera::distanceWhereObjectProjectsToPixelExtent(double objectExtentWorld, double objectExtentPixels) const
{
    CVF_ASSERT(objectExtentPixels > 0);

    if (m_cachedFrontPlanePixelHeight > 0)
    {
        double distance = (objectExtentWorld*m_nearPlane)/(m_cachedFrontPlanePixelHeight*objectExtentPixels);
        return distance;
    }
    else
    {
        return 0;
    }
}


//--------------------------------------------------------------------------------------------------
/// OpenGL like unproject.
/// 
/// The input (window) coordinates \a coord must be specified in OpenGL style coordinates, which means
/// a right handed coordinate system with the origin in the lower left corner of the window.
//--------------------------------------------------------------------------------------------------
bool Camera::unproject(const Vec3d& coord, Vec3d* out) const
{
    CVF_ASSERT(out);

    Vec4d v(coord, 1.0);

    // map from viewport to 0-1
    v.x() = (v.x() - m_viewport->x()) / m_viewport->width();
    v.y() = (v.y() - m_viewport->y()) / m_viewport->height();

    // map to range -1 to 1
    v.x() = v.x() * 2.0f - 1.0f;
    v.y() = v.y() * 2.0f - 1.0f;
    v.z() = v.z() * 2.0f - 1.0f;

    bool invertible=true;
    Mat4d inverse = m_cachedProjectionMultViewMatrix.getInverted(&invertible);
    if (!invertible)
    {
        return false;
    }

    v = inverse * v;
    
    if (v.w() == 0.0)
    {
        return false;
    }

    out->x() = v.x() / v.w();
    out->y() = v.y() / v.w();
    out->z() = v.z() / v.w();

    return true;
}


//--------------------------------------------------------------------------------------------------
/// Maps object coordinates to window coordinates
/// 
/// The returned window coordinates \a out are in OpenGL style coordinates, which means a right handed 
/// coordinate system with the origin in the lower left corner of the window.
//--------------------------------------------------------------------------------------------------
bool Camera::project(const Vec3d& point, Vec3d* out) const
{
    return GeometryUtils::project(m_cachedProjectionMultViewMatrix, Vec2i(m_viewport->x(), m_viewport->y()), Vec2ui(m_viewport->width(), m_viewport->height()), point, out);
}


//--------------------------------------------------------------------------------------------------
/// Get camera's Frustum. Planes have an inward pointing normal
/// 
/// \sa
/// computeViewFrustum()
//--------------------------------------------------------------------------------------------------
Frustum Camera::frustum() const
{
    return m_cachedViewFrustum;
}


//--------------------------------------------------------------------------------------------------
/// Update the various cached variables stored in the view. 
/// 
/// This needs to be called whenever one of the following items are updated:
/// - View matrix
/// - Projection matrix
/// - Clip planes
/// - Viewport dimensions (NB!), as this could be done directly in cvf::Viewport
/// 
/// The following values are cached:
/// - m_cachedInvertedViewMatrix
/// - m_cachedProjectionMultViewMatrix
/// - m_cachedViewFrustum
/// - m_cachedFrontPlanePixelSize
//--------------------------------------------------------------------------------------------------
void Camera::updateCachedValues()
{
    // Update cached matrices
    m_cachedProjectionMultViewMatrix = m_projectionMatrix * m_viewMatrix;
    m_cachedInvertedViewMatrix = m_viewMatrix.getInverted();

    // Update the cached frustum
    m_cachedViewFrustum = computeViewFrustum();

    // Update the front plane pixel size (height)
    CVF_ASSERT(m_viewport.notNull());
    uint vpWidth  = m_viewport->width();
    uint vpHeight = m_viewport->height();
    if (vpWidth > 0 && vpHeight > 0)
    {
        // The height/size of a pixel in the front clipping plane
        m_cachedFrontPlanePixelHeight = m_frontPlaneFrustumHeight/vpHeight;
    }
    else
    {
        // Technically we could compute a pixel height as long as the viewport height is non-zero,
        // but considering normal usage of the pixel height it makes sense to only compute it
        // when the viewport area is non-zero
        m_cachedFrontPlanePixelHeight = 0;
    }
}


//--------------------------------------------------------------------------------------------------
/// Get camera's Frustum. Planes have an inward pointing normal
//--------------------------------------------------------------------------------------------------
Frustum Camera::computeViewFrustum() const
{
    // See: 
    // http://www2.ravensoft.com/users/ggribb/plane%20extraction.pdf
    // http://zach.in.tu-clausthal.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html

    Frustum f;
    const Mat4d& m = m_cachedProjectionMultViewMatrix;

    Plane left =    Plane(m.rowCol(3, 0) + m.rowCol(0, 0),
                          m.rowCol(3, 1) + m.rowCol(0, 1),
                          m.rowCol(3, 2) + m.rowCol(0, 2),
                          m.rowCol(3, 3) + m.rowCol(0, 3));

    Plane right =   Plane(m.rowCol(3, 0) - m.rowCol(0, 0),
                          m.rowCol(3, 1) - m.rowCol(0, 1),
                          m.rowCol(3, 2) - m.rowCol(0, 2),
                          m.rowCol(3, 3) - m.rowCol(0, 3));

    Plane bot =     Plane(m.rowCol(3, 0) + m.rowCol(1, 0),
                          m.rowCol(3, 1) + m.rowCol(1, 1),
                          m.rowCol(3, 2) + m.rowCol(1, 2),
                          m.rowCol(3, 3) + m.rowCol(1, 3));

    Plane top =     Plane(m.rowCol(3, 0) - m.rowCol(1, 0),
                          m.rowCol(3, 1) - m.rowCol(1, 1),
                          m.rowCol(3, 2) - m.rowCol(1, 2),
                          m.rowCol(3, 3) - m.rowCol(1, 3));

    Plane front =   Plane(m.rowCol(3, 0) + m.rowCol(2, 0),
                          m.rowCol(3, 1) + m.rowCol(2, 1),
                          m.rowCol(3, 2) + m.rowCol(2, 2),
                          m.rowCol(3, 3) + m.rowCol(2, 3));

    Plane back =    Plane(m.rowCol(3, 0) - m.rowCol(2, 0),
                          m.rowCol(3, 1) - m.rowCol(2, 1),
                          m.rowCol(3, 2) - m.rowCol(2, 2),
                          m.rowCol(3, 3) - m.rowCol(2, 3));

    
    // Assign planes if all are valid
    if (left.isValid() && right.isValid() && top.isValid() && bot.isValid() && front.isValid() && back.isValid())
    {
        // Left clipping plane
        f.setPlane(Frustum::LEFT, left);

        // Right clipping plane
        f.setPlane(Frustum::RIGHT, right);

        // Top clipping plane
        f.setPlane(Frustum::TOP, top);

        // Bottom clipping plane
        f.setPlane(Frustum::BOTTOM, bot);

        // Near clipping plane
        f.setPlane(Frustum::FRONT, front);

        // Far clipping plane
        f.setPlane(Frustum::BACK, back);
    }
   

    return f;
}


//--------------------------------------------------------------------------------------------------
/// Helper class to compute the projection matrix based on the given view frustum
//--------------------------------------------------------------------------------------------------
Mat4d Camera::createFrustumMatrix(double left, double right, double bottom, double top, double zNear, double zFar)
{
    Matrix4<double> m;

    if (zNear <= 0 || zFar <= 0 || zNear == zFar || left == right || top == bottom)
    {
        return Mat4d::ZERO;
    }

    double x =  (2.0 * zNear)        / (right - left);
    double y =  (2.0 * zNear)        / (top - bottom);
    double a =  (right + left)       / (right - left);
    double b =  (top + bottom)       / (top - bottom);
    double c = -(zFar + zNear)       / (zFar - zNear);  
    double d = -(2.0 * zFar * zNear) / (zFar - zNear);  

    m.setRowCol(0, 0, x);  m.setRowCol(0, 1, 0);  m.setRowCol(0, 2, a);     m.setRowCol(0, 3, 0);
    m.setRowCol(1, 0, 0);  m.setRowCol(1, 1, y);  m.setRowCol(1, 2, b);     m.setRowCol(1, 3, 0);
    m.setRowCol(2, 0, 0);  m.setRowCol(2, 1, 0);  m.setRowCol(2, 2, c);     m.setRowCol(2, 3, d);
    m.setRowCol(3, 0, 0);  m.setRowCol(3, 1, 0);  m.setRowCol(3, 2, -1.0);  m.setRowCol(3, 3, 0);

    return m;
}


//--------------------------------------------------------------------------------------------------
/// Helper class to create a perspective projection matrix
//--------------------------------------------------------------------------------------------------
Mat4d Camera::createPerspectiveMatrix(double fovy, double aspect_ratio, double znear, double zfar)
{
    Mat4d m;

    double rads = fovy / 2.0;
    double dz = zfar - znear;
    double sa = Math::sin(rads);

    if ((dz == 0) || (sa == 0) || (aspect_ratio == 0))
    {
        return Mat4d::ZERO;
    }

    double ctan = Math::cos(rads) / sa;

    m.setRowCol(0,0, ctan / aspect_ratio);
    m.setRowCol(1,1, ctan);
    m.setRowCol(2,2, -(zfar + znear)/dz);
    m.setRowCol(2,3, -(2.0*znear* zfar)/dz);
    m.setRowCol(3,2, -1.0);
    m.setRowCol(3,3, 0);

    // A (0,2) and B (1,2) are zero due to the frustum being symmetrical around zero

    return m;
}


//--------------------------------------------------------------------------------------------------
/// Helper class to create a orthographic projection matrix
//--------------------------------------------------------------------------------------------------
Mat4d Camera::createOrthoMatrix(double left, double right, double bottom, double top, double nearPlane, double farPlane)
{
    Mat4d m;

    m.setRowCol(0,0, 2.0 / (right - left));
    m.setRowCol(0,1, 0);
    m.setRowCol(0,2, 0);
    m.setRowCol(0,3, -(right + left) / (right - left));
 
    m.setRowCol(1,0, 0);
    m.setRowCol(1,1, 2.0 / (top-bottom));
    m.setRowCol(1,2, 0);
    m.setRowCol(1,3, -(top + bottom) / (top - bottom));
 
    m.setRowCol(2,0, 0);
    m.setRowCol(2,1, 0);
    m.setRowCol(2,2, -2.0 / (farPlane - nearPlane));
    m.setRowCol(2,3, -(farPlane + nearPlane) / (farPlane-nearPlane));
 
    m.setRowCol(3,0, 0);
    m.setRowCol(3,1, 0);
    m.setRowCol(3,2, 0);
    m.setRowCol(3,3, 1.0);

    return m;
}


//--------------------------------------------------------------------------------------------------
/// Helper class to create a view matrix from an OpenGL "lookat" specification
//--------------------------------------------------------------------------------------------------
Mat4d Camera::createLookAtMatrix(Vec3d eye, Vec3d vrp, Vec3d up)
{
    Vec3d y = up.getNormalized();
    Vec3d z = (eye - vrp).getNormalized(); // == -(vrp-eye)
    Vec3d x = (y^z).getNormalized();
    y = (z^x).getNormalized();

    Mat4d m;

    m.setRowCol(0, 0, x.x());  m.setRowCol(0, 1, y.x());  m.setRowCol(0, 2, z.x());    m.setRowCol(0, 3, eye.x());
    m.setRowCol(1, 0, x.y());  m.setRowCol(1, 1, y.y());  m.setRowCol(1, 2, z.y());    m.setRowCol(1, 3, eye.y());
    m.setRowCol(2, 0, x.z());  m.setRowCol(2, 1, y.z());  m.setRowCol(2, 2, z.z());    m.setRowCol(2, 3, eye.z());
    m.setRowCol(3, 0, 0);      m.setRowCol(3, 1, 0);      m.setRowCol(3, 2, 0);        m.setRowCol(3, 3, 1);

    return m;
}


//--------------------------------------------------------------------------------------------------
/// Apply the camera settings to OpenGL. Note that the Viewport settings are not applied
//--------------------------------------------------------------------------------------------------
void Camera::applyOpenGL() const
{
#ifndef CVF_OPENGL_ES
    // apply the projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(m_projectionMatrix.ptr());

    // apply the view matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(viewMatrix().ptr());
#endif
}

} // namespace cvf

