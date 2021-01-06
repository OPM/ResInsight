/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RiaFontCache.h"

#include "cvfCollection.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfScalarMapperDiscreteLinear.h"
#include "cvfTransform.h"

namespace cvf
{
class Camera;
class ShaderProgram;
} // namespace cvf

//==================================================================================================
//
//
//==================================================================================================
class RivGridBoxGenerator
{
public:
    RivGridBoxGenerator();

    void setScaleZ( double scaleZ );
    void setDisplayModelOffset( cvf::Vec3d offset );
    void setGridBoxDomainCoordBoundingBox( const cvf::BoundingBox& boundingBox );
    void updateFromBackgroundColor( const cvf::Color3f& backgroundColor );

    void createGridBoxParts();
    void setGridLabelFontSize( int fontSize );
    void updateFromCamera( const cvf::Camera* camera );

    cvf::Model* model();

private:
    enum AxisType
    {
        X_AXIS,
        Y_AXIS,
        Z_AXIS
    };

    enum FaceType
    {
        POS_X,
        NEG_X,
        POS_Y,
        NEG_Y,
        POS_Z,
        NEG_Z
    };

    enum EdgeType
    {
        POS_Z_POS_X,
        POS_Z_NEG_X,
        POS_Z_POS_Y,
        POS_Z_NEG_Y,

        NEG_Z_POS_X,
        NEG_Z_NEG_X,
        NEG_Z_POS_Y,
        NEG_Z_NEG_Y,

        POS_X_POS_Y,
        POS_X_NEG_Y,
        NEG_X_POS_Y,
        NEG_X_NEG_Y
    };

private:
    void createGridBoxFaceParts();
    void createGridBoxLegendParts();
    void createLegend( EdgeType edge, cvf::Collection<cvf::Part>* parts );

    void computeEdgeVisibility( const std::vector<bool>& faceVisibility, std::vector<bool>& edgeVisibility );

    void       computeDisplayCoords();
    cvf::Vec3d displayModelCoordFromDomainCoord( const cvf::Vec3d& domainCoord ) const;

    cvf::Vec3f sideNormalOutwards( FaceType face );
    cvf::Vec3d pointOnSide( FaceType face );
    cvf::Vec3f cornerDirection( FaceType face1, FaceType face2 );

private:
    cvf::BoundingBox    m_domainCoordsBoundingBox;
    std::vector<double> m_domainCoordsXValues;
    std::vector<double> m_domainCoordsYValues;
    std::vector<double> m_domainCoordsZValues;

    cvf::BoundingBox    m_displayCoordsBoundingBox;
    std::vector<double> m_displayCoordsXValues;
    std::vector<double> m_displayCoordsYValues;
    std::vector<double> m_displayCoordsZValues;

    double     m_scaleZ;
    cvf::Vec3d m_displayModelOffset;

    cvf::Collection<cvf::Part> m_gridBoxFaceParts;
    cvf::Collection<cvf::Part> m_gridBoxLegendParts;

    cvf::ref<cvf::ModelBasicList> m_gridBoxModel;

    cvf::Color3f m_gridColor;
    cvf::Color3f m_gridLegendColor;
    int          m_fontPointSize;

    bool m_needsRegeneration;
};
