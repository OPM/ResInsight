
#include "cafCategoryMapper.h"

#include "cvfBase.h"
#include "cvfMath.h"
#include "cvfOverlayColorLegend.h"
#include "cvfTextureImage.h"
#include <cmath>

using namespace cvf;

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CategoryMapper::CategoryMapper()
    : m_textureSize( 2048 )
    , m_maxTexCoord( 1.0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CategoryMapper::setCategories( const std::vector<int>& categoryValues )
{
    m_categoryValues = categoryValues;

    ref<Color3ubArray> colorArr = ScalarMapper::colorTableArray( ColorTable::NORMAL );

    setCycleColors( *( colorArr.p() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CategoryMapper::setCategoriesWithNames( const std::vector<int>&         categoryValues,
                                             const std::vector<cvf::String>& categoryNames )
{
    m_categoryValues = categoryValues;
    m_categoryNames  = categoryNames;

    ref<Color3ubArray> colorArr = ScalarMapper::colorTableArray( ColorTable::NORMAL );

    setInterpolateColors( *( colorArr.p() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CategoryMapper::setCategoriesValueNameColor( const std::vector<int>&         categoryValues,
                                                  const std::vector<cvf::String>& categoryNames,
                                                  const cvf::Color3ubArray&       colorArray )
{
    m_categoryValues = categoryValues;
    m_categoryNames  = categoryNames;
    m_colors         = colorArray;

    recomputeMaxTexCoord();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CategoryMapper::setCycleColors( const Color3ubArray& colorArray )
{
    m_colors.resize( m_categoryValues.size() );

    for ( size_t i = 0; i < m_categoryValues.size(); i++ )
    {
        size_t colIdx = i % colorArray.size();
        m_colors[i]   = colorArray[colIdx];
    }

    recomputeMaxTexCoord();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CategoryMapper::setInterpolateColors( const cvf::Color3ubArray& colorArray )
{
    CVF_ASSERT( colorArray.size() );

    if ( m_categoryValues.size() > 1 && colorArray.size() > 1 )
    {
        m_colors = *interpolateColorArray( colorArray, static_cast<cvf::uint>( m_categoryValues.size() ) );

        recomputeMaxTexCoord();
        return;
    }

    // Either we are requesting one output color, or we have only one input color

    m_colors.clear();
    m_colors.reserve( m_categoryValues.size() );
    for ( size_t cIdx = 0; cIdx < m_categoryValues.size(); ++cIdx )
    {
        m_colors.add( colorArray[0] );
    }

    recomputeMaxTexCoord();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t CategoryMapper::categoryCount() const
{
    return m_categoryValues.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const cvf::String CategoryMapper::textForCategoryIndex( size_t index ) const
{
    CVF_ASSERT( index < m_categoryValues.size() );
    if ( index < m_categoryNames.size() )
    {
        return m_categoryNames[index];
    }
    else
    {
        double tickValue = m_categoryValues[index];
        return String::number( tickValue );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Vec2f CategoryMapper::mapToTextureCoord( double categoryValue ) const
{
    double normVal = normalizedValue( categoryValue );

    double s = normVal * m_maxTexCoord;

    // Clamp to the currently legal texture coord range
    // Might need to add code to correct for float precision, but that is probably not the main enemy.
    // Our real problem is the fact that in most cases the texture coords get treated with even less precision
    // on the graphics hardware. What we would really like is to guess at the HW precision and then correct for that.
    // Currently the workaround is done in updateTexture() which pads the upper end of the texture when we're not
    // filling all the texture pixels.
    s = Math::clamp( s, 0.0, m_maxTexCoord );

    return Vec2f( static_cast<float>( s ), 0.5f );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Color3ub CategoryMapper::mapToColor( double categoryValue ) const
{
    int catIndex = categoryIndexForCategory( categoryValue );

    if ( catIndex != -1 )
    {
        uint colorCount = static_cast<uint>( m_colors.size() );
        CVF_ASSERT( colorCount > static_cast<uint>( catIndex ) );

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
void CategoryMapper::majorTickValues( std::vector<double>* domainValues ) const
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double CategoryMapper::normalizedValue( double categoryValue ) const
{
    int catIndex = categoryIndexForCategory( categoryValue );

    if ( catIndex != -1 )
    {
        double halfLevelHeight = 1.0 / ( m_categoryValues.size() * 2 );

        double normVal = static_cast<double>( catIndex ) / static_cast<double>( m_categoryValues.size() );

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
double CategoryMapper::domainValue( double normalizedValue ) const
{
    double clampedValue = cvf::Math::clamp( normalizedValue, 0.0, 1.0 );

    if ( m_categoryValues.size() == 0 )
    {
        return 0.0;
    }

    size_t catIndex = static_cast<size_t>( clampedValue * m_categoryValues.size() );
    return m_categoryValues[catIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int CategoryMapper::categoryIndexForCategory( double domainValue ) const
{
    int catIndex = -1;

    int intDomainValue = static_cast<int>( nearbyint( domainValue ) );

    size_t i = 0;
    while ( i < m_categoryValues.size() && catIndex == -1 )
    {
        if ( m_categoryValues[i] == intDomainValue )
        {
            catIndex = static_cast<int>( i );
        }

        i++;
    }

    return catIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void CategoryMapper::recomputeMaxTexCoord()
{
    const uint numColors = static_cast<uint>( m_colors.size() );
    if ( numColors == 0 )
    {
        m_maxTexCoord = 1.0;
        return;
    }

    const uint numPixelsPerColor = m_textureSize / numColors;
    if ( numPixelsPerColor == 0 )
    {
        m_maxTexCoord = 1.0;
        return;
    }

    uint texturePixelsInUse = numColors * numPixelsPerColor;
    CVF_ASSERT( texturePixelsInUse <= m_textureSize );

    m_maxTexCoord = static_cast<double>( texturePixelsInUse ) / static_cast<double>( m_textureSize );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool CategoryMapper::updateTexture( TextureImage* image ) const
{
    CVF_ASSERT( image );

    image->allocate( m_textureSize, 1 );

    // For now fill with white so we can see any errors more easily
    image->fill( Color4ub( Color3::WHITE ) );

    const uint numColors = static_cast<uint>( m_colors.size() );
    if ( numColors < 1 )
    {
        return false;
    }

    const uint numPixelsPerColor = m_textureSize / numColors;
    CVF_ASSERT( numPixelsPerColor >= 1 );

    uint ic;
    for ( ic = 0; ic < numColors; ic++ )
    {
        const Color4ub clr( m_colors[ic], 255 );

        uint ip;
        for ( ip = 0; ip < numPixelsPerColor; ip++ )
        {
            image->setPixel( ic * numPixelsPerColor + ip, 0, clr );
        }
    }

    // In cases where we're not using the entire texture we might get into problems with texture coordinate precision on
    // the graphics hardware. Therefore we set one extra pixel with the 'highest' color in the color table
    if ( numColors * numPixelsPerColor < m_textureSize )
    {
        const Color4ub topClr( m_colors[numColors - 1], 255 );
        image->setPixel( numColors * numPixelsPerColor, 0, topClr );
    }

    return true;
}

} // namespace caf
