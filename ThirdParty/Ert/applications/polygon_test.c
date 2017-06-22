

#include <stdlib.h>
#include <geo_surface.h>
#include <geo_polygon.h>
#include <stdbool.h>
#include <util.h>

int main( int argc , char ** argv) {
  const char * irap_file = "polygontest.irap";
  
  geo_polygon_type * polygon = geo_polygon_fload_alloc_irap( irap_file );
  
  geo_polygon_free( polygon );
}
