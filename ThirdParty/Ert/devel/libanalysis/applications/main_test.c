#include <stdlib.h>
#include <stdio.h>
#include <analysis_module.h>
#include <rng.h>


int main( int argc , char ** argv) {
  rng_type * rng = NULL;
  if (0)
  {
    analysis_module_type * module = analysis_module_alloc_external( rng ,  "./simple_enkf.so" , "SimpleEnKF" );
    if (module != NULL) {
      analysis_module_set_var( module , "FLAG" , "42" );
      analysis_module_set_var( module , "VarX" , "42.77" );
      analysis_module_free( module );
    } else
      fprintf(stderr,"Hmmmm - failed to load external analysis module. \n");
    
  }


  {
    analysis_module_type * module = analysis_module_alloc_internal( rng , "simple_enkf_symbol_table", "SimpleEnKF" );
    if (module != NULL) {
      analysis_module_set_var( module , "FLAG" , "42" );
      analysis_module_set_var( module , "VarX" , "42.7708" );
      analysis_module_free( module );
    } else
      fprintf(stderr,"Hmmmm - failed to load internal analysis module. \n");
  }
  
}
