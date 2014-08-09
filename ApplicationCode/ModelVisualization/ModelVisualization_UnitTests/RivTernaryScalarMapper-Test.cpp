/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

// #include "cvfLibCore.h"
// #include "cvfLibViewing.h"
// #include "cvfLibRender.h"
// #include "cvfLibGeometry.h"
// 
// #include "RivPipeGeometryGenerator.h"

#include "RivTernaryScalarMapper.h"
#include "cvfTextureImage.h"

#include <QImage>
#include "cvfqtUtils.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(TernaryScalarMapperTest, BasicFunctions)
{
	cvf::ref<RivTernaryScalarMapper> scalarMapper = new RivTernaryScalarMapper(cvf::Color3f::GRAY, 0.8f);

	cvf::ref<cvf::TextureImage> texImage = new cvf::TextureImage;
	scalarMapper->updateTexture(texImage.p());

	QImage img = cvfqt::Utils::toQImage(*(texImage.p()));

	img.save("c:/tmp/test.bmp");

}

