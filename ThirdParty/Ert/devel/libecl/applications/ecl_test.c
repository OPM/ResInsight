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
#include <signal.h>
#include <util.h>

void install_SIGNALS(void) {
  signal(SIGSEGV , util_abort_signal);    /* Segmentation violation, i.e. overwriting memory ... */
  signal(SIGINT  , util_abort_signal);    /* Control C */
  signal(SIGTERM , util_abort_signal);    /* If killing the program with SIGTERM (the default kill signal) you will get a backtrace. 
                                             Killing with SIGKILL (-9) will not give a backtrace.*/
}


int test_ecl_file_save() {

}


int test_grdecl_loader() {
  FILE * stream = util_fopen("/private/joaho/ERT/NR/python/ctypes/test/data/eclipse/case/PERMX.grdecl" , "r");
  ecl_kw_type * kw = ecl_kw_fscanf_alloc_grdecl_dynamic( stream , "PERMX" , ECL_FLOAT_TYPE );
  ecl_kw_free( kw );
  fclose( stream );
  
  return 1;
}


int main (int argc, char **argv) {
  install_SIGNALS();
  {
    test_grdecl_loader();
    test_ecl_file_save();
  }
}




