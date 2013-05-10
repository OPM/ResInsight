/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'plot.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __plot_h__
#define __plot_h__
#ifdef __cplusplus
extern "c" {
#endif


/**
 * @addtogroup plot_type plot_type: a new unique plot with room for n datasets.
 * @brief defines the plot_type, this is the core for every new plot you 
 * want to make. 
 *
 * @remarks for every new plot, start with plot_alloc() to create a new plot_type.
 *
 * @{
 */
#include <stdbool.h>

#include <ert/util/util.h>

#include <plplot/plplot.h>

#include <ert/plot/plot_const.h>
#include <ert/plot/plot_dataset.h>


typedef struct plot_struct plot_type;


plot_dataset_type * plot_alloc_new_dataset(plot_type *  , const char * __label , plot_data_type );

plot_type * plot_alloc(const char * __driver_type , const void * init_arg, bool logx , bool logy);
int plot_get_stream(plot_type * item);

void plot_set_xlabel(plot_type * , const char *);
void plot_set_ylabel(plot_type * , const char *);
void plot_set_title(plot_type * , const char *);
void plot_set_labels(plot_type * item, const char *xlabel, const char *ylabel, const char *title);

void plot_data(plot_type * item);
void plot_free(plot_type * item);
void plot_update_range(plot_type * item, plot_range_type * );

void plot_set_window_size(plot_type * , int , int );
void plot_invert_y_axis(plot_type * );
void plot_invert_x_axis(plot_type * );

void plot_set_top_padding(plot_type    *  , double );
void plot_set_bottom_padding(plot_type *  , double );
void plot_set_left_padding(plot_type   *  , double );
void plot_set_right_padding(plot_type  *  , double );

void plot_set_xmin(plot_type * plot , double xmin);
void plot_set_xmax(plot_type * plot , double xmax);
void plot_set_ymin(plot_type * plot , double ymin);
void plot_set_ymax(plot_type * plot , double ymax);

void          plot_set_range(plot_type * plot , double xmin , double xmax , double ymin , double ymax);
void          plot_set_label_color(plot_type * , plot_color_type );
void          plot_set_box_color(plot_type *   , plot_color_type );
void          plot_set_label_fontsize(plot_type * , double );
  void          plot_set_axis_fontsize(plot_type * plot , double axis_font_size_scale);
const char *  plot_set_default_timefmt(plot_type * plot , time_t t1 , time_t t2);
void          plot_set_timefmt(plot_type * plot , const char * timefmt);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif
