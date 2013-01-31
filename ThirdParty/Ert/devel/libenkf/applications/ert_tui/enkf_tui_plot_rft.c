/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_tui_plot_rft.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/double_vector.h>
#include <ert/util/util.h>
#include <ert/util/menu.h>
#include <ert/util/arg_pack.h>
#include <ert/util/path_fmt.h>
#include <ert/util/bool_vector.h>
#include <ert/util/msg.h>
#include <ert/util/vector.h>

#include <ert/plot/plot.h>
#include <ert/plot/plot_dataset.h> 

#include <ert/ecl/ecl_rft_file.h>

#include <ert/enkf/enkf_main.h>
#include <ert/enkf/enkf_obs.h>
#include <ert/enkf/block_obs.h>
#include <ert/enkf/gen_obs.h>
#include <ert/enkf/field_config.h>
#include <ert/enkf/obs_vector.h>
#include <ert/enkf/ensemble_config.h>
#include <ert/enkf/enkf_state.h>
#include <ert/enkf/gen_kw_config.h>
#include <ert/enkf/enkf_defaults.h>
#include <ert/enkf/plot_config.h>
#include <ert/enkf/member_config.h>

#include <enkf_tui_util.h>
#include <ert_tui_const.h>
#include <enkf_tui_plot_util.h>
#include <enkf_tui_plot.h>
#include <enkf_tui_fs.h>


/*
  This file contains functions to plot RFT. These are both grid
  independant methods which load a true well trajectory from file, and
  and then lookup pressure nodes afterwards, and also a simpler
  implementation which will plot a enkf block_obs (i.e. in 99% of
  cases and RFT) for a fixed grid.

  The grid independent functions are at the top of the file.
*/


int enkf_tui_plot_read_rft_obs(enkf_main_type * enkf_main, 
                               char * wellname, 
                               double_vector_type * UTM_x, 
                               double_vector_type * UTM_y, 
                               double_vector_type * MD, 
                               double_vector_type * TVD_z, 
                               double_vector_type * RFT_obs){
  const model_config_type * model_config = enkf_main_get_model_config( enkf_main ); 
  const char * pathname = model_config_get_rftpath( model_config );
  path_fmt_type * pathname_fmt = path_fmt_alloc_directory_fmt(pathname);
  strcat(wellname, ".txt"); /* This will append .txt to the wellname*/
  char * pathandfilename = path_fmt_alloc_file(pathname_fmt, false, wellname);
  FILE * stream = util_fopen( pathandfilename , "r" ); 
  int lines = util_count_content_file_lines( stream );
  rewind(stream);
  bool at_eof;
  int tokens;

  for ( int i = 0; i < lines; i++ ) {
    char ** token_list;
    char * line = util_fscanf_alloc_line( stream , &at_eof );
    util_split_string(line , " \t" , &tokens , &token_list);
    if( tokens == 5 ){
      double utm_x, utm_y, md, tvd_z, rft_obs;
      if ( util_sscanf_double( token_list[0] , &utm_x ) && 
           util_sscanf_double( token_list[1] , &utm_y ) && 
           util_sscanf_double( token_list[2] , &md )    && 
           util_sscanf_double( token_list[3] , &tvd_z ) && 
           util_sscanf_double( token_list[4] , &rft_obs )){
        
        double_vector_iset( UTM_x  , i, utm_x );
        double_vector_iset( UTM_y  , i, utm_y );
        double_vector_iset( MD     , i, md );
        double_vector_iset( TVD_z  , i, tvd_z);
        double_vector_iset( RFT_obs, i, rft_obs);
      }
      else{
        util_abort("%s: RFT file has to be on the format UTM_X; UTM_Y; MD; TVD_Z; RFT \n",__func__ , pathandfilename);
      }
    }
    else
      util_abort("%s: RFT file has to be on the format UTM_X; UTM_Y; MD; TVD_Z; RFT \n",__func__ , pathandfilename);
    
    free( line );
    util_free_stringlist(token_list, tokens);
  }

  free( pathandfilename );
  path_fmt_free( pathname_fmt );
  
  wellname=strtok(wellname,".");/* This will remove the appended .txt from the wellname*/
  fclose(stream);
  return lines;
}



void enkf_tui_plot_RFTS__(enkf_main_type * enkf_main , 
                          const char * wellname   ,
                          double_vector_type * MD, 
                          double_vector_type * RFT_obs,
                          double_vector_type * RFT_refcase,
                          bool_vector_type * refcase_has_data,
                          vector_type * pressure_container, 
                          int_vector_type * active,
                          bool rft_file_exists,
                          vector_type * has_data_container,
                          bool isMD) {
                                     
  const int ens_size                        = enkf_main_get_ensemble_size( enkf_main );
  const plot_config_type     * plot_config  = enkf_main_get_plot_config( enkf_main );
  bool  show_plot              = false;
  char * plot_file             = enkf_tui_plot_alloc_plot_file( plot_config , enkf_main_get_current_fs(enkf_main), wellname );
  plot_type * plot ;
  if(isMD)
    plot =  enkf_tui_plot_alloc(plot_config , "RFT pressure", "MD" , wellname, plot_file);
  else
    plot =  enkf_tui_plot_alloc(plot_config , "RFT pressure", "TVD" , wellname, plot_file);
  
  {
    show_plot = true;
  }
  /*
    Start with plotting the simulated rfts
  */
  if(rft_file_exists){
    for (int iens=0; iens < ens_size; iens++){
      const double_vector_type * simulated_pressure = vector_iget_const(pressure_container, iens);
      const bool_vector_type * has_data = vector_iget_const(has_data_container, iens);
      for (int nobs = 0; nobs < double_vector_size(RFT_obs); nobs++){
        if (bool_vector_iget(has_data, nobs)){
          plot_dataset_type * iplot  = plot_alloc_new_dataset( plot , NULL , PLOT_XY );
          double rft_sim_numeric     = double_vector_iget(simulated_pressure , nobs);
          double md_numeric          = double_vector_iget(MD , nobs);
          plot_dataset_append_point_xy( iplot, rft_sim_numeric , md_numeric);
          plot_dataset_set_style( iplot , POINTS );
          plot_dataset_set_point_color( iplot , (iens % 13)+1); /*Can choose between 16 colors, but we dont want 0 which is white or reserved 14 and 15*/ 
        }
      }
    }
  }
  /*
    Now continue with refcase and observations.
  */ 
  plot_dataset_type * rft_obs_value     = plot_alloc_new_dataset( plot , "observation"       , PLOT_XY );
  plot_dataset_type * rft_refcase_value = plot_alloc_new_dataset( plot , "refcase"       , PLOT_XY );

  plot_dataset_set_style( rft_obs_value , POINTS );
  plot_dataset_set_style( rft_refcase_value , POINTS );
          
  plot_dataset_set_point_color( rft_obs_value , 15);
  plot_dataset_set_point_color( rft_refcase_value , 14);

  plot_dataset_set_symbol_type( rft_obs_value , 5);
  plot_dataset_set_symbol_type( rft_refcase_value , 5);
  
  
  for (int nobs = 0; nobs < double_vector_size(RFT_obs); nobs++){
    double rft_obs_numeric     = double_vector_iget(RFT_obs , nobs);
    double md_numeric          = double_vector_iget(MD , nobs);
    plot_dataset_append_point_xy( rft_obs_value, rft_obs_numeric , md_numeric);
    if( bool_vector_iget(refcase_has_data, nobs)){
      double rft_refcase_numeric = double_vector_iget(RFT_refcase , nobs);
      plot_dataset_append_point_xy( rft_refcase_value, rft_refcase_numeric , md_numeric);
    }
  }

  plot_invert_y_axis(plot);
  plot_set_bottom_padding( plot , 0.05);
  plot_set_top_padding( plot    , 0.05);
  plot_set_left_padding( plot   , 0.05);
  plot_set_right_padding( plot  , 0.05);
  
  if (show_plot) {
    enkf_tui_show_plot(plot , plot_config , plot_file); /* Frees the plot - logical ehhh. */
  } else {
    printf("No data to plot \n");
    plot_free(plot);
  }
  
  free(plot_file);
}



void enkf_tui_plot_RFT_simIn(enkf_main_type * enkf_main, path_fmt_type * runpathformat, const path_fmt_type * caseformat, char * wellname , time_t recording_time, bool isMD){
  const int ens_size    = enkf_main_get_ensemble_size( enkf_main );
  const plot_config_type    * plot_config      = enkf_main_get_plot_config( enkf_main );
  const char * data_file       = plot_config_get_plot_refcase( plot_config );
  bool plot_refcase = true;
  if ( strcmp( data_file , "" ) == 0)
    plot_refcase = false;
  /*
    Start by reading RFT measurment
  */
  double_vector_type * UTM_x   = double_vector_alloc( 0 , 0); 
  double_vector_type * UTM_y   = double_vector_alloc( 0 , 0); 
  double_vector_type * MD      = double_vector_alloc( 0 , 0); 
  double_vector_type * TVD_z   = double_vector_alloc( 0 , 0); 
  double_vector_type * RFT_obs = double_vector_alloc( 0 , 0); 
  int lines = enkf_tui_plot_read_rft_obs(enkf_main, wellname, UTM_x, UTM_y, MD, TVD_z, RFT_obs);
  /*
    Find ijk-list
  */
  char * caseending = path_fmt_alloc_path(caseformat, false, 0);               //Use the grid in ensmember 0
  char * casename = path_fmt_alloc_file(runpathformat , false, 0, caseending); 
  ecl_grid_type * grid = ecl_grid_load_case( casename );
  int_vector_type * i_values = int_vector_alloc( lines , 0 );
  int_vector_type * j_values = int_vector_alloc( lines , 0 );
  int_vector_type * k_values = int_vector_alloc( lines , 0 );
  int_vector_type * active   = int_vector_alloc( lines , 0 );
  for (int nobs =0; nobs<lines; nobs++){
    int start_index = 0;
    int i; int j; int k;
    int global_index = ecl_grid_get_global_index_from_xyz(grid,double_vector_iget(UTM_x,nobs) ,double_vector_iget(UTM_y,nobs) ,double_vector_iget(TVD_z,nobs) ,start_index);
    ecl_grid_get_ijk1(grid , global_index, &i, &j , &k);
    int is_active = ecl_grid_get_active_index1(grid , global_index);
    int_vector_iset(i_values, nobs, i);
    int_vector_iset(j_values, nobs, j);
    int_vector_iset(k_values, nobs, k);
    int_vector_iset(active  , nobs, is_active);
    start_index = global_index;
  }
  ecl_grid_free(grid);

  /*
    Find refcase rfts
  */
  double_vector_type * RFT_refcase = double_vector_alloc( 0 , 0);
  bool_vector_type * refcase_has_data = bool_vector_alloc(0, false);
  const char * refcase_file_name = ecl_rft_file_alloc_case_filename(data_file);
  
  if (refcase_file_name == NULL){
    if( plot_refcase )
      util_abort("%s: Cannot find eclipse RFT file",__func__ , refcase_file_name);

  }
  ecl_rft_file_type * rft_refcase_file = ecl_rft_file_alloc( refcase_file_name );
  if (refcase_file_name == NULL){
    if( plot_refcase )
      util_abort("%s: Cannot find eclipse RFT file",__func__ , refcase_file_name);
    
  }
  const ecl_rft_node_type * rft_refcase_node = ecl_rft_file_get_well_time_rft( rft_refcase_file , wellname , recording_time);  
  if(rft_refcase_node == NULL){
    if( plot_refcase )
      printf("No RFT information exists for %s in refcase.\n", wellname);

    for( int nobs = 0; nobs < lines; nobs++){
      double_vector_append(RFT_refcase, 0.0);
      bool_vector_append(refcase_has_data, false);
    }
  }
  else{
    for( int nobs = 0; nobs < lines; nobs++){
      if( int_vector_iget(active,nobs) > -1){
        int cell_index = ecl_rft_node_lookup_ijk( rft_refcase_node , int_vector_iget(i_values,nobs), int_vector_iget(j_values,nobs),int_vector_iget(k_values,nobs) ); //lookup cell
        if(cell_index > -1){
          double pressure_value = ecl_rft_node_iget_pressure( rft_refcase_node , cell_index); // Pressure
          double_vector_append(RFT_refcase, pressure_value);
          bool_vector_append(refcase_has_data, true);
        }
        else{
          double_vector_append(RFT_refcase, 0.0);
          bool_vector_append(refcase_has_data, false);
        }
      }
      else {
        double_vector_append(RFT_refcase, 0.0);
        bool_vector_append(refcase_has_data, false);
      }
    }
  }
  ecl_rft_file_free(rft_refcase_file);
  /*
    Get the simulated RFTs
  */
  vector_type * pressure_container = vector_alloc_new();
  vector_type * has_data_container = vector_alloc_new();
  char * caseending1 = path_fmt_alloc_path(caseformat, false, 0);
  char * casename1 = path_fmt_alloc_file(runpathformat , false, 0, caseending1);
  const char * case_file_name1 = ecl_rft_file_alloc_case_filename(casename1 );
  bool eclipse_rft_exists = false;
  if (case_file_name1 == NULL){
    util_abort("%s: Cannot find eclipse RFT file",__func__ , case_file_name1);
  }
  else{
    eclipse_rft_exists = true;
    for (int iens = 0; iens<ens_size; iens++){
      double_vector_type * simulated_pressures = double_vector_alloc(lines, 0.0);
      bool_vector_type * has_data = bool_vector_alloc(lines, true);
      char * caseending = path_fmt_alloc_path(caseformat, false, iens);
      char * casename = path_fmt_alloc_file(runpathformat , false, iens, caseending);
      const char * case_file_name = ecl_rft_file_alloc_case_filename(casename );
      ecl_rft_file_type * rftfile = ecl_rft_file_alloc( case_file_name );
      const ecl_rft_node_type * rftnode = ecl_rft_file_get_well_time_rft( rftfile , wellname , recording_time);
      if(rftnode == NULL){
        printf("No RFT information exists for %s:\n", wellname);
      }
      else{
        for( int nobs = 0; nobs < lines; nobs++){
          if( int_vector_iget(active,nobs) > -1){
            int cell_index = ecl_rft_node_lookup_ijk( rftnode , int_vector_iget(i_values,nobs), int_vector_iget(j_values,nobs),int_vector_iget(k_values,nobs) ); //lookup cell
            double pressure_value = ecl_rft_node_iget_pressure( rftnode , cell_index); // Pressure
            double_vector_iset(simulated_pressures,nobs , pressure_value);
            if(cell_index > -1)
              bool_vector_iset(has_data, nobs, true);
            else
              bool_vector_iset(has_data, nobs, false);
          }
          else {
            double_vector_iset(simulated_pressures,nobs ,0.0);
            bool_vector_iset(has_data, nobs, false);
          }
        }
      }
      ecl_rft_file_free(rftfile);
      vector_append_owned_ref( pressure_container , simulated_pressures , double_vector_free__ );
      vector_append_owned_ref( has_data_container , has_data , bool_vector_free__ );
    }
  }
  /*
    Do the actual plotting
  */
  if(isMD)
    enkf_tui_plot_RFTS__( enkf_main , wellname , MD, RFT_obs, RFT_refcase, refcase_has_data, pressure_container, active, eclipse_rft_exists, has_data_container, isMD);
  else
    enkf_tui_plot_RFTS__( enkf_main , wellname , TVD_z, RFT_obs, RFT_refcase, refcase_has_data, pressure_container, active, eclipse_rft_exists, has_data_container, isMD);
  double_vector_free( UTM_x );
  double_vector_free( UTM_y );
  double_vector_free( MD  );
  double_vector_free( TVD_z );
  double_vector_free( RFT_obs );
  double_vector_free( RFT_refcase );
  bool_vector_free( refcase_has_data );
  vector_free( pressure_container );  
  vector_free( has_data_container );
  free( caseending );
  free( caseending1 );
  free( casename );
  free( casename1 );
  int_vector_free( i_values );
  int_vector_free( j_values );
  int_vector_free( k_values );
  int_vector_free( active );
};


int enkf_tui_plot_read_rft_config(const char * rft_config_file, stringlist_type * wellnames, time_t_vector_type * dates){
  int lines = 0;
  int day, month, year;
  if ( rft_config_file != NULL ){
    printf( "Reading RFT wellnames and dates \n" );
    FILE * stream = util_fopen( rft_config_file , "r" );
    if(stream == NULL)
      util_abort("%s: RFT config file is NULL \n",__func__ , rft_config_file);  
    
    lines = util_count_content_file_lines( stream );
    rewind(stream);
    bool at_eof;
    char ** token_list;
    int tokens;
    for ( int i = 0; i < lines; i++ ) {
      char * line = util_fscanf_alloc_line( stream , &at_eof );
      util_split_string(line , " \t" , &tokens , &token_list);
      char * name = token_list[0];
      char * ownname = util_alloc_string_copy(name);
      if( tokens == 4 ){
        stringlist_append_owned_ref( wellnames , ownname );
        if ( util_sscanf_int( token_list[1] , &day ) && util_sscanf_int( token_list[2] , &month ) && util_sscanf_int( token_list[3] , &year ) ){
          time_t recording_time = util_make_date(day , month , year);
          time_t_vector_append(dates, recording_time);
        }
        else{
          util_abort("%s: RFT config file has to be on the format NAME DAY MONTH YEAR \n",__func__ , rft_config_file);
        }
      }
      else{
        util_abort("%s: RFT config file has to be on the format NAME DAY MONTH YEAR \n",__func__ , rft_config_file);
      }
      free( line );
      free( name );
    }
    fclose(stream);  
  }
  else{
    printf("RFT config file is not specified.");
  }
  return lines;
}

void enkf_tui_plot_RFT_sim_all_MD( void * arg) {
  enkf_main_type * enkf_main = enkf_main_safe_cast( arg );
  const model_config_type * model_config = enkf_main_get_model_config( enkf_main );
  const char * rft_config_file = enkf_main_get_rft_config_file( enkf_main );
  stringlist_type * wellnames = stringlist_alloc_new();
  time_t_vector_type * dates = time_t_vector_alloc(0,0);
  int lines = enkf_tui_plot_read_rft_config(rft_config_file, wellnames, dates);
  path_fmt_type * runpathformat = model_config_get_runpath_fmt( model_config );
  const path_fmt_type * caseformat = ecl_config_get_eclbase_fmt(enkf_main_get_ecl_config(enkf_main));
  for (int i = 0; i<lines; i++){
    char * wellname = stringlist_iget_copy(wellnames, i);
    time_t  recording_time = time_t_vector_iget(dates, i);
    enkf_tui_plot_RFT_simIn(enkf_main, runpathformat, caseformat, wellname, recording_time, true);
  }
  stringlist_free(wellnames);
  time_t_vector_free(dates);
}

void enkf_tui_plot_RFT_sim_all_TVD( void * arg) {
  enkf_main_type * enkf_main = enkf_main_safe_cast( arg );
  const model_config_type * model_config = enkf_main_get_model_config( enkf_main );
  const char * rft_config_file = enkf_main_get_rft_config_file( enkf_main );
  stringlist_type * wellnames = stringlist_alloc_new();
  time_t_vector_type * dates = time_t_vector_alloc(0,0);
  int lines = enkf_tui_plot_read_rft_config(rft_config_file, wellnames, dates);
  path_fmt_type * runpathformat = model_config_get_runpath_fmt( model_config );
  const path_fmt_type * caseformat = ecl_config_get_eclbase_fmt(enkf_main_get_ecl_config(enkf_main));
  for (int i = 0; i<lines; i++){
    char * wellname = stringlist_iget_copy(wellnames, i);
    time_t  recording_time = time_t_vector_iget(dates, i);
    enkf_tui_plot_RFT_simIn(enkf_main, runpathformat, caseformat, wellname, recording_time, false);
  }
  stringlist_free(wellnames);
  time_t_vector_free(dates);
}


/*****************************************************************/


void enkf_tui_plot_RFT__(enkf_main_type * enkf_main,
                         const char * obs_key , 
                         int report_step) {
  
  const ensemble_config_type * ensemble_config = enkf_main_get_ensemble_config( enkf_main );
  const plot_config_type     * plot_config     = enkf_main_get_plot_config( enkf_main );
  enkf_obs_type              * enkf_obs        = enkf_main_get_obs( enkf_main );
  enkf_fs_type               * fs              = enkf_main_get_fs( enkf_main );
  const obs_vector_type      * obs_vector      = enkf_obs_get_vector( enkf_obs , obs_key );
  const char                 * state_kw        = obs_vector_get_state_kw( obs_vector );
  plot_type                  * plot;
  enkf_node_type             * node;
  
  const int ens_size                  = enkf_main_get_ensemble_size( enkf_main );
  enkf_config_node_type * config_node = ensemble_config_get_node( ensemble_config , state_kw );
  field_config_type * field_config    = enkf_config_node_get_ref( config_node );
  block_obs_type    * block_obs       = obs_vector_iget_node( obs_vector , report_step );
  char * plot_file;
  
  plot_file = enkf_tui_plot_alloc_plot_file(plot_config , enkf_main_get_current_fs(enkf_main), obs_key );
  plot = enkf_tui_plot_alloc(plot_config , state_kw , "Depth" , obs_key , plot_file);
  {
    msg_type * msg             = msg_alloc("Loading realization: ",false);

    const int   obs_size       = block_obs_get_size(block_obs);
    const ecl_grid_type * grid = field_config_get_grid( field_config );
    double * depth             = util_calloc( obs_size , sizeof * depth );
    double min_depth , max_depth;
    
    int l;
    int iens;
    int iens1 = 0;        /* Could be user input */
    int iens2 = ens_size - 1;
    
    plot_dataset_type *  obs;
    node = enkf_node_alloc( config_node );
    
    for (l = 0; l < obs_size; l++) {
      double xpos, ypos,zpos;
      ecl_grid_get_xyz3(grid , 
                        block_obs_iget_i( block_obs ,l ), block_obs_iget_j( block_obs ,l ), block_obs_iget_k( block_obs , l ), 
                        &xpos , &ypos , &zpos);
      depth[l] = zpos;
    }
    
    max_depth = depth[0];
    min_depth = depth[0];
    for (l=1; l< obs_size; l++)
      util_update_double_max_min( depth[l] , &max_depth , &min_depth);
    
    
    msg_show( msg );
    for (iens=iens1; iens <= iens2; iens++) {
      char cens[5];
      sprintf(cens , "%03d" , iens);
      msg_update(msg , cens);
      bool has_node = true;
      node_id_type node_id = {.report_step = report_step , 
                              .iens = iens , 
                              .state = BOTH };
      
      if (enkf_node_try_load( node , fs , node_id)) {
        const field_type * field = enkf_node_value_ptr( node );
        plot_dataset_type * data = plot_alloc_new_dataset( plot , NULL , PLOT_XY);
        plot_dataset_set_style( data , POINTS );
        plot_dataset_set_symbol_size( data , 1.00 );
        for (l = 0; l < obs_size; l++)  /* l : kind of ran out of indices ... */
          plot_dataset_append_point_xy(data , field_ijk_get_double( field , block_obs_iget_i( block_obs ,l ), block_obs_iget_j( block_obs ,l ), block_obs_iget_k( block_obs , l )) , depth[l]);
      } else 
        printf("No data found for :%d/%d \n",iens, report_step);
    }

    
    obs = plot_alloc_new_dataset( plot , NULL , PLOT_X1X2Y );
    for (l = 0; l < obs_size; l++) {
      double value , std;
      
      block_obs_iget(block_obs , l , &value , &std);
      plot_dataset_append_point_x1x2y( obs , value - std , value + std , depth[l]);
    }
    
    plot_set_bottom_padding( plot , 0.05);
    plot_set_top_padding( plot , 0.05);
    plot_set_left_padding( plot , 0.05);
    plot_set_right_padding( plot , 0.05);
    plot_invert_y_axis( plot );
    
    plot_dataset_set_line_color( obs , RED );
    free(depth);
    msg_free(msg , true);
  }
  enkf_tui_show_plot( plot , plot_config , plot_file);
  printf("Plot saved in: %s \n",plot_file);
  free(plot_file);
}


void enkf_tui_plot_select_RFT(const enkf_main_type * enkf_main , char ** _obs_key , int * _report_step) {
  enkf_obs_type              * enkf_obs        = enkf_main_get_obs( enkf_main );
  {
    const char * prompt  = "Which RFT observation: ";

    const obs_vector_type * obs_vector = NULL;
    char  *obs_key;
    int    report_step;
    while (true) {
      util_printf_prompt(prompt , PROMPT_LEN , '=' , "=> ");
      obs_key = util_alloc_stdin_line( );
      if (obs_key != NULL) {
        if (enkf_obs_has_key(enkf_obs , obs_key)) {
          obs_vector = enkf_obs_get_vector( enkf_obs , obs_key );
          if (obs_vector_get_impl_type( obs_vector ) == BLOCK_OBS)
            break; /* Jumping out with a valid obs_vector pointer. */
          else {
            fprintf(stderr,"Observation key:%s does not correspond to a field observation.\n",obs_key);
            obs_vector = NULL;
          }
        } else
          fprintf(stderr,"Do not have observation key:%s \n",obs_key);
      } else
        break; /* Jumping out on blank input */
    }

    if (obs_vector != NULL) {
      do {
        if (obs_vector_get_num_active( obs_vector ) == 1)
          report_step = obs_vector_get_active_report_step( obs_vector );
        else
          report_step = enkf_tui_util_scanf_report_step(enkf_main_get_history_length( enkf_main ) , "Report step" , PROMPT_LEN);
      } while (!obs_vector_iget_active(obs_vector , report_step));
      *_obs_key = obs_key;
      *_report_step = report_step;
    }
  }
}



void enkf_tui_plot_RFT_depth(void * arg) {
  enkf_main_type             * enkf_main       = enkf_main_safe_cast( arg );
  enkf_obs_type              * enkf_obs        = enkf_main_get_obs( enkf_main );
  {
    char * obs_key;
    int report_step;
    obs_vector_type * obs_vector;
    
    enkf_tui_plot_select_RFT(enkf_main , &obs_key , &report_step);
    obs_vector = enkf_obs_get_vector( enkf_obs , obs_key );
    enkf_tui_plot_RFT__(enkf_main , obs_key , report_step);
    free( obs_key );
  }
}







/**
   This function plots all the RFT's - observe that 'RFT' is no
   fundamental type in the enkf_obs type system. It will plot all
   BLOCK_OBS observations, they will typically (99% ??) be Pressure
   observations, but could in principle also be saturation observatioons.
*/



void enkf_tui_plot_all_RFT( void * arg) {
  enkf_main_type             * enkf_main       = enkf_main_safe_cast( arg );
  enkf_obs_type              * enkf_obs        = enkf_main_get_obs( enkf_main );
  {
    int iobs , report_step;
    stringlist_type * RFT_keys = enkf_obs_alloc_typed_keylist(enkf_obs , BLOCK_OBS);
    
    for (iobs = 0; iobs < stringlist_get_size( RFT_keys ); iobs++) {
      const char * obs_key = stringlist_iget( RFT_keys , iobs);
      const obs_vector_type * obs_vector = enkf_obs_get_vector( enkf_obs , obs_key );
      
      do {
        if (obs_vector_get_num_active( obs_vector ) == 1)
          report_step = obs_vector_get_active_report_step( obs_vector );
        else 
          /* An RFT should really be active at only one report step - but ... */
          report_step = enkf_tui_util_scanf_report_step(enkf_main_get_history_length( enkf_main ) , "Report step" , PROMPT_LEN);
      } while (!obs_vector_iget_active(obs_vector , report_step));
      
      enkf_tui_plot_RFT__(enkf_main , obs_key , report_step);
    }
  }
}
