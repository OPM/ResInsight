/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-    Equinor ASA
//  Copyright (C) 2017     Statoil ASA
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

#include "RiaColorTools.h"
#include "RiuGuiTheme.h"

#include "cvfAssert.h"

#include <algorithm>
#include <cmath>

#include <QPalette>

//--------------------------------------------------------------------------------------------------
/// Uses W3.org relative luminance calculation taking into account the different luminance of the different colors
/// https://www.w3.org/TR/WCAG20-TECHS/G18.html
/// Luminance is between [0, 1] so anything above 0.5 is considered in the bright half of the spectrum.
/// However, subjectively the contrast looks better if the threshold is to 0.4 so black contrast is used a bit more
/// often.
//--------------------------------------------------------------------------------------------------
bool RiaColorTools::isBrightnessAboveThreshold( cvf::Color3f backgroundColor )
{
    return relativeLuminance( backgroundColor ) > 0.4;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RiaColorTools::computeOffsetColor( cvf::Color3f color, float offsetFactor )
{
    float gridR = 0.0;
    float gridG = 0.0;
    float gridB = 0.0;

    if ( isBrightnessAboveThreshold( color ) )
    {
        gridR = color.r() - ( color.r() * offsetFactor );
        gridG = color.g() - ( color.g() * offsetFactor );
        gridB = color.b() - ( color.b() * offsetFactor );
    }
    else
    {
        gridR = color.r() + ( 1.0f - color.r() ) * offsetFactor;
        gridG = color.g() + ( 1.0f - color.g() ) * offsetFactor;
        gridB = color.b() + ( 1.0f - color.b() ) * offsetFactor;
    }

    return cvf::Color3f( std::clamp( gridR, 0.0f, 1.0f ), std::clamp( gridG, 0.0f, 1.0f ), std::clamp( gridB, 0.0f, 1.0f ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RiaColorTools::darkContrastColor()
{
    return cvf::Color3f::fromByteColor( 10, 10, 10 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RiaColorTools::brightContrastColor()
{
    return cvf::Color3f::WHITE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RiaColorTools::darkContrastColorSofter()
{
    return cvf::Color3f::fromByteColor( 30, 30, 30 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RiaColorTools::brightContrastColorSofter()
{
    return cvf::Color3f::fromByteColor( 200, 200, 200 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RiaColorTools::contrastColor( cvf::Color3f backgroundColor, bool softerContrast )
{
    if ( isBrightnessAboveThreshold( backgroundColor ) )
    {
        if ( softerContrast ) return darkContrastColorSofter();
        return darkContrastColor();
    }
    if ( softerContrast ) return brightContrastColorSofter();
    return brightContrastColor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QColor RiaColorTools::toQColor( cvf::Color3f color, float alpha )
{
    QColor qcolor( color.rByte(), color.gByte(), color.bByte() );
    qcolor.setAlphaF( alpha );
    return qcolor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QColor RiaColorTools::toQColor( cvf::Color4f color )
{
    return toQColor( color.toColor3f(), color.a() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RiaColorTools::fromQColorTo3f( QColor color )
{
    return cvf::Color3f( color.redF(), color.greenF(), color.blueF() );
}

//--------------------------------------------------------------------------------------------------
/// Find the color with larges distance to the given color based on RGB distance
//--------------------------------------------------------------------------------------------------
cvf::Color3f RiaColorTools::selectContrastColorFromCandiates( cvf::Color3f color, const cvf::Color3fArray& candidates )
{
    if ( candidates.size() == 0 ) return color;

    float        maxDiff       = 0.0f;
    cvf::Color3f selectedColor = color;

    for ( const auto& candidate : candidates )
    {
        const auto diff = std::fabs( color.r() - candidate.r() ) + std::fabs( color.g() - candidate.g() ) +
                          std::fabs( color.b() - candidate.b() );
        if ( diff > maxDiff )
        {
            maxDiff       = diff;
            selectedColor = candidate;
        }
    }

    return selectedColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QColor RiaColorTools::textColor()
{
    return RiuGuiTheme::getColorByVariableName( "textColor" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RiaColorTools::textColor3f()
{
    return fromQColorTo3f( textColor() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RiaColorTools::blendCvfColors( const cvf::Color3f& color1, const cvf::Color3f& color2, int weight1 /*= 1*/, int weight2 /*= 1*/ )
{
    CVF_ASSERT( weight1 > 0 && weight2 > 0 );
    int weightsum = weight1 + weight2;
    return cvf::Color3f( ( color1.r() * weight1 + color2.r() * weight2 ) / weightsum,
                         ( color1.g() * weight1 + color2.g() * weight2 ) / weightsum,
                         ( color1.b() * weight1 + color2.b() * weight2 ) / weightsum );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QColor RiaColorTools::blendQColors( const QColor& color1, const QColor& color2, int weight1 /*= 1*/, int weight2 /*= 1*/ )
{
    CVF_ASSERT( weight1 > 0 && weight2 > 0 );
    int weightsum = weight1 + weight2;
    return QColor( ( color1.red() * weight1 + color2.red() * weight2 ) / weightsum,
                   ( color1.green() * weight1 + color2.green() * weight2 ) / weightsum,
                   ( color1.blue() * weight1 + color2.blue() * weight2 ) / weightsum );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RiaColorTools::makeLighter( const cvf::Color3f& color, float normalizedScalingFactor )
{
    auto qColor = toQColor( color );

    float h = 0.0;
    float s = 0.0;
    float l = 0.0;
    qColor.getHslF( &h, &s, &l );

    // A negative value will make the color darker
    l = l + ( 1.0 - l ) * normalizedScalingFactor;

    l = std::clamp( l, 0.0f, 1.0f );

    qColor.setHslF( h, s, l );

    return fromQColorTo3f( qColor );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QColor RiaColorTools::modifySaturation( const QColor& color, double factor )
{
    auto  colorSaturation( color );
    float h, s, v;
    color.getHsvF( &h, &s, &v );

    s = std::clamp( (float)( s * factor ), 0.0f, 1.0f );

    colorSaturation.setHsvF( h, s, v );
    return colorSaturation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float RiaColorTools::relativeLuminance( cvf::Color3f backgroundColor )
{
    float R = calculateNonLinearColorValue( backgroundColor.r() );
    float G = calculateNonLinearColorValue( backgroundColor.g() );
    float B = calculateNonLinearColorValue( backgroundColor.b() );

    double luminance = 0.2126 * R + 0.7152 * G + 0.0722 * B;
    return luminance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float RiaColorTools::calculateNonLinearColorValue( float colorFraction )
{
    return colorFraction <= 0.03928 ? colorFraction / 12.92 : std::pow( ( colorFraction + 0.055 ) / 1.055, 2.4 );
}
