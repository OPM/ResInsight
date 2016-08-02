
#include "cafCategoryMapper.h"

#include "cvfBase.h"
#include "cvfMath.h"
#include "cvfOverlayColorLegend.h"
#include "cvfTextureImage.h"

using namespace cvf;

namespace caf {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CategoryMapper::CategoryMapper()
    : m_textureSize(2048)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CategoryMapper::setCategories(const IntArray& categories)
{
    m_categories = categories;

    ref<Color3ubArray> colorArr = ScalarMapper::colorTableArray(ColorTable::NORMAL);

    setColors(*(colorArr.p()));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CategoryMapper::setColors(const Color3ubArray& colorArray)
{
    m_colors.resize(m_categories.size());

    for (size_t i = 0; i < m_categories.size(); i++)
    {
        size_t colIdx = i % colorArray.size();
        m_colors[i] = colorArray[colIdx];
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::IntArray CategoryMapper::categories() const
{
    return m_categories;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Vec2f CategoryMapper::mapToTextureCoord(double categoryValue) const
{
    double normVal = normalizedValue(categoryValue);

    return Vec2f(static_cast<float>(normVal), 0.5f);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color3ub CategoryMapper::mapToColor(double categoryValue) const
{
    int catIndex = categoryIndexForCategory(categoryValue);

    if (catIndex != -1)
    {
        uint colorCount = static_cast<uint>(m_colors.size());
        CVF_ASSERT(colorCount > static_cast<uint>(catIndex));

        return m_colors[catIndex];
    }
    else
    {
        return Color3::BLACK;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CategoryMapper::majorTickValues(std::vector<double>* domainValues) const
{
    // Not intended to be supported
    CVF_ASSERT(false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double CategoryMapper::normalizedValue(double categoryValue) const
{
    int catIndex = categoryIndexForCategory(categoryValue);

    if (catIndex != -1)
    {
        double halfLevelHeight = 1.0 / (m_categories.size() * 2);

        double normVal = static_cast<double>(catIndex) / static_cast<double>(m_categories.size());

        return normVal + halfLevelHeight;
    }
    else
    {
        return 0.0;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double CategoryMapper::domainValue(double normalizedValue) const
{
    double clampedValue = cvf::Math::clamp(normalizedValue, 0.0, 1.0);

    if (m_categories.size() == 0)
    {
        return 0.0;
    }

    size_t catIndex = static_cast<size_t>(clampedValue * m_categories.size());
    return m_categories[catIndex];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int CategoryMapper::categoryIndexForCategory(double domainValue) const
{
    int catIndex = -1;

    size_t i = 0;
    while (i < m_categories.size() && catIndex == -1)
    {
        if (m_categories[i] == domainValue)
        {
            catIndex = static_cast<int>(i);
        }

        i++;
    }

    return catIndex;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool CategoryMapper::updateTexture(TextureImage* image) const
{
    CVF_ASSERT(image);

    image->allocate(m_textureSize, 1);

    // For now fill with white so we can see any errors more easily
    image->fill(Color4ub(Color3::WHITE));

    const uint numColors = static_cast<uint>(m_colors.size());
    if (numColors < 1)
    {
        return false;
    }

    const uint numPixelsPerColor = m_textureSize / numColors;
    CVF_ASSERT(numPixelsPerColor >= 1);

    uint ic;
    for (ic = 0; ic < numColors; ic++)
    {
        const Color4ub clr(m_colors[ic], 255);

        uint ip;
        for (ip = 0; ip < numPixelsPerColor; ip++)
        {
            image->setPixel(ic*numPixelsPerColor + ip, 0, clr);
        }
    }

    // In cases where we're not using the entire texture we might get into problems with texture coordinate precision on the graphics hardware.
    // Therefore we set one extra pixel with the 'highest' color in the color table
    if (numColors*numPixelsPerColor < m_textureSize)
    {
        const Color4ub topClr(m_colors[numColors - 1], 255);
        image->setPixel(numColors*numPixelsPerColor, 0, topClr);
    }

    return true;
}

} // namespace cvf

