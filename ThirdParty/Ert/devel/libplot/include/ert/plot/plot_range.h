/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'plot_range.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __PLOT_RANGE_H__
#define __PLOT_RANGE_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdbool.h>

#include <ert/util/double_vector.h>

#define PLOT_RANGE_DEFAULT_PADDING       0.025
#define PLOT_RANGE_DEFAULT_MANUAL_LIMIT  -9999

typedef struct plot_range_struct plot_range_type;



plot_range_type     * plot_range_alloc();
void                  plot_range_free(plot_range_type *);

double plot_range_get_final_xmin(const plot_range_type * plot_range);
double plot_range_get_final_xmax(const plot_range_type * plot_range);
double plot_range_get_final_ymin(const plot_range_type * plot_range);
double plot_range_get_final_ymax(const plot_range_type * plot_range);

double plot_range_get_xmax(const plot_range_type * );
double plot_range_get_ymax(const plot_range_type * );
double plot_range_get_xmin(const plot_range_type * );
double plot_range_get_ymin(const plot_range_type * );

double plot_range_safe_get_xmax(const plot_range_type * );
double plot_range_safe_get_ymax(const plot_range_type * );
double plot_range_safe_get_xmin(const plot_range_type * );
double plot_range_safe_get_ymin(const plot_range_type * );

void plot_range_set_auto_xmax(plot_range_type *  , double);
void plot_range_set_auto_ymax(plot_range_type *  , double);
void plot_range_set_auto_xmin(plot_range_type *  , double);
void plot_range_set_auto_ymin(plot_range_type *  , double);

  void plot_range_set_top_padding(plot_range_type    *  , double );
  void plot_range_set_bottom_padding(plot_range_type *  , double );
  void plot_range_set_left_padding(plot_range_type   *  , double );
  void plot_range_set_right_padding(plot_range_type  *  , double );

  double plot_range_get_left_padding(const plot_range_type * plot_range );
  double plot_range_get_right_padding(const plot_range_type * plot_range );
  double plot_range_get_top_padding(const plot_range_type * plot_range );
  double plot_range_get_bottom_padding(const plot_range_type * plot_range );



  void plot_range_set_invert_y_axis(plot_range_type * range , bool invert);
  void plot_range_set_invert_x_axis(plot_range_type * range , bool invert);
  bool plot_range_get_invert_y_axis(const plot_range_type * range);
  bool plot_range_get_invert_x_axis(const plot_range_type * range);
  
  void   plot_range_update_vector_x( plot_range_type * range , const double_vector_type * x );
  void   plot_range_update_vector_y( plot_range_type * range , const double_vector_type * y );
  void   plot_range_update_vector( plot_range_type * range , const double_vector_type * x , const double_vector_type * y);

  void   plot_range_update_y( plot_range_type * range , double y);
  void   plot_range_update_x( plot_range_type * range , double x );
  void   plot_range_update( plot_range_type * range , double x , double y);

  void plot_range_get_limits( const plot_range_type * range , double * x1 , double * x2 , double * y1 , double * y2 );

  double plot_range_get_current_xmin( const plot_range_type * range );
  double plot_range_get_current_xmax( const plot_range_type * range );
  double plot_range_get_current_ymin( const plot_range_type * range );
  double plot_range_get_current_ymax( const plot_range_type * range );



  double plot_range_get_manual_xmin( const plot_range_type * range );
  double plot_range_get_manual_xmax( const plot_range_type * range );
  double plot_range_get_manual_ymin( const plot_range_type * range );
  double plot_range_get_manual_ymax( const plot_range_type * range );

  void plot_range_set_manual_xmin(  plot_range_type * range , double value);
  void plot_range_set_manual_xmax(  plot_range_type * range , double value);
  void plot_range_set_manual_ymin(  plot_range_type * range , double value);
  void plot_range_set_manual_ymax(  plot_range_type * range , double value);

  void plot_range_unset_manual_xmin(  plot_range_type * range );
  void plot_range_unset_manual_xmax(  plot_range_type * range );
  void plot_range_unset_manual_ymin(  plot_range_type * range );
  void plot_range_unset_manual_ymax(  plot_range_type * range );


#ifdef __cplusplus
}
#endif
#endif

