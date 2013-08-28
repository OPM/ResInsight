/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_config_node.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdio.h>

#include <ert/util/stringlist.h>
#include <ert/util/util.h>
#include <ert/util/path_fmt.h>
#include <ert/util/bool_vector.h>
#include <ert/util/vector.h>

#include <ert/ecl/ecl_grid.h>

#include <ert/enkf/enkf_macros.h>
#include <ert/enkf/enkf_node.h>
#include <ert/enkf/field_config.h>
#include <ert/enkf/gen_data_config.h>
#include <ert/enkf/gen_kw_config.h>
#include <ert/enkf/summary_config.h>
#include <ert/enkf/surface_config.h>
#include <ert/enkf/container_config.h>
#include <ert/enkf/enkf_obs.h>
#include <ert/enkf/gen_obs.h>
#include <ert/enkf/enkf_config_node.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/enkf_types.h>
#include <ert/enkf/enkf_defaults.h>
#include <ert/enkf/config_keys.h>

#define ENKF_CONFIG_NODE_TYPE_ID 776104

struct enkf_config_node_struct {
  UTIL_TYPE_ID_DECLARATION;
  ert_impl_type          impl_type;
  enkf_var_type           var_type; 
  bool                    vector_storage; 
  bool                    forward_init;     /* Should the (parameter) node be initialized by loading results from the Forward model? */

  bool_vector_type      * internalize;      /* Should this node be internalized - observe that question of what to internalize is MOSTLY handled at a higher level - without consulting this variable. Can be NULL. */ 
  stringlist_type       * obs_keys;         /* Keys of observations which observe this node. */
  char                  * key;
  path_fmt_type         * init_file_fmt;    /* Format used to create files for initialization. */
  path_fmt_type         * enkf_infile_fmt;  /* Format used to load in file from forward model - one %d (if present) is replaced with report_step. */
  path_fmt_type         * enkf_outfile_fmt; /* Name of file which is written by EnKF, and read by the forward model. */
  void                  * data;             /* This points to the config object of the actual implementation.        */
  enkf_node_type        * min_std;
  char                  * min_std_file; 
  
  vector_type           * container_nodes;
  /*****************************************************************/
  /* Function pointers to methods working on the underlying config object. */
  get_data_size_ftype   * get_data_size;    /* Function pointer to ask the underlying config object of the size - i.e. number of elements. */
  config_free_ftype     * freef;
};



static bool enkf_config_node_has_container(const enkf_config_node_type * node , enkf_fs_type * fs , node_id_type node_id) {
  bool has_container = true;

  for (int inode=0; inode < vector_get_size( node->container_nodes ); inode++) {
    enkf_config_node_type * child_node = vector_iget( node->container_nodes , inode );
    if (!enkf_config_node_has_node( child_node , fs , node_id )) {
      has_container = false;
      break;
    }
  }
  return has_container;
}



bool enkf_config_node_has_node( const enkf_config_node_type * node , enkf_fs_type * fs , node_id_type node_id) {
  if (node->impl_type == CONTAINER) 
    return enkf_config_node_has_container( node , fs , node_id );
  else
    return enkf_fs_has_node( fs , node->key , node->var_type , node_id.report_step , node_id.iens , node_id.state );
}


bool enkf_config_node_has_vector( const enkf_config_node_type * node , enkf_fs_type * fs , int iens , state_enum state) {
  bool has_vector = enkf_fs_has_vector( fs , node->key , node->var_type , iens , state );
  return has_vector;
}



static enkf_config_node_type * enkf_config_node_alloc__( enkf_var_type   var_type, 
                                                         ert_impl_type  impl_type, 
                                                         const char * key,
                                                         bool forward_init) {

  enkf_config_node_type * node = util_malloc( sizeof *node );
  UTIL_TYPE_ID_INIT( node , ENKF_CONFIG_NODE_TYPE_ID );
  node->forward_init    = forward_init;
  node->var_type        = var_type;
  node->impl_type       = impl_type;
  node->key             = util_alloc_string_copy( key );
  node->container_nodes = vector_alloc_new();
  node->vector_storage  = false;

  node->init_file_fmt    = NULL; 
  node->enkf_infile_fmt  = NULL;
  node->enkf_outfile_fmt = NULL;
  node->internalize      = NULL;
  node->data             = NULL;
  node->obs_keys         = stringlist_alloc_new(); 
  node->min_std          = NULL;
  node->min_std_file     = NULL;
  
  node->get_data_size = NULL;
  node->freef         = NULL; 
  {  
    switch(impl_type) {
    case(FIELD):
      node->freef             = field_config_free__;
      node->get_data_size     = field_config_get_data_size__;  
      break;
    case(STATIC):
      break;
    case(GEN_KW):
      node->freef             = gen_kw_config_free__;
      node->get_data_size     = gen_kw_config_get_data_size__;
      break;
    case(SUMMARY):
      node->vector_storage    = true;
      node->freef             = summary_config_free__;
      node->get_data_size     = summary_config_get_data_size__;
      break;
    case(GEN_DATA):
      node->freef             = gen_data_config_free__;
      node->get_data_size     = NULL;
      break;
    case(SURFACE):
      node->freef             = surface_config_free__;
      node->get_data_size     = surface_config_get_data_size__;
      break;
    case(CONTAINER):
      node->freef             = container_config_free__;
      node->get_data_size     = container_config_get_data_size__;
      break;
    default:
      util_abort("%s : invalid implementation type: %d - aborting \n",__func__ , impl_type);
    }
  }
  return node;
}


bool enkf_config_node_vector_storage( const enkf_config_node_type * config_node) {
  return config_node->vector_storage;
}

/**
   Requires that enkf_outfile_fmt (i.e. the name of the file produced
   by ERT) is set to a non NULL value; in addition to the type
   specific validation in gen_kw_config_is_valid().
*/

static bool enkf_config_node_is_valid_GEN_KW( const enkf_config_node_type * config_node ) {
  bool valid = gen_kw_config_is_valid( config_node->data );
  valid = (valid && (config_node->enkf_outfile_fmt != NULL));
  
  return valid;
}


static bool enkf_config_node_is_valid_FIELD( const enkf_config_node_type * config_node ) {
  bool valid = false;
  if ( config_node->var_type != INVALID_VAR )   
    valid = field_config_is_valid( config_node->data );
  
  
  return valid;
}


static bool enkf_config_node_is_valid_GEN_DATA( const enkf_config_node_type * config_node ) {
  bool valid = gen_kw_config_is_valid( config_node->data );
  valid = (valid && (config_node->enkf_outfile_fmt != NULL));
  
  return valid;
}





bool enkf_config_node_is_valid( const enkf_config_node_type * config_node ) {
  bool valid = false;

  switch(config_node->impl_type) {
  case(FIELD):
    valid = enkf_config_node_is_valid_FIELD( config_node );
    break;
  case(SUMMARY):
    valid = true;
    break;
  case(GEN_KW):
    valid = enkf_config_node_is_valid_GEN_KW( config_node );
    break;
  case(GEN_DATA):
    valid = enkf_config_node_is_valid_GEN_DATA( config_node );
    break;
  default:
    util_abort("%s: - what the fuXX - internal bug. \n",__func__);
  }
  
  return valid;
}

void enkf_config_node_update_min_std( enkf_config_node_type * config_node , const char * min_std_file ) {
  if (!util_string_equal( config_node->min_std_file , min_std_file )) {
    /* The current min_std_file and the new input are different, and
       the min_std node must be cleared. */
    if (config_node->min_std != NULL) {
      enkf_node_free( config_node->min_std );
      config_node->min_std = NULL;
      free( config_node->min_std_file );
    }
  }
  config_node->min_std_file = util_realloc_string_copy( config_node->min_std_file , min_std_file );
  if (config_node->min_std_file != NULL) {
    config_node->min_std = enkf_node_alloc( config_node );
    enkf_node_fload( config_node->min_std , min_std_file );
  }
}


static void enkf_config_node_update( enkf_config_node_type * config_node , 
                                     const char * initfile_fmt , 
                                     const char * enkf_outfile_fmt , 
                                     const char * enkf_infile_fmt ,
                                     const char * min_std_file ) {

  config_node->init_file_fmt    = path_fmt_realloc_path_fmt( config_node->init_file_fmt    , initfile_fmt ); 
  config_node->enkf_infile_fmt  = path_fmt_realloc_path_fmt( config_node->enkf_infile_fmt  , enkf_infile_fmt ); 
  config_node->enkf_outfile_fmt = path_fmt_realloc_path_fmt( config_node->enkf_outfile_fmt , enkf_outfile_fmt ); 
  enkf_config_node_update_min_std( config_node , min_std_file );
}




enkf_config_node_type * enkf_config_node_alloc(enkf_var_type              var_type,
                                               ert_impl_type              impl_type,
                                               bool                       forward_init , 
                                               const char               * key , 
                                               const char               * init_file_fmt , 
                                               const char               * enkf_outfile_fmt , 
                                               const char               * enkf_infile_fmt  , 
                                               void                     * data) {

  enkf_config_node_type * node = enkf_config_node_alloc__( var_type , impl_type , key , forward_init);
  enkf_config_node_update( node , init_file_fmt, enkf_outfile_fmt , enkf_infile_fmt , NULL );
  node->data = data;
  return node;
}



void enkf_config_node_update_gen_kw( enkf_config_node_type * config_node ,
                                     const char * enkf_outfile_fmt ,   /* The include file created by ERT for the forward model. */
                                     const char * template_file    , 
                                     const char * parameter_file   ,
                                     const char * min_std_file     ,
                                     const char * init_file_fmt ) {

  /* 1: Update the low level gen_kw_config stuff. */
  gen_kw_config_update( config_node->data , template_file , parameter_file );    

  /* 2: Update the stuff which is owned by the upper-level enkf_config_node instance. */
  enkf_config_node_update( config_node , init_file_fmt , enkf_outfile_fmt , NULL , min_std_file);
}


/**
   This will create a new gen_kw_config instance which is NOT yet
   valid. 
*/
enkf_config_node_type * enkf_config_node_new_gen_kw( const char * key , const char * tag_fmt , bool forward_init) {
  enkf_config_node_type * config_node = enkf_config_node_alloc__( PARAMETER , GEN_KW , key , forward_init);
  config_node->data = gen_kw_config_alloc_empty( key , tag_fmt );
  return config_node;
}


enkf_config_node_type * enkf_config_node_new_surface( const char * key , bool forward_init) {
  enkf_config_node_type * config_node = enkf_config_node_alloc__( PARAMETER , SURFACE , key , forward_init);
  config_node->data = surface_config_alloc_empty( );
  return config_node;
}


void enkf_config_node_update_surface( enkf_config_node_type * config_node , const char * base_surface, const char * init_file_fmt , const char * output_file , const char * min_std_file ) {

  /* 1: Update the data owned by the surface node. */
  surface_config_set_base_surface( config_node->data , base_surface );
  
  /* 2: Update the stuff which is owned by the upper-level enkf_config_node instance. */
  enkf_config_node_update( config_node , init_file_fmt , output_file , NULL , min_std_file);
}


/*****************************************************************/

enkf_config_node_type * enkf_config_node_alloc_summary( const char * key , load_fail_type load_fail) {
  enkf_config_node_type * config_node = enkf_config_node_alloc__( DYNAMIC_RESULT , SUMMARY , key , false);
  config_node->data = summary_config_alloc( key , config_node->vector_storage , load_fail );
  return config_node;
}


/*****************************************************************/

enkf_config_node_type * enkf_config_node_new_gen_data( const char * key , bool forward_init) {
  enkf_config_node_type * config_node = enkf_config_node_alloc__( INVALID , GEN_DATA , key , forward_init);
  config_node->data = gen_data_config_alloc_empty( key );
  return config_node;
}

/*****************************************************************/

enkf_config_node_type * enkf_config_node_new_container( const char * key ) {
  enkf_config_node_type * config_node = enkf_config_node_alloc__( INVALID , CONTAINER , key , false);
  config_node->data = container_config_alloc( key );
  return config_node;
}

void enkf_config_node_update_container( enkf_config_node_type * config_node , const enkf_config_node_type * child_node) {
  vector_append_ref( config_node->container_nodes , child_node );
}                                       
  
const char * enkf_config_node_iget_container_key( const enkf_config_node_type * config_node , int index) {
  const enkf_config_node_type * child_node = vector_iget_const( config_node->container_nodes , index );
  return child_node->key;
}
                                     

/*****************************************************************/

/**
   This will create a new gen_kw_config instance which is NOT yet
   valid. Mainly support code for the GUI.
*/
enkf_config_node_type * enkf_config_node_new_field( const char * key , ecl_grid_type * ecl_grid, field_trans_table_type * trans_table, bool forward_init) {
  enkf_config_node_type * config_node = enkf_config_node_alloc__( INVALID , FIELD , key , forward_init);
  config_node->data = field_config_alloc_empty( key , ecl_grid , trans_table );
  return config_node;
}



/**
   This is for dynamic ECLIPSE fields like PRESSURE and SWAT; they
   only have truncation as possible parameters.
*/
void enkf_config_node_update_state_field( enkf_config_node_type * config_node , int truncation , double value_min , double value_max ) {
  config_node->var_type = DYNAMIC_STATE;
  field_config_update_state_field( config_node->data , truncation , value_min , value_max );
  enkf_config_node_update( config_node , NULL , NULL , NULL , NULL );
}



void enkf_config_node_update_parameter_field( enkf_config_node_type * config_node , 
                                              const char * enkf_outfile_fmt , 
                                              const char * init_file_fmt , 
                                              const char * min_std_file , 
                                              int truncation , double value_min , double value_max ,
                                              const char * init_transform , 
                                              const char * output_transform ) {

  field_file_format_type export_format = field_config_default_export_format( enkf_outfile_fmt ); /* Purely based on extension, recognizes ROFF and GRDECL, the rest will be ecl_kw format. */
  field_config_update_parameter_field( config_node->data , truncation , value_min , value_max ,
                                       export_format , 
                                       init_transform , 
                                       output_transform );
  config_node->var_type = PARAMETER;
  enkf_config_node_update( config_node , init_file_fmt , enkf_outfile_fmt , NULL , min_std_file);
}





/*****************************************************************/


void enkf_config_node_update_general_field( enkf_config_node_type * config_node , 
                                            const char * enkf_outfile_fmt        , 
                                            const char * enkf_infile_fmt         , 
                                            const char * init_file_fmt           , 
                                            const char * min_std_file            , 
                                            int truncation                       ,
                                            double value_min                     , 
                                            double value_max                     ,            
                                            const char * init_transform          ,
                                            const char * input_transform         ,
                                            const char * output_transform ) {       

  
  field_file_format_type export_format = field_config_default_export_format( enkf_outfile_fmt ); /* Purely based on extension, recognizes ROFF and GRDECL, the rest will be ecl_kw format. */
  {
    enkf_var_type var_type;
    if (enkf_infile_fmt == NULL)
      var_type = PARAMETER;
    else {
      if (enkf_outfile_fmt == NULL)
        var_type = DYNAMIC_RESULT;   /* Probably not very realistic */
      else
        var_type = DYNAMIC_STATE;
    }
    config_node->var_type = var_type;
  }
  field_config_update_general_field( config_node->data , 
                                     truncation , value_min , value_max ,
                                     export_format , 
                                     init_transform , 
                                     input_transform , 
                                     output_transform );

  enkf_config_node_update( config_node , init_file_fmt , enkf_outfile_fmt , enkf_infile_fmt, min_std_file);
}

  




/*****************************************************************/


void enkf_config_node_update_gen_data( enkf_config_node_type * config_node, 
                                       gen_data_file_format_type input_format,
                                       gen_data_file_format_type output_format,
                                       const char * init_file_fmt           , 
                                       const char * template_ecl_file       , 
                                       const char * template_data_key       ,
                                       const char * enkf_outfile_fmt        , 
                                       const char * enkf_infile_fmt         , 
                                       const char * min_std_file) {

  {
    enkf_var_type var_type = INVALID_VAR;
    /*
      The var type parameter is determined by inspecting the
      combination of input parameters. It is possible to specify an
      invalid input combination; that should be identified with a call
      to gen_data_config_is_valid() in the calling scope.


      PARAMETER:      init_file_fmt    != NULL
                      enkf_outfile_fmt != NULL
                      enkf_infile_fmt  == NULL

      DYNAMIC_STATE:  init_file_fmt    != NULL
                      enkf_outfile_fmt != NULL
                      enkf_infile_fmt  != NULL

      DYNAMIC_RESULT: init_file_fmt    == NULL
                      enkf_outfile_fmt == NULL
                      enkf_infile_fmt  != NULL                

    */
    
    if ((init_file_fmt != NULL) && (enkf_outfile_fmt != NULL) && (enkf_infile_fmt == NULL)) var_type = PARAMETER;

    if ((init_file_fmt != NULL) && (enkf_outfile_fmt != NULL) && (enkf_infile_fmt != NULL)) var_type = DYNAMIC_STATE;

    if ((init_file_fmt == NULL) && (enkf_outfile_fmt == NULL) && (enkf_infile_fmt != NULL)) var_type = DYNAMIC_RESULT;

    config_node->var_type = var_type;  /* Can be stuck with INVALID_VAR */
  }
  
  if (config_node->var_type != INVALID_VAR) {
    gen_data_config_update(config_node->data ,           /* Special update */ 
                           config_node->var_type , 
                           input_format , 
                           output_format ,          
                           template_ecl_file , 
                           template_data_key);
    
    enkf_config_node_update( config_node ,               /* Generic update - needs the format settings from the special.*/
                             init_file_fmt ,
                             enkf_outfile_fmt , 
                             enkf_infile_fmt, 
                             min_std_file); 
  }
  
}

/*****************************************************************/                   

enkf_config_node_type * enkf_config_node_container_iget( const enkf_config_node_type * node , int index) {
  return vector_iget( node->container_nodes , index );
}

int enkf_config_node_container_size( const enkf_config_node_type * node ) {
  return vector_get_size( node->container_nodes );
}


/*****************************************************************/

/**
   Invokes the get_data_size() function of the underlying node object.
*/

int enkf_config_node_get_data_size( const enkf_config_node_type * node , int report_step) {
  if (node->impl_type == GEN_DATA)
    return gen_data_config_get_data_size( node->data , report_step);
  else
    return node->get_data_size( node->data );
}

void enkf_config_node_free(enkf_config_node_type * node) {
  /* Freeing the underlying node object. */
  if (node->freef   != NULL) node->freef(node->data);
  free(node->key);
  stringlist_free(node->obs_keys);

  if (node->enkf_infile_fmt != NULL) 
    path_fmt_free( node->enkf_infile_fmt );

  if (node->enkf_outfile_fmt != NULL) 
    path_fmt_free( node->enkf_outfile_fmt );
  
  if (node->init_file_fmt != NULL)
    path_fmt_free( node->init_file_fmt );

  if (node->internalize != NULL)
    bool_vector_free( node->internalize );
  
  if (node->min_std != NULL)
    enkf_node_free( node->min_std );

  vector_free( node->container_nodes );
  free(node);
}



const enkf_node_type * enkf_config_node_get_min_std( const enkf_config_node_type * config_node ) {
  return config_node->min_std;
}

const char * enkf_config_node_get_min_std_file( const enkf_config_node_type * config_node ) {
  return config_node->min_std_file;
}


const char * enkf_config_node_get_enkf_outfile( const enkf_config_node_type * config_node ) {
  return path_fmt_get_fmt( config_node->enkf_outfile_fmt );
}

const char * enkf_config_node_get_enkf_infile( const enkf_config_node_type * config_node ) {
  return path_fmt_get_fmt( config_node->enkf_infile_fmt );
}

const char * enkf_config_node_get_init_file_fmt( const enkf_config_node_type * config_node)
{
  return path_fmt_get_fmt( config_node->init_file_fmt );
}

void enkf_config_node_set_min_std( enkf_config_node_type * config_node , enkf_node_type * min_std ) {
  if (config_node->min_std != NULL)
    enkf_node_free( config_node->min_std );
  
  config_node->min_std = min_std;
}


void enkf_config_node_set_internalize(enkf_config_node_type * node, int report_step) {
  ert_impl_type impl_type = enkf_config_node_get_impl_type( node );
  if (impl_type == CONTAINER) {
    int inode;
    int container_size = enkf_config_node_container_size( node );
    for (inode = 0; inode < container_size; inode++) {
      enkf_config_node_type * child_node = enkf_config_node_container_iget( node , inode );
      enkf_config_node_set_internalize( child_node , report_step );
    }
  } else {
    if (node->internalize == NULL)
      node->internalize = bool_vector_alloc( 0 , false );
    bool_vector_iset( node->internalize , report_step , true);
  }
}


void enkf_config_node_init_internalization(enkf_config_node_type * node) {
  if (node->internalize != NULL)
    bool_vector_reset( node->internalize );
}


/* Query function: */
bool enkf_config_node_internalize(const enkf_config_node_type * node, int report_step) {
  if (node->internalize == NULL)
    return false;
  else
    return bool_vector_safe_iget( node->internalize , report_step); /* Will return default value if report_step is beyond size. */
}



/**
   This is the filename used when loading from a completed forward
   model.
*/

char * enkf_config_node_alloc_infile(const enkf_config_node_type * node , int report_step) {
  if (node->enkf_infile_fmt != NULL)
    return path_fmt_alloc_path(node->enkf_infile_fmt , false , report_step);
  else
    return NULL;
}


char * enkf_config_node_alloc_outfile(const enkf_config_node_type * node , int report_step) {
  if (node->enkf_outfile_fmt != NULL)
    return path_fmt_alloc_path(node->enkf_outfile_fmt , false , report_step);
  else
    return NULL;
}

/*
  The path argument is used when the function is during forward_model
  based initialisation.  
*/

char * enkf_config_node_alloc_initfile( const enkf_config_node_type * node , const char * path , int iens) {
  if (node->init_file_fmt == NULL)
    return NULL;
  else {
    char * file = path_fmt_alloc_file( node->init_file_fmt , false , iens );
    if (util_is_abs_path( file ))
      return file;
    else {
      char * full_path = util_alloc_filename( path , file , NULL );
      free( file );
      return full_path;
    }
  }
}




void *  enkf_config_node_get_ref(const enkf_config_node_type * node) { 
  return node->data; 
}



bool enkf_config_node_include_type(const enkf_config_node_type * config_node , int mask) {
  
  enkf_var_type var_type = config_node->var_type;
  if (var_type & mask)
    return true;
  else
    return false;

}


bool enkf_config_node_use_forward_init(const enkf_config_node_type * config_node) {
  return config_node->forward_init;
}


ert_impl_type enkf_config_node_get_impl_type(const enkf_config_node_type *config_node) { 
  return config_node->impl_type; 
}


enkf_var_type enkf_config_node_get_var_type(const enkf_config_node_type *config_node) { 
  return config_node->var_type; 
}


const char * enkf_config_node_get_key(const enkf_config_node_type * config_node) { return config_node->key; }


const stringlist_type  * enkf_config_node_get_obs_keys(const enkf_config_node_type *config_node) {
  return config_node->obs_keys;
}


int enkf_config_node_get_num_obs( const enkf_config_node_type * config_node ) {
  return stringlist_get_size( config_node->obs_keys );
}


/**
   This checks the index_key - and sums up over all the time points of the observation.
*/

int enkf_config_node_load_obs( const enkf_config_node_type * config_node , enkf_obs_type * enkf_obs ,const char * key_index , int obs_count , time_t * _sim_time , double * _y , double * _std) {
  ert_impl_type impl_type = enkf_config_node_get_impl_type(config_node);
  int num_obs = 0;
  int iobs;

  for (iobs = 0; iobs < stringlist_get_size( config_node->obs_keys ); iobs++) {
    obs_vector_type * obs_vector = enkf_obs_get_vector( enkf_obs , stringlist_iget( config_node->obs_keys , iobs));
    
    int report_step = -1;
    while (true) {
      report_step = obs_vector_get_next_active_step( obs_vector , report_step);
      if (report_step == -1) break;
      {
        bool valid;
        double value , std1;

        /**
           The user index used when calling the user_get function on the
           gen_obs data type is different depending on whether is called with a
           data context user_key (as here) or with a observation context
           user_key (as when plotting an observation plot). See more
           documentation of the function gen_obs_user_get_data_index(). 
        */

        if (impl_type == GEN_DATA)
          gen_obs_user_get_with_data_index( obs_vector_iget_node( obs_vector , report_step ) , key_index , &value , &std1 , &valid);
        else
          obs_vector_user_get( obs_vector , key_index , report_step , &value , &std1 , &valid);
        
        if (valid) {
          if (obs_count > 0) {
            _sim_time[num_obs] = enkf_obs_iget_obs_time( enkf_obs , report_step );
            _y[num_obs]        = value;
            _std[num_obs]      = std1;
          }
          num_obs++;
        }
      }
    }
  }

  /* Sorting the observations in time order. */
  if (obs_count > 0) {
    double_vector_type * y        = double_vector_alloc_shared_wrapper( 0 , 0 , _y        , obs_count );
    double_vector_type * std      = double_vector_alloc_shared_wrapper( 0 , 0 , _std      , obs_count );
    time_t_vector_type * sim_time = time_t_vector_alloc_shared_wrapper( 0 , 0 , _sim_time , obs_count );
    int * sort_perm               = time_t_vector_alloc_sort_perm( sim_time );
    
    time_t_vector_permute( sim_time , sort_perm );
    double_vector_permute( y        , sort_perm );
    double_vector_permute( std      , sort_perm );
    
    free( sort_perm );
    double_vector_free( y );
    double_vector_free( std );
    time_t_vector_free( sim_time );
  }
  return num_obs;
}



void enkf_config_node_add_obs_key(enkf_config_node_type * config_node , const char * obs_key) {
  if (!stringlist_contains(config_node->obs_keys , obs_key))
    stringlist_append_copy(config_node->obs_keys , obs_key);
}


void enkf_config_node_clear_obs_keys(enkf_config_node_type * config_node) {
  stringlist_clear( config_node->obs_keys );
}

/*****************************************************************/



void enkf_config_node_fprintf_config( const enkf_config_node_type * config_node , FILE * stream ) {
  switch( config_node->impl_type) {
  case(GEN_KW):
    fprintf( stream , CONFIG_KEY_FORMAT   , GEN_KW_KEY );
    fprintf( stream , CONFIG_VALUE_FORMAT , config_node->key );
    gen_kw_config_fprintf_config( config_node->data , path_fmt_get_fmt( config_node->enkf_outfile_fmt ) , config_node->min_std_file , stream );
    break;
  case(FIELD):
    fprintf( stream , CONFIG_KEY_FORMAT   , FIELD_KEY );
    fprintf( stream , CONFIG_VALUE_FORMAT , config_node->key );
    field_config_fprintf_config( config_node->data     , 
                                 config_node->var_type , 
                                 path_fmt_get_fmt( config_node->enkf_outfile_fmt ) , 
                                 path_fmt_get_fmt( config_node->enkf_infile_fmt ) , 
                                 config_node->min_std_file , 
                                 stream );
    break;
  case(GEN_DATA):

    if (config_node->var_type == PARAMETER)
      fprintf( stream , CONFIG_KEY_FORMAT , GEN_PARAM_KEY );
    else
      fprintf( stream , CONFIG_KEY_FORMAT , GEN_DATA_KEY );
    
    gen_data_config_fprintf_config( config_node->data     , 
                                    config_node->var_type ,
                                    path_fmt_get_fmt( config_node->enkf_outfile_fmt ) , 
                                    path_fmt_get_fmt( config_node->enkf_infile_fmt ) , 
                                    config_node->min_std_file , 
                                    stream );
    break;
  default:
    util_abort("%s: internal error - function can not store configuration for: %s variables. \n",__func__ , enkf_types_get_impl_name( config_node->impl_type) );
  }
  fprintf( stream , "\n");
}


/*****************************************************************/
UTIL_SAFE_CAST_FUNCTION( enkf_config_node , ENKF_CONFIG_NODE_TYPE_ID)
VOID_FREE(enkf_config_node)
