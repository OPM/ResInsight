/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'plot_range.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include <ert/util/util.h>
#include <ert/util/arg_pack.h>

#include <plplot/plplot.h>
#include <plplot/plplotP.h>

#include <ert/plot/plot_const.h>
#include <ert/plot/plot_range.h>


/**
   This file implements some simple functionality to manipulate / do
   book-keeping on the ranges of a plot. It is used in essentially two
   ways:
   
    1. As a storage area for recording the min and max of x and
       y. Both for automatic sets by inspecting the dataset, and for
       manual setting.

    2. The dataset have access to the range instance of the plot, so
       they can query for min/max values when plotting. This is
       usefull when for instance plotting a line y = 5.
*/


/** 
    There are four different values: min and max for x and y. All four
    can be individually controlled as manual or auto. Observe that the
    final range_mode should be considered as a mask. The relations
    documented in the enum below must be satisfied.
*/



#define XMIN 0
#define XMAX 1
#define YMIN 2
#define YMAX 3


struct plot_range_struct {
  bool   invert_x_axis;
  bool   invert_y_axis; 

  double current_xmin;
  double current_ymin;
  double current_xmax;
  double current_ymax;

  double manual_xmin;
  double manual_xmax;
  double manual_ymin;
  double manual_ymax;
  
  bool   auto_xmin;
  bool   auto_xmax;
  bool   auto_ymin;
  bool   auto_ymax;
  
  double left_padding, right_padding, top_padding, bottom_padding;
};

/*****************************************************************/

void plot_range_set_invert_x_axis(plot_range_type * range, bool invert) {
  range->invert_x_axis = invert;
}

void plot_range_set_invert_y_axis(plot_range_type * range, bool invert) {
  range->invert_y_axis = invert;
}

bool plot_range_get_invert_x_axis(const plot_range_type * range) {
  return range->invert_x_axis;
}

bool plot_range_get_invert_y_axis(const plot_range_type * range) {
  return range->invert_y_axis;
}

/*****************************************************************/


/**
   Allocates a plot_range instance, and initializes it to the
   'auto_range' mode. If you want to use another mode you must call
   plot_range_set_mode() explicitly.
*/

plot_range_type * plot_range_alloc() {
  plot_range_type * range = util_malloc(sizeof * range );

  range->left_padding = PLOT_RANGE_DEFAULT_PADDING;
  range->right_padding = PLOT_RANGE_DEFAULT_PADDING;
  range->top_padding = PLOT_RANGE_DEFAULT_PADDING;
  range->bottom_padding = PLOT_RANGE_DEFAULT_PADDING;

  range->manual_xmin = PLOT_RANGE_DEFAULT_MANUAL_LIMIT;
  range->manual_xmax = PLOT_RANGE_DEFAULT_MANUAL_LIMIT;
  range->manual_ymin = PLOT_RANGE_DEFAULT_MANUAL_LIMIT;
  range->manual_ymax = PLOT_RANGE_DEFAULT_MANUAL_LIMIT;

  range->current_xmin =  DBL_MAX;
  range->current_xmax = -DBL_MAX;
  range->current_ymin =  DBL_MAX;
  range->current_ymax = -DBL_MAX;

  range->auto_xmin = true;
  range->auto_xmax = true;
  range->auto_ymin = true;
  range->auto_ymax = true;
  
  range->invert_x_axis = false;
  range->invert_y_axis = false;
  return range;
}



void plot_range_free(plot_range_type * plot_range) {
  free(plot_range);
}

/*****************************************************************/

void plot_range_update_x( plot_range_type * range , double x ) {
  range->current_xmin = util_double_min( range->current_xmin , x );
  range->current_xmax = util_double_max( range->current_xmax , x );
}


void plot_range_update_y( plot_range_type * range , double y ) {
  range->current_ymin = util_double_min( range->current_ymin , y );
  range->current_ymax = util_double_max( range->current_ymax , y );
}


void plot_range_update( plot_range_type * range , double x , double y) {
  plot_range_update_x( range , x );
  plot_range_update_y( range , y );
}

void plot_range_update_vector_x( plot_range_type * range , const double_vector_type * x ) {
  int i;
  const double * x_data = double_vector_get_const_ptr( x );
  for (i=0; i < double_vector_size( x ); i++)
    plot_range_update_x( range , x_data[i] );
}


void plot_range_update_vector_y( plot_range_type * range , const double_vector_type * y ) {
  int i;
  const double * y_data = double_vector_get_const_ptr( y );
  for (i=0; i < double_vector_size( y ); i++)
    plot_range_update_y( range , y_data[i] );
}


void plot_range_update_vector( plot_range_type * range , const double_vector_type * x , const double_vector_type * y) {
  plot_range_update_vector_x( range , x );
  plot_range_update_vector_y( range , y );
}


double plot_range_get_current_xmin( const plot_range_type * range ) {
  return range->current_xmin;
}

double plot_range_get_current_xmax( const plot_range_type * range ) {
  return range->current_xmax;
}

double plot_range_get_current_ymin( const plot_range_type * range ) {
  return range->current_ymin;
}

double plot_range_get_current_ymax( const plot_range_type * range ) {
  return range->current_ymax;
}


/*****************************************************************/ 


void plot_range_set_left_padding(plot_range_type * plot_range , double value) {
  plot_range->left_padding = value;
}

void plot_range_set_right_padding(plot_range_type * plot_range , double value) {
  plot_range->right_padding = value;
}

void plot_range_set_top_padding(plot_range_type * plot_range , double value) {
  plot_range->top_padding = value;
}

void plot_range_set_bottom_padding(plot_range_type * plot_range , double value) {
  plot_range->bottom_padding = value;
}

double plot_range_get_left_padding(const plot_range_type * plot_range ) {
  return plot_range->left_padding;
}

double plot_range_get_right_padding(const plot_range_type * plot_range ) {
  return   plot_range->right_padding;
}

double plot_range_get_top_padding(const plot_range_type * plot_range ) {
  return   plot_range->top_padding;
}

double plot_range_get_bottom_padding(const plot_range_type * plot_range ) {
  return   plot_range->bottom_padding;
}


/*****************************************************************/

double plot_range_get_manual_xmin( const plot_range_type * range ) {
  return range->manual_xmin;
}

double plot_range_get_manual_xmax( const plot_range_type * range ) {
  return range->manual_xmax;
}

double plot_range_get_manual_ymin( const plot_range_type * range ) {
  return range->manual_ymin;
}

double plot_range_get_manual_ymax( const plot_range_type * range ) {
  return range->manual_ymax;
}

void plot_range_set_manual_xmin(  plot_range_type * range , double value) {
  range->manual_xmin = value;
  range->auto_xmin = false;
}

void plot_range_set_manual_xmax(  plot_range_type * range , double value) {
  range->manual_xmax = value;
  range->auto_xmax = false;
}

void plot_range_set_manual_ymin(  plot_range_type * range , double value) {
  range->manual_ymin = value;
  range->auto_ymin = false;
}

void plot_range_set_manual_ymax(  plot_range_type * range , double value) {
  range->manual_ymax = value;
  range->auto_ymax = false;
}

void plot_range_unset_manual_xmin(  plot_range_type * range ) {
  range->manual_xmin = PLOT_RANGE_DEFAULT_MANUAL_LIMIT;
  range->auto_xmin = true;
}

void plot_range_unset_manual_xmax(  plot_range_type * range ) {
  range->manual_xmax = PLOT_RANGE_DEFAULT_MANUAL_LIMIT;
  range->auto_xmax = true;
}

void plot_range_unset_manual_ymin(  plot_range_type * range ) {
  range->manual_ymin = PLOT_RANGE_DEFAULT_MANUAL_LIMIT;
  range->auto_ymin = true;
}

void plot_range_unset_manual_ymax(  plot_range_type * range ) {
  range->manual_ymax = PLOT_RANGE_DEFAULT_MANUAL_LIMIT;
  range->auto_ymax = true;
}



/*****************************************************************/

void plot_range_get_limits( const plot_range_type * range , double * x1 , double * x2 , double * y1 , double * y2) {
  double xmin = range->auto_xmin ? range->current_xmin : range->manual_xmin;
  double xmax = range->auto_xmax ? range->current_xmax : range->manual_xmax;
  double ymin = range->auto_ymin ? range->current_ymin : range->manual_ymin;
  double ymax = range->auto_ymax ? range->current_ymax : range->manual_ymax;



  double width  = fabs(xmax - xmin);
  double height = fabs(ymax - ymin);
  
  if (width == 0)
    width = 0.05 * xmax;

  if (height == 0)
    height = 0.05 * ymax;
  
  {
    double left_padding , right_padding;
    if (range->invert_x_axis) {
      left_padding = range->auto_xmax ? range->left_padding : 0;
      right_padding = range->auto_xmin ? range->right_padding : 0;  
      
      *x1 = xmax + width  * left_padding;
      *x2 = xmin - width  * right_padding;
    } else {
      left_padding = range->auto_xmin ? range->left_padding : 0;
      right_padding = range->auto_xmax ? range->right_padding : 0;  
      
      *x1 = xmin - width  * left_padding;
      *x2 = xmax + width  * right_padding;
    }
  }

  {
    double top_padding , bottom_padding;
    if (range->invert_y_axis) {
      bottom_padding = range->auto_ymax ? range->bottom_padding : 0;
      top_padding = range->auto_ymin ? range->top_padding : 0;
      
      *y1 = ymax + height * bottom_padding;
      *y2 = ymin - height * top_padding;
    } else {
      bottom_padding = range->auto_ymin ? range->bottom_padding : 0;
      top_padding = range->auto_ymax ? range->top_padding : 0;
      
      *y1 = ymin - height * bottom_padding;
      *y2 = ymax + height * top_padding;
    }
  }
 }
