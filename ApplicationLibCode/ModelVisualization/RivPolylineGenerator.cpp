/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RivPolylineGenerator.h"

#include "RiaColorTools.h"
#include "RiaGuiApplication.h"
#include "RiaPreferences.h"

#include "RiuGuiTheme.h"

#include "cvfCamera.h"
#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfPrimitiveSetDirect.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfqtUtils.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivPolylineGenerator::createLineAlongPolylineDrawable( const std::vector<cvf::Vec3d>& polyLine, bool closeLine )
{
    std::vector<std::vector<cvf::Vec3d>> polyLines;
    polyLines.push_back( polyLine );
    return createLineAlongPolylineDrawable( polyLines, closeLine );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivPolylineGenerator::createLineAlongPolylineDrawable( const std::vector<std::vector<cvf::Vec3d>>& polyLines,
                                                                                  bool                                        closeLine )
{
    std::vector<cvf::uint>  lineIndices;
    std::vector<cvf::Vec3f> vertices;

    for ( const std::vector<cvf::Vec3d>& polyLine : polyLines )
    {
        if ( polyLine.size() < 2 ) continue;

        size_t verticesCount = vertices.size();

        for ( size_t i = 0; i < polyLine.size(); ++i )
        {
            vertices.emplace_back( polyLine[i] );
            if ( i < polyLine.size() - 1 )
            {
                lineIndices.push_back( static_cast<cvf::uint>( verticesCount + i ) );
                lineIndices.push_back( static_cast<cvf::uint>( verticesCount + i + 1 ) );
            }
        }

        if ( closeLine && vertices.front() != vertices.back() )
        {
            lineIndices.push_back( static_cast<cvf::uint>( verticesCount + polyLine.size() - 1 ) );
            lineIndices.push_back( static_cast<cvf::uint>( verticesCount ) );
        }
    }

    if ( vertices.empty() ) return nullptr;

    cvf::ref<cvf::Vec3fArray> vx = new cvf::Vec3fArray;
    vx->assign( vertices );
    cvf::ref<cvf::UIntArray> idxes = new cvf::UIntArray;
    idxes->assign( lineIndices );

    cvf::ref<cvf::PrimitiveSetIndexedUInt> prim = new cvf::PrimitiveSetIndexedUInt( cvf::PT_LINES );
    prim->setIndices( idxes.p() );

    cvf::ref<cvf::DrawableGeo> polylineGeo = new cvf::DrawableGeo;
    polylineGeo->setVertexArray( vx.p() );
    polylineGeo->addPrimitiveSet( prim.p() );

    return polylineGeo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivPolylineGenerator::createPointsFromPolylineDrawable( const std::vector<cvf::Vec3d>& polyLine )
{
    std::vector<std::vector<cvf::Vec3d>> polyLines;
    polyLines.push_back( polyLine );
    return createPointsFromPolylineDrawable( polyLines );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivPolylineGenerator::createPointsFromPolylineDrawable( const std::vector<std::vector<cvf::Vec3d>>& polyLines )
{
    std::vector<cvf::Vec3f> vertices;

    for ( const std::vector<cvf::Vec3d>& polyLine : polyLines )
    {
        for ( const auto& pl : polyLine )
        {
            vertices.emplace_back( pl );
        }
    }

    if ( vertices.empty() ) return nullptr;

    cvf::ref<cvf::PrimitiveSetDirect> primSet = new cvf::PrimitiveSetDirect( cvf::PT_POINTS );
    primSet->setStartIndex( 0 );
    primSet->setIndexCount( vertices.size() );

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;

    cvf::ref<cvf::Vec3fArray> vx = new cvf::Vec3fArray( vertices );
    geo->setVertexArray( vx.p() );
    geo->addPrimitiveSet( primSet.p() );

    return geo;
}

///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivPolylineGenerator::createSetOfLines( const std::vector<std::vector<cvf::Vec3d>>& lines )
{
    std::vector<cvf::uint>  lineIndices;
    std::vector<cvf::Vec3f> vertices;

    for ( const std::vector<cvf::Vec3d>& polyLine : lines )
    {
        if ( polyLine.size() < 2 ) continue;

        size_t verticesCount = vertices.size();

        for ( size_t i = 0; i < polyLine.size(); i += 2 )
        {
            vertices.emplace_back( polyLine[i] );
            vertices.emplace_back( polyLine[i + 1] );
            if ( i < polyLine.size() - 1 )
            {
                lineIndices.push_back( static_cast<cvf::uint>( verticesCount + i ) );
                lineIndices.push_back( static_cast<cvf::uint>( verticesCount + i + 1 ) );
            }
        }
    }

    if ( vertices.empty() ) return nullptr;

    cvf::ref<cvf::Vec3fArray> vx = new cvf::Vec3fArray;
    vx->assign( vertices );
    cvf::ref<cvf::UIntArray> idxes = new cvf::UIntArray;
    idxes->assign( lineIndices );

    cvf::ref<cvf::PrimitiveSetIndexedUInt> prim = new cvf::PrimitiveSetIndexedUInt( cvf::PT_LINES );
    prim->setIndices( idxes.p() );

    cvf::ref<cvf::DrawableGeo> polylineGeo = new cvf::DrawableGeo;
    polylineGeo->setVertexArray( vx.p() );
    polylineGeo->addPrimitiveSet( prim.p() );

    return polylineGeo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableText> RivPolylineGenerator::createOrientedLabel( bool               negativeXDirection,
                                                                       const cvf::Camera* camera,
                                                                       const cvf::Vec3d&  labelPosition,
                                                                       const QString&     labelText )
{
    RiaGuiApplication* app = RiaGuiApplication::instance();

    auto backgroundColor = RiaPreferences::current()->defaultViewerBackgroundColor;
    auto fontColor       = RiuGuiTheme::getColorByVariableName( "textColor" );
    auto font            = app->defaultWellLabelFont();

    cvf::ref<cvf::DrawableText> drawableText = new cvf::DrawableText;
    drawableText->setFont( font );
    drawableText->setCheckPosVisible( false );
    drawableText->setDrawBorder( true );
    drawableText->setDrawBackground( true );
    drawableText->setVerticalAlignment( cvf::TextDrawer::BASELINE );
    drawableText->setBackgroundColor( backgroundColor );
    drawableText->setBorderColor( RiaColorTools::computeOffsetColor( backgroundColor, 0.3f ) );
    drawableText->setTextColor( RiaColorTools::fromQColorTo3f( fontColor ) );

    cvf::String cvfString = cvfqt::Utils::toString( labelText );

    cvf::Vec3d finalPosition = labelPosition;
    if ( camera )
    {
        cvf::Vec3d windowLabelPosition;
        camera->project( labelPosition, &windowLabelPosition );

        auto oneCharBB = drawableText->textBoundingBox( cvf::String( "A" ), cvf::Vec3f( labelPosition ) );
        if ( negativeXDirection )
        {
            auto textBB = drawableText->textBoundingBox( cvfString, cvf::Vec3f( labelPosition ) );
            windowLabelPosition.x() -= textBB.extent().x() + oneCharBB.extent().x();
        }
        else
        {
            windowLabelPosition.x() += oneCharBB.extent().x();
        }

        camera->unproject( windowLabelPosition, &finalPosition );
    }

    drawableText->addText( cvfString, cvf::Vec3f( finalPosition ) );

    return drawableText;
}
