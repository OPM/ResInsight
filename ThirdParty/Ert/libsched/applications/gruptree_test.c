/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'gruptree_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <hash.h>
#include <util.h>

#include <ert/sched/gruptree.h>

int main()
{
  gruptree_type * gruptree = gruptree_alloc();
  gruptree_register_grup(gruptree, "INJE", "FIELD");
  gruptree_register_well(gruptree, "WI1"  , "INJE");
  gruptree_register_well(gruptree, "WI2"  , "INJE");
  gruptree_register_well(gruptree, "WI3"  , "INJE");
  gruptree_register_well(gruptree, "WI4"  , "INJE");

  gruptree_register_grup(gruptree, "PLATA", "FIELD");
  gruptree_register_grup(gruptree, "INJE", "PLATA");
  gruptree_register_grup(gruptree, "PROD", "PLATA");
  gruptree_register_well(gruptree, "WP1",  "PROD");
  gruptree_register_well(gruptree, "WP2",  "PROD");
  gruptree_register_well(gruptree, "WP3",  "PROD");
  gruptree_register_well(gruptree, "WP4",  "PROD");
  gruptree_register_well(gruptree, "WP5",  "PROD");

  gruptree_register_grup(gruptree, "PLATB", "FIELD");
  gruptree_register_well(gruptree, "WPB1", "PLATB");

  gruptree_printf_grup_wells(gruptree, "FIELD");
  gruptree_printf_grup_wells(gruptree, "PLATA");
  gruptree_printf_grup_wells(gruptree, "PLATB");
  gruptree_printf_grup_wells(gruptree, "INJE");
  gruptree_printf_grup_wells(gruptree, "PROD");

  printf("Letting PLATB be a subgroup of PLATA.\n");
  gruptree_register_grup(gruptree, "PLATB", "PLATA");
  gruptree_register_well(gruptree, "SVADA", "FIELD");

  gruptree_printf_grup_wells(gruptree, "PLATA");
  gruptree_printf_grup_wells(gruptree, "FIELD");

  printf("Trying to copy the gruptree...\n");
  gruptree_type * gruptree_cpy = gruptree_copyc(gruptree);

  printf("Freeing the old gruptree..\n");
  gruptree_free(gruptree);

  printf("Trying to access new gruptree..\n");
  gruptree_printf_grup_wells(gruptree_cpy, "FIELD");
  gruptree_printf_grup_wells(gruptree_cpy, "PLATA");
  gruptree_printf_grup_wells(gruptree_cpy, "INJE");

  printf("Writing gruptree to disk..\n");
  FILE * stream = util_fopen("gruptree_stor.bin", "w");
  gruptree_fwrite(gruptree_cpy, stream);

  printf("Closing stream and free'ing gruptre..\n");
  fclose(stream);
  gruptree_free(gruptree_cpy);

  printf("Reading gruptree from disk, hang on to yer helmet..\n");
  stream = util_fopen("gruptree_stor.bin", "r");
  gruptree = gruptree_fread_alloc(stream);
  fclose(stream);

  printf("Trying to access new gruptree..\n");
  gruptree_printf_grup_wells(gruptree, "FIELD");
  gruptree_printf_grup_wells(gruptree, "PLATA");
  gruptree_printf_grup_wells(gruptree, "INJE");

  printf("Cleaning up..\n");
  gruptree_free(gruptree);


  return 0;
}
