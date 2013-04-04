/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'config_error.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdio.h>

#include <ert/util/util.h>
#include <ert/util/stringlist.h>

#include <ert/config/config_error.h>

struct config_error_struct {
  stringlist_type * error_list;
};



config_error_type * config_error_alloc() {
  config_error_type * error = util_malloc( sizeof * error );
  error->error_list = stringlist_alloc_new();
  return error;
}


config_error_type * config_error_alloc_copy( const config_error_type * src_error) {
  config_error_type * config_error = config_error_alloc();
  stringlist_deep_copy( config_error->error_list , src_error->error_list );
  return config_error;
}


bool config_error_equal( const config_error_type * error1 , const config_error_type * error2) {
  return stringlist_equal( error1->error_list , error2->error_list );
}

void config_error_free( config_error_type * error ) {
  stringlist_free( error->error_list );
  free( error );
}



void config_error_add( config_error_type * error , char * new_error) {
  stringlist_append_owned_ref( error->error_list , new_error );
}



void config_error_clear( config_error_type * error ) {
  stringlist_clear( error->error_list );
}


int config_error_count( const config_error_type * error )  {
  return stringlist_get_size( error->error_list );
}


const char * config_error_iget( const config_error_type * error , int index) {
  return stringlist_iget( error->error_list , index );
}


void config_error_fprintf( const config_error_type * error , bool add_count , FILE * stream ) {
  int error_nr;

  for (error_nr = 0; error_nr < stringlist_get_size( error->error_list ); error_nr++) {
    if (add_count) 
      fprintf(stream , "  %02d: " , error_nr);
    
    fprintf( stream , "%s\n" , stringlist_iget( error->error_list , error_nr));
  }
}
