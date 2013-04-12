/*
   Copyright (C) 2011  Statoil ASA, Norway. 
   
   The file 'ecl_rsthead.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/util.h>

#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_rsthead.h>
#include <ert/ecl/ecl_kw_magic.h>


static time_t rsthead_date( int day , int month , int year) {
  return util_make_date( day , month, year );
}


time_t ecl_rsthead_date( const ecl_kw_type * intehead_kw ) {
  return rsthead_date(  ecl_kw_iget_int( intehead_kw , INTEHEAD_DAY_INDEX)   , 
                        ecl_kw_iget_int( intehead_kw , INTEHEAD_MONTH_INDEX) , 
                        ecl_kw_iget_int( intehead_kw , INTEHEAD_YEAR_INDEX)  );
}




ecl_rsthead_type * ecl_rsthead_ialloc( const ecl_file_type * rst_file , int occurence) {
  if (ecl_file_get_num_named_kw( rst_file , INTEHEAD_KW) > occurence) {
    const ecl_kw_type * intehead_kw = ecl_file_iget_named_kw( rst_file , INTEHEAD_KW , occurence);
    const ecl_kw_type * logihead_kw = ecl_file_iget_named_kw( rst_file , LOGIHEAD_KW , occurence);
    const ecl_kw_type * doubhead_kw = ecl_file_iget_named_kw( rst_file , DOUBHEAD_KW , occurence);
    
    ecl_rsthead_type * rsthead = util_malloc( sizeof * rsthead );
    
    {
      const int * data = (const int *) ecl_kw_get_void_ptr( intehead_kw );
      
      rsthead->day       = data[INTEHEAD_DAY_INDEX];
      rsthead->month     = data[INTEHEAD_MONTH_INDEX];
      rsthead->year      = data[INTEHEAD_YEAR_INDEX];
      rsthead->version   = data[INTEHEAD_IPROG_INDEX];
      rsthead->phase_sum = data[INTEHEAD_PHASE_INDEX];
      
      rsthead->nx        = data[INTEHEAD_NX_INDEX];
      rsthead->ny        = data[INTEHEAD_NY_INDEX];
      rsthead->nz        = data[INTEHEAD_NZ_INDEX];
      rsthead->nactive   = data[INTEHEAD_NACTIVE_INDEX];
      
      rsthead->nwells    = data[INTEHEAD_NWELLS_INDEX];
      rsthead->niwelz    = data[INTEHEAD_NIWELZ_INDEX];
      rsthead->nzwelz    = data[INTEHEAD_NZWELZ_INDEX];
      
      rsthead->niconz    = data[INTEHEAD_NICONZ_INDEX];
      rsthead->ncwmax    = data[INTEHEAD_NCWMAX_INDEX];
      
      rsthead->nisegz    = data[INTEHEAD_NISEGZ_INDEX];
      rsthead->nsegmx    = data[INTEHEAD_NSEGMX_INDEX];
      rsthead->nswlmx    = data[INTEHEAD_NSWLMX_INDEX];
      
      // The only derived quantity
      rsthead->sim_time  = rsthead_date( rsthead->day , rsthead->month , rsthead->year );
    }
    rsthead->dualp    = ecl_kw_iget_bool( logihead_kw , LOGIHEAD_DUALP_INDEX);
    rsthead->sim_days = ecl_kw_iget_double( doubhead_kw , DOUBHEAD_DAYS_INDEX );
    
    return rsthead;
  } else
    return NULL;
}


ecl_rsthead_type * ecl_rsthead_alloc( const ecl_file_type * rst_file) {
  return ecl_rsthead_ialloc( rst_file , 0 );
}


void ecl_rsthead_fprintf( const ecl_rsthead_type * header , FILE * stream) {
  fprintf(stream , "nx      %d \n",header->nx);
  fprintf(stream , "nwells  %d \n",header->nwells);
  fprintf(stream , "niwelz  %d \n\n",header->niwelz);
}


bool ecl_rsthead_equal( const ecl_rsthead_type * header1 , const ecl_rsthead_type * header2) {
  bool equal = true;
  equal = equal && (header1->day == header2->day);
  equal = equal && (header1->year == header2->year);
  equal = equal && (header1->month == header2->month);
  equal = equal && (header1->sim_time == header2->sim_time);
  equal = equal && (header1->version == header2->version);
  equal = equal && (header1->phase_sum == header2->phase_sum);
  equal = equal && (header1->nx == header2->nx);
  equal = equal && (header1->ny == header2->ny);
  equal = equal && (header1->nz == header2->nz);
  equal = equal && (header1->nactive == header2->nactive);
  equal = equal && (header1->nwells == header2->nwells);
  equal = equal && (header1->niwelz == header2->niwelz);
  equal = equal && (header1->nzwelz == header2->nzwelz);
  equal = equal && (header1->niconz == header2->niconz);
  equal = equal && (header1->ncwmax == header2->ncwmax);
  equal = equal && (header1->nisegz == header2->nisegz);
  equal = equal && (header1->nsegmx == header2->nsegmx);
  equal = equal && (header1->nswlmx == header2->nswlmx);
  equal = equal && (header1->nlbrmx == header2->nlbrmx);
  equal = equal && (header1->nilbrz == header2->nilbrz);
  equal = equal && (header1->dualp == header2->dualp);
  equal = equal && util_double_approx_equal(header1->sim_days , header2->sim_days );

  return equal;
}

void ecl_rsthead_fprintf_struct( const ecl_rsthead_type * header , FILE * stream) {
  fprintf(stream , "{.day = %d,\n",header->day);
  fprintf(stream , ".year = %d,\n",header->year);
  fprintf(stream , ".month = %d,\n",header->month);
  fprintf(stream , ".sim_time = %ld,\n",header->sim_time);
  fprintf(stream , ".version = %d,\n",header->version);
  fprintf(stream , ".phase_sum = %d,\n",header->phase_sum);
  fprintf(stream , ".nx = %d,\n",header->nx);
  fprintf(stream , ".ny = %d,\n",header->ny);
  fprintf(stream , ".nz = %d,\n",header->nz);
  fprintf(stream , ".nactive = %d,\n",header->nactive);
  fprintf(stream , ".nwells = %d,\n",header->nwells);
  fprintf(stream , ".niwelz = %d,\n",header->niwelz);  
  fprintf(stream , ".nzwelz = %d,\n",header->nzwelz);
  fprintf(stream , ".niconz = %d,\n",header->niconz);  
  fprintf(stream , ".ncwmax = %d,\n",header->ncwmax);
  fprintf(stream , ".nisegz = %d,\n",header->nisegz);
  fprintf(stream , ".nsegmx = %d,\n",header->nsegmx);
  fprintf(stream , ".nswlmx = %d,\n",header->nswlmx);
  fprintf(stream , ".nlbrmx = %d,\n",header->nlbrmx);
  fprintf(stream , ".nilbrz = %d,\n",header->nilbrz);
  fprintf(stream , ".dualp  = %d,\n",header->dualp);
  fprintf(stream , ".sim_days  = %g};\n",header->sim_days);
}


void ecl_rsthead_free( ecl_rsthead_type * rsthead ) {
  free( rsthead );
}
