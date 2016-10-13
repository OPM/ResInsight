/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_kw_untyped.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ctype.h>

#include <ert/util/vector.h>
#include <ert/util/util.h>
#include <ert/util/stringlist.h>

#include <ert/sched/sched_kw_untyped.h>
#include <ert/sched/sched_util.h>




struct sched_kw_untyped_struct {
  int        rec_len;   
  char      *kw_name;  /* The name of the current keyword. */
  char      *buffer;   /* The content of the keyword is just appended in one char * pointer. */
};



/*****************************************************************/

//static int get_fixed_record_length(const char * kw_name)
//{
//  
//   if( strcmp(kw_name , "RPTSCHED") == 0) { return  1;}
//   if( strcmp(kw_name , "DRSDT"   ) == 0) { return  1;}
//   if( strcmp(kw_name , "SKIPREST") == 0) { return  0;}
//   if( strcmp(kw_name , "RPTRST"  ) == 0) { return  1;}
//   if( strcmp(kw_name , "TUNING"  ) == 0) { return  3;}
//   if( strcmp(kw_name , "WHISTCTL") == 0) { return  1;}
//   if( strcmp(kw_name , "TIME"    ) == 0) { return  1;}
//   if( strcmp(kw_name , "VAPPARS" ) == 0) { return  1;}
//   if( strcmp(kw_name , "NETBALAN") == 0) { return  1;}
//   if( strcmp(kw_name , "WPAVE"   ) == 0) { return  1;}
//   if( strcmp(kw_name , "VFPTABL" ) == 0) { return  1;}
//   if( strcmp(kw_name , "GUIDERAT") == 0) { return  1;} 
//   
//   return -1;  /* Can not use 0 - because some KW actually have 0 as a valid fixed value. */
//}



sched_kw_untyped_type * sched_kw_untyped_alloc_empty(const char * kw_name , int rec_len) {
  sched_kw_untyped_type * kw = util_malloc(sizeof *kw );
  kw->kw_name   = util_alloc_string_copy(kw_name);
  kw->rec_len   = rec_len;
  kw->buffer    = NULL; 
  return kw;
}



/** This is exported for the keywords  which are just a minimum extension of untyped. */
void sched_kw_untyped_add_line(sched_kw_untyped_type * kw , const char *line, bool pad) {
  if (pad) {
    char * padded_line = util_alloc_sprintf("   %s\n" , line);
    kw->buffer = util_strcat_realloc(kw->buffer , padded_line);
    free(padded_line);
  } else
    kw->buffer = util_strcat_realloc(kw->buffer , line);
}



/*****************************************************************/


void sched_kw_untyped_add_tokens( sched_kw_untyped_type * kw , const stringlist_type * line_tokens) {
  char * line_buffer = stringlist_alloc_joined_string( line_tokens , "  ");
  sched_kw_untyped_add_line(kw, line_buffer , true );
  free( line_buffer );
}





sched_kw_untyped_type * sched_kw_untyped_alloc(const stringlist_type * tokens , int * token_index , int rec_len) {
  const char * kw_name = NULL;

  /* First part - get hold of the kw name */
  {
    int kw_index = (*token_index) - 1;
    do {
      kw_name = stringlist_iget( tokens , kw_index);
      if (util_string_isspace( kw_name ))
        kw_name = NULL;  /* Try again */
      kw_index--;
    } while (kw_name == NULL && (kw_index >= 0));

    if (kw_name == NULL)
      util_abort("%s: internal error - failed to identify untyped kw name \n",__func__);
  }
  
  
  {
    bool eokw                  = false;
    sched_kw_untyped_type * kw = sched_kw_untyped_alloc_empty( kw_name , rec_len);
    int line_nr                = 0;
    do {
      stringlist_type * line_tokens = sched_util_alloc_line_tokens( tokens , true , 0 , token_index );
      line_nr++;
      if (line_tokens == NULL) {
        eokw = true;
        if (line_nr < kw->rec_len)
          util_abort("%s: premature end of keyword:%s \n",__func__ , kw_name);
      } else {
        sched_kw_untyped_add_tokens( kw , line_tokens );
        stringlist_free( line_tokens );
      }
      
      if (line_nr == kw->rec_len)
        eokw = true;
      
    } while (!eokw);
    return kw;
  }
}




void sched_kw_untyped_fprintf(const sched_kw_untyped_type *kw , FILE *stream) {
  fprintf(stream , "%s \n" , kw->kw_name);
  {
    if (kw->buffer != NULL)
      fprintf(stream , "%s" , kw->buffer);
    
    if(kw->rec_len < 0)
      fprintf(stream , "/\n\n");
    else
      fprintf(stream, "\n\n");
  }
}



void sched_kw_untyped_free(sched_kw_untyped_type * kw) {
  util_safe_free(kw->buffer);
  free(kw->kw_name);
  free(kw);
}

sched_kw_untyped_type * sched_kw_untyped_copyc(const sched_kw_untyped_type * kw) {
  util_abort("%s: not implemented ... \n",__func__);
  return NULL;
}

/*****************************************************************/

KW_FREE_IMPL(untyped)         
KW_FPRINTF_IMPL(untyped)   
KW_COPYC_IMPL(untyped)










