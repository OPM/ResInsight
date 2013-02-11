/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'rms_tag.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/hash.h>
#include <ert/util/vector.h>
#include <ert/util/util.h>

#include <ert/rms/rms_tag.h>
#include <ert/rms/rms_util.h>
#include <ert/rms/rms_tagkey.h>

static const char * rms_eof_tag           = "eof";
static const char * rms_starttag_string   = "tag";
static const char * rms_endtag_string     = "endtag";


#define SHARED    0
#define OWNED_REF 1
#define COPY      2

#define RMS_TAG_TYPE_ID  4431296

struct rms_tag_struct {
  UTIL_TYPE_ID_DECLARATION;
  char         * name;
  vector_type  * key_list;
  hash_type    * key_hash;   /* Hash of tagkey instances */
};

/*****************************************************************/


rms_tag_type * rms_tag_alloc(const char * name) {
  rms_tag_type *tag = malloc(sizeof *tag);
  UTIL_TYPE_ID_INIT( tag , RMS_TAG_TYPE_ID )
  tag->name = NULL;
  tag->key_hash = hash_alloc();
  tag->key_list = vector_alloc_new();
  if (name != NULL)
    tag->name = util_alloc_string_copy(name);
  return tag;
}


static UTIL_SAFE_CAST_FUNCTION( rms_tag , RMS_TAG_TYPE_ID )


void rms_tag_free(rms_tag_type *tag) {
  free(tag->name);
  hash_free(tag->key_hash);
  vector_free(tag->key_list);
  free(tag);
}

void rms_tag_free__(void * arg) {
  rms_tag_type * tag = rms_tag_safe_cast( arg );
  rms_tag_free( tag );
}

  
const char * rms_tag_get_name(const rms_tag_type *tag) {
  return tag->name;
}



/*static*/ 
void rms_tag_fread_header(rms_tag_type *tag , FILE *stream , bool *eof_tag) {
  char *buffer;
  *eof_tag = false;
  buffer = util_calloc( 4 , sizeof * buffer);
  if (rms_util_fread_string(buffer , 4 , stream )) {
    if (strcmp(buffer , rms_starttag_string) == 0) {
      /* OK */
      {
        char *tmp = util_calloc( rms_util_fread_strlen(stream) + 1 , sizeof * tmp);
        rms_util_fread_string(tmp , 0 , stream);
        tag->name = tmp;
        if (strcmp(tag->name , rms_eof_tag) == 0)
          *eof_tag = true;
      }
    } else 
      util_abort("%s: not at tag - header aborting \n",__func__);
  } else 
    util_abort("%s: not at tag - header aborting \n",__func__);
  
  free(buffer);
}



/**
   This function does a "two-level" comparison. 
   
   1. tag->name is compared with tagname.
   2. Iff test number one succeeds we go further to step2. The second will always suceed if tagkey_name == NULL.
   
*/
   
bool rms_tag_name_eq(const rms_tag_type *tag , const char * tagname , const char *tagkey_name , const char *keyvalue) {
  bool eq = false;
  if (strcmp(tag->name , tagname) == 0) {
    if (tagkey_name != NULL && keyvalue != NULL) {
      if (hash_has_key(tag->key_hash , tagkey_name)) {
        const rms_tagkey_type *tagkey = hash_get(tag->key_hash , tagkey_name);
        eq = rms_tagkey_char_eq(tagkey , keyvalue);
      }
    } else
      eq = true;
  }
  return eq;
}




rms_tagkey_type * rms_tag_get_key(const rms_tag_type *tag , const char *keyname) {
  if (hash_has_key(tag->key_hash , keyname)) 
    return hash_get(tag->key_hash, keyname);
  else 
    return NULL;
}


rms_tagkey_type * rms_tag_get_datakey(const rms_tag_type *tag) {
  return rms_tag_get_key(tag , "data");
}


const char * rms_tag_get_namekey_name(const rms_tag_type * tag) {
  rms_tagkey_type * name_key = rms_tag_get_key(tag , "name");
  if (name_key == NULL) 
    util_abort("%s: no name tagkey defined for this tag - aborting \n",__func__);
  
  return rms_tagkey_get_data_ref(name_key);
}


int rms_tag_get_datakey_sizeof_ctype(const rms_tag_type * tag) {
  rms_tagkey_type * data_key = rms_tag_get_key(tag , "data");
  if (data_key == NULL) 
    util_abort("%s: no data tagkey defined for this tag - aborting \n",__func__);
  
  return rms_tagkey_get_sizeof_ctype(data_key);
}





static void rms_tag_add_tagkey(const rms_tag_type *tag , const rms_tagkey_type *tagkey, int mem_mode) {

  switch (mem_mode) {
  case(COPY):
    vector_append_owned_ref( tag->key_list , rms_tagkey_copyc( tagkey ) , rms_tagkey_free_ );
    break;
  case(OWNED_REF):
    vector_append_owned_ref( tag->key_list , tagkey , rms_tagkey_free_ );
    break;
  case(SHARED):
    vector_append_ref( tag->key_list , tagkey );
    break;
  }
  
  hash_insert_ref(tag->key_hash , rms_tagkey_get_name(tagkey) , tagkey);
}



static bool rms_tag_at_endtag(FILE *stream) {
  const int init_pos = ftell(stream);
  bool at_endtag;
  char tag[7];
  if (rms_util_fread_string(tag , 7 , stream)) {
    if (strcmp(tag , rms_endtag_string) == 0)
      at_endtag = true;
    else
      at_endtag = false;
  } else
    at_endtag = false;
  
  if (!at_endtag)
    fseek(stream , init_pos , SEEK_SET);
  return at_endtag;
}


void rms_fread_tag(rms_tag_type *tag, FILE *stream , hash_type *type_map , bool endian_convert , bool *at_eof) {
  rms_tag_fread_header(tag , stream , at_eof);
  if (!*at_eof) {
    rms_tagkey_type *tagkey = rms_tagkey_alloc_empty(endian_convert);
    while (! rms_tag_at_endtag(stream)) {
      rms_tagkey_load(tagkey , endian_convert , stream , type_map);
      rms_tag_add_tagkey(tag , tagkey , COPY);
    }
    rms_tagkey_free(tagkey);
  }
}



rms_tag_type * rms_tag_fread_alloc(FILE *stream , hash_type *type_map , bool endian_convert , bool *at_eof) {
  rms_tag_type *tag = rms_tag_alloc(NULL);
  rms_fread_tag(tag , stream , type_map , endian_convert , at_eof);
  return tag;
}



void rms_tag_fwrite(const rms_tag_type * tag , FILE * stream) {
  rms_util_fwrite_string("tag"     , stream);
  rms_util_fwrite_string(tag->name , stream);
  {
    
    int i;
    for (i=0; i < vector_get_size( tag->key_list ); i++) {
      const rms_tagkey_type * tagkey = vector_iget_const( tag->key_list , i );
      rms_tagkey_fwrite( tagkey , stream);
    }

  }
  rms_util_fwrite_string("endtag" , stream);
}


void rms_tag_fprintf(const rms_tag_type * tag , FILE * stream) {
  fprintf(stream , "  <%s>\n",tag->name);
  {
    
    int i;
    for (i=0; i < vector_get_size( tag->key_list ); i++) {
      const rms_tagkey_type * tagkey = vector_iget_const( tag->key_list , i );
      rms_tagkey_fprintf( tagkey , stream);
    }
    
  }
  fprintf(stream , "  </%s>\n",tag->name);
}


void rms_tag_fwrite_eof(FILE *stream) {
  rms_tag_type * tag = rms_tag_alloc("eof");
  rms_tag_fwrite(tag , stream);
  rms_tag_free(tag);
}


void rms_tag_fwrite_filedata(const char * filetype, FILE *stream) {
  rms_tag_type * tag = rms_tag_alloc("filedata");

  rms_tag_add_tagkey(tag , rms_tagkey_alloc_byteswap()         , OWNED_REF);
  rms_tag_add_tagkey(tag , rms_tagkey_alloc_filetype(filetype) , OWNED_REF);
  rms_tag_add_tagkey(tag , rms_tagkey_alloc_creationDate()     , OWNED_REF);
  
  rms_tag_fwrite(tag , stream);
  rms_tag_free(tag);
}


rms_tag_type * rms_tag_alloc_dimensions(int nX , int nY , int nZ) {
  rms_tag_type * tag = rms_tag_alloc("dimensions");
  
  rms_tag_add_tagkey(tag , rms_tagkey_alloc_dim("nX", nX) , OWNED_REF);
  rms_tag_add_tagkey(tag , rms_tagkey_alloc_dim("nY", nY) , OWNED_REF);
  rms_tag_add_tagkey(tag , rms_tagkey_alloc_dim("nZ", nZ) , OWNED_REF);

  return tag;
}


void rms_tag_fwrite_dimensions(int nX , int nY , int nZ , FILE *stream) {
  rms_tag_type * tag = rms_tag_alloc_dimensions(nX , nY , nZ);
  rms_tag_fwrite(tag , stream);
  rms_tag_free(tag);
}


void rms_tag_fwrite_parameter(const char *param_name , const rms_tagkey_type *data_key, FILE *stream) {
  rms_tag_type * tag = rms_tag_alloc("parameter");
  
  rms_tag_add_tagkey(tag , rms_tagkey_alloc_parameter_name(param_name) , OWNED_REF);
  rms_tag_add_tagkey(tag , data_key , SHARED);
  rms_tag_fwrite(tag , stream);
  rms_tag_free(tag);
  
}


const char *rms_tag_name_ref(const rms_tag_type * tag) {
  return tag->name;
}


