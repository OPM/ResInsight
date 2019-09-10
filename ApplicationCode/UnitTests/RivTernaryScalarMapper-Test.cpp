/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "gtest/gtest.h"

#include "RivTernaryScalarMapper.h"

#include "cvfTextureImage.h"
#include "cvfqtUtils.h"

#include <QImage>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TernaryScalarMapperTest, BasicFunctions )
{
    cvf::ref<RivTernaryScalarMapper> scalarMapper = new RivTernaryScalarMapper( cvf::Color3f::GRAY );

    cvf::ref<cvf::TextureImage> texImage = new cvf::TextureImage;
    scalarMapper->updateTexture( texImage.p(), 1.0 );

    QImage img = cvfqt::Utils::toQImage( *( texImage.p() ) );

    img.save( "c:/tmp/test.png" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( TernaryScalarMapperTest, TextureMapping )
{
    cvf::ref<RivTernaryScalarMapper> scalarMapper = new RivTernaryScalarMapper( cvf::Color3f::GRAY );

    // See RivTernaryScalarMapper::mapToTextureCoord
    double edgeClampDelta = 1.1 * 0.001;

    // Without opacity
    {
        cvf::Vec2f texCoord = scalarMapper->mapToTextureCoord( 0.0, 0.0, false );
        EXPECT_NEAR( 0.0, texCoord.x(), edgeClampDelta );
        EXPECT_NEAR( 0.0, texCoord.y(), edgeClampDelta );
    }

    {
        cvf::Vec2f texCoord = scalarMapper->mapToTextureCoord( 1.0, 0.0, false );
        EXPECT_NEAR( 1.0, texCoord.x(), edgeClampDelta );
        EXPECT_NEAR( 0.0, texCoord.y(), edgeClampDelta );
    }

    {
        cvf::Vec2f texCoord = scalarMapper->mapToTextureCoord( 0.0, 1.0, false );
        EXPECT_NEAR( 0.0, texCoord.x(), edgeClampDelta );
        EXPECT_NEAR( 0.5, texCoord.y(), edgeClampDelta );
    }

    {
        cvf::Vec2f texCoord = scalarMapper->mapToTextureCoord( 3.0, 3.0, false );
        EXPECT_NEAR( 1.0, texCoord.x(), edgeClampDelta );
        EXPECT_NEAR( 0.0, texCoord.y(), edgeClampDelta );
    }

    {
        cvf::Vec2f texCoord = scalarMapper->mapToTextureCoord( -1.0, -1.0, false );
        EXPECT_NEAR( 0.0, texCoord.x(), edgeClampDelta );
        EXPECT_NEAR( 0.0, texCoord.y(), edgeClampDelta );
    }

    {
        cvf::Vec2f texCoord = scalarMapper->mapToTextureCoord( 0.5, 3.0, false );
        EXPECT_NEAR( 0.5, texCoord.x(), edgeClampDelta );
        EXPECT_NEAR( 0.25, texCoord.y(), edgeClampDelta );
    }

    // Opacity
    {
        cvf::Vec2f texCoord = scalarMapper->mapToTextureCoord( 0.0, 0.0, true );
        EXPECT_NEAR( 0.0, texCoord.x(), edgeClampDelta );
        EXPECT_NEAR( 0.5, texCoord.y(), edgeClampDelta );
    }

    {
        cvf::Vec2f texCoord = scalarMapper->mapToTextureCoord( 0.0, 1.0, true );
        EXPECT_NEAR( 0.0, texCoord.x(), edgeClampDelta );
        EXPECT_NEAR( 1.0, texCoord.y(), edgeClampDelta );
    }
}
