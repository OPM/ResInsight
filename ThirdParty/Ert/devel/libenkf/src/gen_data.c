/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'gen_data.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdbool.h>
#include <math.h>

#include <ert/util/util.h>
#include <ert/util/matrix.h>
#include <ert/util/log.h>
#include <ert/util/bool_vector.h>
#include <ert/util/rng.h>

#include <ert/ecl/fortio.h>
#include <ert/ecl/ecl_sum.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_util.h>

#include <ert/enkf/enkf_serialize.h>
#include <ert/enkf/enkf_types.h>
#include <ert/enkf/enkf_macros.h>
#include <ert/enkf/enkf_util.h>
#include <ert/enkf/gen_data_config.h>
#include <ert/enkf/gen_data.h>
#include <ert/enkf/gen_data_common.h>
#include <ert/enkf/gen_common.h>

/**
   The file implements a general data type which can be used to update
   arbitrary data which the EnKF system has *ABSOLUTELY NO IDEA* of
   how is organised; how it should be used in the forward model and so
   on. Similarly to the field objects, the gen_data objects can be
   treated both as parameters and as dynamic data.

   Whether the forward_load function should be called (i.e. it is dynamic
   data) is determined at the enkf_node level, and no busissiness of
   the gen_data implementation.
*/
   



struct gen_data_struct {
  int                     __type_id;
  gen_data_config_type  * config;               /* Thin config object - mainly contains filename for remote load */
  char                  * data;                 /* Actual storage - will be casted to double or float on use. */
  int                     current_report_step;  /* Need this to look up the correct size in the config object. */
  bool_vector_type      * active_mask;          /* Mask of active/not active - loaded from a "_active" file created by the forward model. Not used when used as parameter*/
};



void gen_data_assert_size( gen_data_type * gen_data , int size , int report_step) {
  gen_data_config_assert_size(gen_data->config , size , report_step);
  gen_data->current_report_step = report_step;
}




gen_data_config_type * gen_data_get_config(const gen_data_type * gen_data) { return gen_data->config; }

int gen_data_get_size( const gen_data_type * gen_data ) {
  return gen_data_config_get_data_size( gen_data->config , gen_data->current_report_step );
}

/**
   It is a bug to call this before some function has set the size. 
*/
void gen_data_realloc_data(gen_data_type * gen_data) {
  int byte_size  = gen_data_config_get_byte_size(gen_data->config , gen_data->current_report_step );
  gen_data->data = util_realloc(gen_data->data , byte_size );
}



gen_data_type * gen_data_alloc(const gen_data_config_type * config) {
  gen_data_type * gen_data = util_malloc(sizeof * gen_data);
  gen_data->config              = (gen_data_config_type *) config;
  gen_data->data                = NULL;   
  gen_data->__type_id           = GEN_DATA;
  gen_data->active_mask         = bool_vector_alloc( 0 , true );
  gen_data->current_report_step = -1;  /* God - if you ever read this .... */
  return gen_data;
}


void gen_data_copy(const gen_data_type * src , gen_data_type * target) {
  if (src->config == target->config) {
    target->current_report_step = src->current_report_step;
    
    if (src->data != NULL) {
      int byte_size  = gen_data_config_get_byte_size( src->config , src->current_report_step );
      target->data   = util_realloc_copy(target->data , src->data , byte_size );
    }
  } else
    util_abort("%s: do not share config object \n",__func__);
}
  


void gen_data_free(gen_data_type * gen_data) {
  util_safe_free(gen_data->data);
  bool_vector_free( gen_data->active_mask );
  free(gen_data);
}




/**
   Observe that this function writes parameter size to disk, that is
   special. The reason is that the config object does not know the
   size (on allocation).

   The function currently writes an empty file (with only a report
   step and a size == 0) in the case where it does not have data. This
   is controlled by the value of the variable write_zero_size; if this
   is changed to false some semantics in the load code must be
   changed.
*/


bool gen_data_write_to_buffer(const gen_data_type * gen_data , buffer_type * buffer , int report_step , state_enum state) {
  const bool write_zero_size = true; /* true:ALWAYS write a file   false:only write files with size > 0. */
  {
    bool write    = write_zero_size;
    int size      = gen_data_config_get_data_size( gen_data->config , report_step );
    if (size > 0) 
      write = true;
    
    if (write) {
      int byte_size = gen_data_config_get_byte_size( gen_data->config , report_step );
      buffer_fwrite_int( buffer , GEN_DATA );
      buffer_fwrite_int( buffer , size );
      buffer_fwrite_int( buffer , report_step);   /* Why the heck do I need to store this ????  It was a mistake ...*/
      
      buffer_fwrite_compressed( buffer , gen_data->data , byte_size);
      return true;
    } else
      return false;   /* When false is returned - the (empty) file will be removed */
  }
}



void gen_data_read_from_buffer(gen_data_type * gen_data , buffer_type * buffer , int report_step, state_enum state) {
  int size;
  enkf_util_assert_buffer_type(buffer , GEN_DATA);
  size = buffer_fread_int(buffer);
  buffer_fskip_int( buffer );  /* Skipping report_step from the buffer - was a mistake to store it - I think ... */
  {
    size_t byte_size       = size * ecl_util_get_sizeof_ctype( gen_data_config_get_internal_type ( gen_data->config ));
    size_t compressed_size = buffer_get_remaining_size( buffer ); 
    gen_data->data         = util_realloc( gen_data->data , byte_size );
    buffer_fread_compressed( buffer , compressed_size , gen_data->data , byte_size );
  }
  gen_data_assert_size( gen_data , size , report_step );
  gen_data_config_load_active( gen_data->config , report_step , false );
}








void gen_data_serialize(const gen_data_type * gen_data , node_id_type node_id , const active_list_type * active_list , matrix_type * A , int row_offset , int column) {
  const gen_data_config_type *config   = gen_data->config;
  const int                data_size   = gen_data_config_get_data_size( gen_data->config , gen_data->current_report_step );
  ecl_type_enum ecl_type               = gen_data_config_get_internal_type( config );

  enkf_matrix_serialize( gen_data->data , data_size , ecl_type , active_list , A , row_offset , column );
}


void gen_data_deserialize(gen_data_type * gen_data , node_id_type node_id , const active_list_type * active_list , const matrix_type * A , int row_offset , int column) {
  {
    const gen_data_config_type *config   = gen_data->config;
    const int                data_size   = gen_data_config_get_data_size( gen_data->config , gen_data->current_report_step );
    ecl_type_enum ecl_type               = gen_data_config_get_internal_type(config);
    
    enkf_matrix_deserialize( gen_data->data , data_size , ecl_type , active_list , A , row_offset , column);
  }
}




/*
  This function sets the data field of the gen_data instance after the
  data has been loaded from file.  
*/ 

static void gen_data_set_data__(gen_data_type * gen_data , int size, int report_step , ecl_type_enum load_type , const void * data) {
  gen_data_assert_size(gen_data , size, report_step);

  if (gen_data_config_is_dynamic( gen_data->config )) 
    gen_data_config_update_active( gen_data->config , report_step , gen_data->active_mask);

  gen_data_realloc_data(gen_data);

  if (size > 0) {
    ecl_type_enum internal_type = gen_data_config_get_internal_type( gen_data->config );
    int byte_size = ecl_util_get_sizeof_ctype( internal_type ) * size ;

    if (load_type == internal_type)
      memcpy(gen_data->data , data , byte_size );
    else {
      if (load_type == ECL_FLOAT_TYPE)
        util_float_to_double((double *) gen_data->data , data , size);
      else
        util_double_to_float((float *) gen_data->data , data , size);
    }
  }

}
      
      



/**
   This functions loads data from file. Observe that there is *NO*
   header information in this file - the size is determined by seeing
   how much can be successfully loaded.

   The file is loaded with the gen_common_fload_alloc() function, and
   can be in formatted ASCII or binary_float / binary_double. 

   When the read is complete it is checked/verified with the config
   object that this file was as long as the others we have loaded for
   other members; it is perfectly OK for the file to not exist. In
   which case a size of zero is set, for this report step.

   Return value is whether file was found - might have to check this
   in calling scope.
*/

bool gen_data_fload_with_report_step( gen_data_type * gen_data , const char * filename , int report_step) {
  bool   has_file = util_file_exists(filename);
  void * buffer   = NULL;
  int    size     = 0;
  ecl_type_enum load_type;
  
  if ( has_file ) {
    ecl_type_enum internal_type            = gen_data_config_get_internal_type(gen_data->config);
    gen_data_file_format_type input_format = gen_data_config_get_input_format( gen_data->config );
    buffer = gen_common_fload_alloc( filename , input_format , internal_type , &load_type , &size);
    
    /* 
       Look for file @filename_active - if that file is found it is
       interpreted as a an active|inactive mask created by the forward
       model.

       The file is assumed to be an ASCII file with integers, 0
       indicates inactive elements and 1 active elements. The file
       should of course be as long as @filename.
       
       If the file is not found the gen_data->active_mask is set to
       all-true (i.e. the default true value is invoked).
    */
    if (gen_data_config_is_dynamic( gen_data->config )) {
      bool_vector_reset( gen_data->active_mask );  
      bool_vector_iset( gen_data->active_mask , size - 1, true );
      {
        char * active_file = util_alloc_sprintf("%s_active" , filename );
        if (util_file_exists( active_file )) {
          FILE * stream = util_fopen( active_file , "r");
          int active_int;
          for (int index=0; index < size; index++) {
            if (fscanf( stream ,  "%d" , &active_int) == 1) {
              if (active_int == 1)
                bool_vector_iset( gen_data->active_mask , index , true);
              else if (active_int == 0)
                bool_vector_iset( gen_data->active_mask , index , false);
              else
                util_abort("%s: error when loading active mask from:%s only 0 and 1 allowed \n",__func__ , active_file);
            } else
              util_abort("%s: error when loading active mask from:%s - file not long enough.\n",__func__ , active_file );
          }
          fclose( stream );
        }
        free( active_file );
      }
    }
  } 
  gen_data_set_data__(gen_data , size , report_step , load_type , buffer );
  util_safe_free(buffer);
  printf("Returning %s:%d \n",filename , has_file);
  return has_file;
}


bool gen_data_fload( gen_data_type * gen_data , const char * filename) {
  return gen_data_fload_with_report_step( gen_data , filename , 0);
}




/**
   The gen_data_forward_load() function is called by enkf_node objects
   which represent dynamic data. The gen_data objects are very weakly
   structured; and in particular we do not know in advance whether a
   particular file should be present or not, it is therefor not an
   error as such if a file can not be found. For this reason this
   function must return true unconditionally, otherwise the scalling
   scope will interpret a false return value as an error and signal
   load failure.
*/


bool gen_data_forward_load(gen_data_type * gen_data , const char * ecl_file , const ecl_sum_type * ecl_sum, const ecl_file_type * restart_file , int report_step) {
  gen_data_fload_with_report_step( gen_data , ecl_file , report_step );
  return true;
}



/**
   This function initializes the parameter. This is based on loading a
   file. The name of the file is derived from a path_fmt instance
   owned by the config object. Observe that there is *NO* header
   information in this file. We just read floating point numbers until
   we reach EOF.
   
   When the read is complete it is checked/verified with the config
   object that this file was as long as the files we have loaded for
   other members.
   
   If gen_data_config_alloc_initfile() returns NULL that means that
   the gen_data instance does not have any init function - that is OK.
*/



bool gen_data_initialize(gen_data_type * gen_data , int iens , const char * init_file , rng_type * rng) {
  if (init_file != NULL) {
    if (!gen_data_fload_with_report_step(gen_data , init_file , 0))
      util_abort("%s: could not find file:%s \n",__func__ , init_file);
    return true;
  } else
    return false; /* No init performed ... */
}





static void gen_data_ecl_write_ASCII(const gen_data_type * gen_data , const char * file , gen_data_file_format_type export_format) {
  FILE * stream   = util_fopen(file , "w");
  char * template_buffer;
  int    template_data_offset, template_buffer_size , template_data_skip;

  if (export_format == ASCII_TEMPLATE) {
    gen_data_config_get_template_data( gen_data->config , &template_buffer , &template_data_offset , &template_buffer_size , &template_data_skip);
    util_fwrite( template_buffer , 1 , template_data_offset , stream , __func__);
  }
  
  {
    ecl_type_enum internal_type = gen_data_config_get_internal_type(gen_data->config);
    const int size              = gen_data_config_get_data_size( gen_data->config , gen_data->current_report_step );
    int i;
    if (internal_type == ECL_FLOAT_TYPE) {
      float * float_data = (float *) gen_data->data;
      for (i=0; i < size; i++)
        fprintf(stream , "%g\n",float_data[i]);
    } else if (internal_type == ECL_DOUBLE_TYPE) {
      double * double_data = (double *) gen_data->data;
      for (i=0; i < size; i++)
        fprintf(stream , "%lg\n",double_data[i]);
    } else 
      util_abort("%s: internal error - wrong type \n",__func__);
  }
  
  if (export_format == ASCII_TEMPLATE) {
    int new_offset = template_data_offset + template_data_skip;
    util_fwrite( &template_buffer[new_offset] , 1 , template_buffer_size - new_offset , stream , __func__);
  }
  fclose(stream);
}



static void gen_data_ecl_write_binary(const gen_data_type * gen_data , const char * file , ecl_type_enum export_type) {
  FILE * stream    = util_fopen(file , "w");
  int sizeof_ctype = ecl_util_get_sizeof_ctype( export_type );
  util_fwrite( gen_data->data , sizeof_ctype , gen_data_config_get_data_size( gen_data->config , gen_data->current_report_step) , stream , __func__);
  fclose(stream);
}


gen_data_file_format_type gen_data_guess_export_type( const gen_data_type * gen_data ) {
  gen_data_file_format_type export_type = gen_data_config_get_output_format( gen_data->config );
  if (export_type == GEN_DATA_UNDEFINED)
    export_type = gen_data_config_get_input_format( gen_data->config );
  
  if (export_type == GEN_DATA_UNDEFINED)
    util_abort("%s: both input_format and output_format are set to UNDEFINED \n",__func__);
  return export_type;
}


void gen_data_export(const gen_data_type * gen_data , const char * full_path , gen_data_file_format_type export_type , fortio_type * fortio) {
  switch (export_type) {
  case(ASCII):
    gen_data_ecl_write_ASCII(gen_data , full_path , export_type);
    break;
  case(ASCII_TEMPLATE):
    gen_data_ecl_write_ASCII(gen_data , full_path , export_type);
    break;
  case(BINARY_DOUBLE):
    gen_data_ecl_write_binary(gen_data , full_path , ECL_DOUBLE_TYPE);
    break;
  case(BINARY_FLOAT):
    gen_data_ecl_write_binary(gen_data , full_path , ECL_FLOAT_TYPE);
    break;
  default:
    util_abort("%s: internal error - export type is not set.\n",__func__);
  }
}

/** 
    It is the enkf_node layer which knows whether the node actually
    has any data to export. If it is not supposed to write data to the
    forward model, i.e. it is of enkf_type 'dynamic_result' that is
    signaled down here with eclfile == NULL.
*/


void gen_data_ecl_write(const gen_data_type * gen_data , const char * run_path , const char * eclfile , fortio_type * fortio) {
  if (eclfile != NULL) {  
    char * full_path = util_alloc_filename( run_path , eclfile  , NULL);
    
    gen_data_file_format_type export_type = gen_data_config_get_output_format( gen_data->config );
    gen_data_export( gen_data , full_path , export_type , fortio );
    free( full_path );
  }
}


static void gen_data_assert_index(const gen_data_type * gen_data, int index) {
  int current_size = gen_data_config_get_data_size( gen_data->config , gen_data->current_report_step );
  if ((index < 0) || (index >= current_size ))
    util_abort("%s: index:%d invalid. Valid range: [0,%d) \n",__func__ , index , current_size);
}


double gen_data_iget_double(const gen_data_type * gen_data, int index) {
  gen_data_assert_index(gen_data , index); 
  {
    ecl_type_enum internal_type = gen_data_config_get_internal_type(gen_data->config);
    if (internal_type == ECL_DOUBLE_TYPE) {
      double * data = (double *) gen_data->data;
      return data[index];
    } else {
      float * data = (float *) gen_data->data;
      return data[index];
    }
  }
}



/**
   The filesystem will (currently) store gen_data instances which do
   not hold any data. Therefor it will be quite common to enter this
   function with an empty instance, we therefor just set valid =>
   false, and return silently in that case.
*/

bool gen_data_user_get(const gen_data_type * gen_data, const char * index_key, int report_step , state_enum state, double * value)
{
  int index;
  *value = 0.0;

  if (index_key != NULL) {
    if (util_sscanf_int(index_key , &index)) {
      if (index < gen_data_config_get_data_size( gen_data->config , gen_data->current_report_step )) {
        *value = gen_data_iget_double( gen_data , index );
        return true;
      }
    } 
  }
  
  return false;
}


const char * gen_data_get_key( const gen_data_type * gen_data) {
  return gen_data_config_get_key( gen_data->config );
}


void gen_data_clear( gen_data_type * gen_data ) {
  const gen_data_config_type * config = gen_data->config;
  ecl_type_enum internal_type         = gen_data_config_get_internal_type( config );
  const int data_size                 = gen_data_config_get_data_size( gen_data->config , gen_data->current_report_step );

  if (internal_type == ECL_FLOAT_TYPE) {
    float * data = (float * ) gen_data->data;
    for (int i = 0; i < data_size; i++)
      data[i] = 0;
  } else if (internal_type == ECL_DOUBLE_TYPE) {
    double * data = (double * ) gen_data->data;
    for (int i = 0; i < data_size; i++)
      data[i] = 0;
  } 
}



void gen_data_isqrt(gen_data_type * gen_data) {
  const int data_size                 = gen_data_config_get_data_size( gen_data->config , gen_data->current_report_step );
  const ecl_type_enum internal_type = gen_data_config_get_internal_type(gen_data->config);
  
  if (internal_type == ECL_FLOAT_TYPE) {
    float * data = (float *) gen_data->data;
    for (int i=0; i < data_size; i++)
      data[i] = sqrtf( data[i] );
  } else if (internal_type == ECL_DOUBLE_TYPE) {
    double * data = (double *) gen_data->data;
    for (int i=0; i < data_size; i++)
      data[i] = sqrt( data[i] );
  }
}




void gen_data_iadd(gen_data_type * gen_data1, const gen_data_type * gen_data2) {
  //gen_data_config_assert_binary(gen_data1->config , gen_data2->config , __func__); 
  {
    const int data_size                 = gen_data_config_get_data_size( gen_data1->config , gen_data1->current_report_step );
    const ecl_type_enum internal_type = gen_data_config_get_internal_type(gen_data1->config);
    int i;

    if (internal_type == ECL_FLOAT_TYPE) {
      float * data1       = (float *) gen_data1->data;
      const float * data2 = (const float *) gen_data2->data;
      for (i = 0; i < data_size; i++)
        data1[i] += data2[i];
    } else if (internal_type == ECL_DOUBLE_TYPE) {
      double * data1       = (double *) gen_data1->data;
      const double * data2 = (const double *) gen_data2->data;
      for (i = 0; i < data_size; i++) {
        data1[i] += data2[i];
      }
    }
  }
}


void gen_data_imul(gen_data_type * gen_data1, const gen_data_type * gen_data2) {
  //gen_data_config_assert_binary(gen_data1->config , gen_data2->config , __func__); 
  {
    const int data_size               = gen_data_config_get_data_size( gen_data1->config , gen_data1->current_report_step );
    const ecl_type_enum internal_type = gen_data_config_get_internal_type(gen_data1->config);
    int i;

    if (internal_type == ECL_FLOAT_TYPE) {
      float * data1       = (float *) gen_data1->data;
      const float * data2 = (const float *) gen_data2->data;
      for (i = 0; i < data_size; i++)
        data1[i] *= data2[i];
    } else if (internal_type == ECL_DOUBLE_TYPE) {
      double * data1       = (double *) gen_data1->data;
      const double * data2 = (const double *) gen_data2->data;
      for (i = 0; i < data_size; i++) 
        data1[i] *= data2[i];
    }
  }
}


void gen_data_iaddsqr(gen_data_type * gen_data1, const gen_data_type * gen_data2) {
  //gen_data_config_assert_binary(gen_data1->config , gen_data2->config , __func__); 
  {
    const int data_size               = gen_data_config_get_data_size( gen_data1->config , gen_data1->current_report_step );
    const ecl_type_enum internal_type = gen_data_config_get_internal_type(gen_data1->config);
    int i;

    if (internal_type == ECL_FLOAT_TYPE) {
      float * data1       = (float *) gen_data1->data;
      const float * data2 = (const float *) gen_data2->data;
      for (i = 0; i < data_size; i++)
        data1[i] += data2[i] * data2[i];
    } else if (internal_type == ECL_DOUBLE_TYPE) {
      double * data1       = (double *) gen_data1->data;
      const double * data2 = (const double *) gen_data2->data;
      for (i = 0; i < data_size; i++)
        data1[i] += data2[i] * data2[i];
    }
  }
}


void gen_data_scale(gen_data_type * gen_data, double scale_factor) {
  //gen_data_config_assert_unary(gen_data->config, __func__); 
  {
    const int data_size                 = gen_data_config_get_data_size( gen_data->config , gen_data->current_report_step );
    const ecl_type_enum internal_type = gen_data_config_get_internal_type(gen_data->config);
    int i;

    if (internal_type == ECL_FLOAT_TYPE) {
      float * data       = (float *) gen_data->data;
      for (i = 0; i < data_size; i++)
        data[i] *= scale_factor;
    } else if (internal_type == ECL_DOUBLE_TYPE) {
      double * data       = (double *) gen_data->data;
      for (i = 0; i < data_size; i++)
        data[i] *= scale_factor;
    }
  }
}


const bool_vector_type * gen_data_get_forward_mask( const gen_data_type * gen_data ) {
  return gen_data_config_get_active_mask( gen_data->config );
}



#define INFLATE(inf,std,min)                                                                                                                                     \
{                                                                                                                                                                \
   for (int i=0; i < data_size; i++) {                                                                                                                           \
     if (std_data[i] > 0)                                                                                                                                        \
        inflation_data[i] = util_float_max( 1.0 , min_std_data[i] / std_data[i]);                                                                                \
      else                                                                                                                                                       \
        inflation_data[i] = 1.0;                                                                                                                                 \
   }                                                                                                                                                             \
}                                                                   


/**
   If the size changes during the simulation this will go 100% belly
   up.
*/

void gen_data_set_inflation(gen_data_type * inflation , const gen_data_type * std , const gen_data_type * min_std) {
  const gen_data_config_type * config = inflation->config;
  ecl_type_enum ecl_type              = gen_data_config_get_internal_type( config );
  const int data_size                 = gen_data_config_get_data_size( std->config , std->current_report_step );

  if (ecl_type == ECL_FLOAT_TYPE) {
    float       * inflation_data = (float *)       inflation->data;
    const float * std_data       = (const float *) std->data;
    const float * min_std_data   = (const float *) min_std->data;
    
    INFLATE(inflation_data , std_data , min_std_data );
    
  } else {
    double       * inflation_data = (double *)       inflation->data;
    const double * std_data       = (const double *) std->data;
    const double * min_std_data   = (const double *) min_std->data;
    
    INFLATE(inflation_data , std_data , min_std_data );
  }
}
#undef INFLATE


/******************************************************************/
/* Anonumously generated functions used by the enkf_node object   */
/******************************************************************/
UTIL_SAFE_CAST_FUNCTION_CONST(gen_data , GEN_DATA)
UTIL_SAFE_CAST_FUNCTION(gen_data , GEN_DATA)
VOID_USER_GET(gen_data)
VOID_ALLOC(gen_data)
VOID_FREE(gen_data)
VOID_COPY     (gen_data)
VOID_INITIALIZE(gen_data)
VOID_ECL_WRITE(gen_data)
VOID_FORWARD_LOAD(gen_data)
VOID_READ_FROM_BUFFER(gen_data);
VOID_WRITE_TO_BUFFER(gen_data);
VOID_SERIALIZE(gen_data)
VOID_DESERIALIZE(gen_data)
VOID_SET_INFLATION(gen_data)
VOID_CLEAR(gen_data)
VOID_SCALE(gen_data)
VOID_IMUL(gen_data)
VOID_IADD(gen_data)
VOID_IADDSQR(gen_data)
VOID_ISQRT(gen_data)
VOID_FLOAD(gen_data)
