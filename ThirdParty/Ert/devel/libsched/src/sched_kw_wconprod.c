/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_kw_wconprod.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <string.h>

#include <ert/util/stringlist.h>
#include <ert/util/util.h>

#include <ert/sched/sched_kw_wconprod.h>
#include <ert/sched/sched_kw_untyped.h>
#include <ert/sched/sched_util.h>




/**
   This file implements a very basic support for the WCONPROD
   keyword. It internalizes a list of well names, apart from that all
   information is just tucked into a untyped keyword.

   This means that all the functionality which is sppurted by the
   sched_kw_wconprod implementation is asking for well names.
*/




struct sched_kw_wconprod_struct {
  sched_kw_untyped_type * untyped_kw;
  stringlist_type       * wells;
};





static sched_kw_wconprod_type * sched_kw_wconprod_alloc_empty(bool alloc_untyped)
{
  sched_kw_wconprod_type * kw = util_malloc(sizeof * kw);
  kw->wells      = stringlist_alloc_new();
  if (alloc_untyped)
    kw->untyped_kw = sched_kw_untyped_alloc_empty("WCONPROD" , -1 /* -1: Variable length keyword */);
  else
    kw->untyped_kw = NULL;
  return kw;
}



void sched_kw_wconprod_free(sched_kw_wconprod_type * kw)
{
  stringlist_free( kw->wells );
  sched_kw_untyped_free(kw->untyped_kw);
  free( kw );
}


static void sched_kw_wconprod_add_well(sched_kw_wconprod_type * kw , const char * well) {
  stringlist_append_copy(kw->wells , well);
}





sched_kw_wconprod_type * sched_kw_wconprod_alloc(const stringlist_type * tokens , int * token_index ) {
  sched_kw_wconprod_type * kw = sched_kw_wconprod_alloc_empty( true );
  int eokw                    = false;
  do {
    stringlist_type * line_tokens = sched_util_alloc_line_tokens( tokens , true , 0 , token_index );
    if (line_tokens == NULL)
      eokw = true;
    else {
      char * well = util_alloc_dequoted_copy( stringlist_iget( line_tokens , 0 ) );
      sched_kw_wconprod_add_well(kw , well);
      sched_kw_untyped_add_tokens(kw->untyped_kw , line_tokens);
      stringlist_free( line_tokens );
      free( well );
    } 
    
  } while (!eokw);
  return kw;
}


void sched_kw_wconprod_fprintf(const sched_kw_wconprod_type * kw , FILE * stream) {
  sched_kw_untyped_fprintf( kw->untyped_kw , stream );
}


char ** sched_kw_wconprod_alloc_wells_copy( const sched_kw_wconprod_type * kw , int * num_wells) {
  *num_wells = stringlist_get_size( kw->wells );
  return stringlist_alloc_char_copy( kw->wells );
}


sched_kw_wconprod_type * sched_kw_wconprod_copyc(const sched_kw_wconprod_type * kw) {
  util_abort("%s: not implemented ... \n",__func__);
  return NULL;
}


void sched_kw_wconprod_init_well_list( const sched_kw_wconprod_type * kw , stringlist_type * well_list) {
  stringlist_clear( well_list );
  {
    int iw;
    for (iw = 0; iw < stringlist_get_size( kw->wells ); iw++) 
      stringlist_append_ref( well_list , stringlist_iget( kw->wells , iw) );
  }
}



/*****************************************************************/

KW_IMPL(wconprod)
     
