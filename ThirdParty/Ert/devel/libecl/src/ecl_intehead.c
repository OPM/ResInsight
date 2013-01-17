/*
   Copyright (C) 2011  Statoil ASA, Norway. 
   
   The file 'ecl_intehead.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <util.h>
#include <ecl_kw.h>
#include <ecl_intehead.h>
#include <ecl_kw_magic.h>

static time_t intehead_date( int day , int month , int year) {
  return util_make_date( day , month, year );
}


time_t ecl_intehead_date( const ecl_kw_type * intehead_kw ) {
  return intehead_date( ecl_kw_iget_int( intehead_kw , INTEHEAD_DAY_INDEX)   , 
                        ecl_kw_iget_int( intehead_kw , INTEHEAD_MONTH_INDEX) , 
                        ecl_kw_iget_int( intehead_kw , INTEHEAD_YEAR_INDEX)  );
}



ecl_intehead_type * ecl_intehead_alloc( const ecl_kw_type * intehead_kw ) {
  ecl_intehead_type * intehead = util_malloc( sizeof * intehead );
  const int * data = (const int *) ecl_kw_get_void_ptr( intehead_kw );

  intehead->day       = data[INTEHEAD_DAY_INDEX];
  intehead->month     = data[INTEHEAD_MONTH_INDEX];
  intehead->year      = data[INTEHEAD_YEAR_INDEX];
  intehead->version   = data[INTEHEAD_IPROG_INDEX];
  intehead->phase_sum = data[INTEHEAD_PHASE_INDEX];

  intehead->nx        = data[INTEHEAD_NX_INDEX];
  intehead->ny        = data[INTEHEAD_NY_INDEX];
  intehead->nz        = data[INTEHEAD_NZ_INDEX];
  intehead->nactive   = data[INTEHEAD_NACTIVE_INDEX];

  intehead->nwells    = data[INTEHEAD_NWELLS_INDEX];
  intehead->niwelz    = data[INTEHEAD_NIWELZ_INDEX];
  intehead->nzwelz    = data[INTEHEAD_NZWELZ_INDEX];

  intehead->niconz    = data[INTEHEAD_NICONZ_INDEX];
  intehead->ncwmax    = data[INTEHEAD_NCWMAX_INDEX];

  intehead->nisegz    = data[INTEHEAD_NISEGZ_INDEX];
  intehead->nsegmx    = data[INTEHEAD_NSEGMX_INDEX];
  intehead->nswlmx    = data[INTEHEAD_NSWLMX_INDEX];

  // The only derived quantity
  intehead->sim_time  = intehead_date( intehead->day , intehead->month , intehead->year );
  return intehead;
}


void ecl_intehead_fprintf( const ecl_intehead_type * header , FILE * stream) {
  fprintf(stream , "nx      %d \n",header->nx);
  fprintf(stream , "nwells  %d \n",header->nwells);
  fprintf(stream , "niwelz  %d \n\n",header->niwelz);
}

void ecl_intehead_free( ecl_intehead_type * intehead ) {
  free( intehead );
}
