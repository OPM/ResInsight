/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'get_well_list.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdbool.h>
#include <util.h>

#include <ert/sched/sched_file.h>
#include <ert/sched/history.h>

int main(int argc, char **argv)
{
  if(argc < 2)
  {
    printf("Usage: get_well_list.x my_sched_file.SCH\n");
    return 0;
  }
  
  int     num_wells;
  char ** well_list;

  sched_file_type * sched_file = NULL;
  history_type    * history    = NULL;


  sched_file = sched_file_alloc(-1);
  sched_file_parse(sched_file, -1 , argv[1]);


  history = history_alloc_from_sched_file(sched_file);

  well_list = history_alloc_well_list(history, &num_wells);

  for(int well_nr = 0; well_nr < num_wells; well_nr++)
    printf("%s\n", well_list[well_nr]);

  history_free(history);
  sched_file_free(sched_file);
  util_free_stringlist(well_list, num_wells);

  return 0;
}
