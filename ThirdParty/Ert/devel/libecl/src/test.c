/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ecl_grav.h>
#include <ecl_file.h>

int main (int argc, char **argv) {
  const char * path = "/d/proj/bg/restroll2/restek2/TEG/simu_HM2011";
  const char * base = "BCUPD_HISTORYMATCH_JAN11_ECL20072";
  {
    char * grid_file = ecl_util_alloc_exfilename( path , base , ECL_EGRID_FILE , false , 0 );
    char * init_file = ecl_util_alloc_exfilename( path , base , ECL_INIT_FILE , false , 0 );
    char * restart_file = ecl_util_alloc_exfilename( path , base , ECL_UNIFIED_RESTART_FILE , false , 0 );

    ecl_grav_type * ecl_grav = ecl_grav_alloc( grid_file , init_file );
    {
      ecl_file_type * base_survey    = ecl_file_fread_alloc_unrst_section( restart_file , 23 );
      ecl_file_type * monitor_survey = ecl_file_fread_alloc_unrst_section( restart_file , 206 );

      ecl_grav_add_survey(ecl_grav  , "BASE"    , base_survey );
      ecl_grav_add_survey( ecl_grav , "MONITOR" , monitor_survey );
      
      printf("grav_eval: %g \n",ecl_grav_eval( ecl_grav , "BASE" , "MONITOR" , 541003 , 6709907 , 297.023);
             
      ecl_file_free( base_survey );
      ecl_file_free( monitor_survey );
    }
    free( grid_file );
    free( init_file );
    free( restart_file );
  }
}




