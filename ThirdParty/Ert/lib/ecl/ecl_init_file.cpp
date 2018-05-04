/*
   Copyright (C) 2012 Statoil ASA, Norway.

   The file 'ecl_init_file.c' is part of ERT - Ensemble based Reservoir Tool.

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

/*
  This file contains functionality to write an ECLIPSE INIT file. The
  file does (currently) not contain any datastructures, only
  functions. Essentially this file is only a codifying of the ECLIPSE
  documentation of INIT files.

  The functionality is mainly targeted at saving grid properties like
  PORO, PERMX and FIPNUM. The thermodynamic/relperm properties are
  mainly hardcoded to FALSE (in particular in the
  ecl_init_file_alloc_LOGIHEAD() function); this implies that the INIT
  files produced in this way are probably not 100% valid, and can
  certainly not be used to query for e.g. relperm properties.
*/


#include <ert/util/util.hpp>

#include <ert/ecl/fortio.h>
#include <ert/ecl/ecl_kw_magic.hpp>
#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_grid.hpp>
#include <ert/ecl/ecl_util.hpp>
#include <ert/ecl/ecl_type.hpp>
#include <ert/ecl/ecl_init_file.hpp>

static ecl_kw_type * ecl_init_file_alloc_INTEHEAD( const ecl_grid_type * ecl_grid , ert_ecl_unit_enum unit_system, int phases, time_t start_date , int simulator) {
  ecl_kw_type * intehead_kw = ecl_kw_alloc( INTEHEAD_KW , INTEHEAD_INIT_SIZE , ECL_INT );
  ecl_kw_scalar_set_int( intehead_kw , 0 );

  ecl_kw_iset_int( intehead_kw , INTEHEAD_UNIT_INDEX    , unit_system );
  ecl_kw_iset_int( intehead_kw , INTEHEAD_NX_INDEX      , ecl_grid_get_nx( ecl_grid ));
  ecl_kw_iset_int( intehead_kw , INTEHEAD_NY_INDEX      , ecl_grid_get_ny( ecl_grid ));
  ecl_kw_iset_int( intehead_kw , INTEHEAD_NZ_INDEX      , ecl_grid_get_nz( ecl_grid ));
  ecl_kw_iset_int( intehead_kw , INTEHEAD_NACTIVE_INDEX , ecl_grid_get_active_size( ecl_grid ));
  ecl_kw_iset_int( intehead_kw , INTEHEAD_PHASE_INDEX   , phases );
  {
    int mday,month,year;
    ecl_util_set_date_values( start_date , &mday , &month , &year );
    ecl_kw_iset_int( intehead_kw , INTEHEAD_DAY_INDEX    , mday );
    ecl_kw_iset_int( intehead_kw , INTEHEAD_MONTH_INDEX  , month );
    ecl_kw_iset_int( intehead_kw , INTEHEAD_YEAR_INDEX   , year );
  }
  ecl_kw_iset_int( intehead_kw , INTEHEAD_IPROG_INDEX , simulator);

  return intehead_kw;
}


static ecl_kw_type * ecl_init_file_alloc_LOGIHEAD( int simulator ) {
  /*
    This function includes lots of hardcoded options; the main
    intention of writing the INIT file is to store the grid properties
    like e.g. PORO and SATNUM. The relperm/thermodynamics properties
    are just all defaulted to False.

    The documentation
  */
  bool with_RS                            = false;
  bool with_RV                            = false;
  bool with_directional_relperm           = false;
  bool with_reversible_relperm_ECLIPSE100 = false;
  bool radial_grid_ECLIPSE100             = false;
  bool radial_grid_ECLIPSE300             = false;
  bool with_reversible_relperm_ECLIPSE300 = false;
  bool with_hysterisis                    = false;
  bool dual_porosity                      = false;
  bool endpoint_scaling                   = false;
  bool directional_endpoint_scaling       = false;
  bool reversible_endpoint_scaling        = false;
  bool alternative_endpoint_scaling       = false;
  bool miscible_displacement              = false;
  bool scale_water_PC_at_max_sat          = false;
  bool scale_gas_PC_at_max_sat            = false;


  ecl_kw_type * logihead_kw = ecl_kw_alloc( LOGIHEAD_KW , LOGIHEAD_INIT_SIZE , ECL_BOOL );

  ecl_kw_scalar_set_bool( logihead_kw , false );

  ecl_kw_iset_bool( logihead_kw , LOGIHEAD_RS_INDEX                        , with_RS);
  ecl_kw_iset_bool( logihead_kw , LOGIHEAD_RV_INDEX                        , with_RV);
  ecl_kw_iset_bool( logihead_kw , LOGIHEAD_DIR_RELPERM_INDEX               , with_directional_relperm);

  if (simulator == INTEHEAD_ECLIPSE100_VALUE) {
    ecl_kw_iset_bool( logihead_kw , LOGIHEAD_REV_RELPERM100_INDEX            , with_reversible_relperm_ECLIPSE100);
    ecl_kw_iset_bool( logihead_kw , LOGIHEAD_RADIAL100_INDEX                 , radial_grid_ECLIPSE100);
  } else {
    ecl_kw_iset_bool( logihead_kw , LOGIHEAD_REV_RELPERM300_INDEX            , with_reversible_relperm_ECLIPSE300 );
    ecl_kw_iset_bool( logihead_kw , LOGIHEAD_RADIAL300_INDEX                 , radial_grid_ECLIPSE300);
  }

  ecl_kw_iset_bool( logihead_kw , LOGIHEAD_HYSTERISIS_INDEX                , with_hysterisis);
  ecl_kw_iset_bool( logihead_kw , LOGIHEAD_DUALP_INDEX                     , dual_porosity);
  ecl_kw_iset_bool( logihead_kw , LOGIHEAD_ENDPOINT_SCALING_INDEX          , endpoint_scaling);
  ecl_kw_iset_bool( logihead_kw , LOGIHEAD_DIR_ENDPOINT_SCALING_INDEX      , directional_endpoint_scaling);
  ecl_kw_iset_bool( logihead_kw , LOGIHEAD_REV_ENDPOINT_SCALING_INDEX      , reversible_endpoint_scaling);
  ecl_kw_iset_bool( logihead_kw , LOGIHEAD_ALT_ENDPOINT_SCALING_INDEX      , alternative_endpoint_scaling);
  ecl_kw_iset_bool( logihead_kw , LOGIHEAD_MISC_DISPLACEMENT_INDEX         , miscible_displacement);
  ecl_kw_iset_bool( logihead_kw , LOGIHEAD_SCALE_WATER_PC_AT_MAX_SAT_INDEX , scale_water_PC_at_max_sat);
  ecl_kw_iset_bool( logihead_kw , LOGIHEAD_SCALE_GAS_PC_AT_MAX_SAT_INDEX   , scale_gas_PC_at_max_sat);

  return logihead_kw;
}


static ecl_kw_type * ecl_init_file_alloc_DOUBHEAD( ) {
  ecl_kw_type * doubhead_kw = ecl_kw_alloc( DOUBHEAD_KW , DOUBHEAD_INIT_SIZE , ECL_DOUBLE );

  ecl_kw_scalar_set_double( doubhead_kw , 0);

  return doubhead_kw;
}


/**
   The writing of the PORO field is somewhat special cased; the INIT
   file should contain the PORV keyword with nx*ny*nz elements. The
   cells which are inactive have the PORV volume explicitly set to
   zero; this way the active/inactive status can be inferred from PORV
   field in the INIT file.

   In this code the PORO field is considered to be the fundamental
   quantity, and the PORV field is calculated from PORO and the volume
   of the grid cells. Apart from PORV all the remaining fields in the
   INIT file should have nactive elements.

   If you do not wish this function to be used for the PORV special
   casing you can just pass NULL as the poro_kw in the
   ecl_init_file_fwrite_header() function.
 */

static void ecl_init_file_fwrite_poro( fortio_type * fortio , const ecl_grid_type * ecl_grid , const ecl_kw_type * poro ) {
  {
    ecl_kw_type * porv = ecl_kw_alloc( PORV_KW , ecl_grid_get_global_size( ecl_grid ) , ECL_FLOAT);
    int global_index;
    bool global_poro = (ecl_kw_get_size( poro ) == ecl_grid_get_global_size( ecl_grid )) ? true : false;
    for ( global_index = 0; global_index < ecl_grid_get_global_size( ecl_grid ); global_index++) {
      int active_index = ecl_grid_get_active_index1( ecl_grid , global_index );
      if (active_index >= 0) {
        int poro_index = global_poro ? global_index : active_index;
        ecl_kw_iset_float( porv , global_index , ecl_kw_iget_float( poro , poro_index ) * ecl_grid_get_cell_volume1( ecl_grid , global_index ));
      } else
        ecl_kw_iset_float( porv , global_index , 0 );
    }
    ecl_kw_fwrite( porv , fortio );
    ecl_kw_free( porv );
  }

  ecl_kw_fwrite( poro , fortio );
}


/*
  If the poro keyword is non NULL this function will write both the
  PORO keyword itself and also calculate the PORV keyword and write
  that.
*/

void ecl_init_file_fwrite_header( fortio_type * fortio , const ecl_grid_type * ecl_grid , const ecl_kw_type * poro , ert_ecl_unit_enum unit_system, int phases , time_t start_date) {
  int simulator = INTEHEAD_ECLIPSE100_VALUE;
  {
    ecl_kw_type * intehead_kw = ecl_init_file_alloc_INTEHEAD( ecl_grid , unit_system , phases , start_date , simulator );
    ecl_kw_fwrite( intehead_kw , fortio );
    ecl_kw_free( intehead_kw );
  }

  {
    ecl_kw_type * logihead_kw = ecl_init_file_alloc_LOGIHEAD( simulator );
    ecl_kw_fwrite( logihead_kw , fortio );
    ecl_kw_free( logihead_kw );
  }

  {
    ecl_kw_type * doubhead_kw = ecl_init_file_alloc_DOUBHEAD( );
    ecl_kw_fwrite( doubhead_kw , fortio );
    ecl_kw_free( doubhead_kw );
  }

  if (poro) {
    int poro_size = ecl_kw_get_size( poro );
    if ((poro_size == ecl_grid_get_nactive( ecl_grid )) || (poro_size == ecl_grid_get_global_size(ecl_grid)))
      ecl_init_file_fwrite_poro( fortio , ecl_grid , poro );
    else
      util_abort("%s: keyword PORO has wrong size:%d  Grid: %d/%d \n",__func__  , ecl_kw_get_size( poro ) , ecl_grid_get_nactive( ecl_grid ) , ecl_grid_get_global_size( ecl_grid ));
  }
}
