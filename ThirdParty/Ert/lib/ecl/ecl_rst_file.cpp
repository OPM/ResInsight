
/*
   Copyright (C) 2012  Statoil ASA, Norway.

   The file 'ecl_rst_file.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <time.h>

#include <ert/util/hash.hpp>
#include <ert/util/util.hpp>
#include <ert/util/vector.hpp>
#include <ert/util/int_vector.hpp>
#include <ert/util/stringlist.hpp>

#include <ert/ecl/fortio.h>
#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_file.hpp>
#include <ert/ecl/ecl_endian_flip.hpp>
#include <ert/ecl/ecl_kw_magic.hpp>
#include <ert/ecl/ecl_file_kw.hpp>
#include <ert/ecl/ecl_rst_file.hpp>
#include <ert/ecl/ecl_rsthead.hpp>
#include <ert/ecl/ecl_type.hpp>

struct ecl_rst_file_struct {
  fortio_type * fortio;
  bool          unified;
  bool          fmt_file;
};



static ecl_rst_file_type * ecl_rst_file_alloc( const char * filename ) {
  bool unified  = ecl_util_unified_file( filename );
  bool fmt_file;
  ecl_rst_file_type * rst_file = (ecl_rst_file_type*)util_malloc( sizeof * rst_file );

  if (ecl_util_fmt_file( filename , &fmt_file)) {
    rst_file->unified = unified;
    rst_file->fmt_file = fmt_file;
    return rst_file;
  } else {
    util_abort("%s: invalid restart filename:%s - could not determine formatted/unformatted status\n",__func__ , filename);
    return NULL;
  }
}

/**
   Observe that all the open() functions expect that filename conforms
   to the standard ECLIPSE conventions, i.e. with extension .FUNRST /
   .UNRST / .Xnnnn / .Fnnnn.
*/

ecl_rst_file_type * ecl_rst_file_open_read( const char * filename ) {
  ecl_rst_file_type * rst_file = ecl_rst_file_alloc( filename );
  rst_file->fortio = fortio_open_reader( filename , rst_file->fmt_file , ECL_ENDIAN_FLIP );
  return rst_file;
}



/*
  This function will scan through the file and look for seqnum
  headers, and position the file pointer in the right location to
  start writing data for the report step given by @report_step. The
  file is truncated, so that the filepointer will be at the (new) EOF
  when returning.
*/


ecl_rst_file_type * ecl_rst_file_open_write_seek( const char * filename , int report_step) {
  ecl_rst_file_type * rst_file = ecl_rst_file_alloc( filename  );
  offset_type target_pos = 0;
  bool seqnum_found = false;
  rst_file->fortio = fortio_open_readwrite( filename , rst_file->fmt_file , ECL_ENDIAN_FLIP );
  /*
     If the file does not exist at all the fortio_open_readwrite()
     will fail, we just try again - opening a new file in normal write
     mode, and then immediately returning.
  */
  if (!rst_file->fortio) {
    rst_file->fortio = fortio_open_writer( filename , rst_file->fmt_file , ECL_ENDIAN_FLIP );
    return rst_file;
  }

  fortio_fseek( rst_file->fortio , 0 , SEEK_SET );
  {
    ecl_kw_type * work_kw = ecl_kw_alloc_new("WORK-KW" , 0 , ECL_INT, NULL);

    while (true) {
      offset_type current_offset = fortio_ftell( rst_file->fortio );

      if (fortio_read_at_eof(rst_file->fortio)) {
        if (seqnum_found)
          target_pos = current_offset;
        break;
      }

      if (ecl_kw_fread_header( work_kw , rst_file->fortio) == ECL_KW_READ_FAIL)
        break;

      if (ecl_kw_name_equal( work_kw , SEQNUM_KW)) {
        ecl_kw_fread_realloc_data( work_kw , rst_file->fortio);
        int file_step = ecl_kw_iget_int( work_kw , 0 );
        if (file_step >= report_step) {
          target_pos = current_offset;
          break;
        }
        seqnum_found = true;
      } else
          ecl_kw_fskip_data( work_kw , rst_file->fortio );

    }

    ecl_kw_free( work_kw );
  }

  fortio_fseek( rst_file->fortio , target_pos , SEEK_SET);
  fortio_ftruncate_current( rst_file->fortio );
  return rst_file;
}

ecl_rst_file_type * ecl_rst_file_open_write( const char * filename ) {
  ecl_rst_file_type * rst_file = ecl_rst_file_alloc( filename  );
  rst_file->fortio = fortio_open_writer( filename , rst_file->fmt_file , ECL_ENDIAN_FLIP );
  return rst_file;
}

ecl_rst_file_type * ecl_rst_file_open_append( const char * filename ) {
  ecl_rst_file_type * rst_file = ecl_rst_file_alloc( filename );
  rst_file->fortio = fortio_open_append( filename , rst_file->fmt_file , ECL_ENDIAN_FLIP );
  return rst_file;
}

void ecl_rst_file_close( ecl_rst_file_type * rst_file ) {
  fortio_fclose( rst_file->fortio );
  free( rst_file );
}


/*****************************************************************/

static void ecl_rst_file_fwrite_SEQNUM( ecl_rst_file_type * rst_file , int seqnum ) {
  ecl_kw_type * seqnum_kw = ecl_kw_alloc( SEQNUM_KW , 1 , ECL_INT );
  ecl_kw_iset_int( seqnum_kw , 0 , seqnum );
  ecl_kw_fwrite( seqnum_kw , rst_file->fortio );
  ecl_kw_free( seqnum_kw );
}

void ecl_rst_file_start_solution( ecl_rst_file_type * rst_file ) {
  ecl_kw_type * startsol_kw = ecl_kw_alloc( STARTSOL_KW , 0 , ECL_MESS );
  ecl_kw_fwrite( startsol_kw , rst_file->fortio );
  ecl_kw_free( startsol_kw );
}

void ecl_rst_file_end_solution( ecl_rst_file_type * rst_file ) {
  ecl_kw_type * endsol_kw = ecl_kw_alloc( ENDSOL_KW , 0 , ECL_MESS );
  ecl_kw_fwrite( endsol_kw , rst_file->fortio );
  ecl_kw_free( endsol_kw );
}



static ecl_kw_type * ecl_rst_file_alloc_INTEHEAD( ecl_rst_file_type * rst_file,
                                                  ecl_rsthead_type * rsthead,
                                                  int simulator ) {
  ecl_kw_type * intehead_kw = ecl_kw_alloc( INTEHEAD_KW , INTEHEAD_RESTART_SIZE , ECL_INT );
  ecl_kw_scalar_set_int( intehead_kw , 0 );

  ecl_kw_iset_int( intehead_kw , INTEHEAD_UNIT_INDEX    , rsthead->unit_system );
  ecl_kw_iset_int( intehead_kw , INTEHEAD_NX_INDEX      , rsthead->nx);
  ecl_kw_iset_int( intehead_kw , INTEHEAD_NY_INDEX      , rsthead->ny);
  ecl_kw_iset_int( intehead_kw , INTEHEAD_NZ_INDEX      , rsthead->nz);
  ecl_kw_iset_int( intehead_kw , INTEHEAD_NACTIVE_INDEX , rsthead->nactive);
  ecl_kw_iset_int( intehead_kw , INTEHEAD_PHASE_INDEX   , rsthead->phase_sum );

  /* All well properties are hardcoded to zero. */
  {
    int NGMAXZ = 0;
    int NWGMAX = 0;
    int NIGRPZ = 0;
    int NSWLMX = 0;
    int NSEGMX = 0;
    int NISEGZ = 0;

    ecl_kw_iset_int( intehead_kw , INTEHEAD_NWELLS_INDEX  , rsthead->nwells );
    ecl_kw_iset_int( intehead_kw , INTEHEAD_NCWMAX_INDEX  , rsthead->ncwmax );
    ecl_kw_iset_int( intehead_kw , INTEHEAD_NWGMAX_INDEX  , NWGMAX );
    ecl_kw_iset_int( intehead_kw , INTEHEAD_NGMAXZ_INDEX  , NGMAXZ );
    ecl_kw_iset_int( intehead_kw , INTEHEAD_NIWELZ_INDEX  , rsthead->niwelz );
    ecl_kw_iset_int( intehead_kw , INTEHEAD_NZWELZ_INDEX  , rsthead->nzwelz );
    ecl_kw_iset_int( intehead_kw , INTEHEAD_NICONZ_INDEX  , rsthead->niconz );
    ecl_kw_iset_int( intehead_kw , INTEHEAD_NIGRPZ_INDEX  , NIGRPZ );

    {
      ecl_util_set_date_values( rsthead->sim_time , &rsthead->day , &rsthead->month , &rsthead->year );
      ecl_kw_iset_int( intehead_kw , INTEHEAD_DAY_INDEX    , rsthead->day );
      ecl_kw_iset_int( intehead_kw , INTEHEAD_MONTH_INDEX  , rsthead->month );
      ecl_kw_iset_int( intehead_kw , INTEHEAD_YEAR_INDEX   , rsthead->year );
    }

    ecl_kw_iset_int( intehead_kw , INTEHEAD_IPROG_INDEX , simulator);
    ecl_kw_iset_int( intehead_kw , INTEHEAD_NSWLMX_INDEX  , NSWLMX );
    ecl_kw_iset_int( intehead_kw , INTEHEAD_NSEGMX_INDEX  , NSEGMX );
    ecl_kw_iset_int( intehead_kw , INTEHEAD_NISEGZ_INDEX  , NISEGZ );
  }
  return intehead_kw;
}



static ecl_kw_type * ecl_rst_file_alloc_LOGIHEAD( int simulator ) {
  bool dual_porosity          = false;
  bool radial_grid_ECLIPSE100 = false;
  bool radial_grid_ECLIPSE300 = false;

  ecl_kw_type * logihead_kw = ecl_kw_alloc( LOGIHEAD_KW , LOGIHEAD_RESTART_SIZE , ECL_BOOL );

  ecl_kw_scalar_set_bool( logihead_kw , false );

  if (simulator == INTEHEAD_ECLIPSE100_VALUE)
    ecl_kw_iset_bool( logihead_kw , LOGIHEAD_RADIAL100_INDEX , radial_grid_ECLIPSE100);
  else
    ecl_kw_iset_bool( logihead_kw , LOGIHEAD_RADIAL300_INDEX , radial_grid_ECLIPSE300);

  ecl_kw_iset_bool( logihead_kw , LOGIHEAD_DUALP_INDEX , dual_porosity);
  return logihead_kw;
}


static ecl_kw_type * ecl_rst_file_alloc_DOUBHEAD( ecl_rst_file_type * rst_file , double days) {
  ecl_kw_type * doubhead_kw = ecl_kw_alloc( DOUBHEAD_KW , DOUBHEAD_RESTART_SIZE , ECL_DOUBLE );

  ecl_kw_scalar_set_double( doubhead_kw , 0);
  ecl_kw_iset_double( doubhead_kw , DOUBHEAD_DAYS_INDEX , days );


  return doubhead_kw;
}



void ecl_rst_file_fwrite_header( ecl_rst_file_type * rst_file ,
                                 int seqnum ,
                                 ecl_rsthead_type * rsthead_data ) {

  if (rst_file->unified)
    ecl_rst_file_fwrite_SEQNUM( rst_file , seqnum );

  {
    ecl_kw_type * intehead_kw = ecl_rst_file_alloc_INTEHEAD( rst_file , rsthead_data , INTEHEAD_ECLIPSE100_VALUE);
    ecl_kw_fwrite( intehead_kw , rst_file->fortio );
    ecl_kw_free( intehead_kw );
  }

  {
    ecl_kw_type * logihead_kw = ecl_rst_file_alloc_LOGIHEAD( INTEHEAD_ECLIPSE100_VALUE);
    ecl_kw_fwrite( logihead_kw , rst_file->fortio );
    ecl_kw_free( logihead_kw );
  }


  {
    ecl_kw_type * doubhead_kw = ecl_rst_file_alloc_DOUBHEAD( rst_file , rsthead_data->sim_days );
    ecl_kw_fwrite( doubhead_kw , rst_file->fortio );
    ecl_kw_free( doubhead_kw );
  }
}

void ecl_rst_file_add_kw(ecl_rst_file_type * rst_file , const ecl_kw_type * ecl_kw ) {
  ecl_kw_fwrite( ecl_kw , rst_file->fortio );
}


offset_type ecl_rst_file_ftell(const ecl_rst_file_type * rst_file ) {
  return fortio_ftell( rst_file->fortio );
}
