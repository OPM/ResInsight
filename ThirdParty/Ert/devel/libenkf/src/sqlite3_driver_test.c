/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sqlite3_driver_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <sqlite3_driver.h>
#include <stdio.h>
#include <buffer.h>

static const char SQLITE3_DB_FILE[]         = "my_sqlite3_db.bin";


int main(
  int argc,
  char ** argv)
{
  //const char * id             = "poro";
  //int          realization_nr = 1;
  //int          restart_nr     = 123;
  //const char * data           = "svadafesalsdfhalkdfhasldkfahklsdfhasdkfjht";
  //int          bytesize_data  = strlen(data) + 1;
  //
  //char       * my_data;
  //int          bytesize_my_data;
  //
  //sqlite3_driver_type * driver  = sqlite3_driver_alloc(SQLITE3_DB_FILE, "default");
  //void                * _driver = (void *) driver;

  //printf("Attempting to save a node.\n");
  //sqlite3_driver_save_node( _driver, id, realization_nr, restart_nr, data, bytesize_data);
  //if( sqlite3_driver_has_node( _driver, id, realization_nr, restart_nr ) )
  //  printf("Successfully saved a node.\n");
  //else
  //  printf("Failed to save a node!!\n");
  //
  //
  //printf("Attempting to load the node.\n");
  //if( sqlite3_driver_load_node( _driver, id, realization_nr, restart_nr, (void **) &my_data, &bytesize_my_data) )
  //{
  //  printf("Loaded: %s\n", my_data);
  //  free(my_data);
  //}
  //else
  //  printf("Coulnd't find the node.\n");
  //
  //
  //printf("Deleting node.\n");
  //sqlite3_driver_unlink_node( _driver, id, realization_nr, restart_nr);
  //if( sqlite3_driver_has_node( _driver, id, realization_nr, restart_nr) )
  //  printf("Failed to delete node!!!\n");
  //else
  //  printf("Successfully deleted node.\n");
  //
  //sqlite3_driver_free( _driver );
  return 0;
};
