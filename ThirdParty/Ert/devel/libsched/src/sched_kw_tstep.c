/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_kw_tstep.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <string.h>
#include <time.h>

#include <ert/util/double_vector.h>
#include <ert/util/util.h>

#include <ert/sched/sched_util.h>
#include <ert/sched/sched_kw_tstep.h>
#include <ert/sched/sched_macros.h>



struct sched_kw_tstep_struct {
  double_vector_type * tstep_list;
}; 



/*****************************************************************/


static void sched_kw_tstep_add_tstep( sched_kw_tstep_type * kw, double tstep ) {
  double_vector_append( kw->tstep_list , tstep );
}


static void sched_kw_tstep_add_tstep_string( sched_kw_tstep_type * kw, const char * tstep_string) {
  double tstep;
  if (util_sscanf_double( tstep_string , &tstep ))
    sched_kw_tstep_add_tstep(kw , tstep );
  else
    util_abort("%s: failed to parse:%s as a floating point number \n",__func__ , tstep_string);
}






static sched_kw_tstep_type * sched_kw_tstep_alloc_empty(){
  sched_kw_tstep_type *tstep = util_malloc(sizeof * tstep );
  tstep->tstep_list          = double_vector_alloc(0 , 0);
  return tstep;
}



/*****************************************************************/

sched_kw_tstep_type * sched_kw_tstep_alloc(const stringlist_type * tokens , int * token_index ) {
  sched_kw_tstep_type * kw = sched_kw_tstep_alloc_empty();
  stringlist_type * line_tokens = sched_util_alloc_line_tokens( tokens , false , 0 , token_index );

  if (line_tokens == NULL)
    util_abort("%s: hmmmm - TSTEP keyword without and data \n",__func__);
  else {
    int i;
    
    for (i=0; i < stringlist_get_size( line_tokens ); i++)
      sched_kw_tstep_add_tstep_string( kw , stringlist_iget( line_tokens , i ));
    
    stringlist_free( line_tokens );
  } 
  
  return kw;
}


void sched_kw_tstep_fprintf(const sched_kw_tstep_type *kw , FILE *stream) {
  fprintf(stream,"TSTEP\n  ");
  {
    int i;
    for (i=0; i < double_vector_size( kw->tstep_list ); i++)
      fprintf(stream, "%7.3f", double_vector_iget( kw->tstep_list , i));
  }
  fprintf(stream , " /\n\n");
}



void sched_kw_tstep_free(sched_kw_tstep_type * kw) {
  double_vector_free(kw->tstep_list);
  free(kw);
}



int sched_kw_tstep_get_size(const sched_kw_tstep_type * kw)
{
  return double_vector_size(kw->tstep_list);
}



sched_kw_tstep_type * sched_kw_tstep_alloc_from_double(double step)
{
  sched_kw_tstep_type * kw = sched_kw_tstep_alloc_empty();
  double_vector_append( kw->tstep_list , step );
  return kw;
}

sched_kw_tstep_type * sched_kw_tstep_copyc(const sched_kw_tstep_type * kw) {
  util_abort("%s: not implemented ... \n",__func__);
  return NULL;
}



double sched_kw_tstep_iget_step(const sched_kw_tstep_type * kw, int i)
{
  return double_vector_iget( kw->tstep_list , i );
}


int sched_kw_tstep_get_length( const sched_kw_tstep_type * kw) {
  return double_vector_size( kw->tstep_list );
}

time_t sched_kw_tstep_get_new_time(const sched_kw_tstep_type *kw, time_t curr_time)
{
  double step_days = sched_kw_tstep_iget_step(kw , 0);
  time_t new_time  = curr_time;
  util_inplace_forward_days(&new_time, step_days);
  return new_time;
}


/*****************************************************************/

KW_IMPL(tstep)


