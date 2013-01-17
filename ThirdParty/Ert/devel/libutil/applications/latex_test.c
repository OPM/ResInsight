#include <stdlib.h>
#include <stdio.h>
#include <util.h>
#include <latex.h>


int main( int argc , char ** argv) {
  latex_type * latex = latex_alloc( argv[1] , false );

  latex_link_path( latex , "include/figs");
  if (latex_compile( latex , false , false ))
    printf("Have created pdf: %s \n",latex_get_target_file( latex ));
  else
    printf("Compilation failed - check directory: %s \n",latex_get_runpath( latex ));

  latex_free( latex );
}
