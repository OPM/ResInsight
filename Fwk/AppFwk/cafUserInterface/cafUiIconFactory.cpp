//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2022- Ceetron Solutions AS
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

#include "cafUiIconFactory.h"

#include "QIcon"
#include "QImage"
#include "QPainter"
#include "QPixmap"
#include "QtSvg/QSvgRenderer"

namespace caf
{
/* GIMP RGBA C-Source image dump (StepDown.c) */

static const struct
{
    unsigned int  width;
    unsigned int  height;
    unsigned int  bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
    unsigned char pixel_data[16 * 16 * 4 + 1];
} stepDownImageData = {
    16,
    16,
    4,
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000AA"
    "A\001\030\030\030\001"
    "\037\037\037\001\020\020\020\001\004\004\004\001\016\016\016\001!!!\001\"\"\"\001(((\001\060\060\060\001$$"
    "$\001\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000UUU\014FFF\242\030\030\030\256\037\037\037"
    "\256"
    "\022\022\022\256\005\005\005\256\021\021\021\256'''\256...\256\061\061\061\256\067\067\067"
    "\256&&&\256AAAzTTT\010\000\000\000\000\000\000\000\000xxx\014```\273\033\033\033\377&&&\377\""
    "\"\"\377\017\017\017\377\"\"\"\377LLL\377___\377^^^\377^^^\377AAA\376OOOXTT"
    "T\001\000\000\000\000\000\000\000\000\000\000\000\000JJJ\071+++\343&&&\377%%%\377\017\017\017\377'''\377"
    "WWW\377]]]\377hhh\377WWW\376NNN\300\177\177\177\032\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000KKK\004\066\066\066z\040\040\040\370\"\"\"\377\014\014\014\377$$$\377SSS\377"
    "ccc\377bbb\377NNN\362\202\202\202=\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\064\064\064\040===\312\032\032\032\375\017\017\017\377$$$\377WWW\377bbb"
    "\377MMM\374LLL\200iii\006\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000W"
    "WW\001AAA\063###\330\007\007\007\377(((\377VVV\377UUU\377WWW\314\217\217\217\040\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000;;;\001\066\066"
    "\066}\027\027\027\371(((\377TTT\377FFF\360\\\\\\C\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000TTT\015\025\025\025\036\040\040\040!<<<<???\360\"\"\""
    "\377===\377ddd\266GGG\062\026\026\026\061\040\040\040\066\"\"\"\022\000\000\000\000\000\000\000\000"
    "\000\000\000\000HHH\015\071\071\071\256\007\007\007\314\015\015\015\316\024\024\024\326\034\034\034"
    "\374\022\022\022\377!!!\377###\335###\326\035\035\035\336\032\032\032\343///\220A"
    "AA\010\000\000\000\000\000\000\000\000bbb\014QQQ\264%%%\355$$$\363\035\035\035\352\034\034\034\351"
    "&&&\353$$$\344)))\346\061\061\061\345\066\066\066\350\062\062\062\335\064\064\064\201"
    "???\007\000\000\000\000\000\000\000\000\000\000\000\000SSS\023@@@?\070\070\070E---=,,,<///>\"\"\"\067&&"
    "&\070$$$\070---:CCC\060;;;"
    "\015\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000",
};

/* GIMP RGBA C-Source image dump (StepUp.c) */

static const struct
{
    unsigned int  width;
    unsigned int  height;
    unsigned int  bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
    unsigned char pixel_data[16 * 16 * 4 + 1];
} stepUpImageData = {
    16,
    16,
    4,
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000;;"
    ";\015CCC\060---:"
    "$$$\070&&&\070\"\"\"\067///>,,,<---=\070\070\070E@@@?SSS\023\000\000\000\000\000\000\000\000\000\000"
    "\000\000???\007\064\064\064\201\062\062\062\335\066\066\066\350\061\061\061\345)))\346$$$\344"
    "&&&\353\034\034\034\351\035\035\035\352$$$\363%%%\355QQQ\264bbb\014\000\000\000\000\000\000"
    "\000\000AAA\010///\220\032\032\032\343\035\035\035\336###\326###\335!!!\377\022\022\022"
    "\377\034\034\034\374\024\024\024\326\015\015\015\316\007\007\007\314\071\071\071\256HHH\015"
    "\000\000\000\000\000\000\000\000\000\000\000\000\"\"\"\022\040\040\040\066\026\026\026\061GGG\062ddd\266=="
    "=\377\"\"\"\377???\360<<<<\040\040\040!\025\025\025\036TTT\015\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\\\\\\CFFF\360TTT\377(((\377\027\027"
    "\027\371\066\066\066};;;"
    "\001\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\217\217\217\040WWW\314UUU\377VVV\377(((\377\007\007\007\377###\330"
    "AAA\063WWW\001\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000iii"
    "\006LLL\200M"
    "MM\374bbb\377WWW\377$$$\377\017\017\017\377\032\032\032\375===\312\064\064\064\040"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\202\202\202="
    "NNN\362bbb\377"
    "ccc\377SSS\377$$$\377\014\014\014\377\"\"\"\377\040\040\040\370\066\066\066zKKK\004"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\177\177\177\032NNN\300WWW\376hhh\377]]]\377"
    "WWW\377'''\377\017\017\017\377%%%\377&&&\377+++\343JJJ\071\000\000\000\000\000\000\000\000\000"
    "\000\000\000TTT\001OOOXAAA\376^^^\377^^^\377___\377LLL\377\"\"\"\377\017\017\017\377"
    "\"\"\"\377&&&\377\033\033\033\377```\273xxx\014\000\000\000\000\000\000\000\000TTT\010AAAz&&&"
    "\256\067\067\067\256\061\061\061\256...\256'''\256\021\021\021\256\005\005\005\256\022\022"
    "\022\256\037\037\037\256\030\030\030\256FFF\242UUU\014\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000$$$\001\060\060\060\001(((\001\"\"\"\001!!!\001\016\016\016\001\004\004\004\001\020\020\020\001\037"
    "\037\037\001\030\030\030\001AAA\001\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000"
    "\000\000\000\000",
};

// clang-format off

static const char* linked_svg_data = R"(

<svg version="1.1" id="Layer_1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" x="0px" y="0px"
	 viewBox="0 0 48 48" style="enable-background:new 0 0 48 48;" xml:space="preserve">
<path d="M14,22.5V14c0-2.8,1-5.1,2.9-7.1C18.9,5,21.2,4,24,4s5.1,1,7.1,2.9S34,11.2,34,14v8.5h-3V14c0-1.9-0.7-3.6-2-4.9
	S25.9,7,24,7s-3.6,0.7-5,2.1c-1.4,1.4-2,3-2,4.9v8.5H14z M22.5,16.2h3v15.5h-3V16.2z M14,25.5h3V34c0,1.9,0.7,3.6,2,5
	c1.4,1.4,3,2,5,2s3.6-0.7,5-2c1.4-1.4,2-3,2-5v-8.5h3V34c0,2.8-1,5.1-2.9,7.1C29.1,43,26.8,44,24,44s-5.1-1-7.1-2.9
	C15,39.1,14,36.8,14,34V25.5z"/>
</svg>

)";

static const char* linked_white_svg_data = R"(

<svg version="1.1" id="Layer_1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" x="0px" y="0px"
	 viewBox="0 0 48 48" style="enable-background:new 0 0 48 48;" xml:space="preserve">
<style type="text/css">
	.st0{fill:#FFFFFF;}
</style>
<path class="st0" d="M14,22.5V14c0-2.8,1-5.1,2.9-7.1C18.9,5,21.2,4,24,4s5.1,1,7.1,2.9S34,11.2,34,14v8.5h-3V14
	c0-1.9-0.7-3.6-2-4.9S25.9,7,24,7s-3.6,0.7-5,2.1c-1.4,1.4-2,3-2,4.9v8.5H14z M22.5,16.2h3v15.5h-3V16.2z M14,25.5h3V34
	c0,1.9,0.7,3.6,2,5c1.4,1.4,3,2,5,2s3.6-0.7,5-2c1.4-1.4,2-3,2-5v-8.5h3V34c0,2.8-1,5.1-2.9,7.1C29.1,43,26.8,44,24,44
	s-5.1-1-7.1-2.9C15,39.1,14,36.8,14,34V25.5z"/>
</svg>

)";

static const char* unlinked_svg_data= R"(

<svg version="1.1" id="Layer_1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" x="0px" y="0px"
	 viewBox="0 0 48 48" style="enable-background:new 0 0 48 48;" xml:space="preserve">
<rect x="-1.9" y="20.8" transform="matrix(0.7071 -0.7071 0.7071 0.7071 -8.5358 23.9451)\" width="53.1" height="3"/>
<g>
	<polygon points="22.5,16.2 22.5,26.4 25.5,23.4 25.5,16.2 	"/>
	<polygon points="25.5,27.7 22.5,30.7 22.5,31.8 25.5,31.8 	"/>
	<path d="M17,14c0-1.9,0.7-3.6,2-5c1.4-1.4,3-2,5-2s3.6,0.7,5,2c1.4,1.4,2,3,2,5v3.9l3-3V14c0-2.8-1-5.1-2.9-7.1S26.8,4,24,4
		s-5.1,1-7.1,2.9S14,11.2,14,14v8.5h3V14z"/>
	<path d="M31,34c0,1.9-0.7,3.6-2,5c-1.4,1.4-3,2-5,2s-3.6-0.7-5-2c-0.9-0.9-1.5-1.9-1.8-3l-2.3,2.3c0.5,1,1.1,1.9,2,2.8
		c2,2,4.3,2.9,7.1,2.9s5.1-1,7.1-2.9S34,36.8,34,34v-8.5h-3V34z"/>
</g>
</svg>

)";


static const char* unlinked_white_svg_data = R"(

<svg version="1.1" id="Layer_1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" x="0px" y="0px"
	 viewBox="0 0 48 48" style="enable-background:new 0 0 48 48;" xml:space="preserve">
<style type="text/css">
	.st0{fill:#FFFFFF;}
</style>
<rect x="-1.9" y="20.8" transform="matrix(0.7071 -0.7071 0.7071 0.7071 -8.5333 23.9657)\" class="st0" width="53.1" height="3"/>
<g>
	<polygon class="st0" points="22.5,16.2 22.5,26.4 25.5,23.4 25.5,16.2 	"/>
	<polygon class="st0" points="25.5,27.7 22.5,30.7 22.5,31.8 25.5,31.8 	"/>
	<path class="st0" d="M17,14c0-1.9,0.7-3.6,2-5c1.4-1.4,3-2,5-2s3.6,0.7,5,2c1.4,1.4,2,3,2,5v3.9l3-3V14c0-2.8-1-5.1-2.9-7.1
		S26.8,4,24,4s-5.1,1-7.1,2.9S14,11.2,14,14v8.5h3V14z"/>
	<path class="st0" d="M31,34c0,1.9-0.7,3.6-2,5c-1.4,1.4-3,2-5,2s-3.6-0.7-5-2c-0.9-0.9-1.5-1.9-1.8-3l-2.3,2.3c0.5,1,1.1,1.9,2,2.8
		c2,2,4.3,2.9,7.1,2.9s5.1-1,7.1-2.9S34,36.8,34,34v-8.5h-3V34z"/>
</g>
</svg>

)";

// clang-format on

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QIcon UiIconFactory::stepUpIcon()
{
    static QIcon expandDownIcon(
        UiIconFactory::createIcon( stepUpImageData.pixel_data, stepUpImageData.width, stepUpImageData.height ) );
    return expandDownIcon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QIcon UiIconFactory::stepDownIcon()
{
    static QIcon expandDownIcon(
        UiIconFactory::createIcon( stepDownImageData.pixel_data, stepDownImageData.width, stepDownImageData.height ) );
    return expandDownIcon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QIcon UiIconFactory::createTwoStateChainIcon()
{
    static QIcon icon( UiIconFactory::createTwoStateIcon( linked_svg_data,
                                                          unlinked_svg_data,
                                                          UiIconFactory::iconWidth(),
                                                          UiIconFactory::iconHeight() ) );

    return icon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QIcon UiIconFactory::createTwoStateWhiteChainIcon()
{
    static QIcon icon( UiIconFactory::createTwoStateIcon( linked_white_svg_data,
                                                          unlinked_white_svg_data,
                                                          UiIconFactory::iconWidth(),
                                                          UiIconFactory::iconHeight() ) );

    return icon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int UiIconFactory::iconWidth()
{
    return 32;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int UiIconFactory::iconHeight()
{
    return 32;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QIcon UiIconFactory::createTwoStateIcon( const char*  onStateSvgData,
                                               const char*  offStateSvgData,
                                               unsigned int width,
                                               unsigned int height )
{
    auto onStatePixmap  = UiIconFactory::createPixmap( onStateSvgData, width, height );
    auto offStatePixmap = UiIconFactory::createPixmap( offStateSvgData, width, height );

    QIcon icon;
    icon.addPixmap( onStatePixmap, QIcon::Normal, QIcon::On );
    icon.addPixmap( offStatePixmap, QIcon::Normal, QIcon::Off );

    return icon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QIcon UiIconFactory::createIcon( const unsigned char* data, unsigned int width, unsigned int height )
{
    QImage  img( data, width, height, QImage::Format_ARGB32 );
    QPixmap pxMap;
    pxMap = QPixmap::fromImage( img );

    return QIcon( pxMap );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QIcon UiIconFactory::createSvgIcon( const char* data, unsigned int width, unsigned int height )
{
    QPixmap pxMap = createPixmap( data, width, height );

    return QIcon( pxMap );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QPixmap UiIconFactory::createPixmap( const char* svgData, unsigned int width, unsigned int height )
{
    auto svg = QSvgRenderer( QByteArray( svgData ) );

    auto qim = QImage( width, height, QImage::Format_ARGB32 );

    qim.fill( 0 );

    auto painter = QPainter();
    painter.begin( &qim );
    svg.render( &painter );
    painter.end();

    QPixmap pxMap;
    pxMap = QPixmap::fromImage( qim );

    return pxMap;
}

} // namespace caf
