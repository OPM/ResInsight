/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'plot_text.c' is part of ERT - Ensemble based Reservoir Tool. 
    
   ERT is free software: you can redistribute it and/or modify 
   it under the terms of the GNU General Public License as published by 
   the Free Software Foundation, either version 3 of the License, or 
   (at your option) any later version. 
    
   ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
   WARRANTY; without even the implied warranty of MERCHANTABILITY or 
   FITNESS FOR A PARTICULAR PURPOSE.   
    
   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
   for more details. 
*/


#include <stdlib.h>

#include <ert/util/util.h>
#include <ert/util/type_macros.h>

#include <ert/plot/plot_text.h>

#define PLOT_TEXT_TYPE_ID 660991
struct plot_text_struct {
  UTIL_TYPE_ID_DECLARATION;
  double xpos;
  double ypos;
  double font_scale;
  char * text;
};


static UTIL_SAFE_CAST_FUNCTION(plot_text , PLOT_TEXT_TYPE_ID )
UTIL_IS_INSTANCE_FUNCTION( plot_text , PLOT_TEXT_TYPE_ID)

plot_text_type * plot_text_alloc( double xpos , double ypos , double font_scale , const char * text) {
  if (text) {
    plot_text_type * plot_text = util_malloc( sizeof * plot_text );

    UTIL_TYPE_ID_INIT( plot_text , PLOT_TEXT_TYPE_ID );
    plot_text->xpos = xpos;
    plot_text->ypos = ypos;
    plot_text->font_scale = font_scale;
    plot_text->text = util_alloc_string_copy( text );
    
    return plot_text;
  } else
    return NULL;
}



void plot_text_free( plot_text_type * plot_text ) {
  free( plot_text->text );
  free( plot_text );
}



void plot_text_free__( void * arg) {
  plot_text_type * plot_text = plot_text_safe_cast( arg );
  plot_text_free( plot_text );
}




double plot_text_get_x( const plot_text_type * plot_text ) {
  return plot_text->xpos;
}

double plot_text_get_y( const plot_text_type * plot_text ) {
  return plot_text->ypos;
}

double plot_text_get_font_scale( const plot_text_type * plot_text ) {
  return plot_text->font_scale;
}

const char *  plot_text_get_text( const plot_text_type * plot_text ) {
  return plot_text->text;
}


void plot_text_update_range( const plot_text_type * plot_text , plot_range_type * range) {
  plot_range_update( range , plot_text_get_x( plot_text ), plot_text_get_y( plot_text ));
}
