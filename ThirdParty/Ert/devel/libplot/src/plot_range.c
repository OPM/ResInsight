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
  double padding[4];     
  double limits[4];
  double final[4];  
  bool   final_set[4];
  bool   set[4];
  bool   invert_x_axis;
  bool   invert_y_axis; 
  bool   auto_range[4];
};

/*****************************************************************/

void plot_range_fprintf(const plot_range_type * range, FILE * stream) {
  printf("x1: %g    x2:%g   y1:%g   y2:%g \n",range->limits[XMIN] , range->limits[XMAX] , range->limits[YMIN] , range->limits[YMAX]);
}


/*****************************************************************/

static void plot_range_set__(plot_range_type * plot_range , int index , double value) {
  plot_range->limits[index]     = value;
  plot_range->set[index]        = true;
  plot_range->auto_range[index] = false;
  plot_range->padding[index]    = 0;   /* If you are explicitly setting a limit - you get no padding. */
}


void plot_range_set_ymax(plot_range_type * plot_range , double ymax) {
  plot_range_set__(plot_range , YMAX , ymax);
}

void plot_range_set_ymin(plot_range_type * plot_range , double ymin) {
  plot_range_set__(plot_range , YMIN , ymin);
}

void plot_range_set_xmax(plot_range_type * plot_range , double xmax) {
  plot_range_set__(plot_range , XMAX , xmax);
}

void plot_range_set_xmin(plot_range_type * plot_range , double xmin) {
  plot_range_set__(plot_range , XMIN , xmin);
}

/*****************************************************************/

/**
   The set_auto functions can always be called, but if the range in
   question has been set manually, the function just returns without
   doing anything.
*/

static void plot_range_set_auto__(plot_range_type * plot_range , int index , double value) {
  if (plot_range->auto_range[index]) {
    plot_range->limits[index] = value;
    plot_range->set[index]    = true;
  }
}

void plot_range_set_auto_ymax(plot_range_type * plot_range , double ymax) {
  plot_range_set_auto__(plot_range , YMAX , ymax);
}

void plot_range_set_auto_ymin(plot_range_type * plot_range , double ymin) {
  plot_range_set_auto__(plot_range , YMIN , ymin);
}

void plot_range_set_auto_xmax(plot_range_type * plot_range , double xmax) {
  plot_range_set_auto__(plot_range , XMAX , xmax);
}

void plot_range_set_auto_xmin(plot_range_type * plot_range , double xmin) {
  plot_range_set_auto__(plot_range , XMIN , xmin);
}

/*****************************************************************/
/* 
   These functions will fail if the corresponding value has not
   been set, either from an automatic set, or manually.
*/

static double plot_range_get_final__(const plot_range_type * plot_range , int index) {
  if (plot_range->final_set[index])
    return plot_range->final[index];
  else {
    util_abort("%s: tried to get xmin - but that has not been set.\n",__func__);
    return 0;
  }
}

double plot_range_get_final_xmin(const plot_range_type * plot_range) {
  return plot_range_get_final__(plot_range , XMIN);
}

double plot_range_get_final_xmax(const plot_range_type * plot_range) {
  return plot_range_get_final__(plot_range , XMAX);
}

double plot_range_get_final_ymin(const plot_range_type * plot_range) {
  return plot_range_get_final__(plot_range , YMIN);
}

double plot_range_get_final_ymax(const plot_range_type * plot_range) {
  return plot_range_get_final__(plot_range , YMAX);
}



/*****************************************************************/
/* 
   These functions will fail if the corresponding value has not
   been set, either from an automatic set, or manually.
*/

static double plot_range_get__(const plot_range_type * plot_range , int index) {
  if (plot_range->set[index])
    return plot_range->limits[index];
  else {
    util_abort("%s: tried to get xmin - but that has not been set.\n",__func__);
    return 0;
  }
}

double plot_range_get_xmin(const plot_range_type * plot_range) {
  return plot_range_get__(plot_range , XMIN);
}

double plot_range_get_xmax(const plot_range_type * plot_range) {
  return plot_range_get__(plot_range , XMAX);
}

double plot_range_get_ymin(const plot_range_type * plot_range) {
  return plot_range_get__(plot_range , YMIN);
}

double plot_range_get_ymax(const plot_range_type * plot_range) {
  return plot_range_get__(plot_range , YMAX);
}

/*****************************************************************/
/*
  The _safe_ functions will return the value of range->limits[]
  irrespective of whether it has been set with a sensible value or
  not.
*/

static double plot_range_safe_get__(const plot_range_type * plot_range , int index) {
  return plot_range->limits[index];
}


double plot_range_safe_get_xmin(const plot_range_type * plot_range) {
  return plot_range_safe_get__(plot_range , XMIN);
}

double plot_range_safe_get_xmax(const plot_range_type * plot_range) {
  return plot_range_safe_get__(plot_range , XMAX);
}

double plot_range_safe_get_ymin(const plot_range_type * plot_range) {
  return plot_range_safe_get__(plot_range , YMIN);
}

double plot_range_safe_get_ymax(const plot_range_type * plot_range) {
  return plot_range_safe_get__(plot_range , YMAX);
}

/*****************************************************************/

static void plot_range_set_padding__(plot_range_type * plot_range , int index , double value) {
  plot_range->padding[index] = value;
}


void plot_range_set_left_padding(plot_range_type * plot_range , double value) {
  plot_range_set_padding__(plot_range , XMIN , value);
}

void plot_range_set_right_padding(plot_range_type * plot_range , double value) {
  plot_range_set_padding__(plot_range , XMAX , value);
}

void plot_range_set_top_padding(plot_range_type * plot_range , double value) {
  plot_range_set_padding__(plot_range , YMAX , value);
}

void plot_range_set_bottom_padding(plot_range_type * plot_range , double value) {
  plot_range_set_padding__(plot_range , YMIN , value);
}

/*****************************************************************/

void plot_range_invert_x_axis(plot_range_type * range, bool invert) {
  range->invert_x_axis = invert;
}

void plot_range_invert_y_axis(plot_range_type * range, bool invert) {
  range->invert_y_axis = invert;
}



/*****************************************************************/


/**
   Allocates a plot_range instance, and initializes it to the
   'auto_range' mode. If you want to use another mode you must call
   plot_range_set_mode() explicitly.
*/

plot_range_type * plot_range_alloc() {
  plot_range_type * range = util_malloc(sizeof * range );
  int i;
  
  for (i=0; i < 4; i++) {
    range->limits[i]      = 0;
    range->padding[i]     = 0.025;  /* Default add some padding */
    range->set[i]         = false;
    range->final_set[i]   = false;
    range->auto_range[i]  = true;
  }
  
  range->invert_x_axis = false;
  range->invert_y_axis = false;
  return range;
}



void plot_range_free(plot_range_type * plot_range) {
  free(plot_range);
}


void plot_range_set_range( plot_range_type * range , double xmin , double xmax , double ymin , double ymax) {
  plot_range_set_xmin(range , xmin);
  plot_range_set_xmax(range , xmax);
  plot_range_set_ymin(range , ymin);
  plot_range_set_ymax(range , ymax);
}


/**
   This function return the final xmin,xmax,ymin and ymax
   functions. To avvoid filling up plplot specific function calls,
   this function does not call plwind(), which would have been
   natural.

   From the calling scope:
   {
      double x1,x2,y1,y2;
      plot_range_apply(range , &x1 , &x2 , &y1 , &y2);
      plwind( x1,x2,y1,y2);
   }

*/

   
void plot_range_apply(plot_range_type * plot_range) {
  double x1 = 0;
  double x2 = 0;
  double y1 = 0;
  double y2 = 0;
  {
    double xmin   = plot_range_get__(plot_range , XMIN );
    double xmax   = plot_range_get__(plot_range , XMAX );
    double ymin   = plot_range_get__(plot_range , YMIN );
    double ymax   = plot_range_get__(plot_range , YMAX );
    double width  = fabs(xmax - xmin);
    double height = fabs(ymax - ymin);
    
    
    if (plot_range->invert_x_axis) {
      x1 = xmax;
      x2 = xmin;
      
      if (plot_range->auto_range[XMAX]) x1 += width * plot_range->padding[XMAX];
      if (plot_range->auto_range[XMIN]) x2 -= width * plot_range->padding[XMIN];
    } else {
      x1 = xmin;
      x2 = xmax;
      
      if (plot_range->auto_range[XMIN]) x1 -= width * plot_range->padding[XMIN];
      if (plot_range->auto_range[XMAX]) x2 += width * plot_range->padding[XMAX];
    }
    
    if (plot_range->invert_y_axis) {
      y1 = ymax;
      y2 = ymin;
      
      if (plot_range->auto_range[YMAX]) y1 += height * plot_range->padding[YMAX];
      if (plot_range->auto_range[YMIN]) y2 -= height * plot_range->padding[YMIN];
    } else {
      y1 = ymin;
      y2 = ymax;
      if (plot_range->auto_range[YMIN]) y1 -= height * plot_range->padding[YMIN];
      if (plot_range->auto_range[YMAX]) y2 += height * plot_range->padding[YMIN];
    }
  } 
  
  
  /* Special case for only one point. */
  {
    if (x1 == x2) {
      if (x1 == 0) {
        x1 = -0.50;
        x2 =  0.50;
      } else {
        x1 -= 0.05 * abs(x1);
        x2 += 0.05 * abs(x2);
      }
    }
    
    if (y1 == y2) {
      if (y1 == 0.0) {
        y1 = -0.50;
        y2 =  0.50;
      } else {
        y1 -= 0.05 * abs(y1);
        y2 += 0.05 * abs(y2);
      }
    }
  }
  {
    int i;
    for (i=0; i < 4; i++)
      plot_range->final_set[i] = true;
  }
  
  plot_range->final[XMIN] = x1;
  plot_range->final[XMAX] = x2;
  plot_range->final[YMIN] = y1;
  plot_range->final[YMAX] = y2;
}




