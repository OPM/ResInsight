/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'ecl_RSTHEAD.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_ECL_RSTHEAD_H
#define ERT_ECL_RSTHEAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

#include <ert/ecl/ecl_file.hpp>
#include <ert/ecl/ecl_file_view.hpp>
#include <ert/ecl/ecl_kw.hpp>


  typedef struct {
    // The report step is from the SEQNUM keyword for unified files,
    // and inferred from the filename for non unified files.
    int    report_step;
    int    day;
    int    year;
    int    month;
    time_t sim_time;
    int    version;         // 100, 300, 500 (Eclipse300-Thermal)
    int    phase_sum;       // Oil = 1   Gas = 2    Water = 4

    ert_ecl_unit_enum unit_system;

    int    nx;
    int    ny;
    int    nz;
    int    nactive;
    /*-----------------------------------------------------------------*/
    /* All fields below the line are taken literally (apart from
       lowercasing) from the section about restart files in the
       ECLIPSE File Formats Reference Manual. The elements typically
       serve as dimensions in the ?WEL, ?SEG and ?CON arrays.
    */

    // Pure well properties
    int    nwells;          // Number of wells
    int    niwelz;          // Number of elements pr well in IWEL array
    int    nzwelz;          // Number of 8 character words pr well in ZWEL array
    int    nxwelz;          // Number of elements pr well in XWEL array.

    // Connection properties
    int    niconz;          // Number of elements per completion in ICON array
    int    ncwmax;          // Maximum number of completions per well
    int    nsconz;          // Number of elements per completion in SCON array
    int    nxconz;          // Number of elements per completion in XCON array

    // Segment properties
    int    nisegz;          // Number of entries pr segment in the ISEG array
    int    nsegmx;          // The maximum number of segments pr well
    int    nswlmx;          // The maximum number of segmented wells
    int    nlbrmx;          // The maximum number of lateral branches pr well
    int    nilbrz;          // The number of entries pr segment in ILBR array
    int    nrsegz;          // The number of entries pr segment in RSEG array

    // Properteies from the LOGIHEAD keyword:
    bool   dualp;


    // Properties from the DOUBHEAD keyword:
    double sim_days;
  } ecl_rsthead_type;



  void                ecl_rsthead_free( ecl_rsthead_type * rsthead );
  ecl_rsthead_type  * ecl_rsthead_alloc_from_kw( int report_step , const ecl_kw_type * intehead_kw , const ecl_kw_type * doubhead_kw , const ecl_kw_type * logihead_kw );
  ecl_rsthead_type  * ecl_rsthead_alloc( const ecl_file_view_type * rst_file , int report_step);
  ecl_rsthead_type  * ecl_rsthead_alloc_empty(void);
  time_t              ecl_rsthead_date( const ecl_kw_type * intehead_kw );
  void                ecl_rsthead_fprintf( const ecl_rsthead_type * header , FILE * stream);
  void                ecl_rsthead_fprintf_struct( const ecl_rsthead_type * header , FILE * stream);
  bool                ecl_rsthead_equal( const ecl_rsthead_type * header1 , const ecl_rsthead_type * header2);
  double              ecl_rsthead_get_sim_days( const ecl_rsthead_type * header );
  int                 ecl_rsthead_get_report_step( const ecl_rsthead_type * header );
  time_t              ecl_rsthead_get_sim_time( const ecl_rsthead_type * header );
  int                 ecl_rsthead_get_nxconz( const ecl_rsthead_type * rsthead );
  int                 ecl_rsthead_get_ncwmax( const ecl_rsthead_type * rsthead );

#ifdef __cplusplus
}
#endif
#endif
