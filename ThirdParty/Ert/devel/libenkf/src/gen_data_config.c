/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'gen_data_config.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <ert/util/util.h>
#include <ert/util/int_vector.h>
#include <ert/util/bool_vector.h>

#include <ert/config/config.h>

#include <ert/ecl/ecl_util.h>

#include <ert/enkf/enkf_macros.h>
#include <ert/enkf/gen_data_config.h>
#include <ert/enkf/enkf_types.h>
#include <ert/enkf/gen_data_common.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/config_keys.h>
#include <ert/enkf/enkf_defaults.h>

/**
   About deactivating by the forward model
   ---------------------------------------

   For the gen_data instances the forward model has the capability to
   deactivate elements in a gen_data vector. This is implemented in
   the function gen_data_ecl_load which will look for a file with
   extension "_data" and then activate / deactivate elements
   accordingly.
*/




#define GEN_DATA_CONFIG_ID 90051
struct gen_data_config_struct {
  CONFIG_STD_FIELDS;
  char                         * key;                   /* The key this gen_data instance is known under - needed for debugging. */
  ecl_type_enum                  internal_type;         /* The underlying type (float | double) of the data in the corresponding gen_data instances. */
  char                         * template_file;        
  char                         * template_buffer;       /* Buffer containing the content of the template - read and internalized at boot time. */
  char                         * template_key;
  int                            template_data_offset;  /* The offset into the template buffer before the data should come. */
  int                            template_data_skip;    /* The length of data identifier in the template.*/ 
  int                            template_buffer_size;  /* The total size (bytes) of the template buffer .*/
  gen_data_file_format_type      input_format;          /* The format used for loading gen_data instances when the forward model has completed *AND* for loading the initial files.*/
  gen_data_file_format_type      output_format;         /* The format used when gen_data instances are written to disk for the forward model. */
  int_vector_type              * data_size_vector;      /* Data size, i.e. number of elements , indexed with report_step */
  bool                           update_valid; 
  pthread_mutex_t                update_lock;  
  /*****************************************************************/
  /* All the fields below this line are related to the capability of
     the forward model to deactivate elements in a gen_data
     instance. See documentation above.
  */
  bool                           dynamic;
  enkf_fs_type                 * fs;                   /* NBNB This will be NULL in the case of instances which are used as parameters. */
  int                            ens_size;     
  bool                           mask_modified;
  bool_vector_type             * active_mask;
  int                            active_report_step;
};


/*****************************************************************/

UTIL_SAFE_CAST_FUNCTION(gen_data_config , GEN_DATA_CONFIG_ID)
UTIL_SAFE_CAST_FUNCTION_CONST(gen_data_config , GEN_DATA_CONFIG_ID)

gen_data_file_format_type gen_data_config_get_input_format ( const gen_data_config_type * config) { return config->input_format; }
gen_data_file_format_type gen_data_config_get_output_format( const gen_data_config_type * config) { return config->output_format; }


ecl_type_enum gen_data_config_get_internal_type(const gen_data_config_type * config) {
  return config->internal_type;
}


/**
   If current_size as queried from config->data_size_vector == -1
   (i.e. not set); we seek through 
*/

int gen_data_config_get_data_size( const gen_data_config_type * config , int report_step) {
  int current_size = int_vector_safe_iget( config->data_size_vector , report_step );
  if (current_size < 0) 
    util_abort("%s: Size not set for object:%s report_step:%d - internal error: \n",__func__ , config->key , report_step);
  return current_size; 
}



int gen_data_config_get_initial_size( const gen_data_config_type * config ) {
  int initial_size = int_vector_safe_iget( config->data_size_vector , 0);
  if (initial_size < 0)
    initial_size = 0;
  
  return initial_size;
}



int gen_data_config_get_byte_size( const gen_data_config_type * config , int report_step) {
  int byte_size = gen_data_config_get_data_size( config , report_step ) * ecl_util_get_sizeof_ctype( config->internal_type );
  return byte_size;
}



gen_data_config_type * gen_data_config_alloc_empty( const char * key ) {
  gen_data_config_type * config = util_malloc(sizeof * config );
  UTIL_TYPE_ID_INIT( config , GEN_DATA_CONFIG_ID);

  config->key               = util_alloc_string_copy( key );

  config->template_file        = NULL;
  config->template_key         = NULL;
  config->template_buffer      = NULL;
  config->template_data_offset = 0;
  config->template_buffer_size = 0;
  config->template_data_skip   = 0;

  config->data_size          = 0;
  config->internal_type      = ECL_DOUBLE_TYPE;
  config->input_format       = GEN_DATA_UNDEFINED;
  config->output_format      = GEN_DATA_UNDEFINED;
  config->data_size_vector   = int_vector_alloc( 0 , -1 );   /* The default value: -1 - indicates "NOT SET" */
  config->update_valid       = false;
  config->active_mask        = bool_vector_alloc(0 , true ); /* Elements are explicitly set to FALSE - this MUST default to true. */ 
  config->active_report_step = -1;
  config->ens_size           = -1;
  config->fs                 = NULL;
  config->dynamic            = false;
  pthread_mutex_init( &config->update_lock , NULL );

  return config;
}



const bool_vector_type * gen_data_config_get_active_mask( const gen_data_config_type * config ) {
  if (config->dynamic)
    return config->active_mask;
  else
    return NULL;     /* GEN_PARAM instance will never be deactivated by the forward model. */
}


static void gen_data_config_set_template( gen_data_config_type * config , const char * template_ecl_file , const char * template_data_key ) {
  util_safe_free( config->template_buffer ); 
  config->template_buffer = NULL;

  if (template_ecl_file != NULL) {
    char *data_ptr;
    config->template_buffer = util_fread_alloc_file_content( template_ecl_file , &config->template_buffer_size);
    if (template_data_key != NULL) {
      data_ptr = strstr(config->template_buffer , template_data_key);
      if (data_ptr == NULL) 
        util_abort("%s: template:%s can not be used - could not find data key:%s \n",__func__ , template_ecl_file , template_data_key);
      else {
        config->template_data_offset = data_ptr - config->template_buffer;
        config->template_data_skip   = strlen( template_data_key );
      }
    } else { /* We are using a template without a template_data_key - the
                data is assumed to come at the end of the template. */
      config->template_data_offset = strlen( config->template_buffer );
      config->template_data_skip   = 0;
    }
  } else 
    config->template_buffer = NULL;
  
  config->template_file = util_realloc_string_copy( config->template_file , template_ecl_file );
  config->template_key  = util_realloc_string_copy( config->template_key , template_data_key );
}


const char * gen_data_config_get_template_file( const gen_data_config_type * config ) {
  return config->template_file;
}

const char * gen_data_config_get_template_key( const gen_data_config_type * config ) {
  return config->template_key;   
}


static void gen_data_config_set_io_format( gen_data_config_type * config , gen_data_file_format_type output_format , gen_data_file_format_type input_format) {
  
  config->output_format = output_format;
  config->input_format  = input_format;
}


bool gen_data_config_is_valid( const gen_data_config_type * gen_data_config) {
  return gen_data_config->update_valid;
}



/**
   Observe that all the consistency checks are in this functions, and
   not in the various small static functions called by this function,
   it is therefor important that only this full function is used, and
   not the small individual (static for a reason ...) functions.

   Observe that the checks on == NULL and != NULL for the various parameters
   should already have been performed (in enkf_config_node_update_gen_data).
*/

void gen_data_config_update(gen_data_config_type * config           , 
                            enkf_var_type var_type                  , /* This is ONLY included too be able to do a sensible consistency check. */
                            gen_data_file_format_type input_format  ,
                            gen_data_file_format_type output_format ,
                            const char * template_ecl_file          , 
                            const char * template_data_key) {
  bool valid = true;
  
  if ((var_type != DYNAMIC_RESULT) && (output_format == GEN_DATA_UNDEFINED))
    valid = false;
    //util_abort("%s: When specifying an enkf output file you must specify an output format as well. \n",__func__);
  
  if ((var_type != PARAMETER) && (input_format == GEN_DATA_UNDEFINED))
    valid = false;
    //util_abort("%s: When specifying a file for ERT to load from - you must also specify the format of the loaded files. \n",__func__);
  
  if (input_format == ASCII_TEMPLATE)
    valid = false;
    //util_abort("%s: Format ASCII_TEMPLATE is not valid as INPUT_FORMAT \n",__func__);

  if ((template_ecl_file != NULL) && (template_data_key == NULL))
    valid = false;
    //util_abort("%s: When using a template you MUST also set a template_key \n",__func__);

  if ((output_format == ASCII_TEMPLATE) && (template_ecl_file == NULL))
    valid = false;
  // When using format ASCII_TEMPLATE you also must set a template file. 

  /*****************************************************************/
  if (valid) {
    gen_data_config_set_template( config , template_ecl_file , template_data_key );
    gen_data_config_set_io_format( config , output_format , input_format);
    
    if ((output_format == ASCII_TEMPLATE) && (config->template_buffer == NULL))
      valid = false;
    //util_abort("%s: When specifying output_format == ASCII_TEMPLATE you must also supply a template_ecl_file\n",__func__);
  }
  config->update_valid = valid;
}




/**
   This function takes a string representation of one of the
   gen_data_file_format_type values, and returns the corresponding
   integer value.
   
   Will return gen_data_undefined if the string is not recognized,
   calling scope must check on this return value.
*/


gen_data_file_format_type gen_data_config_check_format( const void * format_string ) {
  gen_data_file_format_type type = GEN_DATA_UNDEFINED;
  
  if (format_string != NULL) {
    
    if (strcmp(format_string , "ASCII") == 0)
      type = ASCII;
    else if (strcmp(format_string , "ASCII_TEMPLATE") == 0)
      type = ASCII_TEMPLATE;
    else if (strcmp(format_string , "BINARY_DOUBLE") == 0)
      type = BINARY_DOUBLE;
    else if (strcmp(format_string , "BINARY_FLOAT") == 0)
      type = BINARY_FLOAT;
    
    if (type == GEN_DATA_UNDEFINED)
      util_exit("Sorry: format:\"%s\" not recognized - valid values: ASCII / ASCII_TEMPLATE / BINARY_DOUBLE / BINARY_FLOAT \n", format_string);
  }
  
  return type;
}


/**
   The valid options are:

   INPUT_FORMAT:(ASCII|ASCII_TEMPLATE|BINARY_DOUBLE|BINARY_FLOAT)
   OUTPUT_FORMAT:(ASCII|ASCII_TEMPLATE|BINARY_DOUBLE|BINARY_FLOAT)
   TEMPLATE:/some/template/file
   KEY:<SomeKeyFoundInTemplate>
   ECL_FILE:<filename to write EnKF ==> Forward model>  (In the case of gen_param - this is extracted in the calling scope).
   RESULT_FILE:<filename to read EnKF <== Forward model> 

*/




void gen_data_config_free(gen_data_config_type * config) {
  int_vector_free( config->data_size_vector );
  
  util_safe_free( config->key );
  util_safe_free( config->template_buffer );
  util_safe_free( config->template_file );
  util_safe_free( config->template_key );
  bool_vector_free( config->active_mask );
  
  free(config);
}




/**
   This function gets a size (from a gen_data) instance, and verifies
   that the size agrees with the currently stored size and
   report_step. If the report_step is new we just record the new info,
   otherwise it will break hard.  
*/


/**
   Does not work properly with:
   
   1. keep_run_path - the load_file will be left hanging around - and loaded again and again.
   2. Doing forward several steps - how to (time)index the files?
     
*/


void gen_data_config_assert_size(gen_data_config_type * config , int data_size, int report_step) {
  pthread_mutex_lock( &config->update_lock );
  {
    int current_size = int_vector_safe_iget( config->data_size_vector , report_step );
    if (current_size < 0) {
      int_vector_iset( config->data_size_vector , report_step , data_size );
      current_size = data_size;
    }
    
    if (current_size != data_size) {
      util_abort("%s: Size mismatch when loading:%s from file - got %d elements - expected:%d [report_step:%d] \n",
                 __func__ , 
                 gen_data_config_get_key( config ),
                 data_size , 
                 current_size , 
                 report_step);
    }
  }
  pthread_mutex_unlock( &config->update_lock );
}

/**
   When the forward model is creating results for GEN_DATA instances,
   it can optionally signal that not all elements in the gen_data
   should be active (i.e. the forward model failed in some way); that
   is handled through this function. When all ensemble members have
   called this function the mask config->active_mask should be true
   ONLY for the elements which are true for all members.
   
   This MUST be called after gen_data_config_assert_size(). 
*/

void gen_data_config_update_active(gen_data_config_type * config , int report_step , const bool_vector_type * data_mask) {
  pthread_mutex_lock( &config->update_lock );
  {
    if ( int_vector_iget( config->data_size_vector , report_step ) > 0) {
      if (config->active_report_step != report_step) {
        /* This is the first ensemeble member loading for this
           particular report_step. */
        bool_vector_reset( config->active_mask );
        bool_vector_iset( config->active_mask , int_vector_iget( config->data_size_vector , report_step ) - 1 , true );
        config->mask_modified = true;
      }
      
      {
        int i;
        for (i=0; i < bool_vector_size( data_mask ); i++) {
          if (!bool_vector_iget( data_mask , i )) {
            bool_vector_iset( config->active_mask , i , false );
            config->mask_modified = true;
          }
        }
      } 
      
      if (config->mask_modified) {
        /**
           The global mask has been modified after the last load;
           i.e. we update the on-disk representation.
        */
        char * filename = util_alloc_sprintf("%s_active" , config->key );
        FILE * stream   = enkf_fs_open_case_tstep_file( config->fs , filename , report_step , "w");
        
        bool_vector_fwrite( config->active_mask , stream );

        fclose( stream );
        free( filename );
        config->mask_modified = false;
      }
    }
    config->active_report_step = report_step;
  }
  pthread_mutex_unlock( &config->update_lock );
}



/**
   This function will load an active map from the enkf_fs filesystem.
*/
void gen_data_config_load_active( gen_data_config_type * config , int report_step , bool force_load) {
  if (config->fs == NULL)
    return;                /* This is used as a GEN_PARAM instance - and the loading of mask is not an option. */
  
  
  pthread_mutex_lock( &config->update_lock );
  {
    if ( force_load || (int_vector_iget( config->data_size_vector , report_step ) > 0)) {
      if (config->active_report_step != report_step) {
        char * filename = util_alloc_sprintf("%s_active" , config->key );
        FILE * stream   = enkf_fs_open_excase_tstep_file( config->fs , filename , report_step);

        if (stream != NULL) {
          bool_vector_fread( config->active_mask , stream );
          fclose( stream );
        } else 
          fprintf(stderr,"** Warning: could not find file:%s \n",filename);

        free( filename );
      }
    }
    config->active_report_step = report_step;
  }
  pthread_mutex_unlock( &config->update_lock );
}



void gen_data_config_set_ens_size( gen_data_config_type * config , int ens_size) {
  config->ens_size = ens_size;
}


void gen_data_config_set_dynamic( gen_data_config_type * config , enkf_fs_type * fs) {
  config->dynamic = true;
  config->fs      = fs;
}


bool gen_data_config_is_dynamic( const gen_data_config_type * config ) {
  return config->dynamic;
}

void gen_data_config_get_template_data( const gen_data_config_type * config , 
                                        char ** template_buffer    , 
                                        int * template_data_offset , 
                                        int * template_buffer_size , 
                                        int * template_data_skip) {
  
  *template_buffer      = config->template_buffer;
  *template_data_offset = config->template_data_offset;
  *template_buffer_size = config->template_buffer_size;
  *template_data_skip   = config->template_data_skip;
  
}






const char * gen_data_config_get_key( const gen_data_config_type * config) {
  return config->key;
}

static const char * gen_data_config_format_name( gen_data_file_format_type format_type) {
  switch (format_type ) {
  case GEN_DATA_UNDEFINED:
    return "UNDEFINED";
    break;
  case ASCII:
    return "ASCII";
    break;
  case ASCII_TEMPLATE:
    return "ASCII_TEMPLATE";
    break;
  case BINARY_FLOAT:
    return "BINARY_FLOAT";
    break;
  case BINARY_DOUBLE:
    return "BINARY_DOUBLE";
    break;
  default:
    util_abort("%s: What the f.. \n",__func__);
    return NULL;
  }
}


void gen_data_config_fprintf_config( const gen_data_config_type * config , enkf_var_type var_type , const char * outfile , const char * infile , 
                                     const char * min_std_file , FILE * stream) {
  if (var_type == PARAMETER) 
    fprintf( stream , CONFIG_VALUE_FORMAT , outfile );
  else
    fprintf( stream , CONFIG_OPTION_FORMAT , ECL_FILE_KEY , outfile );
  
  if (min_std_file != NULL)
    fprintf( stream , CONFIG_OPTION_FORMAT , MIN_STD_KEY , min_std_file );

  if (config->template_file != NULL)
    fprintf( stream , CONFIG_OPTION_FORMAT , TEMPLATE_KEY , config->template_file );

  if (config->template_key != NULL)
    fprintf( stream , CONFIG_OPTION_FORMAT , KEY_KEY , config->template_key );

  if (infile != NULL)
    fprintf( stream , CONFIG_OPTION_FORMAT , RESULT_FILE_KEY , infile );

  if (config->input_format != GEN_DATA_UNDEFINED)
    fprintf( stream , CONFIG_OPTION_FORMAT , INPUT_FORMAT_KEY , gen_data_config_format_name( config->input_format ));

  if (config->output_format != GEN_DATA_UNDEFINED)
    fprintf( stream , CONFIG_OPTION_FORMAT , OUTPUT_FORMAT_KEY , gen_data_config_format_name( config->output_format ));
}



/*****************************************************************/

VOID_FREE(gen_data_config)
