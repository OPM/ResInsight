/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "cvfColor3.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include <vector>

//==================================================================================================
///
///
//==================================================================================================
class RigPolyLinesData : public cvf::Object
{
public:
    RigPolyLinesData();
    ~RigPolyLinesData() override;

    const std::vector<std::vector<cvf::Vec3d>>& polyLines() const;

    void setPolyLines( const std::vector<std::vector<cvf::Vec3d>>& polyLines );
    void setPolyLine( const std::vector<cvf::Vec3d>& polyline );
    void addPolyLine( const std::vector<cvf::Vec3d>& polyline );

    void setVisibility( bool showLines, bool showSpheres );
    void setLineAppearance( int lineThickness, cvf::Color3f color, bool closePolyline );
    void setSphereAppearance( double radiusFactor, cvf::Color3f color );
    void setZPlaneLock( bool lockToZ, double lockZValue );

    bool         showLines() const;
    bool         showSpheres() const;
    int          lineThickness() const;
    bool         closePolyline() const;
    cvf::Color3f lineColor() const;
    cvf::Color3f sphereColor() const;
    double       sphereRadiusFactor() const;
    double       lockedZValue() const;
    bool         lockToZPlane() const;

private:
    std::vector<std::vector<cvf::Vec3d>> m_polylines;

    bool m_showLines;
    int  m_lineThickness;
    bool m_closePolyline;

    bool   m_showSpheres;
    double m_sphereRadiusFactor;

    bool   m_lockToZPlane;
    double m_lockedZValue;

    cvf::Color3f m_lineColor;
    cvf::Color3f m_sphereColor;
};
