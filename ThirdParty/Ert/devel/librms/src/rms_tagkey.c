/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'rms_tagkey.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include <ert/util/util.h>
#include <ert/util/hash.h>

#include <ert/rms/rms_type.h>
#include <ert/rms/rms_tagkey.h>
#include <ert/rms/rms_util.h>

static const char * rms_array_string      = "array";

static const char rms_type_names[6][7] = {{"char\0"},
                                          {"float\0"},
                                          {"double\0"},
                                          {"bool\0"},
                                          {"byte\0"},
                                          {"int\0"}};

static const int rms_type_size[6] = {1 , 4 , 8 , 1 , 1 , 4}; /* */


struct rms_tagkey_struct {
  int                  size;
  int                  sizeof_ctype;
  int                  data_size;
  int                  alloc_size;
  rms_type_enum        rms_type;
  char                *name;
  void                *data;
  bool                 endian_convert;
  bool                 shared_data;
};


/*****************************************************************/


                            
/*****************************************************************/


static void rms_tagkey_assert_fnum(const rms_tagkey_type * tagkey) {
  if (!(tagkey->rms_type == rms_float_type || tagkey->rms_type == rms_double_type)) {
    fprintf(stderr,"%s: tried to perform numerical operataion on rms_type: %s invalid/not implemented\n",__func__ , rms_type_names[tagkey->rms_type]);
    abort();
  }
}

static void rms_tagkey_assert_fnum2(const rms_tagkey_type * tagkey1 , const rms_tagkey_type *tagkey2) {
  rms_tagkey_assert_fnum(tagkey1);
  rms_tagkey_assert_fnum(tagkey2);
  if (tagkey1->size != tagkey2->size || tagkey1->rms_type != tagkey2->rms_type) {
    fprintf(stderr,"%s: tried to combine tagkey with different size/type - aborting \n",__func__);
    abort();
  }
}
    

void rms_tagkey_clear(rms_tagkey_type * tagkey) {
  int i;
  rms_tagkey_assert_fnum(tagkey);
  switch (tagkey->rms_type) {
  case(rms_double_type):
    {
      double *tmp = (double *) tagkey->data;
      for (i=0; i < tagkey->size; i++)
        tmp[i] = 0;
    }
    break;
  case(rms_float_type):
    {
      float *tmp = (float *) tagkey->data;
      for (i=0; i < tagkey->size; i++)
        tmp[i] = 0;
    }
    break;
  default:
    fprintf(stderr,"%s: only implemented for rms_double_type and rms_float_type - aborting \n",__func__);
    abort();
  }
}


void rms_tagkey_apply(rms_tagkey_type * tagkey , double (f) (double)) {
int i;
  rms_tagkey_assert_fnum(tagkey);
  switch (tagkey->rms_type) {
  case(rms_double_type):
    {
      double *tmp = (double *) tagkey->data;
      for (i=0; i < tagkey->size; i++)
        tmp[i] = f(tmp[i]);
    }
    break;

  case(rms_float_type):
    {
      float *tmp = (float *) tagkey->data;
      for (i=0; i < tagkey->size; i++)
        tmp[i] = f(tmp[i]);
    }
    break;

  default:
    fprintf(stderr,"%s: only implemented for rms_double_type and rms_float_type - aborting \n",__func__);
    abort();
  }
}


void rms_tagkey_inplace_sqr(rms_tagkey_type * tagkey) {
  int i;
  rms_tagkey_assert_fnum(tagkey);
  switch (tagkey->rms_type) {
  case(rms_double_type):
    {
      double *tmp = (double *) tagkey->data;
      for (i=0; i < tagkey->size; i++)
        tmp[i] *= tmp[i];
    }
    break;

  case(rms_float_type):
    {
      float *tmp = (float *) tagkey->data;
      for (i=0; i < tagkey->size; i++)
        tmp[i] *= tmp[i];
    }
    break;

  default:
    fprintf(stderr,"%s: only implemented for rms_double_type and rms_float_type - aborting \n",__func__);
    abort();
  }
}


void rms_tagkey_assign(rms_tagkey_type * new, const rms_tagkey_type *src) {
  memcpy(new->data , src->data , new->size * new->sizeof_ctype);
}


void rms_tagkey_inplace_log10(rms_tagkey_type * tagkey) {
  int i;
  rms_tagkey_assert_fnum(tagkey);
  switch (tagkey->rms_type) {
  case(rms_double_type):
    {
      double *tmp = (double *) tagkey->data;
      for (i=0; i < tagkey->size; i++)
        tmp[i] = log10(tmp[i]);
    }
    break;

  case(rms_float_type):
    {
      float *tmp = (float *) tagkey->data;
      for (i=0; i < tagkey->size; i++)
        tmp[i] = log10(tmp[i]);
    }
    break;
    
  default:
    fprintf(stderr,"%s: only implemented for rms_double_type and rms_float_type - aborting \n",__func__);
    abort();
  }
}


void rms_tagkey_inplace_sqrt(rms_tagkey_type * tagkey) {
  int i;
  rms_tagkey_assert_fnum(tagkey);
  switch (tagkey->rms_type) {
  case(rms_double_type):
    {
      double *tmp = (double *) tagkey->data;
      for (i=0; i < tagkey->size; i++)
        tmp[i] = sqrt(util_double_max(0.0 , tmp[i]));
    }
    break;
    
  case(rms_float_type):
    {
      float *tmp = (float *) tagkey->data;
      for (i=0; i < tagkey->size; i++)
        tmp[i] = sqrtf(util_float_max(0.0 , tmp[i]));
    }
    break;

  default:
    fprintf(stderr,"%s: only implemented for rms_double_type and rms_float_type - aborting \n",__func__);
    abort();
  }
}


void rms_tagkey_scale(rms_tagkey_type * tagkey , double scale_factor) {
  int i;
  rms_tagkey_assert_fnum(tagkey);
  switch (tagkey->rms_type) {
  case(rms_double_type):
    {
      double *tmp = (double *) tagkey->data;
      for (i=0; i < tagkey->size; i++)
        tmp[i] *= scale_factor;
    }
    break;

  case(rms_float_type):
    {
      float *tmp = (float *) tagkey->data;
      for (i=0; i < tagkey->size; i++)
        tmp[i] *= scale_factor;
    }
    break;

  default:
    fprintf(stderr,"%s: only implemented for rms_double_type and rms_float_type - aborting \n",__func__);
    abort();
  }
}


void rms_tagkey_inplace_add(rms_tagkey_type * tagkey , const rms_tagkey_type *delta) {
  int i;
  rms_tagkey_assert_fnum2(tagkey , delta);
  switch (tagkey->rms_type) {
  case(rms_double_type):
    {
      double *tmp1       = (double *) tagkey->data;
      const double *tmp2 = (const double *) delta->data;
      for (i=0; i < tagkey->size; i++)
        tmp1[i] += tmp2[i];
    }
    break;
    
  case(rms_float_type):
    {
      float *tmp1       = (float *) tagkey->data;
      const float *tmp2 = (const float *) delta->data;
      for (i=0; i < tagkey->size; i++)
        tmp1[i] += tmp2[i];
    }
    break;

  default:
    fprintf(stderr,"%s: only implemented for rms_double_type and rms_float_type - aborting \n",__func__);
    abort();
  }
}


void rms_tagkey_inplace_add_scaled(rms_tagkey_type * tagkey , const rms_tagkey_type *delta, double factor) {
  int i;
  rms_tagkey_assert_fnum2(tagkey , delta);
  switch (tagkey->rms_type) {
  case(rms_double_type):
    {
      double *tmp1       = (double *) tagkey->data;
      const double *tmp2 = (const double *) delta->data;
      for (i=0; i < tagkey->size; i++)
        tmp1[i] += tmp2[i] * factor;
    }
    break;
    
  case(rms_float_type):
    {
      float *tmp1       = (float *) tagkey->data;
      const float *tmp2 = (const float *) delta->data;
      for (i=0; i < tagkey->size; i++)
        tmp1[i] += tmp2[i] * factor;
    }
    break;

  default:
    fprintf(stderr,"%s: only implemented for rms_double_type and rms_float_type - aborting \n",__func__);
    abort();
  }
}


void rms_tagkey_inplace_mul(rms_tagkey_type * tagkey , const rms_tagkey_type *delta) {
  int i;
  rms_tagkey_assert_fnum2(tagkey , delta);
  switch (tagkey->rms_type) {
  case(rms_double_type):
    {
      double *tmp1       = (double *) tagkey->data;
      const double *tmp2 = (const double *) delta->data;
      for (i=0; i < tagkey->size; i++)
        tmp1[i] *= tmp2[i];
    }
    break;

  case(rms_float_type):
    {
      float *tmp1       = (float *) tagkey->data;
      const float *tmp2 = (const float *) delta->data;
      for (i=0; i < tagkey->size; i++)
        tmp1[i] *= tmp2[i];
    }
    break;
    
  default:
    fprintf(stderr,"%s: only implemented for rms_double_type and rms_float_type - aborting \n",__func__);
    abort();
  }
}


#define TAGKEY_MAX_MIN(type)                             \
{                                                        \
  type * data = rms_tagkey_get_data_ref(tagkey);         \
  type max = -data[0];                                   \
  type min =  data[0];                                   \
  int i;                                                 \
  for (i=1; i < tagkey->size; i++)                       \
      util_update_ ## type ## _max_min(data[i] , &max , &min); \
  memcpy(_max , &max , tagkey->sizeof_ctype);            \
  memcpy(_min , &min , tagkey->sizeof_ctype);            \
}



void rms_tagkey_max_min(const rms_tagkey_type * tagkey , void * _max , void *_min) {
  switch (tagkey->rms_type) {
  case(rms_float_type):
    TAGKEY_MAX_MIN(float);
    break;
  case(rms_double_type):
    TAGKEY_MAX_MIN(double);
    break;
  case(rms_int_type):
    TAGKEY_MAX_MIN(int);
    break;
  default:
    fprintf(stderr,"%s: invalid type for element sum \n",__func__);
    abort();
  }
}

#undef TAGKEY_MAX_MIN





void rms_tagkey_free_(void *_tagkey) {
  rms_tagkey_type * tagkey = (rms_tagkey_type *) _tagkey;
  rms_tagkey_free(tagkey);
}


static void rms_tagkey_alloc_data(rms_tagkey_type *tagkey) {
  if (!tagkey->shared_data) {

    if (tagkey->data_size > tagkey->alloc_size) {
      void *tmp = realloc(tagkey->data , tagkey->data_size);
      if (tmp == NULL) {
        fprintf(stderr,"%s: failed to allocate: %d bytes of storage - aborting \n",__func__ , tagkey->data_size);
        abort();
      } 
      tagkey->data       = tmp;
      tagkey->alloc_size = tagkey->data_size;
      
    }
  }
}




rms_tagkey_type * rms_tagkey_copyc(const rms_tagkey_type *tagkey) {
  rms_tagkey_type *new_tagkey = rms_tagkey_alloc_empty(tagkey->endian_convert);
  
  new_tagkey->alloc_size     = 0;
  new_tagkey->size           = tagkey->size;
  new_tagkey->sizeof_ctype   = tagkey->sizeof_ctype;
  new_tagkey->data_size      = tagkey->data_size;
  new_tagkey->rms_type       = tagkey->rms_type;
  new_tagkey->data           = NULL;
  new_tagkey->shared_data    = tagkey->shared_data;

  rms_tagkey_alloc_data(new_tagkey);    
  memcpy(new_tagkey->data , tagkey->data , tagkey->data_size);
  new_tagkey->name = util_alloc_string_copy(tagkey->name);
  return new_tagkey;
}


void * rms_tagkey_copyc_(const void * _tagkey) {
  const  rms_tagkey_type * tagkey = (const rms_tagkey_type *) _tagkey;
  return rms_tagkey_copyc(tagkey);
}


static void rms_tagkey_set_data_size(rms_tagkey_type *tagkey , FILE *stream , int strlen) {

  if (tagkey->rms_type == rms_char_type) {
    if (stream != NULL) {
      const long int init_pos = ftell(stream);
      int i;
      for (i=0; i < tagkey->size; i++)
        rms_util_fskip_string(stream);
      tagkey->data_size = ftell(stream) - init_pos;
      fseek(stream , init_pos , SEEK_SET);
    } else 
      tagkey->data_size = strlen + 1;
  } else
    tagkey->data_size = tagkey->size * tagkey->sizeof_ctype;  
}



static void rms_tagkey_fread_data(rms_tagkey_type *tagkey , bool endian_convert , FILE *stream) {
  if (tagkey->alloc_size < tagkey->data_size) {
    fprintf(stderr,"%s: fatal error buffer to small - aborting \n",__func__);
    abort();
  }

  int bytes_read = fread(tagkey->data , 1 , tagkey->data_size ,  stream);
  if (bytes_read != tagkey->data_size) {
    fprintf(stderr,"%s: failed to read %d bytes - premature EOF? \n",__func__ , tagkey->data_size);
    fprintf(stderr,"%s: tagkey:  %s \n",__func__ , tagkey->name);
    abort();
  }
  if (endian_convert) 
    if (tagkey->sizeof_ctype > 1)
      util_endian_flip_vector(tagkey->data , tagkey->sizeof_ctype , tagkey->size);
}

void rms_tagkey_manual_realloc_data(rms_tagkey_type * tagkey , int data_size) {
  tagkey->data_size = data_size;
  rms_tagkey_alloc_data(tagkey);
}

void rms_tagkey_set_data(rms_tagkey_type * tagkey , const void * data) {
  if (tagkey->shared_data)
    tagkey->data = (void *) data;
  else
    memcpy(tagkey->data , data , tagkey->data_size);
}




static void rms_fread_tagkey_header(rms_tagkey_type *tagkey , FILE *stream, hash_type *type_map) {
  bool is_array;
  char type_string[7];
  
  rms_util_fread_string(type_string , 7 , stream);
  if (strcmp(type_string , rms_array_string) == 0) {
    is_array = true;
    rms_util_fread_string(type_string , 7 , stream);
  } else
    is_array = false;
  
  {
    __rms_type * rms_t   = hash_get(type_map , type_string);
    tagkey->rms_type     = rms_t->rms_type;
    tagkey->sizeof_ctype = rms_t->sizeof_ctype;
  }

  tagkey->name = realloc(tagkey->name , rms_util_fread_strlen(stream) + 1);

  rms_util_fread_string(tagkey->name , 0 , stream);
  if (is_array)
    fread(&tagkey->size , 1 , sizeof tagkey->size, stream);
  else
    tagkey->size = 1;
  rms_tagkey_set_data_size(tagkey , stream , -1);
}



static void rms_fread_realloc_tagkey(rms_tagkey_type *tagkey , bool endian_convert , FILE *stream , hash_type *type_map) {

  rms_fread_tagkey_header(tagkey , stream , type_map);
  rms_tagkey_alloc_data(tagkey);
  rms_tagkey_fread_data(tagkey , endian_convert , stream);

}



static void rms_tagkey_fwrite_data(const rms_tagkey_type * tagkey , FILE *stream) {
  int elm = fwrite(tagkey->data , 1 , tagkey->data_size , stream);
  if (elm != tagkey->data_size) {
    fprintf(stderr,"%s: failed to write %d bytes to file [tagkey:%s] - aborting \n",__func__ , tagkey->data_size , tagkey->name);
    abort();
  }
}


void rms_tagkey_fwrite(const rms_tagkey_type * tagkey , FILE *stream) {
  if (tagkey->size > 1)
    rms_util_fwrite_string("array" , stream);
  rms_util_fwrite_string(rms_type_names[tagkey->rms_type] , stream);
  rms_util_fwrite_string(tagkey->name , stream);
  if (tagkey->size > 1) {
    fwrite(&tagkey->size , sizeof tagkey->size , 1 , stream);
    rms_util_fwrite_newline(stream);
  }
  rms_tagkey_fwrite_data(tagkey , stream);
}

void rms_tagkey_fprintf(const rms_tagkey_type * tagkey, FILE *stream) {
  fprintf(stream,"    <%s> %6d %s",tagkey->name , tagkey->size , rms_type_names[tagkey->rms_type]);
  if (tagkey->size == 1) {
    if (tagkey->rms_type == rms_int_type)
      fprintf(stream, " = %d ",(( int *) tagkey->data)[0]);
    else if (tagkey->rms_type == rms_char_type)
      fprintf(stream, " = %s ",( char *) tagkey->data);
  } 
  fprintf(stream,"\n");
}


const char * rms_tagkey_get_name(const rms_tagkey_type *tagkey) {
  return tagkey->name;
}

void * rms_tagkey_get_data_ref(const rms_tagkey_type *tagkey) {
  return tagkey->data;
}


void rms_tagkey_load(rms_tagkey_type *tagkey , bool endian_convert , FILE *stream, hash_type *type_map) {
  rms_fread_realloc_tagkey(tagkey , endian_convert , stream , type_map);
}


bool rms_tagkey_char_eq(const rms_tagkey_type *tagkey , const char *keyvalue) {
  bool eq = false;
  if (tagkey->rms_type == rms_char_type) {
    if (strcmp(keyvalue , tagkey->data) == 0)
      eq = true;
  }
  return eq;
}


rms_tagkey_type * rms_tagkey_alloc_empty(bool endian_convert) {
  
  rms_tagkey_type *tagkey = malloc(sizeof *tagkey);
  tagkey->alloc_size      = 0;
  tagkey->data_size       = 0;
  tagkey->size            = 0;
  tagkey->name            = NULL;
  tagkey->data            = NULL;
  tagkey->endian_convert  = endian_convert;
  tagkey->shared_data     = false;
  
  return tagkey;
  
}


static rms_tagkey_type * rms_tagkey_alloc_initialized(const char * name , int size , rms_type_enum rms_type , bool endian_convert) {
  rms_tagkey_type *tagkey = rms_tagkey_alloc_empty(endian_convert);
  tagkey->size            = size;
  tagkey->rms_type        = rms_type;
  tagkey->sizeof_ctype    = rms_type_size[rms_type];
  tagkey->data_size       = tagkey->size * tagkey->sizeof_ctype;
  tagkey->name            = util_alloc_string_copy(name);
  return tagkey;
}


rms_tagkey_type * rms_tagkey_alloc_complete(const char * name , int size , rms_type_enum rms_type , const void * data , bool shared_data) {
  rms_tagkey_type * tag = rms_tagkey_alloc_initialized(name , size , rms_type , false);
  tag->shared_data = shared_data;

  rms_tagkey_alloc_data(tag);
  rms_tagkey_set_data(tag , data);
  
  return tag;
}


int rms_tagkey_get_sizeof_ctype(const rms_tagkey_type * key) {
  return key->sizeof_ctype;
}


rms_type_enum rms_tagkey_get_rms_type(const rms_tagkey_type * key) {
  return key->rms_type;
}


ecl_type_enum rms_tagkey_get_ecl_type(const rms_tagkey_type * key) {
  ecl_type_enum ecl_type;
  switch (key->rms_type) {
  case(rms_float_type):
    ecl_type = ECL_FLOAT_TYPE;
    break;
  case(rms_double_type):
    ecl_type = ECL_DOUBLE_TYPE;
    break;
  case(rms_int_type):
    ecl_type = ECL_INT_TYPE;
    break;
  default:
    fprintf(stderr,"%s: sorry rms_type: %d not implemented - aborting \n",__func__ , key->rms_type);
    abort();
  }
  return ecl_type;
}




void rms_tagkey_free(rms_tagkey_type *tagkey) {
  if (tagkey->name != NULL) free(tagkey->name);

  if (!tagkey->shared_data)
    if (tagkey->data != NULL) free(tagkey->data);
  
  free(tagkey);
}


rms_tagkey_type * rms_tagkey_alloc_byteswap() {
  rms_tagkey_type *tagkey = rms_tagkey_alloc_initialized("byteswaptest" , 1 , rms_int_type , false);
  rms_tagkey_alloc_data(tagkey);
  ((int *) tagkey->data)[0] = 1;
  return tagkey;
}


rms_tagkey_type * rms_tagkey_alloc_filetype(const char * filetype) {
  rms_tagkey_type *tagkey = rms_tagkey_alloc_initialized("filetype" , 1 , rms_char_type , false);
  rms_tagkey_set_data_size(tagkey , NULL , strlen(filetype));
  rms_tagkey_alloc_data(tagkey);
  sprintf(tagkey->data , "%s" , filetype);
  return tagkey;
}


rms_tagkey_type * rms_tagkey_alloc_parameter_name(const char * parameter_name) {
  rms_tagkey_type *tagkey = rms_tagkey_alloc_initialized("name" , 1 , rms_char_type , false);
  rms_tagkey_set_data_size(tagkey , NULL , strlen(parameter_name));
  rms_tagkey_alloc_data(tagkey);
  sprintf(tagkey->data , "%s" , parameter_name);
  return tagkey;
}


rms_tagkey_type * rms_tagkey_alloc_creationDate() {
  const int len = strlen("08/05/2007 08:31:39");
  struct tm ts;
  time_t now;
  rms_tagkey_type *tagkey = rms_tagkey_alloc_initialized("creationDate" , 1 , rms_char_type , false);
  
  now = time(NULL);
  localtime_r(&now , &ts);

  rms_tagkey_set_data_size(tagkey , NULL , len);
  rms_tagkey_alloc_data(tagkey);
  sprintf(tagkey->data , "%02d/%02d/%4d %02d:%02d:%02d" , 
          ts.tm_mday, 
          ts.tm_mon,
          ts.tm_year + 1900, 
          ts.tm_hour,
          ts.tm_min,
          ts.tm_sec);

  return tagkey;
}


rms_tagkey_type * rms_tagkey_alloc_dim(const char * dim, int value) {
  rms_tagkey_type *tagkey = rms_tagkey_alloc_initialized(dim , 1 , rms_int_type , false);
  rms_tagkey_alloc_data(tagkey);
  ((int *) tagkey->data)[0] = value;
  return tagkey;
}


int rms_tagkey_get_size(const rms_tagkey_type * tagkey) { return tagkey->size; }


/**
   This function will compare two tagkeys, and return true if they are
   equal. If they differ in size/type/...  or other fundamental ways a
   false is immediately returned.
*/

bool rms_tagkey_cmp(const rms_tagkey_type * tagkey1 , const rms_tagkey_type * tagkey2) {
  if (tagkey1->size     != tagkey2->size)     return false;
  if (tagkey1->rms_type != tagkey2->rms_type) return false;
  if (memcmp(tagkey1->data , tagkey1->data , tagkey1->size * tagkey1->sizeof_ctype) == 0)
    return true;
  else
    return false;
}
