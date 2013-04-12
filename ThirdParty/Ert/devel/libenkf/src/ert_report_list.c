/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'ert_report_list.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/util.h>
#include <ert/util/stringlist.h>
#include <ert/util/vector.h>
#include <ert/util/subst_list.h>

#include <ert/config/config.h>

#include <ert/ecl/ecl_sum.h>

#include <ert/enkf/ert_report.h>
#include <ert/enkf/ert_report_list.h>
#include <ert/enkf/config_keys.h>
#include <ert/enkf/enkf_defaults.h>

#define WELL_LIST_TAG   "$WELL_LIST"
#define GROUP_LIST_TAG  "$GROUP_LIST"
#define PLOT_CASE_TAG   "$PLOT_CASE" 
#define USER_TAG        "$USER"
#define CONFIG_FILE_TAG "$CONFIG_FILE"




struct ert_report_list_struct {
  stringlist_type * path_list;
  vector_type     * report_list;
  char            * target_path;
  char            * plot_path;
  stringlist_type * well_list;
  stringlist_type * group_list;
  subst_list_type * global_context;
  int               latex_timeout;
  bool              init_large_report;
};


void ert_report_list_set_latex_timeout( ert_report_list_type * report_list , int timeout) {
  report_list->latex_timeout = timeout;
}

int ert_report_list_get_latex_timeout( const ert_report_list_type * report_list ) {
  return report_list->latex_timeout;
}

static void ert_report_list_init_large_report( ert_report_list_type * report_list ) {
  if (report_list->init_large_report) {
    printf("Running script \'fmtutil --all\' to regenerate pdflatex config information ..... ");  
    fflush(stdout);
    util_fork_exec("fmtutil" , 1 , (const char *[1]) {"--all"} , true , NULL , NULL , NULL , "/dev/null" , "/dev/null");
    printf("\n");
  }
  report_list->init_large_report = false;
}

bool ert_report_list_get_init_large_report( const ert_report_list_type * report_list ) {
  return report_list->init_large_report;
}

void ert_report_list_set_large_report(ert_report_list_type * report_list , bool init_large_report) {
  report_list->init_large_report = init_large_report;
}


ert_report_list_type * ert_report_list_alloc(const char * target_path, const char * plot_path ) {
  ert_report_list_type * report_list = util_malloc( sizeof * report_list );
  report_list->path_list   = stringlist_alloc_new( );
  report_list->report_list = vector_alloc_new( );
  report_list->target_path = NULL;
  report_list->plot_path   = NULL;
  report_list->init_large_report = DEFAULT_REPORT_LARGE;

  report_list->well_list      = stringlist_alloc_new();
  report_list->group_list     = stringlist_alloc_new();
  report_list->global_context = subst_list_alloc( NULL );
  ert_report_list_set_plot_path( report_list , plot_path );
  ert_report_list_set_target_path( report_list , target_path );
  ert_report_list_set_latex_timeout( report_list , DEFAULT_REPORT_TIMEOUT);
  return report_list;
}

void ert_report_list_add_global_context( ert_report_list_type * report_list , const char * key , const char * value) {
  subst_list_append_copy( report_list->global_context , key , value , NULL );
}

void ert_report_list_set_target_path( ert_report_list_type * report_list , const char * target_path ) {
  report_list->target_path = util_realloc_string_copy( report_list->target_path , target_path );
}


void ert_report_list_set_plot_path( ert_report_list_type * report_list , const char * plot_path ) {
  report_list->plot_path = util_realloc_string_copy( report_list->plot_path , plot_path );
}

/**
   Observe that the new path is added to the front of the list; this is to ensure
   that it is possible to insert user specifed report paths which come before the
   system wide search path.
*/
bool ert_report_list_add_path( ert_report_list_type * report_list , const char * path ) {
  if (util_is_directory( path )) {
    stringlist_insert_copy( report_list->path_list , 0 , path );
    return true;
  } else {
    fprintf(stderr,"** Warning: Path:%s does not exist - not added to report template search path.\n",path);
    return false;
  }
}


static void ert_report_list_add__( ert_report_list_type * report_list , const char * template_path , const char * target_name) {
  ert_report_type * report = ert_report_alloc(template_path , target_name);

  vector_append_owned_ref( report_list->report_list , report , ert_report_free__ );
}


bool ert_report_list_add_report( ert_report_list_type * report_list , const char * input_template) {
  char * input_name;
  char * target_name;
  bool exists = false;

  util_binary_split_string( input_template , ":" , true , &input_name , &target_name);
  if (util_is_file( input_name )) {
    ert_report_list_add__( report_list , input_name , target_name);
    exists = true;
  } else {
    for (int i=0; i < stringlist_get_size( report_list->path_list ); i++) {
      const char * template_file = util_alloc_filename( stringlist_iget( report_list->path_list , i ) , input_name , NULL);
      if (util_is_file( template_file )) {
        ert_report_list_add__( report_list , template_file , target_name );
        exists = true;
        break;
      }
    }
  }

  util_safe_free( input_name );
  util_safe_free( target_name );
  return exists;
}


void ert_report_list_free( ert_report_list_type * report_list ){ 
  stringlist_free( report_list->path_list );
  stringlist_free( report_list->group_list );
  stringlist_free( report_list->well_list );
  subst_list_free( report_list->global_context );
  vector_free( report_list->report_list );
  free( report_list );
}


int ert_report_list_get_num( const ert_report_list_type * report_list ) {
  return vector_get_size( report_list->report_list );
}

static void ert_report_list_add_well( ert_report_list_type * report_list , const char * well ) {
  if (!stringlist_contains( report_list->well_list , well ))
    stringlist_append_copy( report_list->well_list , well);
}

static void ert_report_list_add_group( ert_report_list_type * report_list , const char * group ) {
  if (!stringlist_contains( report_list->group_list , group ))
    stringlist_append_copy( report_list->group_list , group);
}

void ert_report_list_add_wells( ert_report_list_type * report_list , const ecl_sum_type * ecl_sum , const char * well_pattern ) {
  if (ecl_sum != NULL) {
    stringlist_type * well_list = ecl_sum_alloc_well_list( ecl_sum , well_pattern );
    for (int i=0; i < stringlist_get_size( well_list ); i++)
      ert_report_list_add_well( report_list , stringlist_iget( well_list , i ));
    stringlist_free( well_list );
  } else
    ert_report_list_add_well( report_list , well_pattern);
}


void ert_report_list_add_groups( ert_report_list_type * report_list , const ecl_sum_type * ecl_sum , const char * group_pattern ) {
  if (ecl_sum != NULL) {
    stringlist_type * group_list = ecl_sum_alloc_group_list( ecl_sum , group_pattern );
    for (int i=0; i < stringlist_get_size( group_list ); i++)
      ert_report_list_add_group( report_list , stringlist_iget( group_list , i ));
    stringlist_free( group_list );
  } else
    ert_report_list_add_group( report_list , group_pattern);
}



char * alloc_list(const stringlist_type * list) {
  char * body = stringlist_alloc_joined_string( list , ",");
  char * list_str = util_alloc_sprintf("[%s]" , body );
  free( body );
  return list_str;
}

/*****************************************************************/

void ert_report_list_create( ert_report_list_type * report_list , const char * current_case , bool verbose ) {
  if (vector_get_size( report_list->report_list ) > 0) {
    subst_list_type * context = subst_list_alloc( report_list->global_context );
    char * target_path = util_alloc_filename( report_list->target_path , current_case , NULL );
    util_make_path( target_path );

    subst_list_append_ref( context , PLOT_CASE_TAG , current_case , "The value of the current case - used to pick up plot figures.");
    subst_list_append_owned_ref( context , WELL_LIST_TAG  , alloc_list( report_list->well_list ), "The list of wells for plotting");
    subst_list_append_owned_ref( context , GROUP_LIST_TAG , alloc_list( report_list->group_list ), "The list of groups for plotting");

    ert_report_list_init_large_report( report_list );
    for (int ir = 0; ir < vector_get_size( report_list->report_list ); ir++) {
      ert_report_type * ert_report = vector_iget( report_list->report_list , ir);
      
      if (verbose) {
        printf("Creating report: %s/%s.pdf [Work-path:%s] ....... " , target_path , ert_report_get_basename( ert_report ) , ert_report_get_work_path( ert_report ));
        fflush( stdout );
      }
      {
        bool success = ert_report_create( vector_iget( report_list->report_list , ir ) , report_list->latex_timeout , context , report_list->plot_path , target_path );
        if (success)
          printf("OK??\n");
        else
          printf("error - check path:%s \n",ert_report_get_work_path( ert_report ));
      }
    }
    subst_list_free( context );
  }
}


void ert_report_list_site_init( ert_report_list_type * report_list , config_type * config ) {

  /* Installing the directories to search in. */
  for (int i=0; i < config_get_occurences( config , REPORT_SEARCH_PATH_KEY ); i++) {
    const stringlist_type * path_list = config_iget_stringlist_ref( config , REPORT_SEARCH_PATH_KEY , i);
    for (int j=0; j < stringlist_get_size( path_list ); j++) 
      ert_report_list_add_path( report_list , stringlist_iget( path_list , j ));
  }
}


void ert_report_list_init( ert_report_list_type * report_list , config_type * config , const ecl_sum_type * refcase) {
  ert_report_list_site_init( report_list , config );

  if (config_item_set( config , REPORT_LARGE_KEY))
    ert_report_list_set_large_report(report_list , config_get_value_as_bool( config , REPORT_LARGE_KEY ));
  
  if (config_item_set( config , REPORT_TIMEOUT_KEY))
    ert_report_list_set_latex_timeout( report_list , config_get_value_as_int( config , REPORT_TIMEOUT_KEY ));
  
  /* Installing the list of reports. */
  for (int i=0; i < config_get_occurences( config , REPORT_LIST_KEY ); i++) {
    const stringlist_type * list = config_iget_stringlist_ref( config , REPORT_LIST_KEY , i);
    for (int j=0; j < stringlist_get_size( list ); j++) {
      if (!ert_report_list_add_report( report_list , stringlist_iget( list , j )))
        fprintf(stderr,"** Warning: Could not find report template:%s - ignored\n", stringlist_iget( list , j ));
    }
  }
  
  /* Installing the list of report wells */
  for (int i=0; i < config_get_occurences( config , REPORT_WELL_LIST_KEY ); i++) {
    const stringlist_type * well_list = config_iget_stringlist_ref( config , REPORT_WELL_LIST_KEY , i);
    for (int j=0; j < stringlist_get_size( well_list ); j++) 
      ert_report_list_add_wells( report_list , refcase , stringlist_iget( well_list , j ));
  }  
  
  /* Installing the list of report groups */
  for (int i=0; i < config_get_occurences( config , REPORT_GROUP_LIST_KEY ); i++) {
    const stringlist_type * group_list = config_iget_stringlist_ref( config , REPORT_GROUP_LIST_KEY , i);
    for (int j=0; j < stringlist_get_size( group_list ); j++) 
      ert_report_list_add_groups( report_list , refcase , stringlist_iget( group_list , j ));
  }  
  
  /* Installing arbitrary context keys. */
  for (int i=0; i < config_get_occurences( config , REPORT_CONTEXT_KEY ); i++) {
    const char * key   = config_iget( config , REPORT_CONTEXT_KEY , i , 0 );
    const char * value = config_iget( config , REPORT_CONTEXT_KEY , i , 1 );
    ert_report_list_add_global_context( report_list , key , value );
  }  
  
  /* Installing the target path for reports*/
  if (config_item_set(config , REPORT_PATH_KEY))
    ert_report_list_set_target_path( report_list , config_iget( config , REPORT_PATH_KEY , 0 , 0));

  ert_report_list_add_global_context( report_list , CONFIG_FILE_TAG , config_get_config_file( config , true ));
  ert_report_list_add_global_context( report_list , USER_TAG , getenv("USER"));
}


void ert_report_list_add_config_items( config_type * config ) {
  config_schema_item_type * item;
  
  item = config_add_schema_item(config , REPORT_LIST_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1 , CONFIG_DEFAULT_ARG_MAX);

  item = config_add_schema_item(config , REPORT_CONTEXT_KEY , false  );
  config_schema_item_set_argc_minmax(item , 2 , 2);
  
  item = config_add_schema_item(config , REPORT_PATH_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1 , 1);

  item = config_add_schema_item( config , REPORT_WELL_LIST_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1 , CONFIG_DEFAULT_ARG_MAX);
  
  item = config_add_schema_item( config , REPORT_GROUP_LIST_KEY , false  );
  config_schema_item_set_argc_minmax(item , 1 , CONFIG_DEFAULT_ARG_MAX);

  item = config_add_schema_item( config , REPORT_TIMEOUT_KEY , false );
  config_schema_item_set_argc_minmax(item , 1 , 1 );
  config_schema_item_iset_type( item , 0 , CONFIG_INT);

  item = config_add_schema_item( config , REPORT_LARGE_KEY , false );
  config_schema_item_set_argc_minmax(item , 1 , 1 );
  config_schema_item_iset_type( item , 0 , CONFIG_BOOL);
}
