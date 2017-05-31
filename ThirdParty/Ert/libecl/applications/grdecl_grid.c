/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'grdecl_grid.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdlib.h>
#include <stdio.h>

#include <ert/util/util.h>

#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/ecl_kw_magic.h>


int main(int argc , char ** argv) {
  FILE * stream = util_fopen( argv[1] , "r");
  ecl_kw_type * gridhead_kw = ecl_kw_fscanf_alloc_grdecl_dynamic__( stream , SPECGRID_KW , false , ECL_INT );
  ecl_kw_type * zcorn_kw    = ecl_kw_fscanf_alloc_grdecl_dynamic( stream , ZCORN_KW  , ECL_FLOAT );
  ecl_kw_type * coord_kw    = ecl_kw_fscanf_alloc_grdecl_dynamic( stream , COORD_KW  , ECL_FLOAT );
  ecl_kw_type * actnum_kw   = ecl_kw_fscanf_alloc_grdecl_dynamic( stream , ACTNUM_KW , ECL_INT );
  
  {
    int nx = ecl_kw_iget_int( gridhead_kw , SPECGRID_NX_INDEX );
    int ny = ecl_kw_iget_int( gridhead_kw , SPECGRID_NY_INDEX );
    int nz = ecl_kw_iget_int( gridhead_kw , SPECGRID_NZ_INDEX );
    ecl_grid_type * ecl_grid = ecl_grid_alloc_GRDECL_kw( nx , ny , nz, zcorn_kw, coord_kw , actnum_kw , NULL );
    /* .... */
    ecl_grid_free( ecl_grid );
  }
  ecl_kw_free( gridhead_kw );
  ecl_kw_free( zcorn_kw );
  ecl_kw_free( actnum_kw );
  ecl_kw_free( coord_kw );
  fclose( stream );
}
