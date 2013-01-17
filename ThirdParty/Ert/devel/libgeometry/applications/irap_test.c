

#include <stdlib.h>
#include <geo_surface.h>
#include <stdbool.h>
#include <util.h>

int main( int argc , char ** argv) {
  const char * irap_file = "surface.irap";
  {
    geo_surface_type * surface = geo_surface_fload_alloc_irap( irap_file , true );
    geo_surface_fprintf_irap( surface , "surface2.irap");
    geo_surface_free( surface );
  }

  {
    geo_surface_type * surface = geo_surface_fload_alloc_irap( irap_file , false );

    {
      double * zlist = util_calloc( geo_surface_get_size( surface ) , sizeof * zlist );
      geo_surface_fload_irap_zcoord( surface , irap_file , zlist );
      free( zlist );
    }
    
    geo_surface_free( surface );
  }
}
