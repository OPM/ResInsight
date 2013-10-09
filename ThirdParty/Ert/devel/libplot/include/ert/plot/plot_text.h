/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'plot_text.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __PLOT_TEXT_H__
#define __PLOT_TEXT_H__
#ifdef __cplusplus
extern "c" {
#endif

#include <ert/util/type_macros.h>

#include <ert/plot/plot_range.h>

  typedef struct plot_text_struct plot_text_type;


  plot_text_type * plot_text_alloc( double xpos , double ypos , double font_scale , const char * text) ;
  void plot_text_free( plot_text_type * plot_text );
  void plot_text_free__( void * arg );
  
  double plot_text_get_x( const plot_text_type * plot_text );

  double plot_text_get_y( const plot_text_type * plot_text );

  double plot_text_get_font_scale( const plot_text_type * plot_text );

  const char *  plot_text_get_text( const plot_text_type * plot_text );
  void          plot_text_update_range( const plot_text_type * plot_text , plot_range_type * range);

  UTIL_IS_INSTANCE_HEADER( plot_text );

#ifdef __cplusplus
}
#endif
#endif
