//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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
#include "cvfFile.h"

#include "cvfuImageTga.h"

#include <stdio.h>
#include <vector>
#include <string.h>

CVF_GCC_DIAGNOSTIC_IGNORE("-Wconversion")

// Apparently, this warning doesn't exist until GCC 4.5
#if defined(__GNUC__) && (CVF_GCC_VER >= 40500)
CVF_GCC_DIAGNOSTIC_IGNORE("-Wunused-result")
#endif

namespace cvfu {

using cvf::ref;
using cvf::Vec3f;
using cvf::Vec2f;
using cvf::Vec3d;


//==================================================================================================
//
// Small class to load a TGA-file into a memory buffer.
//
//==================================================================================================
class TgaLoader
{
public:
	TgaLoader();

	bool	LoadTGA (const char* szFile);
	int		getWidth() const	{ return (m_iImageWidth); }
	int		getHeight() const	{ return (m_iImageHeight); }

	//! Returns the pixel at location (x,y). 
	// ATTENTION:
	//	"getPixel(x,y)[0]" is BLUE
	//	"getPixel(x,y)[1]" is GREEN
	//	"getPixel(x,y)[2]" is RED
	//	"getPixel(x,y)[3]" is ALPHA
	const unsigned char* getPixel (int x, int y) const {return (&m_Pixels[(static_cast<size_t>(y * m_iImageWidth + x) *4)]);}

private:
	void LoadCompressedTGA (FILE* pFile);
	void LoadUncompressedTGA (FILE* pFile);

	std::vector<unsigned char>	m_Pixels;
	int							m_iImageWidth;
	int							m_iImageHeight;
	int							m_iBytesPerPixel;
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TgaLoader::TgaLoader ()
{
	m_iImageWidth = 0;
	m_iImageHeight = 0;
	m_iBytesPerPixel = 0;
}


//--------------------------------------------------------------------------------------------------
/// Loads a TGA. Can be 8, 24 or 32 Bits per pixel, uncompressed or (RLE) compressed. Returns false, if the TGA could not be loaded.
//--------------------------------------------------------------------------------------------------
bool TgaLoader::LoadTGA (const char* szFile)
{
	FILE* pFile = cvf::File::fopen (szFile, "rb");

	if (!pFile)
		return (false);

	// Read the header of the TGA, compare it with the known headers for compressed and uncompressed TGAs
	unsigned char ucHeader[18];
	fread (ucHeader, sizeof (unsigned char) * 18, 1, pFile);

	while (ucHeader[0] > 0)
	{
		--ucHeader[0];

		unsigned char temp;
		fread (&temp, sizeof (unsigned char), 1, pFile);
	}

	m_iImageWidth = ucHeader[13] * 256 + ucHeader[12];
	m_iImageHeight = ucHeader[15] * 256 + ucHeader[14];
	m_iBytesPerPixel = ucHeader[16] / 8;


	// check whether width, height an BitsPerPixel are valid
	if ((m_iImageWidth <= 0) || (m_iImageHeight <= 0) || ((m_iBytesPerPixel != 1) && (m_iBytesPerPixel != 3) && (m_iBytesPerPixel != 4)))
	{
		fclose (pFile);
		return (false);
	}

	// allocate the image-buffer
	m_Pixels.resize(static_cast<size_t>(m_iImageWidth * m_iImageHeight * 4));


	// call the appropriate loader-routine
	if (ucHeader[2] == 2)
	{
		LoadUncompressedTGA (pFile);
	}
	else 
	if (ucHeader[2] == 10)
	{
		LoadCompressedTGA (pFile);
	}
	else
	{
		fclose (pFile);
		return (false);
	}

	fclose (pFile);
	return (true);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TgaLoader::LoadUncompressedTGA (FILE* pFile)
{
	unsigned char ucBuffer[4] = {255, 255, 255, 255};

	unsigned int* pIntPointer = (unsigned int*) &m_Pixels[0];
	unsigned int* pIntBuffer = (unsigned int*) &ucBuffer[0];

	const int iPixelCount	= m_iImageWidth * m_iImageHeight;

	for (int i = 0; i < iPixelCount; ++i)
	{
		fread (ucBuffer, sizeof (unsigned char) * m_iBytesPerPixel, 1, pFile);

		// if this is an 8-Bit TGA only, store the one channel in all four channels
		// if it is a 24-Bit TGA (3 channels), the fourth channel stays at 255 all the time, since the 4th value in ucBuffer is never overwritten
		if (m_iBytesPerPixel == 1)
		{
			ucBuffer[1] = ucBuffer[0];
			ucBuffer[2] = ucBuffer[0];
			ucBuffer[3] = ucBuffer[0];
		}

		// copy all four values in one operation
		(*pIntPointer) = (*pIntBuffer);
		++pIntPointer;
	}
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TgaLoader::LoadCompressedTGA (FILE* pFile)
{
	int iCurrentPixel	= 0;
	//int iCurrentByte	= 0;
	unsigned char ucBuffer[4] = {255, 255, 255, 255};
	const int iPixelCount	= m_iImageWidth * m_iImageHeight;

	unsigned int* pIntPointer = (unsigned int*) &m_Pixels[0];
	unsigned int* pIntBuffer = (unsigned int*) &ucBuffer[0];

	do
	{
		unsigned char ucChunkHeader = 0;

		fread (&ucChunkHeader, sizeof (unsigned char), 1, pFile);

		if (ucChunkHeader < 128)
		{
			// If the header is < 128, it means it is the number of RAW color packets minus 1
			// that follow the header
			// add 1 to get number of following color values

			ucChunkHeader++;	

			// Read RAW color values
			for (int i = 0; i < (int) ucChunkHeader; ++i)	
			{
				fread (&ucBuffer[0], static_cast<size_t>(m_iBytesPerPixel), 1, pFile);

				// if this is an 8-Bit TGA only, store the one channel in all four channels
				// if it is a 24-Bit TGA (3 channels), the fourth channel stays at 255 all the time, since the 4th value in ucBuffer is never overwritten
				if (m_iBytesPerPixel == 1)
				{
					ucBuffer[1] = ucBuffer[0];
					ucBuffer[2] = ucBuffer[0];
					ucBuffer[3] = ucBuffer[0];
				}

				// copy all four values in one operation
				(*pIntPointer) = (*pIntBuffer);

				++pIntPointer;
				++iCurrentPixel;
			}
		}
		else // chunkheader > 128 RLE data, next color reapeated (chunkheader - 127) times
		{
			ucChunkHeader -= 127;	// Subteact 127 to get rid of the ID bit

			// read the current color
			fread (&ucBuffer[0], static_cast<size_t>(m_iBytesPerPixel), 1, pFile);

			// if this is an 8-Bit TGA only, store the one channel in all four channels
			// if it is a 24-Bit TGA (3 channels), the fourth channel stays at 255 all the time, since the 4th value in ucBuffer is never overwritten
			if (m_iBytesPerPixel == 1)
			{
				ucBuffer[1] = ucBuffer[0];
				ucBuffer[2] = ucBuffer[0];
				ucBuffer[3] = ucBuffer[0];
			}

			// copy the color into the image data as many times as dictated 
			for (int i = 0; i < (int) ucChunkHeader; ++i)
			{
				(*pIntPointer) = (*pIntBuffer);
				++pIntPointer;

				++iCurrentPixel;
			}
		}
	}
	while (iCurrentPixel < iPixelCount);
}



//==================================================================================================
///
/// \class cvfu::ImageTga
/// \ingroup Utilities
///
/// Reading of TGA image files.
/// Supports 24 and 32 bit images, both RLE compressed and uncompressed. 
/// Does not support color mapped images, nor 8 and 16 bit images.
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::TextureImage> ImageTga::loadImage(cvf::String fileName)
{
	TgaLoader tgaImg;
	if (!tgaImg.LoadTGA(fileName.toAscii().ptr()))
    {
        return NULL;
    }

	int width = tgaImg.getWidth();
	int height = tgaImg.getHeight();
    if (width <= 0 || height <= 0)
    {
        return NULL;
    }

	cvf::ref<cvf::TextureImage> image = new cvf::TextureImage;
    image->allocate(static_cast<cvf::uint>(width), static_cast<cvf::uint>(height));

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			const unsigned char* pixBGRA  = tgaImg.getPixel(x, y);
			cvf::Color4ub pixelClr(pixBGRA[2], pixBGRA[1], pixBGRA[0], pixBGRA[3]);
			image->setPixel(static_cast<cvf::uint>(x), static_cast<cvf::uint>(y), pixelClr);
		}
	}

	return image;
}

} // namespace cvfu
