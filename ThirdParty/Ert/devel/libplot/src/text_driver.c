/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'text_driver.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <string.h>
#include <math.h>

#include <ert/util/util.h>
#include <ert/util/arg_pack.h>

#include <ert/plot/plot_driver.h>
#include <ert/plot/plot_const.h>



typedef struct {
  char       * plot_path;
} text_state_type;





static text_state_type * text_state_alloc( void * init_arg ) {
  text_state_type * state = util_malloc( sizeof * state );
  {
    arg_pack_type * arg_pack = arg_pack_safe_cast( init_arg );
    state->plot_path         = util_alloc_string_copy( arg_pack_iget_ptr( arg_pack , 0) );
    util_make_path( state->plot_path );
  }
  
  return state;
}


static void text_state_close( text_state_type * state ) {
  free( state->plot_path );
  free( state );
}



/*****************************************************************/





static void text_close_driver( plot_driver_type * driver ) {
  text_state_close( driver->state );
}


char * text_alloc_filename( const char * plot_path , const char * label) {
  return util_alloc_filename(plot_path , label , NULL);
}


FILE * text_fopen(const plot_driver_type * driver , const char * label) {
  text_state_type * state = driver->state;
  char * filename = text_alloc_filename( state->plot_path , label );
  FILE * stream   = util_fopen( filename , "w");
  free( filename );
  return stream;
}


static void text_fprintf1(plot_driver_type * driver , const char * label , const double_vector_type * d) {
  FILE * stream = text_fopen( driver , label );

  for (int i=0; i < double_vector_size( d ); i++) 
    fprintf(stream , "%12.7f  \n",double_vector_iget(d , i));
  
  fclose( stream );
}



static void text_fprintf2(plot_driver_type * driver , const char * label , const double_vector_type * d1 , const double_vector_type * d2) {
  FILE * stream = text_fopen( driver , label );

  for (int i=0; i < double_vector_size( d1 ); i++) 
    fprintf(stream , "%12.7f  %12.7f  \n",double_vector_iget(d1 , i) , double_vector_iget(d2 , i));
  
  fclose( stream );
}


static void text_fprintf3(plot_driver_type * driver , const char * label , const double_vector_type * d1 , const double_vector_type * d2, const double_vector_type * d3) {
  FILE * stream = text_fopen( driver , label );

  for (int i=0; i < double_vector_size( d1 ); i++) 
    fprintf(stream , "%12.7f  %12.7f  %12.7f  \n",double_vector_iget(d1 , i) , double_vector_iget(d2 , i), double_vector_iget(d3 , i));
  
  fclose( stream );
}





void text_plot_xy1y2(plot_driver_type * driver     , 
                     const char * label , 
                     double_vector_type * x  , 
                     double_vector_type * y1  , 
                     double_vector_type * y2  , 
                     line_attribute_type line_attr) {
  
  text_fprintf3( driver , label , x , y1 , y2);
  
}





void text_plot_x1x2y(plot_driver_type * driver      , 
                     const char * label             , 
                     double_vector_type * x1  , 
                     double_vector_type * x2  , 
                     double_vector_type * y   , 
                     line_attribute_type line_attr) {

  text_fprintf3( driver , label , x1 , x2 , y);

}






void text_plot_xy(plot_driver_type * driver     , 
                  const char * label , 
                  double_vector_type * x  , 
                  double_vector_type * y  , 
                  plot_style_type style         , 
                  line_attribute_type line_attr , 
                  point_attribute_type point_attr) {

  text_fprintf2( driver , label , x ,  y);
  
}




void text_plot_hist( plot_driver_type * driver, const char * label , double_vector_type * x , line_attribute_type line_attr) {

  text_fprintf1( driver , label , x );

}


/**
   init_arg should be an arg_pack instance with one string; the string
   should be the name of a directory, wherein all the plot files will
   be put. If the directory does not exist, it will be created.

   Example from calling scope:
   ---------------------------
   {
      arg_pack_type * arg_pack = arg_pack_alloc();
      arg_pack_append_ptr( arg_pack , plot_path );

      plot = plot_alloc( "TEXT" , arg_pack );
      arg_pack_free( arg_pack );
   }
*/
   

plot_driver_type * text_driver_alloc(void * init_arg) {
  plot_driver_type * driver = plot_driver_alloc_empty("TEXT");
  driver->state             = text_state_alloc( init_arg );
  
  driver->close_driver      = text_close_driver;
  driver->plot_xy           = text_plot_xy;
  driver->plot_xy1y2        = text_plot_xy1y2;
  driver->plot_x1x2y        = text_plot_x1x2y;
  driver->plot_hist         = text_plot_hist;
  return driver;
}
