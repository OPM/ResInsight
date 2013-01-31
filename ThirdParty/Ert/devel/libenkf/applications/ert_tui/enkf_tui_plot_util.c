/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'enkf_tui_plot_util.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#include <ert/util/util.h>
#include <ert/util/menu.h>
#include <ert/util/arg_pack.h>
#include <ert/util/msg.h>
#include <ert/util/vector.h>
#include <ert/util/bool_vector.h>
#include <ert/util/double_vector.h>
#include <ert/util/path_fmt.h>

#include <ert/ecl/ecl_rft_file.h>

#include <ert/plot/plot.h>
#include <ert/plot/plot_dataset.h> 

#include <ert/enkf/ensemble_config.h>
#include <ert/enkf/enkf_state.h>
#include <ert/enkf/gen_kw_config.h>
#include <ert/enkf/enkf_defaults.h>
#include <ert/enkf/plot_config.h>
#include <ert/enkf/member_config.h>
#include <ert/enkf/enkf_main.h>
#include <ert/enkf/enkf_obs.h>
#include <ert/enkf/block_obs.h>
#include <ert/enkf/gen_obs.h>
#include <ert/enkf/field_config.h>
#include <ert/enkf/obs_vector.h>

#include <ert_tui_const.h>
#include <enkf_tui_util.h>
#include <enkf_tui_plot.h>
#include <enkf_tui_fs.h>



char * enkf_tui_plot_alloc_plot_file(const plot_config_type * plot_config , const char * case_name , const char * base_name) {
  char * base      = util_alloc_string_copy( base_name );
  char * path      = util_alloc_filename(plot_config_get_path( plot_config ) , case_name , NULL); /* It is really a path - but what the fuck. */ 
  char * plot_file;

  util_string_tr( base , '/' , '-');              /* Replace '/' -> '-' in the key name. */
  plot_file = util_alloc_filename(path , base , plot_config_get_image_type( plot_config ));
  util_make_path( path );                        /* Ensure that the path where the plots are stored exists. */

  free(path);
  free(base);
  return plot_file;
}
                                           

void enkf_tui_show_plot(plot_type * plot , const plot_config_type * plot_config , const char * file) {
  plot_data(plot);
  plot_free(plot);
  if (util_file_exists( file )) {
    const char * viewer = plot_config_get_viewer( plot_config );
    printf("Plot saved in: %s \n",file);
    if (viewer != NULL)
      util_fork_exec(viewer , 1 , (const char *[1]) { file } , false , NULL , NULL , NULL , NULL , NULL);
  }
  /*
    else: the file does not exist - that might be OK?
  */
}


plot_type * enkf_tui_plot_alloc(const plot_config_type * plot_config , const char * x_label , const char * y_label , const char * title , const char * file) {
  
  arg_pack_type * arg_pack = arg_pack_alloc();
  plot_type * plot;
  
  if (util_string_equal( plot_config_get_driver( plot_config ) , "PLPLOT")) {
    arg_pack_append_ptr( arg_pack , file );
    arg_pack_append_ptr( arg_pack , plot_config_get_image_type( plot_config ));
  } else if (util_string_equal( plot_config_get_driver( plot_config ) , "TEXT")) {

    char * plot_path, *basename;
    char * path;
    util_alloc_file_components( file , &plot_path , &basename , NULL);
    
    path = util_alloc_filename( plot_path , basename , NULL);
    arg_pack_append_owned_ptr( arg_pack , path , free);
    
    free( plot_path );
    free( basename );
  } else 
    util_abort("%s: unrecognized driver type: %s \n",__func__ , plot_config_get_driver( plot_config ));
  
  plot = plot_alloc(plot_config_get_driver( plot_config ) , arg_pack , false , plot_config_get_logy( plot_config ));
  
  plot_set_window_size(plot , plot_config_get_width( plot_config ) , plot_config_get_height( plot_config ));
  plot_set_labels(plot, x_label , y_label , title);
  arg_pack_free( arg_pack );
  
  return plot;
}
