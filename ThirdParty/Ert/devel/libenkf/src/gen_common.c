/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'gen_common.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/util.h>

#include <ert/ecl/fortio.h>
#include <ert/ecl/ecl_util.h>

#include <ert/enkf/gen_data_config.h>
#include <ert/enkf/gen_common.h>

/**
   This file implements some (very basic) functionality which is used
   by both the gen_data and gen_obs objects.
*/


void * gen_common_fscanf_alloc(const char * file , ecl_type_enum load_type , int * size) {
  FILE * stream           = util_fopen(file , "r");
  int sizeof_ctype        = ecl_util_get_sizeof_ctype(load_type);
  int buffer_elements     = *size;
  int current_size        = 0;
  int fscanf_return       = 1; /* To keep the compiler happy .*/
  void * buffer;
  
  if (buffer_elements == 0)
    buffer_elements = 100;
  
  buffer = util_calloc( buffer_elements , sizeof_ctype );
  {
    do {
      if (load_type == ECL_FLOAT_TYPE) {
        float  * float_buffer = (float *) buffer;
        fscanf_return = fscanf(stream , "%g" , &float_buffer[current_size]);
      } else if (load_type == ECL_DOUBLE_TYPE) {
        double  * double_buffer = (double *) buffer;
        fscanf_return = fscanf(stream , "%lg" , &double_buffer[current_size]);
      } else if (load_type == ECL_INT_TYPE) {
        int * int_buffer = (int *) buffer;
        fscanf_return = fscanf(stream , "%d" , &int_buffer[current_size]);
      }  else 
        util_abort("%s: god dammit - internal error \n",__func__);
      
      if (fscanf_return == 1)
        current_size += 1;
      
      if (current_size == buffer_elements) {
        buffer_elements *= 2;
        buffer = util_realloc( buffer , buffer_elements * sizeof_ctype );
      }
    } while (fscanf_return == 1);
  }
  if (fscanf_return != EOF) 
    util_abort("%s: scanning of %s terminated before EOF was reached -- fix your file.\n" , __func__ , file);
  
  fclose(stream);
  *size = current_size;
  return buffer;
}



void * gen_common_fread_alloc(const char * file , ecl_type_enum load_type , int * size) {
  const int max_read_size = 100000;
  FILE * stream           = util_fopen(file , "r");
  int sizeof_ctype        = ecl_util_get_sizeof_ctype(load_type);
  int read_size           = 4096; /* Shot in the wild */
  int current_size        = 0;
  int buffer_elements;
  int fread_return;
  char * buffer;
  
  
  buffer_elements = read_size;
  buffer = util_calloc( buffer_elements , sizeof_ctype );
  {
    do {
      fread_return  = fread( &buffer[ current_size * sizeof_ctype] , sizeof_ctype , read_size , stream);
      current_size += fread_return;
      
      if (!feof(stream)) {
        /* Allocate more elements. */
        if (current_size == buffer_elements) {
          read_size *= 2;
          read_size = util_int_min(read_size , max_read_size);
          buffer_elements += read_size;
          buffer = util_realloc( buffer , buffer_elements * sizeof_ctype );
        } else 
          util_abort("%s: internal error ?? \n",__func__);
      }
    } while (!feof(stream));
  }
  *size = current_size;
  return buffer;
}


/*
  If the load_format is binary_float or binary_double, the ASCII_type
  is *NOT* consulted. The load_type is set to float/double depending
  on what was actually used when the data was loaded.
*/

void * gen_common_fload_alloc(const char * file , gen_data_file_format_type load_format , ecl_type_enum ASCII_type , ecl_type_enum * load_type , int * size) { 
  void * buffer = NULL;
  
  if (load_format == ASCII) {
    *load_type = ASCII_type;
    buffer =  gen_common_fscanf_alloc(file , ASCII_type , size);
  } else if (load_format == BINARY_FLOAT) {
    *load_type = ECL_FLOAT_TYPE;
    buffer = gen_common_fread_alloc(file , ECL_FLOAT_TYPE , size);
  } else if (load_format == BINARY_DOUBLE) {
    *load_type = ECL_DOUBLE_TYPE;
    buffer = gen_common_fread_alloc(file , ECL_DOUBLE_TYPE , size);
  } else 
    util_abort("%s: trying to load with unsupported format:%s... \n" , load_format);
  
  return buffer;
}
