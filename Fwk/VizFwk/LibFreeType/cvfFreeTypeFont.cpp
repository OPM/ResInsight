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
#include "cvfGlyph.h"
#include "cvfTextureImage.h"
#include "cvfFreeTypeFont.h"

#ifdef WIN32
#define CVF_USE_FREETYPE_LIB
#endif
// Todo: Implement and check on Linux and iOS

#ifdef CVF_USE_FREETYPE_LIB
#include "cvfBase64.h"

#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H

#endif // CVF_USE_FREETYPE_LIB


namespace cvf {

//==================================================================================================
//
// Private implementation of the FreeType font interface
//
//==================================================================================================
class FreeTypeFontInterface : public Object
{
#ifdef CVF_USE_FREETYPE_LIB
public:

    FT_Library  m_ftLib;    // FreeType interface library
    FT_Face     m_ftFace;   // FreeType face for currently loaded font
    UByteArray  m_data;     // Locally stored font data. Only used when loading font via FT_New_Memory_Face(..)

    uint        m_dpiX;     // Horizontal DPI
    uint        m_dpiY;     // Vertical DPI

public:
    //--------------------------------------------------------------------------------------------------
    /// Constructor
    //--------------------------------------------------------------------------------------------------
    FreeTypeFontInterface(uint dpiX, uint dpiY)
    :   m_ftLib(NULL),
        m_ftFace(NULL),
        m_dpiX(dpiX),
        m_dpiY(dpiY)
    {
        // Initialize FreeType library
        // Seems that init the reasons for failure here would be memory allocations
        // Since we're inside a constructor this will amount to an exception
        FT_Error error = FT_Init_FreeType(&m_ftLib);
        CVF_ASSERT(!error);
        CVF_UNUSED(error);
        //if (error)
        //{
        //    m_ftLib = NULL;
        //    CVF_ERROR(String("Unable to initialize the FreeType library: Error [%1]").arg((int)error));
        //}
    };


    //--------------------------------------------------------------------------------------------------
    /// Destructor
    //--------------------------------------------------------------------------------------------------
    ~FreeTypeFontInterface()
    {
        unload();

        // Unload FreeType library
        FT_Done_FreeType(m_ftLib);
    }
    

    //--------------------------------------------------------------------------------------------------
    /// Returns true if the font is set up properly and ready to be used, otherwise returns false.
    //--------------------------------------------------------------------------------------------------
    bool isEmpty() const
    {
        return m_ftFace ? false : true;
    }
    

    //--------------------------------------------------------------------------------------------------
    /// Get the name of the font
    //--------------------------------------------------------------------------------------------------
    String name()
    {
        String fontName;

        if (!isEmpty())
        {
            if (m_ftFace->family_name)
            {
                fontName += m_ftFace->family_name;

                if (m_ftFace->style_name)
                {
                    fontName += " ";
                    fontName += m_ftFace->style_name;
                }
            }
        }

        return fontName;
    }


    //--------------------------------------------------------------------------------------------------
    /// Load font from FreeType data format. Note: updateResolution(..) needs to be called after this.
    //--------------------------------------------------------------------------------------------------
    bool load(const ubyte* data, size_t numBytes)
    {
        CVF_TIGHT_ASSERT(data);
        CVF_TIGHT_ASSERT(numBytes > 0);
        CVF_TIGHT_ASSERT(m_ftLib);

        // Close currently loaded font, if any
        unload();

        // Make a copy of the font data to ensure it's alive as long as the font is open as required
        // by FT_New_Memory_Face(..). This is, of course, not a requirement for FT_New_Face(<filename>)
        m_data.assign(data, numBytes);

        // Translate into FreeType data-types
        const FT_Byte* file_base = static_cast<const FT_Byte*>(m_data.ptr());
        FT_Long file_size = static_cast<FT_Long>(m_data.size());

        FT_Error error = FT_New_Memory_Face(m_ftLib, file_base, file_size, 0, &m_ftFace);
        return error ? false : true;
    }


    //--------------------------------------------------------------------------------------------------
    /// Load font from file. Note: updateResolution(..) needs to be called after this.
    //--------------------------------------------------------------------------------------------------
    bool load(const cvf::String& filename)
    {
        // Close currently loaded font, if any
        unload();

        FT_Error error = FT_New_Face(m_ftLib, filename.toUtf8().ptr(), 0, &m_ftFace);
        return error ? false : true;
    }


    //--------------------------------------------------------------------------------------------------
    /// Load base64 encoded font from memory. Note: updateResolution(..) needs to be called after this.
    //--------------------------------------------------------------------------------------------------
    bool load(const char** data, size_t numBlocks)
    {
        std::string encodedFontData;
        size_t i;
        for (i = 0; i < numBlocks; i++)
        {
            encodedFontData += data[i];
        }
        // Todo: Optimize this!

        ref<UByteArray> decodedFontData = Base64::decode(encodedFontData);
        if (decodedFontData.isNull()) return false;
        if (decodedFontData->size() == 0) return false;
        
        const FT_Byte* file_base = static_cast<const FT_Byte*>(static_cast<const void*>(decodedFontData->ptr()));
        FT_Long file_size = static_cast<FT_Long>(decodedFontData->size());

        return load(file_base, file_size);
    }


    //--------------------------------------------------------------------------------------------------
    /// Update font size
    //--------------------------------------------------------------------------------------------------
    bool setSize(uint size)
    {
        CVF_ASSERT(!isEmpty());

        FT_Error error = FT_Set_Char_Size(m_ftFace, 0, static_cast<FT_F26Dot6>(size) << 6, m_dpiX, m_dpiY);
        if (error)
        {
            //CVF_ERROR(String("Unable to update FreeType font size: Error [%1]").arg((int)error));
            return false;
        }

        return true;
    }


    //--------------------------------------------------------------------------------------------------
    /// Close font if open
    //--------------------------------------------------------------------------------------------------
    void unload()
    {
        if (isEmpty()) return;

        FT_Done_Face(m_ftFace);
        m_ftFace = NULL;
        m_data.clear();
    }


    //--------------------------------------------------------------------------------------------------
    /// Create a glyph for the given character
    //--------------------------------------------------------------------------------------------------
    ref<Glyph> createGlyph(wchar_t character)
    {
        if (isEmpty()) return NULL;

        // Look up the glyph index corresponding to the given character code in the charmap that is currently 
        // selected for the face. This should really be the UTF-32 representation form of Unicode; for example, 
        // if you want to load character U+1F028, use value 0x1F028 as the value for character.
        //
        // Note that FT_Get_Char_Index is one of the rare FreeType functions that do not return an error code. 
        // However, when a given character code has no glyph image in the face, the value 0 is returned. 
        // By convention, it always corresponds to a special glyph image called the missing glyph, which 
        // is commonly displayed as a box or a space.
        FT_UInt glyph_index = FT_Get_Char_Index(m_ftFace, character);

        // Load a single glyph into the glyph slot of a face object
        FT_Error error = FT_Load_Glyph(m_ftFace, glyph_index, FT_LOAD_DEFAULT);
        if (error)
        {
            //CVF_ERROR(String("Unable to load glyph from FreeType font: Error [%1]").arg((int)error));
            return NULL;
        }

        // Convert a given glyph image to a bitmap. It does so by inspecting the glyph image format,
        // finding the relevant renderer, and invoking it.
        error = FT_Render_Glyph(m_ftFace->glyph, FT_RENDER_MODE_NORMAL);
        if (error)
        {
            //CVF_ERROR(String("Unable to convert FreeType font glyph image to bitmap: Error [%1]").arg((int)error));
            return NULL;
        }
        CVF_ASSERT(m_ftFace->glyph->advance.x == m_ftFace->glyph->metrics.horiAdvance);

        //
        ref<Glyph> glyph = new Glyph(character);
        glyph->setHorizontalBearingX(static_cast<short>(m_ftFace->glyph->metrics.horiBearingX) >> 6);   // NB! May be negative
        glyph->setHorizontalBearingY(static_cast<short>(m_ftFace->glyph->metrics.horiBearingY) >> 6);   // NB! May be negative
        glyph->setHorizontalAdvance(m_ftFace->glyph->metrics.horiAdvance >> 6);


        // Create from the given FreeType glyph, if valid
        if (m_ftFace->glyph && (m_ftFace->glyph->bitmap.width > 0) && (m_ftFace->glyph->bitmap.rows > 0))
        {
            CVF_ASSERT(static_cast<int>(m_ftFace->glyph->metrics.width >> 6)  == static_cast<int>(m_ftFace->glyph->bitmap.width));
            CVF_ASSERT(static_cast<int>(m_ftFace->glyph->metrics.height >> 6) == static_cast<int>(m_ftFace->glyph->bitmap.rows));

            // Safe to cast
            uint bitmapWidth = static_cast<uint>(m_ftFace->glyph->bitmap.width);
            uint bitmapHeight = static_cast<uint>(m_ftFace->glyph->bitmap.rows);

            // Prepare texture image
            TextureImage* textureImage = new TextureImage;
            textureImage->allocate(bitmapWidth, bitmapHeight);

            ubyte colorComponent;

            // Populate m_ftFace->glyph texture image
            uint x, y; 
            for (y = 0; y < bitmapHeight; y++)
            {
                for (x = 0; x < bitmapWidth; x++)
                {
                    colorComponent = m_ftFace->glyph->bitmap.buffer[((m_ftFace->glyph->bitmap.rows-1-y)*m_ftFace->glyph->bitmap.width)+x];

                    // Todo: Consider replacing this with a two-channel GL_LUMINANCE_ALPHA
                    textureImage->setPixel(x, y, Color4ub(255, 255, 255, colorComponent));
                }
            }

            glyph->setTextureImage(textureImage);

            glyph->setWidth(textureImage->width());
            glyph->setHeight(textureImage->height());
        }

        // Or create an empty m_ftFace->glyph
        else
        {
            // Make sure we've got an advance. 
            // This will be used to create a totally transparent textureImage.
            if (glyph->horizontalAdvance() == 0)
            {
                // Load the missing glyph and get advance from there
                FT_Error error = FT_Load_Glyph(m_ftFace, 0, FT_LOAD_DEFAULT);
                if (error)
                {
                    //CVF_ERROR(String("Unable to load the 'missing-glyph' from FreeType font: Error [%1]").arg((int)error));

                    // Still unable to get an advance -> hardcode it
                    glyph->setHorizontalAdvance(10);
                }
                else
                {
                    glyph->setHorizontalAdvance(m_ftFace->glyph->advance.x >> 6);
                }

                // Todo: Cach up value
            }
            CVF_ASSERT(glyph->horizontalAdvance() > 0);

            // Prepare texture image
            TextureImage* textureImage = new TextureImage;
            textureImage->allocate(glyph->horizontalAdvance(), glyph->horizontalAdvance());
            textureImage->fill(Color4ub(0, 0, 0, 0));  // Totally transparent!
            glyph->setTextureImage(textureImage);

            glyph->setWidth(textureImage->width());
            glyph->setHeight(textureImage->height());

            // Bearing is not needed since we've filled the whole area based on the advance
            glyph->setHorizontalBearingX(0);
            glyph->setHorizontalBearingY(0);
        }

        return glyph;
    }


    //--------------------------------------------------------------------------------------------------
    /// Get kerning values for in-between-characters for given string
    //--------------------------------------------------------------------------------------------------
    bool getKerning(String* str, std::vector<int>* kerningVector)
    {
        CVF_TIGHT_ASSERT(str);
        CVF_TIGHT_ASSERT(kerningVector);

        if (isEmpty()) return false;
        if (!FT_HAS_KERNING(m_ftFace)) return false;

        size_t numCharacters = str->size();
        if (numCharacters < 2) return false;

        kerningVector->clear();
        kerningVector->reserve(numCharacters-1);

        // Initialized to 'missing glyph'.
        // There is never any kerning distance associated with this glyph.
        FT_UInt curr_glyph_index = 0;   // Current glyph
        FT_UInt prev_glyph_index = 0;   // Previous glyph

        FT_Vector delta;

        size_t i;
        for (i = 0; i < numCharacters; i++)
        {
            const wchar_t character = str->c_str()[i];

            curr_glyph_index = FT_Get_Char_Index(m_ftFace, character);

            if (i > 0)
            {
                FT_Get_Kerning(m_ftFace, prev_glyph_index, curr_glyph_index, FT_KERNING_DEFAULT, &delta);

                // Note: We do not check the error code returned by FT_Get_Kerning. This is because
                // the function always sets the content of delta to (0,0) when an error occurs.

                kerningVector->push_back(delta.x >> 6);
            }

            prev_glyph_index = curr_glyph_index;
        }

        return true;
    }


    //--------------------------------------------------------------------------------------------------
    /// Get kerning value between two characters
    //--------------------------------------------------------------------------------------------------
    bool getKerning(wchar_t character, wchar_t nextCharacter, int* kerning)
    {
        if (isEmpty()) return false;
        if (!FT_HAS_KERNING(m_ftFace)) return false;

        FT_UInt curr_glyph_index = FT_Get_Char_Index(m_ftFace, character);
        FT_UInt next_glyph_index = FT_Get_Char_Index(m_ftFace, nextCharacter);

        FT_Vector delta;
        FT_Get_Kerning(m_ftFace, curr_glyph_index, next_glyph_index, FT_KERNING_DEFAULT, &delta);

        // Note: We do not check the error code returned by FT_Get_Kerning. This is because
        // the function always sets the content of delta to (0,0) when an error occurs.

        *kerning = delta.x >> 6;

        return true;
    }


    //--------------------------------------------------------------------------------------------------
    /// Get FreeType library version
    //--------------------------------------------------------------------------------------------------
    static bool version(int* major, int* minor, int* patch)
    {
        FT_Library ftLib;
        FT_Error error = FT_Init_FreeType(&ftLib);
        if (error) return false;

        FT_Library_Version(ftLib, major, minor, patch);
        FT_Done_FreeType(ftLib);
        
        return true;
    }


    //--------------------------------------------------------------------------------------------------
    /// Get FreeType library copyright year.
    //  NB! Must be updated when upgrading the FreeType library!
    //--------------------------------------------------------------------------------------------------
    static String copyRightYear()
    {
        return String("1996-2002, 2006");
    }


    //--------------------------------------------------------------------------------------------------
    /// Get FreeType library credit text
    //--------------------------------------------------------------------------------------------------
    static String credit()
    {    
        int major = 0;
        int minor = 0;
        int patch = 0;
        version(&major, &minor, &patch);

        return String("FreeType %1.%2.%3\n"
                      "Portions of this software are Copyright © %4\n"
                      "The FreeType Project (www.freetype.org).\n"
                      "All rights reserved.")
                      .arg(major).arg(minor).arg(patch).arg(copyRightYear());
    }    
#endif // CVF_USE_FREETYPE_LIB
};  // FreeTypeFontInterface


//==================================================================================================
///
/// \class cvf::FreeTypeFont
/// \ingroup FreeType
///
/// Loads a given FreeType font to be used when drawing text
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
FreeTypeFont::FreeTypeFont(uint dpiX, uint dpiY)
:   m_name(L"")
{
#ifdef CVF_USE_FREETYPE_LIB
    m_fontInterface = new FreeTypeFontInterface(dpiX, dpiY);
#endif
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
FreeTypeFont::~FreeTypeFont()
{
    unload();
}


//--------------------------------------------------------------------------------------------------
/// Get the name of the currently loaded font. An empty string is returned if no font is loaded.
//--------------------------------------------------------------------------------------------------
const String& FreeTypeFont::name() const
{
    CVF_TIGHT_ASSERT(m_fontInterface.notNull());

    return m_name;
}


//--------------------------------------------------------------------------------------------------
/// Get glyph for the given character. Will be created if not already present.
//--------------------------------------------------------------------------------------------------
ref<Glyph> FreeTypeFont::getGlyph(wchar_t character)
{
    CVF_TIGHT_ASSERT(m_fontInterface.notNull());

    ref<Glyph> glyph = NULL;
    if (!isEmpty())
    {
        // Glyph already available?
        MapType::iterator it = m_atlasMap.find(character);
        if (it != m_atlasMap.end())
        {
            // Yes, return existing glyph
            glyph = it->second;
        }
        else
        {
#ifdef CVF_USE_FREETYPE_LIB
            CVF_TIGHT_ASSERT(m_fontInterface.notNull());
            glyph = m_fontInterface->createGlyph(character);

            m_atlasMap.insert(MapType::value_type(character, glyph));
#endif // CVF_USE_FREETYPE_LIB
        }
    }

    CVF_TIGHT_ASSERT(glyph.notNull());
    return glyph;
}


//--------------------------------------------------------------------------------------------------
/// Get advance for character glyph in relation to the next character
//--------------------------------------------------------------------------------------------------
uint FreeTypeFont::advance(wchar_t character, wchar_t nextCharacter)
{
    CVF_TIGHT_ASSERT(m_fontInterface.notNull());

    int kerning = 0;
#ifdef CVF_USE_FREETYPE_LIB
    m_fontInterface->getKerning(character, nextCharacter, &kerning);
#endif

    ref<Glyph> characterGlyph = getGlyph(character);
    CVF_TIGHT_ASSERT(characterGlyph.notNull());
    CVF_TIGHT_ASSERT(characterGlyph->textureImage());

    ref<Glyph> nextCharacterGlyph = getGlyph(nextCharacter);
    CVF_TIGHT_ASSERT(nextCharacterGlyph.notNull());
    CVF_TIGHT_ASSERT(nextCharacterGlyph->textureImage());

    uint characterWidth = characterGlyph->horizontalBearingX() + characterGlyph->textureImage()->width() + kerning;

    return CVF_MAX(characterWidth, characterGlyph->horizontalAdvance());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool FreeTypeFont::isEmpty()
{
#ifdef CVF_USE_FREETYPE_LIB
    CVF_TIGHT_ASSERT(m_fontInterface.notNull());
    return m_fontInterface->isEmpty();
#else
    return true;
#endif
}


//--------------------------------------------------------------------------------------------------
/// Set font size
//--------------------------------------------------------------------------------------------------
void FreeTypeFont::setSize(uint size)
{
    m_atlasMap.clear();

    CVF_TIGHT_ASSERT(m_fontInterface.notNull());

#ifdef CVF_USE_FREETYPE_LIB
    m_fontInterface->setSize(size);
#endif
}


//--------------------------------------------------------------------------------------------------
/// Load font from file. Note: Any existing font will be removed!
//--------------------------------------------------------------------------------------------------
bool FreeTypeFont::load(const cvf::String& path)
{
    unload();

#ifdef CVF_USE_FREETYPE_LIB
    CVF_TIGHT_ASSERT(m_fontInterface.notNull());
    bool loaded = m_fontInterface->load(path);
    m_name = m_fontInterface->name();

    return loaded;
#else
    CVF_UNUSED(path);
    return false;
#endif
}


//--------------------------------------------------------------------------------------------------
/// Load font from memory. 
///
/// Note that any existing font will be removed. Also, the the array must 
/// exist as long as the font is in use.
//--------------------------------------------------------------------------------------------------
bool FreeTypeFont::load(const UByteArray* data)
{
    CVF_TIGHT_ASSERT(data);
    CVF_TIGHT_ASSERT(data->size() > 0);

    unload();

#ifdef CVF_USE_FREETYPE_LIB
    CVF_TIGHT_ASSERT(m_fontInterface.notNull());
    bool loaded = m_fontInterface->load(data->ptr(), data->size());
    m_name = m_fontInterface->name();

    return loaded;
#else
    return false;
#endif
}


//--------------------------------------------------------------------------------------------------
/// Load base64 encoded font from memory
///
/// Note that any existing font will be removed. Also, the the array must 
/// exist as long as the font is in use.
//--------------------------------------------------------------------------------------------------
bool FreeTypeFont::load(const char** data, size_t numBlocks)
{
    CVF_TIGHT_ASSERT(data);
    CVF_TIGHT_ASSERT(numBlocks > 0);

    unload();

#ifdef CVF_USE_FREETYPE_LIB
    CVF_TIGHT_ASSERT(m_fontInterface.notNull());
    bool loaded = m_fontInterface->load(data, numBlocks);
    m_name = m_fontInterface->name();

    return loaded;
#else
    return false;
#endif
}


//--------------------------------------------------------------------------------------------------
/// Unload current font
//--------------------------------------------------------------------------------------------------
void FreeTypeFont::unload()
{
    m_atlasMap.clear();
    m_name = "";
    
#ifdef CVF_USE_FREETYPE_LIB
    CVF_TIGHT_ASSERT(m_fontInterface.notNull());
    m_fontInterface->unload();
#endif
}


//--------------------------------------------------------------------------------------------------
/// Get FreeType library version
//--------------------------------------------------------------------------------------------------
bool FreeTypeFont::version(int* major, int* minor, int* patch)
{
    CVF_ASSERT(major);
    CVF_ASSERT(minor);
    CVF_ASSERT(patch);

#ifdef CVF_USE_FREETYPE_LIB
    return FreeTypeFontInterface::version(major, minor, patch);
#else
    *major = *minor = *patch = -1;
    return false;
#endif
}


//--------------------------------------------------------------------------------------------------
/// Get FreeType library credit text
//--------------------------------------------------------------------------------------------------
String FreeTypeFont::credit()
{
#ifdef CVF_USE_FREETYPE_LIB
    return FreeTypeFontInterface::credit();
#else
    return L"";
#endif
}

}  // namespace cvf
