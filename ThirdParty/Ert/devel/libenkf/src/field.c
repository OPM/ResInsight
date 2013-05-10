/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'field.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <ert/util/util.h>
#include <ert/util/buffer.h>
#include <ert/util/rng.h>

#include <ert/ecl/fortio.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_util.h>
#include <ert/ecl/ecl_endian_flip.h>

#include <ert/rms/rms_file.h>
#include <ert/rms/rms_tagkey.h>
#include <ert/rms/rms_type.h>
#include <ert/rms/rms_util.h>

#include <ert/enkf/field.h>
#include <ert/enkf/field_config.h>
#include <ert/enkf/enkf_serialize.h>


GET_DATA_SIZE_HEADER(field);


/*****************************************************************/

/**
  The field data type contains for "something" which is distributed
  over the full grid, i.e. permeability or pressure. All configuration
  information is stored in the config object, which is of type
  field_config_type. Observe the following:

  * The field **only** contains the active cells - the config object
    has a reference to actnum information.

  * The data is stored in a char pointer; the real underlying data can
    be (at least) of the types int, float and double.
*/

struct field_struct {
  int    __type_id;                              
  const  field_config_type * config;              /* The field config object - containing information of active cells++ */
  bool   private_config;
  char  *data;                                    /* The actual storage for the field - suitabley casted to int/float/double on use*/
             
  bool   shared_data;                             /* If the data is shared - i.e. managed (xalloc & free) from another scope. */
  int    shared_byte_size;                        /* The size of the shared buffer (if it is shared). */
  char  *export_data;                             /* IFF an output transform should be applied this pointer will hold the transformed data. */
  char  *__data;                                  /* IFF an output transform, this pointer will hold the original data during the transform and export. */
};



#define EXPORT_MACRO                                                                                      \
{                                                                                                         \
  int nx,ny,nz;                                                                                           \
  field_config_get_dims(field->config , &nx , &ny , &nz);                                                 \
  int i,j,k;                                                                                              \
   for (k=0; k < nz; k++) {                                                                               \
     for (j=0; j < ny; j++) {                                                                             \
       for (i=0; i < nx; i++) {                                                                           \
         int index1D = field_config_active_index(config , i , j , k);                                     \
         int index3D;                                                                                     \
         if (rms_index_order)                                                                             \
           index3D = rms_util_global_index_from_eclipse_ijk(nx,ny,nz,i,j,k);                              \
         else                                                                                             \
           index3D = i + j * nx + k* nx*ny;                                                               \
         if (index1D >= 0)                                                                                \
           target_data[index3D] = src_data[index1D];                                                      \
         else                                                                                             \
           memcpy(&target_data[index3D] , fill_value , sizeof_ctype_target);                              \
        }                                                                                                 \
     }                                                                                                    \
   }                                                                                                      \
}                                                                                                         


void field_export3D(const field_type * field , void *_target_data , bool rms_index_order , ecl_type_enum target_type , void *fill_value) {
  const field_config_type * config = field->config;
  ecl_type_enum ecl_type = field_config_get_ecl_type( config );
  int   sizeof_ctype_target = ecl_util_get_sizeof_ctype(target_type);
  
  switch(ecl_type) {
  case(ECL_DOUBLE_TYPE):
    {
      const double * src_data = (const double *) field->data;
      if (target_type == ECL_FLOAT_TYPE) {
        float *target_data = (float *) _target_data;
        EXPORT_MACRO;
      } else if (target_type == ECL_DOUBLE_TYPE) {
        double *target_data = (double *) _target_data;
        EXPORT_MACRO;
      } else {
        fprintf(stderr,"%s: double field can only export to double/float\n",__func__);
        abort();
      }
    }
    break;
  case(ECL_FLOAT_TYPE):
    {
      const float * src_data = (const float *) field->data;
      if (target_type == ECL_FLOAT_TYPE) {
        float *target_data = (float *) _target_data;
        EXPORT_MACRO;
      } else if (target_type == ECL_DOUBLE_TYPE) {
        double *target_data = (double *) _target_data;
        EXPORT_MACRO;
      } else {
        fprintf(stderr,"%s: float field can only export to double/float\n",__func__);
        abort();
      }
    }
    break;
  case(ECL_INT_TYPE):
    {
      const int * src_data = (const int *) field->data;
      if (target_type == ECL_FLOAT_TYPE) {
        float *target_data = (float *) _target_data;
        EXPORT_MACRO;
      } else if (target_type == ECL_DOUBLE_TYPE) {
        double *target_data = (double *) _target_data;
        EXPORT_MACRO;
      } else if (target_type == ECL_INT_TYPE) {
        int *target_data = (int *) _target_data;
        EXPORT_MACRO;
      }  else {
        fprintf(stderr,"%s: int field can only export to int/double/float\n",__func__);
        abort();
      }
    }
    break;
  default:
    fprintf(stderr,"%s: Sorry field has unexportable type ... \n",__func__);
    break;
  }
}
#undef EXPORT_MACRO
  

/*****************************************************************/
#define IMPORT_MACRO                                                                                        \
{                                                                                                           \
  int i,j,k;                                                                                                \
  int nx,ny,nz;                                                                                             \
  field_config_get_dims(field->config , &nx , &ny , &nz);                                                   \
  for (k=0; k < nz; k++) {                                                                                  \
     for (j=0; j < ny; j++) {                                                                               \
       for (i=0; i < nx; i++) {                                                                             \
         int index1D = field_config_active_index(config , i , j , k);                                       \
         int index3D;                                                                                       \
         if (index1D >= 0) {                                                                                \
           if (rms_index_order)                                                                             \
             index3D = rms_util_global_index_from_eclipse_ijk(nx,ny,nz,i,j,k);                              \
           else                                                                                             \
             index3D = i + j * nx + k* nx*ny;                                                               \
           target_data[index1D] = src_data[index3D] ;                                                       \
         }                                                                                                  \
      }                                                                                                     \
     }                                                                                                      \
   }                                                                                                        \
}



/**
   The main function of the field_import3D and field_export3D
   functions are to skip the inactive cells (field_import3D) and
   distribute inactive cells (field_export3D). In addition we can
   reorganize input/output according to the RMS Roff index convention,
   and also perform float <-> double conversions.

   Observe that these functions only import/export onto memory
   buffers, the actual reading and writing of files is done in other
   functions (calling these).
*/

static void field_import3D(field_type * field , const void *_src_data , bool rms_index_order , ecl_type_enum src_type) {
  const field_config_type * config = field->config;
  ecl_type_enum ecl_type = field_config_get_ecl_type(config);
  
  switch(ecl_type) {
  case(ECL_DOUBLE_TYPE):
    {
      double * target_data = (double *) field->data;
      if (src_type == ECL_FLOAT_TYPE) {
        float *src_data = (float *) _src_data;
        IMPORT_MACRO;
      } else if (src_type == ECL_DOUBLE_TYPE) {
        double *src_data = (double *) _src_data;
        IMPORT_MACRO;
      } else if (src_type == ECL_INT_TYPE) {
        int *src_data = (int *) _src_data;
        IMPORT_MACRO;
      } else {
        fprintf(stderr,"%s: double field can only import from int/double/float\n",__func__);
        abort();
      }
    }
    break;
  case(ECL_FLOAT_TYPE):
    {
      float * target_data = (float *) field->data;
      if (src_type == ECL_FLOAT_TYPE) {
        float *src_data = (float *) _src_data;
        IMPORT_MACRO;
      } else if (src_type == ECL_DOUBLE_TYPE) {
        double *src_data = (double *) _src_data;
        IMPORT_MACRO;
      } else if (src_type == ECL_INT_TYPE) {
        int *src_data = (int *) _src_data;
        IMPORT_MACRO;
      } else {
        fprintf(stderr,"%s: double field can only import from int/double/float\n",__func__);
        abort();
      }
    }
    break;
  case(ECL_INT_TYPE):
    {
      int * target_data = (int *) field->data;
      if (src_type == ECL_INT_TYPE) {
        int *src_data = (int *) _src_data;
        IMPORT_MACRO;
      }  else {
        fprintf(stderr,"%s: int field can only import from int\n",__func__);
        abort();
      }
    }
    break;
  default:
    fprintf(stderr,"%s: Sorry field has unimportable type ... \n",__func__);
    break;
  }
}
#undef IMPORT_MACRO


/*****************************************************************/

#define CLEAR_MACRO(d,s) { int k; for (k=0; k < (s); k++) (d)[k] = 0; }
void field_clear(field_type * field) {
  const ecl_type_enum ecl_type = field_config_get_ecl_type(field->config);
  const int data_size          = field_config_get_data_size(field->config );   

  switch (ecl_type) {
  case(ECL_DOUBLE_TYPE):
    {
      double * data = (double *) field->data;
      CLEAR_MACRO(data , data_size);
      break;
    }
  case(ECL_FLOAT_TYPE):
    {
      float * data = (float *) field->data;
      CLEAR_MACRO(data , data_size);
      break;
    }
  case(ECL_INT_TYPE):
    {
      int * data = (int *) field->data;
      CLEAR_MACRO(data , data_size);
      break;
    }
  default:
    util_abort("%s: not implemeneted for data_type: %d \n",__func__ , ecl_type);
  }
}
#undef CLEAR_MACRO




static field_type * __field_alloc(const field_config_type * field_config , void * shared_data , int shared_byte_size) {
  field_type * field  = util_malloc(sizeof *field);
  field->config = field_config;
  field->private_config = false;
  if (shared_data == NULL) {
    field->shared_data = false;
    field->data        = util_calloc(field_config_get_byte_size(field->config) , sizeof * field->data );
  } else {
    field->data             = shared_data;
    field->shared_data      = true;
    field->shared_byte_size = shared_byte_size;
    if (shared_byte_size < field_config_get_byte_size(field->config)) 
      util_abort("%s: the shared buffer is to small to hold the input field - aborting \n",__func__);
    
  }
  field->export_data = NULL;  /* This NULL is checked for in the revert_output_transform() */
  field->__type_id   = FIELD;
  return field;
}



field_type * field_alloc(const field_config_type * field_config) {
  return __field_alloc(field_config , NULL , 0);
}


field_type * field_alloc_shared(const field_config_type * field_config, void * shared_data , int shared_byte_size) {
  return __field_alloc(field_config , shared_data , shared_byte_size);
}



void field_copy(const field_type *src , field_type * target ) {
  if (src->config == target->config)
    memcpy(target->data , src->data , field_config_get_byte_size(src->config));
  else
    util_abort("%s: instances do not share config \n",__func__);
}






void field_read_from_buffer(field_type * field , buffer_type * buffer, int report_step, state_enum state) {
  int byte_size = field_config_get_byte_size( field->config );
  enkf_util_assert_buffer_type(buffer , FIELD);
  buffer_fread_compressed(buffer , buffer_get_remaining_size( buffer ) , field->data , byte_size);
}





static void * __field_alloc_3D_data(const field_type * field , int data_size , bool rms_index_order , ecl_type_enum ecl_type , ecl_type_enum target_type) {
  void * data = util_calloc(data_size , ecl_util_get_sizeof_ctype(target_type) );
  if (ecl_type == ECL_DOUBLE_TYPE) {
    double fill;
    if (rms_index_order)
      fill = RMS_INACTIVE_DOUBLE;
    else
      fill = 0;
    field_export3D(field , data , rms_index_order , target_type , &fill);
  } else if (ecl_type == ECL_FLOAT_TYPE) {
    float fill;
    if (rms_index_order)
      fill = RMS_INACTIVE_FLOAT;
    else
      fill = 0;
    field_export3D(field , data , rms_index_order , target_type , &fill);
  } else if (ecl_type == ECL_INT_TYPE) {
    int fill;
    if (rms_index_order)
      fill = RMS_INACTIVE_INT;
    else
      fill = 0;
    field_export3D(field , data , rms_index_order , target_type , &fill);
  } else 
    util_abort("%s: trying to export type != int/float/double - aborting \n",__func__);
  return data;
}


/**
   A general comment about writing fields to disk:

   The writing of fields to disk can be done in **MANY** different ways:

   o The native function field_fwrite() will save the field in the
     format most suitable for use with enkf. This function will only
     save the active cells, and compress the field if the variable
     write_compressed is true. Most of the configuration information
     is with the field_config object, and not saved with the field.

   o Export as ECLIPSE input. This again has three subdivisions:

     * The function field_ecl_grdecl_export() will write the field to
       disk in a format suitable for ECLIPSE INCLUDE statements. This
       means that both active and inactive cells are written, with a
       zero fill for the inactive.

     * The functions field_xxxx_fortio() writes the field in the
       ECLIPSE restart format. The function field_ecl_write3D_fortio()
       writes all the cells - with zero filling for inactive
       cells. This is suitable for IMPORT of e.g. PORO.
       
       The function field_ecl_write1D_fortio() will write only the
       active cells in an ECLIPSE restart file. This is suitable for
       e.g. the pressure.

       Observe that the function field_ecl_write() should get config
       information and automatically select the right way to export to
       eclipse format.

   o Export in RMS ROFF format. 
*/

  

/** 
    This function exports *one* field instance to the rms_file
    instance. It is the responsibility of the field_ROFF_export()
    function to initialize and close down the rms_file instance. 
*/

static void field_ROFF_export__(const field_type * field , rms_file_type * rms_file) {
  const int data_size             = field_config_get_volume(field->config);
  const ecl_type_enum target_type = field_config_get_ecl_type(field->config); /* Could/should in principle be input */
  const ecl_type_enum ecl_type    = field_config_get_ecl_type(field->config);
  
  void *data  = __field_alloc_3D_data(field , data_size , true ,ecl_type , target_type);
  rms_tagkey_type * data_key = rms_tagkey_alloc_complete("data" , data_size , rms_util_convert_ecl_type(target_type) , data , true);
  rms_tag_fwrite_parameter(field_config_get_ecl_kw_name(field->config) , data_key , rms_file_get_FILE(rms_file));
  rms_tagkey_free(data_key);
  free(data);
}


static rms_file_type * field_init_ROFF_export(const field_type * field, const char * filename) {
  rms_file_type * rms_file = rms_file_alloc(filename , false);
  rms_file_fopen_w(rms_file);
  rms_file_init_fwrite(rms_file , "parameter");          /* Version / byteswap ++ */
  {
    int nx,ny,nz;
    field_config_get_dims(field->config , &nx , &ny , &nz);
    rms_tag_fwrite_dimensions(nx , ny , nz , rms_file_get_FILE(rms_file));  /* Dimension header */
  }
  return rms_file;
}


static void field_complete_ROFF_export(const field_type * field , rms_file_type * rms_file) {
  rms_file_complete_fwrite(rms_file);
  rms_file_fclose(rms_file);
  rms_file_free(rms_file);
}




/** 
    This function exports the data of a field as a parameter to an RMS
    roff file. The export process is divided in three parts:

    1. The rms_file is opened, and initialized with some basic data
       for dimensions++
    2. The field is written to file.
    3. The file is completed / closed.

    The reason for doing it like this is that it should be easy to
    export several fields (of the same dimension+++) with repeated
    calls to 2 (i.e. field_ROFF_export__()) - that is currently not
    implemented.
*/
    
void field_ROFF_export(const field_type * field , const char * filename) {
  rms_file_type * rms_file = field_init_ROFF_export(field , filename);
  field_ROFF_export__(field , rms_file);             /* Should now be possible to several calls to field_ROFF_export__() */
  field_complete_ROFF_export(field , rms_file);
}



bool field_write_to_buffer(const field_type * field , buffer_type * buffer , int report_step , state_enum state) {
  int byte_size = field_config_get_byte_size( field->config );
  buffer_fwrite_int( buffer , FIELD );
  buffer_fwrite_compressed( buffer , field->data , byte_size );
  return true;
}



void field_ecl_write1D_fortio(const field_type * field , fortio_type * fortio) {
  const int data_size = field_config_get_data_size(field->config );
  const ecl_type_enum ecl_type = field_config_get_ecl_type(field->config); 
  
  ecl_kw_fwrite_param_fortio(fortio , field_config_get_ecl_kw_name(field->config), ecl_type , data_size , field->data);
}


void field_ecl_write3D_fortio(const field_type * field , fortio_type * fortio ) {
  const int data_size             = field_config_get_volume(field->config);
  const ecl_type_enum target_type = field_config_get_ecl_type(field->config); /* Could/should in principle be input */
  const ecl_type_enum ecl_type    = field_config_get_ecl_type(field->config);
  void *data = __field_alloc_3D_data(field , data_size , false ,ecl_type , target_type);

  ecl_kw_fwrite_param_fortio(fortio , field_config_get_ecl_kw_name(field->config), ecl_type , data_size , data);
  free(data);
}


static ecl_kw_type * field_alloc_ecl_kw_wrapper__(const field_type * field, void * data) {
  const int data_size             = field_config_get_volume(field->config);
  const ecl_type_enum target_type = field_config_get_ecl_type(field->config); /* Could/should in principle be input */

  ecl_kw_type            * ecl_kw = ecl_kw_alloc_new_shared(field_config_get_ecl_kw_name(field->config) , data_size , target_type , data);

  return ecl_kw;
}


void field_ecl_grdecl_export(const field_type * field , FILE * stream) {
  const int data_size             = field_config_get_volume(field->config);
  const ecl_type_enum target_type = field_config_get_ecl_type(field->config); /* Could/should in principle be input */
  const ecl_type_enum ecl_type    = field_config_get_ecl_type(field->config);
  void *data                      = __field_alloc_3D_data(field , data_size , false , ecl_type , target_type );
  ecl_kw_type * ecl_kw = field_alloc_ecl_kw_wrapper__(field , data);
  ecl_kw_fprintf_grdecl(ecl_kw , stream);
  ecl_kw_free(ecl_kw);
  free(data);
}


/**
   This allocates a ecl_kw instance representing the field. The
   size/header/type are copied from the field. whereas the data is
   *SHARED* with the field->data.

   The ecl_kw instance knows that the data is only shared, and it is
   safe to call ecl_kw_free() on it.
*/

ecl_kw_type * field_alloc_ecl_kw_wrapper(const field_type * field) {
  ecl_kw_type  * ecl_kw = field_alloc_ecl_kw_wrapper__(field , field->data);
  return ecl_kw;
}


static void field_apply(field_type * field , field_func_type * func) {
  field_config_assert_unary(field->config , __func__);
  {
    const int data_size          = field_config_get_data_size( field->config );   
    const ecl_type_enum ecl_type = field_config_get_ecl_type(field->config);
    
    if (ecl_type == ECL_FLOAT_TYPE) {
      float * data = (float *) field->data;
      for (int i=0; i < data_size; i++)
        data[i] = func(data[i]);
    } else if (ecl_type == ECL_DOUBLE_TYPE) {
      double * data = (double *) field->data;
      for (int i=0; i < data_size; i++)
        data[i] = func(data[i]);
    } 
  }
}


static bool field_check_finite( const field_type * field) {
  const int data_size          = field_config_get_data_size( field->config );   
  const ecl_type_enum ecl_type = field_config_get_ecl_type(field->config);
  bool  ok = true;
  
  if (ecl_type == ECL_FLOAT_TYPE) {
    float * data = (float *) field->data;
    for (int i=0; i < data_size; i++)
      if (!isfinite( data[i] ))
        ok = false;
  } else if (ecl_type == ECL_DOUBLE_TYPE) {
    double * data = (double *) field->data;
    for (int i=0; i < data_size; i++)
      if (!isfinite( data[i] ))
        ok = false;
  } 
  return ok;
}





void  field_inplace_output_transform(field_type * field ) {
  field_func_type * output_transform = field_config_get_output_transform(field->config);
  if (output_transform != NULL) 
    field_apply(field , output_transform);
}



#define TRUNCATE_MACRO(s , d , t , min , max)  \
for (int i=0; i < s; i++) {                    \
  if ( t & TRUNCATE_MIN )                      \
    if (d[i] < min)                            \
      d[i] = min;                              \
  if ( t & TRUNCATE_MAX )                      \
    if (d[i] > max)                            \
      d[i] = max;                              \
}
    

static void field_apply_truncation(field_type * field) {
  truncation_type   truncation = field_config_get_truncation_mode( field->config );
  if (truncation != TRUNCATE_NONE) {
    double min_value = field_config_get_truncation_min( field->config );
    double max_value = field_config_get_truncation_max( field->config );

    const int data_size          = field_config_get_data_size(field->config );   
    const ecl_type_enum ecl_type = field_config_get_ecl_type(field->config);
    if (ecl_type == ECL_FLOAT_TYPE) {
      float * data = (float *) field->data;
      TRUNCATE_MACRO(data_size , data , truncation , min_value , max_value);
    } else if (ecl_type == ECL_DOUBLE_TYPE) {
      double * data = (double *) field->data;
      TRUNCATE_MACRO(data_size , data , truncation , min_value , max_value);
    } else 
      util_abort("%s: Field type not supported for truncation \n",__func__);
  }
}
  

/** 
    Does both the explicit output transform *AND* the truncation.
*/

static void field_output_transform(field_type * field) {
  field_func_type * output_transform = field_config_get_output_transform(field->config);
  truncation_type   truncation       = field_config_get_truncation_mode( field->config );
  if ((output_transform != NULL) || (truncation != TRUNCATE_NONE)) {
    field->export_data = util_alloc_copy(field->data , field_config_get_byte_size(field->config) );
    field->__data = field->data;  /* Storing a pointer to the original data. */
    field->data   = field->export_data;
    
    if (output_transform != NULL)
      field_inplace_output_transform(field);
    
    field_apply_truncation(field);
  }
}


static void field_revert_output_transform(field_type * field) {
  if (field->export_data != NULL) {
    free(field->export_data);
    field->export_data = NULL;
    field->data = field->__data; /* Recover the original pointer. */
  }
}


/**
  This is the generic "export field to eclipse" function. It will
  check up the config object to determine how to export the field,
  and then call the appropriate function. The alternatives are:

  * Restart format - only active cells (field_ecl_write1D_fortio).
  * Restart format - all cells         (field_ecl_write3D_fortio).
  * GRDECL  format                     (field_ecl_grdecl_export)

  Observe that the output transform is hooked in here, that means
  that if you call e.g. the ROFF export function directly, the output
  transform will *NOT* be applied.
*/  

void field_export(const field_type * __field, const char * file , fortio_type * restart_fortio , field_file_format_type file_type, bool output_transform) {
  field_type * field = (field_type *) __field;  /* Net effect is no change ... but */

  if (output_transform) field_output_transform(field);
  {
    
    /*  Writes the field to in ecl_kw format to a new file.  */
    if ((file_type == ECL_KW_FILE_ALL_CELLS) || (file_type == ECL_KW_FILE_ACTIVE_CELLS)) {
      fortio_type * fortio;
      bool fmt_file = false;                /* For formats which support both formatted and unformatted output this is hardwired to unformatted. */
      
      fortio = fortio_open_writer(file , fmt_file , ECL_ENDIAN_FLIP);

      if (file_type == ECL_KW_FILE_ALL_CELLS)
        field_ecl_write3D_fortio(field , fortio);
      else
        field_ecl_write1D_fortio(field , fortio);
      
      fortio_fclose(fortio);
    } else if (file_type == ECL_GRDECL_FILE) {
      /* Writes the field to a new grdecl file. */
      FILE * stream = util_fopen(file , "w");
      field_ecl_grdecl_export(field , stream);
      fclose(stream);
    } else if (file_type == RMS_ROFF_FILE) 
      /* Roff export */
      field_ROFF_export(field , file);
    else if (file_type == ECL_FILE) 
      /* This entry point is used by the ecl_write() function to write to an ALREADY OPENED eclipse restart file. */
      field_ecl_write1D_fortio( field , restart_fortio);
    else
      util_abort("%s: internal error file_type = %d - aborting \n",__func__ , file_type);
  }
  if (output_transform) field_revert_output_transform(field);
}


/**
   Observe that the output transform is hooked in here, that means
   that if you call e.g. the ROFF export function directly, the output
   transform will *NOT* be applied.

   Observe that the output transform is done one a copy of the data -
   not in place. When the export is complete the field->data will be
   unchanged.
*/

void field_ecl_write(const field_type * field , const char * run_path , const char * file , fortio_type * restart_fortio) {
  field_file_format_type export_format = field_config_get_export_format(field->config);
  
  if (export_format == ECL_FILE)
    field_export(field , NULL , restart_fortio , export_format , true); 
  else {
    char * full_path = util_alloc_filename( run_path , file  , NULL);
    field_export(field , full_path , NULL , export_format , true);
    free( full_path );
  }
}





bool field_initialize(field_type *field , int iens , const char * init_file , rng_type * rng) {
  if (init_file != NULL) {
    if (field_fload(field , init_file )) {
      field_func_type * init_transform   = field_config_get_init_transform(field->config);
      /* 
         Doing the input transform - observe that this is done inplace on
         the data, not as the output transform which is done on a copy of
         prior to export.
      */
      if (init_transform != NULL) {
        field_apply(field , init_transform);
        if (!field_check_finite( field ))
          util_exit("Sorry: after applying the init transform field:%s contains nan/inf or similar malformed values.\n" , field_config_get_key( field->config ));
      }
      return true; 
    }
  } 
  
  return false;  /* The field is initialized as part of the forward model. */
} 


void field_free(field_type *field) {
  if (!field->shared_data) {
    free(field->data);
    field->data = NULL;
  }
  free(field);
}





void field_serialize(const field_type * field , node_id_type node_id , const active_list_type * active_list , matrix_type * A , int row_offset , int column) {
  const field_config_type *config      = field->config;
  const int                data_size   = field_config_get_data_size(config );
  ecl_type_enum ecl_type               = field_config_get_ecl_type(config);
  
  enkf_matrix_serialize( field->data , data_size , ecl_type , active_list , A , row_offset , column);
}


void field_deserialize(field_type * field , node_id_type node_id , const active_list_type * active_list , const matrix_type * A , int row_offset , int column) {
  const field_config_type *config      = field->config;
  const int                data_size   = field_config_get_data_size(config );
  ecl_type_enum ecl_type               = field_config_get_ecl_type(config);
  
  enkf_matrix_deserialize( field->data , data_size , ecl_type , active_list , A , row_offset , column);
}




void field_ijk_get(const field_type * field , int i , int j , int k , void * value) {
  int active_index = field_config_active_index(field->config , i , j , k);
  int sizeof_ctype = field_config_get_sizeof_ctype(field->config);
  memcpy(value , &field->data[active_index * sizeof_ctype] , sizeof_ctype);
}



double field_ijk_get_double(const field_type * field, int i , int j , int k) {
  int active_index = field_config_active_index(field->config , i , j , k);
  return field_iget_double( field , active_index );
}


float field_ijk_get_float(const field_type * field, int i , int j , int k) {
  int active_index = field_config_active_index(field->config , i , j , k);
  return field_iget_float( field , active_index );
}



/**
   Takes an active index as input, and returns a double.
*/
double field_iget_double(const field_type * field , int active_index) {
  ecl_type_enum ecl_type = field_config_get_ecl_type(field->config);
  int sizeof_ctype       = field_config_get_sizeof_ctype(field->config);
  char buffer[8]; /* Enough to hold one double */
  memcpy(buffer , &field->data[active_index * sizeof_ctype] , sizeof_ctype);
  if ( ecl_type == ECL_DOUBLE_TYPE ) 
    return *((double *) buffer);
  else if (ecl_type == ECL_FLOAT_TYPE) {
    double double_value;
    float  float_value;
    
    float_value  = *((float *) buffer);
    double_value = float_value;
    
    return double_value;
  } else {
    util_abort("%s: failed - wrong internal type \n",__func__);
    return -1;
  }
}


/**
   Takes an active index as input, and returns a double.
*/
float field_iget_float(const field_type * field , int active_index) {
  ecl_type_enum ecl_type = field_config_get_ecl_type(field->config);
  int sizeof_ctype       = field_config_get_sizeof_ctype(field->config);
  char buffer[8];          /* Enough to hold one double */
  memcpy(buffer , &field->data[active_index * sizeof_ctype] , sizeof_ctype);
  if ( ecl_type == ECL_FLOAT_TYPE ) 
    return *((float *) buffer);
  else if (ecl_type == ECL_DOUBLE_TYPE) {
    double double_value;
    float  float_value;
    
    double_value = *((double *) buffer);
    float_value  = double_value;
    
    return float_value;
  } else {
    util_abort("%s: failed - wrong internal type \n",__func__);
    return -1;
  }
}




double field_iget(const field_type * field, int active_index) {
  return field_iget_double(field , active_index);
}




void field_ijk_set(field_type * field , int i , int j , int k , const void * value) {
  int active_index = field_config_active_index(field->config , i , j , k);
  int sizeof_ctype = field_config_get_sizeof_ctype(field->config);
  memcpy(&field->data[active_index * sizeof_ctype] , value , sizeof_ctype);
}


#define INDEXED_UPDATE_MACRO(t,s,n,index,add) \
{                                             \
   int i;                                     \
   if (add)                                   \
      for (i=0; i < (n); i++)                 \
          (t)[index[i]] += (s)[i];            \
   else                                       \
      for (i=0; i < (n); i++)                 \
          (t)[index[i]]  = (s)[i];            \
}



static void field_indexed_update(field_type * field, ecl_type_enum src_type , int len , const int * index_list , const void * value , bool add) {
  ecl_type_enum target_type = field_config_get_ecl_type(field->config);

  switch (target_type) {
  case(ECL_FLOAT_TYPE):
    {
      float * field_data = (float *) field->data;
      if (src_type == ECL_DOUBLE_TYPE) {
        double * src_data = (double *) value;
        INDEXED_UPDATE_MACRO(field_data , src_data , len , index_list , add);
      } else if (src_type == ECL_FLOAT_TYPE) {
        float * src_data = (float *) value;
        INDEXED_UPDATE_MACRO(field_data , src_data , len , index_list , add);
      } else 
        util_abort("%s both existing field - and indexed values must be float / double - aborting\n",__func__);
    }
    break;
  case(ECL_DOUBLE_TYPE):
    {
      double * field_data = (double *) field->data;
      if (src_type == ECL_DOUBLE_TYPE) {
        double * src_data = (double *) value;
        INDEXED_UPDATE_MACRO(field_data , src_data , len , index_list , add);
      } else if (src_type == ECL_FLOAT_TYPE) {
        float * src_data = (float *) value;
        INDEXED_UPDATE_MACRO(field_data , src_data , len , index_list , add);
      } else 
        util_abort("%s both existing field - and indexed values must be float / double - aborting\n",__func__);
    }
    break;
  default:
    util_abort("%s existing field must be of type float/double - aborting \n",__func__);
  }
}


void field_indexed_set(field_type * field, ecl_type_enum src_type , int len , const int * index_list , const void * value) {
  field_indexed_update(field , src_type , len , index_list , value , false);
}


void field_indexed_add(field_type * field, ecl_type_enum src_type , int len , const int * index_list , const void * value) {
  field_indexed_update(field , src_type , len , index_list , value , true);
}



double * field_indexed_get_alloc(const field_type * field, int len, const int * index_list)
{
  double * export_data = util_calloc(len , sizeof * export_data);
  ecl_type_enum src_type = field_config_get_ecl_type(field->config);
  
  if(src_type == ECL_DOUBLE_TYPE) {
    /* double -> double */
    double * field_data = (double *) field->data;
    for (int i=0; i<len; i++)
      export_data[i] = field_data[index_list[i]];
  } else if (src_type == ECL_FLOAT_TYPE) {
    /* float -> double */
    float * field_data = (float *) field->data;
    for (int i=0; i<len; i++)
      export_data[i] = field_data[index_list[i]];
  } else
    util_abort("%s: existing field must of type float/double - aborting. \n", __func__);
  
  return export_data;
}



bool field_ijk_valid(const field_type * field , int i , int j , int k) {
  int active_index = field_config_active_index(field->config , i , j , k);
  if (active_index >=0)
    return true;
  else
    return false;
}


void field_ijk_get_if_valid(const field_type * field , int i , int j , int k , void * value , bool * valid) {
  int active_index = field_config_active_index(field->config , i , j , k);
  if (active_index >=0) {
    *valid = true;
    field_ijk_get(field , i , j , k , value);
  } else 
    *valid = false;
}


int field_get_active_index(const field_type * field , int i , int j  , int k) {
  return field_config_active_index(field->config , i , j , k);
}




/**
   Copying data from a (PACKED) ecl_kw instance down to a fields data.
*/

void field_copy_ecl_kw_data(field_type * field , const ecl_kw_type * ecl_kw) {
  const field_config_type * config = field->config;
  const int data_size              = field_config_get_data_size(config );
  ecl_type_enum field_type         = field_config_get_ecl_type(field->config);
  ecl_type_enum kw_type            = ecl_kw_get_type(ecl_kw);

  if (data_size != ecl_kw_get_size(ecl_kw)) {
    fprintf(stderr,"\n");
    fprintf(stderr," ** Fatal error - the number of active cells has changed \n");
    fprintf(stderr," **   Grid:%s has %d active cells. \n",field_config_get_grid_name( config ) , data_size);
    fprintf(stderr," **   %s loaded from file has %d active cells.\n",field_config_get_key(config), ecl_kw_get_size(ecl_kw));
    fprintf(stderr," ** MINPV / MINPVV problem?? \n");
    util_abort("%s: Aborting \n",__func__ );
  }
  
  ecl_util_memcpy_typed_data(field->data , ecl_kw_get_void_ptr(ecl_kw) , field_type , kw_type , ecl_kw_get_size(ecl_kw));
}



/*****************************************************************/

bool field_fload_rms(field_type * field , const char * filename) {
  {
    FILE * stream = util_fopen__( filename , "r");
    if (!stream)
      return false;
    
    fclose( stream );
  }

  {
    const char * key           = field_config_get_ecl_kw_name(field->config);
    ecl_type_enum   ecl_type;
    rms_file_type * rms_file   = rms_file_alloc(filename , false);
    rms_tagkey_type * data_tag;
    if (field_config_enkf_mode(field->config)) 
      data_tag = rms_file_fread_alloc_data_tagkey(rms_file , "parameter" , "name" , key);
    else {
      /** 
          Setting the key - purely to support converting between
          different types of files, without knowing the key. A usable
          feature - but not really well defined.
      */
      
      rms_tag_type * rms_tag = rms_file_fread_alloc_tag(rms_file , "parameter" , NULL , NULL);
      const char * parameter_name = rms_tag_get_namekey_name(rms_tag);
      field_config_set_key( (field_config_type *) field->config , parameter_name );
      data_tag = rms_tagkey_copyc( rms_tag_get_key(rms_tag , "data") );
      rms_tag_free(rms_tag);
    }
    
    ecl_type = rms_tagkey_get_ecl_type(data_tag);
    if (rms_tagkey_get_size(data_tag) != field_config_get_volume(field->config)) 
      util_abort("%s: trying to import rms_data_tag from:%s with wrong size - aborting \n",__func__ , filename);
    
    field_import3D(field , rms_tagkey_get_data_ref(data_tag) , true , ecl_type);
    rms_tagkey_free(data_tag);
    rms_file_free(rms_file);
  }
  return true;
}



bool field_fload_ecl_kw(field_type * field , const char * filename ) {
  const char * key = field_config_get_ecl_kw_name(field->config);
  ecl_kw_type * ecl_kw = NULL;
  
  {
    bool fmt_file;

    if (ecl_util_fmt_file( filename , &fmt_file)) {
      fortio_type * fortio = fortio_open_reader(filename , fmt_file , ECL_ENDIAN_FLIP);
      if (fortio) {
        ecl_kw_fseek_kw(key , true , true , fortio);
        ecl_kw = ecl_kw_fread_alloc( fortio );
        fortio_fclose(fortio);
        
        if (field_config_get_volume(field->config) == ecl_kw_get_size(ecl_kw)) 
          field_import3D(field , ecl_kw_get_void_ptr(ecl_kw) , false , ecl_kw_get_type(ecl_kw));
        else 
          /* Keyword is already packed - e.g. from a restart file. Size is
             verified in the _copy function.*/
          field_copy_ecl_kw_data(field , ecl_kw);
        
        ecl_kw_free(ecl_kw);
        return true;
      }
    } else
      util_abort("%s: could not determine formatted/unformatted status of file:%s \n",filename);
  }
  return false;
}



/* No type translation possible */
bool field_fload_ecl_grdecl(field_type * field , const char * filename ) {
  const char * key = field_config_get_ecl_kw_name(field->config);
  int size = field_config_get_volume(field->config);
  ecl_type_enum ecl_type = field_config_get_ecl_type(field->config);
  ecl_kw_type * ecl_kw   = NULL;
  {
    FILE * stream = util_fopen__(filename , "r");
    if (stream) {
      if (ecl_kw_grdecl_fseek_kw(key , false , stream))
        ecl_kw = ecl_kw_fscanf_alloc_grdecl_data(stream , size , ecl_type);
      else 
        util_exit("%s: Can not locate %s keyword in %s \n",__func__ , key , filename);
      fclose(stream);
      
      field_import3D(field , ecl_kw_get_void_ptr(ecl_kw) , false , ecl_kw_get_type(ecl_kw));
      ecl_kw_free(ecl_kw);
      return true;
    }
  }
  return false;
}




bool field_fload_typed(field_type * field , const char * filename ,  field_file_format_type file_type) {
  bool loadOK = false;
  switch (file_type) {
  case(RMS_ROFF_FILE):
    loadOK = field_fload_rms(field , filename );
    break;
  case(ECL_KW_FILE):
    loadOK = field_fload_ecl_kw(field , filename  );
    break;
  case(ECL_GRDECL_FILE):
    loadOK = field_fload_ecl_grdecl(field , filename);
    break;
  default:
    util_abort("%s: file_type:%d not recognized - aborting \n",__func__ , file_type);
  }
  return loadOK;
}




bool field_fload(field_type * field , const char * filename ) {
  if (util_file_readable( filename )) {
    field_file_format_type file_type = field_config_guess_file_type( filename );
    if (file_type == UNDEFINED_FORMAT) 
      file_type = field_config_manual_file_type(filename , true);
    
    return field_fload_typed(field , filename , file_type);
  } else
    return false;
}



bool field_fload_auto(field_type * field , const char * filename ) {
  field_file_format_type file_type = field_config_guess_file_type(filename);
  return field_fload_typed(field , filename , file_type);
}



/**
   This function compares two fields, and return true if they are
   equal. Observe that the config comparison is done with plain
   pointer comparison, i.e. the actual content of the config objects
   is not compared. If the two fields point to different config
   objects, the comparision will fail immediately - without checking the
   content of the fields.
*/

bool field_cmp(const field_type * f1 , const field_type * f2) {
  if (f1->config != f2->config) {
    fprintf(stderr,"The two fields have different config objects - and the comparison fails trivially.\n");
    return false;
  } else {
    const int byte_size = field_config_get_byte_size(f1->config);   
    if (memcmp( f1->data , f2->data , byte_size) != 0)
      return false;
    else
      return true;
  }
}


/*****************************************************************/




/**
   This function loads a field from a complete forward run. The
   original implementation is to load e.g. pressure and saturations
   from a block of restart data. Current implementation can only
   handle that, but in principle other possibilities should be
   possible.
   
   Observe that forward_load loads from a (already loaded) restart_block,
   and not from a file.
*/


bool field_forward_load(field_type * field , const char * ecl_file_name , const ecl_sum_type * ecl_sum, const ecl_file_type * restart_file , int report_step) {
  bool loadOK                          = true;
  field_file_format_type import_format = field_config_get_import_format(field->config);
    
  if (import_format == ECL_FILE) {
    if (restart_file != NULL) {
      ecl_kw_type * field_kw = ecl_file_iget_named_kw(restart_file , field_config_get_ecl_kw_name(field->config) , 0);
      field_copy_ecl_kw_data(field , field_kw);
    } else 
      loadOK = false;
    //util_abort("%s: fatal error when loading: %s - no restart information has been loaded \n",__func__ , field_config_get_key( field->config ));
  } else 
    /* Loading from unique file - currently this only applies to the modelerror implementation. */
    field_fload_typed(field , ecl_file_name , import_format);
  
  
  if (loadOK) {
    field_func_type * input_transform = field_config_get_input_transform(field->config);
    /* The input transform is done in-place. */
    if (input_transform != NULL) 
      field_apply(field , input_transform);

  }
  return loadOK;
}



void field_get_dims(const field_type * field, int *nx, int *ny , int *nz) {
  field_config_get_dims(field->config , nx , ny ,nz);
}








void field_iadd(field_type * field1, const field_type * field2) {
  field_config_assert_binary(field1->config , field2->config , __func__); 
  {
    const int data_size          = field_config_get_data_size( field1->config );   
    const ecl_type_enum ecl_type = field_config_get_ecl_type( field1->config );
    int i;

    if (ecl_type == ECL_FLOAT_TYPE) {
      float * data1       = (float *) field1->data;
      const float * data2 = (const float *) field2->data;
      for (i = 0; i < data_size; i++)
        data1[i] += data2[i];
    } else if (ecl_type == ECL_DOUBLE_TYPE) {
      double * data1       = (double *) field1->data;
      const double * data2 = (const double *) field2->data;
      for (i = 0; i < data_size; i++)
        data1[i] += data2[i];
    }
  }
}


void field_imul(field_type * field1, const field_type * field2) {
  field_config_assert_binary(field1->config , field2->config , __func__); 
  {
    const int data_size          = field_config_get_data_size(field1->config );   
    const ecl_type_enum ecl_type = field_config_get_ecl_type(field1->config);
    int i;

    if (ecl_type == ECL_FLOAT_TYPE) {
      float * data1       = (float *) field1->data;
      const float * data2 = (const float *) field2->data;
      for (i = 0; i < data_size; i++)
        data1[i] *= data2[i];
    } else if (ecl_type == ECL_DOUBLE_TYPE) {
      double * data1       = (double *) field1->data;
      const double * data2 = (const double *) field2->data;
      for (i = 0; i < data_size; i++)
        data1[i] *= data2[i];
    }
  }
}


void field_iaddsqr(field_type * field1, const field_type * field2) {
  field_config_assert_binary(field1->config , field2->config , __func__); 
  {
    const int data_size          = field_config_get_data_size(field1->config );   
    const ecl_type_enum ecl_type = field_config_get_ecl_type(field1->config);
    int i;

    if (ecl_type == ECL_FLOAT_TYPE) {
      float * data1       = (float *) field1->data;
      const float * data2 = (const float *) field2->data;
      for (i = 0; i < data_size; i++)
        data1[i] += data2[i] * data2[i];
    } else if (ecl_type == ECL_DOUBLE_TYPE) {
      double * data1       = (double *) field1->data;
      const double * data2 = (const double *) field2->data;
      for (i = 0; i < data_size; i++)
        data1[i] += data2[i] * data2[i];
    }
  }
}


void field_scale(field_type * field, double scale_factor) {
  field_config_assert_unary(field->config, __func__); 
  {
    const int data_size          = field_config_get_data_size(field->config );   
    const ecl_type_enum ecl_type = field_config_get_ecl_type(field->config);
    int i;

    if (ecl_type == ECL_FLOAT_TYPE) {
      float * data       = (float *) field->data;
      for (i = 0; i < data_size; i++)
        data[i] *= scale_factor;
    } else if (ecl_type == ECL_DOUBLE_TYPE) {
      double * data       = (double *) field->data;
      for (i = 0; i < data_size; i++)
        data[i] *= scale_factor;
    }
  }
}


static inline float __sqr(float x) { return x*x; }

void field_isqr(field_type * field) {
  field_apply(field , __sqr);
}


void field_isqrt(field_type * field) {
  field_apply(field , sqrtf);
}

void field_imul_add(field_type * field1 , double factor , const field_type * field2) {
  field_config_assert_binary(field1->config , field2->config , __func__); 
  {
    const int data_size          = field_config_get_data_size(field1->config );   
    const ecl_type_enum ecl_type = field_config_get_ecl_type(field1->config);
    int i;

    if (ecl_type == ECL_FLOAT_TYPE) {
      float * data1       = (float *) field1->data;
      const float * data2 = (const float *) field2->data;
      for (i = 0; i < data_size; i++)
        data1[i] += factor * data2[i];
    } else if (ecl_type == ECL_DOUBLE_TYPE) {
      double * data1       = (double *) field1->data;
      const double * data2 = (const double *) field2->data;
      for (i = 0; i < data_size; i++)
        data1[i] += factor * data2[i];
    }
  }
}


void field_update_sum(field_type * sum , field_type * field , double lower_limit , double upper_limit) {
  field_output_transform( field );
  {
    const int data_size          = field_config_get_data_size(field->config );   
    const ecl_type_enum ecl_type = field_config_get_ecl_type(field->config);
    int i;
    
    if (ecl_type == ECL_FLOAT_TYPE) {
      float * data       = (float *) field->data;
      float * sum_data   = (float *) sum->data;
      for (i = 0; i < data_size; i++) {
        if (data[i] >= lower_limit)
          if (data[i] < upper_limit)
            sum_data[i] += 1;
      } 
    } else if (ecl_type == ECL_DOUBLE_TYPE) {
        double * data       = (double *) field->data;
        double * sum_data   = (double *) sum->data;
        for (i = 0; i < data_size; i++) {
          if (data[i] >= lower_limit)
            if (data[i] < upper_limit)
              sum_data[i] += 1;
        }
    }
  } 
  field_revert_output_transform( field );
}



/**
  Here, index_key is i a tree digit string with the i, j and k indicies of
  the requested block separated by comma. E.g., 1,1,1. 

  The string is supposed to contain indices in the range [1...nx] ,
  [1..ny] , [1...nz], they are immediately converted to C-based zero
  offset indices.
*/
bool field_user_get(const field_type * field, const char * index_key, int report_step , state_enum state, double * value)
{
  printf("index_key : %s \n",index_key);
  const    bool internal_value = false;
  bool     valid;
  int      i,j,k;
  int      parse_user_key = field_config_parse_user_key(field->config , index_key , &i, &j , &k);
  

  if (parse_user_key == 0) {
    int active_index = field_config_active_index(field->config , i,j,k);
    *value =  field_iget_double(field, active_index);
    valid = true;
  } else {
    if (parse_user_key == 1)
      fprintf(stderr,"Failed to parse \"%s\" as three integers \n",index_key);
    else if (parse_user_key == 2)
      fprintf(stderr," ijk: %d , %d, %d is invalid \n",i+1 , j + 1 , k + 1);
    else if (parse_user_key == 3)
      fprintf(stderr," ijk: %d , %d, %d is an inactive cell. \n",i+1 , j + 1 , k + 1);
    else
      util_abort("%s: internal error -invalid value:%d \n",__func__ , parse_user_key);
    *value = 0.0;
    valid = false;
  }
  
  if (!internal_value && valid) {
    field_func_type * output_transform = field_config_get_output_transform(field->config);
    if (output_transform != NULL)
      *value = output_transform( *value );
    /* Truncation - ignored for now */
  }
  return valid;
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


void field_set_inflation(field_type * inflation , const field_type * std , const field_type * min_std) {
  const field_config_type * config = inflation->config;
  ecl_type_enum ecl_type           = field_config_get_ecl_type( config );
  const int data_size              = field_config_get_data_size( config );   

  if (ecl_type == ECL_FLOAT_TYPE) {
    float       * inflation_data = (float *)       inflation->data;
    const float * std_data       = (const float *) std->data;
    const float * min_std_data   = (const float *) min_std->data;
    
    INFLATE(inflation_data , std_data , min_std_data );
    
  } else if (ecl_type == ECL_DOUBLE_TYPE) {
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

/*
  These two functions assume float/double storage; will not work with
  field which is internally based on char *.

  MATH_OPS(field)
*/
UTIL_SAFE_CAST_FUNCTION(field , FIELD)
UTIL_SAFE_CAST_FUNCTION_CONST(field , FIELD)
UTIL_IS_INSTANCE_FUNCTION(field , FIELD)
VOID_ALLOC(field)
VOID_FREE(field)
VOID_ECL_WRITE (field)
VOID_FORWARD_LOAD(field)
VOID_COPY     (field)
VOID_INITIALIZE(field);
VOID_USER_GET(field)
VOID_READ_FROM_BUFFER(field)
VOID_WRITE_TO_BUFFER(field)
VOID_CLEAR(field)
VOID_SERIALIZE(field)
VOID_DESERIALIZE(field)
VOID_SET_INFLATION(field)
VOID_IADD(field)
VOID_SCALE(field)
VOID_IADDSQR(field)
VOID_IMUL(field)
VOID_ISQRT(field)
VOID_FLOAD(field)
