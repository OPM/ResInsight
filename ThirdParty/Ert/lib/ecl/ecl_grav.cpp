/*
   Copyright (C) 2011  Statoil ASA, Norway.

   This file is part of ERT - Ensemble based Reservoir Tool.

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
#include <math.h>
#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/hash.hpp>
#include <ert/util/vector.hpp>

#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_util.hpp>
#include <ert/ecl/ecl_file.hpp>
#include <ert/ecl/ecl_grid.hpp>
#include <ert/ecl/ecl_region.hpp>
#include <ert/ecl/ecl_grav.hpp>
#include <ert/ecl/ecl_kw_magic.hpp>
#include <ert/ecl/ecl_grav_common.hpp>

#include "detail/ecl/ecl_grid_cache.hpp"

/**
   This file contains datastructures for calculating changes in
   gravitational response in reservoirs. The main datastructure is the
   ecl_grav_type structure (which is the only structure which is
   exported).
*/




#define GRAV_CALC_USE_PORV 128
#define GRAV_CALC_USE_RHO  256    // The GRAV_CALC_USE_RHO value is currently not used.

typedef enum {
  GRAV_CALC_RPORV  = 1 + GRAV_CALC_USE_PORV + GRAV_CALC_USE_RHO,
  GRAV_CALC_PORMOD = 2 + GRAV_CALC_USE_PORV + GRAV_CALC_USE_RHO,
  GRAV_CALC_FIP    = 3,
  GRAV_CALC_RFIP   = 4 + GRAV_CALC_USE_RHO
} grav_calc_type;


typedef struct ecl_grav_phase_struct ecl_grav_phase_type;


/**
   The ecl_grav_struct datastructure is the main structure for
   calculating the gravimetric response from time lapse ECLIPSE
   simulations.
*/

struct ecl_grav_struct {
  const ecl_file_type      * init_file;    /* The init file - a shared reference owned by calling scope. */
  ecl::ecl_grid_cache      * grid_cache;   /* An internal specialized structure to facilitate fast grid lookup. */
  bool                     * aquifer_cell; /* Numerical aquifer cells should be ignored. */
  hash_type                * surveys;      /* A hash table containg ecl_grav_survey_type instances; one instance
                                              for each interesting time. */
  hash_type                * std_density;  /* Hash table indexed with "SWAT" , "SGAS" and "SOIL"; each element
                                              is a double_vector() instance which is indexed by PVTNUM
                                              values. Used to lookup standard condition mass densities. Must
                                              be suuplied by user __BEFORE__ adding a FIP based survey. */
};





/**
   Data structure representing one gravimetric survey.
*/

#define ECL_GRAV_SURVEY_ID 88517
struct ecl_grav_survey_struct {
  UTIL_TYPE_ID_DECLARATION;
  const ecl::ecl_grid_cache      * grid_cache;
  const bool                     * aquifer_cell;
  char                           * name;           /* Name of the survey - arbitrary string. */
  double                         * porv;           /* Reference shared by the ecl_grav_phase structures - i.e. it must not be updated. */
  vector_type                    * phase_list;     /* ecl_grav_phase_type objects - one for each phase present in the model. */
  hash_type                      * phase_map;      /* The same objects as in the phase_list vector - accessible by the "SWAT", "SGAS" and "SOIL" keys. */
};


/**
   Data structure representing the results from one phase at one survey.
*/

#define ECL_GRAV_PHASE_TYPE_ID 1066652
struct ecl_grav_phase_struct {
  UTIL_TYPE_ID_DECLARATION;
  const ecl::ecl_grid_cache  * grid_cache;
  const bool                 * aquifer_cell;
  double                          * fluid_mass;  /* The total fluid in place (mass) of this phase - for each active cell.*/
  double                           * work;           /* Temporary used in the summation over all cells. */
  ecl_phase_enum                    phase;
};



/*****************************************************************/



static const char * get_den_kw( ecl_phase_enum phase , ecl_version_enum ecl_version) {
  if (ecl_version == ECLIPSE100) {
    switch( phase ) {
    case( ECL_OIL_PHASE ):
      return ECLIPSE100_OIL_DEN_KW;
      break;
    case( ECL_GAS_PHASE ):
      return ECLIPSE100_GAS_DEN_KW;
      break;
    case( ECL_WATER_PHASE ):
      return ECLIPSE100_WATER_DEN_KW;
      break;
    default:
      util_abort("%s: unrecognized phase id:%d \n",__func__ , phase);
      return NULL;
    }
  } else if ((ecl_version == ECLIPSE300) || (ecl_version == ECLIPSE300_THERMAL)) {
    switch( phase ) {
    case( ECL_OIL_PHASE ):
      return ECLIPSE300_OIL_DEN_KW;
      break;
    case( ECL_GAS_PHASE ):
      return ECLIPSE300_GAS_DEN_KW;
      break;
    case( ECL_WATER_PHASE ):
      return ECLIPSE300_WATER_DEN_KW;
      break;
    default:
      util_abort("%s: unrecognized phase id:%d \n",__func__ , phase);
      return NULL;
    }
  } else {
    util_abort("%s: unrecognized simulator id:%d \n",__func__ , ecl_version);
    return NULL;
  }
}


static void ecl_grav_phase_ensure_work( ecl_grav_phase_type * grav_phase) {
  if (grav_phase->work == NULL)
    grav_phase->work = (double*)util_calloc( grav_phase->grid_cache->size() , sizeof * grav_phase->work  );
}


static double ecl_grav_phase_eval( ecl_grav_phase_type * base_phase ,
                                   const ecl_grav_phase_type * monitor_phase,
                                   ecl_region_type * region ,
                                   double utm_x , double utm_y , double depth) {

  ecl_grav_phase_ensure_work( base_phase );
  if ((monitor_phase == NULL) || (base_phase->phase == monitor_phase->phase)) {
    const ecl::ecl_grid_cache& grid_cache = *(base_phase->grid_cache);
    const bool   * aquifer   = base_phase->aquifer_cell;
    double * mass_diff       = base_phase->work;
    double deltag;
    /*
       Initialize a work array to contain the difference in mass for
       every cell.
    */
    {
      int index;
      if (monitor_phase == NULL) {
        for (index = 0; index < grid_cache.size(); index++)
          mass_diff[index] = - base_phase->fluid_mass[index];
      } else {
        for (index = 0; index < grid_cache.size(); index++)
          mass_diff[index] = monitor_phase->fluid_mass[index] - base_phase->fluid_mass[index];
      }
    }

    /**
       The Gravitational constant is 6.67E-11 N (m/kg)^2, we
       return the result in microGal, i.e. we scale with 10^2 *
       10^6 => 6.67E-3.
    */
    deltag = 6.67428E-3 * ecl_grav_common_eval_biot_savart( grid_cache , region , aquifer , mass_diff , utm_x , utm_y , depth);

    return deltag;
  } else {
    util_abort("%s comparing different phases ... \n",__func__);
    return -1;
  }
}



static ecl_grav_phase_type * ecl_grav_phase_alloc( ecl_grav_type * ecl_grav ,
                                                   ecl_grav_survey_type * survey ,
                                                   ecl_phase_enum phase ,
                                                   const ecl_file_view_type * restart_file,
                                                   grav_calc_type calc_type) {

  const ecl_file_type * init_file        = ecl_grav->init_file;
  const ecl::ecl_grid_cache * grid_cache = ecl_grav->grid_cache;
  const char * sat_kw_name               = ecl_util_get_phase_name( phase );
  {
    ecl_grav_phase_type * grav_phase = (ecl_grav_phase_type*)util_malloc( sizeof * grav_phase );
    const int size                   = grid_cache->size();

    UTIL_TYPE_ID_INIT( grav_phase , ECL_GRAV_PHASE_TYPE_ID );
    grav_phase->grid_cache   = grid_cache;
    grav_phase->aquifer_cell = ecl_grav->aquifer_cell;
    grav_phase->fluid_mass   = (double*)util_calloc( size , sizeof * grav_phase->fluid_mass );
    grav_phase->phase        = phase;
    grav_phase->work         = NULL;

    if (calc_type == GRAV_CALC_FIP) {
      ecl_kw_type * pvtnum_kw = ecl_file_iget_named_kw( init_file , PVTNUM_KW , 0 );
      double_vector_type * std_density = (double_vector_type*)hash_get( ecl_grav->std_density , ecl_util_get_phase_name( phase ));
      ecl_kw_type * fip_kw;

      if ( phase == ECL_OIL_PHASE)
        fip_kw = ecl_file_view_iget_named_kw( restart_file , FIPOIL_KW , 0 );
      else if (phase == ECL_GAS_PHASE)
        fip_kw = ecl_file_view_iget_named_kw( restart_file , FIPGAS_KW , 0 );
      else
        fip_kw = ecl_file_view_iget_named_kw( restart_file , FIPWAT_KW , 0 );

      {
        int iactive;
        for (iactive=0; iactive < size; iactive++) {
          double fip    = ecl_kw_iget_as_double( fip_kw , iactive );
          int    pvtnum = ecl_kw_iget_int( pvtnum_kw , iactive );
          grav_phase->fluid_mass[ iactive ] = fip * double_vector_safe_iget( std_density , pvtnum );
        }
      }
    } else {
      ecl_version_enum      ecl_version = ecl_file_get_ecl_version( init_file );
      const char          * den_kw_name = get_den_kw( phase , ecl_version );
      const ecl_kw_type   * den_kw      = ecl_file_view_iget_named_kw( restart_file , den_kw_name , 0 );

      if (calc_type == GRAV_CALC_RFIP) {
        ecl_kw_type * rfip_kw;
        if ( phase == ECL_OIL_PHASE)
          rfip_kw = ecl_file_view_iget_named_kw( restart_file , RFIPOIL_KW , 0 );
        else if (phase == ECL_GAS_PHASE)
          rfip_kw = ecl_file_view_iget_named_kw( restart_file , RFIPGAS_KW , 0 );
        else
          rfip_kw = ecl_file_view_iget_named_kw( restart_file , RFIPWAT_KW , 0 );

        {
          int iactive;
          for (iactive=0; iactive < size; iactive++) {
            double rho   = ecl_kw_iget_as_double( den_kw  , iactive );
            double rfip  = ecl_kw_iget_as_double( rfip_kw , iactive );
            grav_phase->fluid_mass[ iactive ] = rho * rfip;
          }
        }
      } else {
        /* (calc_type == GRAV_CALC_RPORV) || (calc_type == GRAV_CALC_PORMOD) */
        ecl_kw_type * sat_kw;
        bool private_sat_kw = false;
        if (ecl_file_view_has_kw( restart_file , sat_kw_name ))
          sat_kw = ecl_file_view_iget_named_kw( restart_file , sat_kw_name , 0 );
        else {
          /* We are targeting the residual phase, e.g. the OIL phase in a three phase system. */
          const ecl_kw_type * swat_kw = ecl_file_view_iget_named_kw( restart_file , "SWAT" , 0 );
          sat_kw = ecl_kw_alloc_copy( swat_kw );
          ecl_kw_scalar_set_float( sat_kw , 1.0 );
          ecl_kw_inplace_sub( sat_kw , swat_kw );  /* sat = 1 - SWAT */

          if (ecl_file_view_has_kw( restart_file , "SGAS" )) {
            const ecl_kw_type * sgas_kw = ecl_file_view_iget_named_kw( restart_file , "SGAS" , 0 );
            ecl_kw_inplace_sub( sat_kw , sgas_kw );  /* sat -= SGAS */
          }
          private_sat_kw = true;
        }

        {
          int iactive;
          for (iactive=0; iactive < size; iactive++) {
            double rho  = ecl_kw_iget_as_double( den_kw , iactive );
            double sat  = ecl_kw_iget_as_double( sat_kw , iactive );
            grav_phase->fluid_mass[ iactive ] = rho * sat * survey->porv[ iactive ];
          }
        }

        if (private_sat_kw)
          ecl_kw_free( sat_kw );
      }
    }

    return grav_phase;
  }
}


static void ecl_grav_phase_free( ecl_grav_phase_type * grav_phase ) {
  free( grav_phase->work );
  free( grav_phase->fluid_mass );
  free( grav_phase );
}

static UTIL_SAFE_CAST_FUNCTION( ecl_grav_phase , ECL_GRAV_PHASE_TYPE_ID )

static void ecl_grav_phase_free__( void * __grav_phase) {
  ecl_grav_phase_type * grav_phase = ecl_grav_phase_safe_cast( __grav_phase );
  ecl_grav_phase_free( grav_phase );
}


/*****************************************************************/


static void ecl_grav_survey_add_phase( ecl_grav_survey_type * survey, ecl_phase_enum phase , ecl_grav_phase_type * grav_phase ) {
  vector_append_owned_ref( survey->phase_list , grav_phase , ecl_grav_phase_free__ );
  hash_insert_ref( survey->phase_map , ecl_util_get_phase_name( phase ) , grav_phase );
}


static void ecl_grav_survey_add_phases( ecl_grav_type * ecl_grav , ecl_grav_survey_type * survey, const ecl_file_view_type * restart_file , grav_calc_type calc_type) {
  int phases = ecl_file_get_phases( ecl_grav->init_file );
  if (phases & ECL_OIL_PHASE) {
    ecl_grav_phase_type * oil_phase = ecl_grav_phase_alloc( ecl_grav , survey , ECL_OIL_PHASE ,  restart_file , calc_type);
    ecl_grav_survey_add_phase( survey , ECL_OIL_PHASE , oil_phase );
  }

  if (phases & ECL_GAS_PHASE) {
    ecl_grav_phase_type * gas_phase = ecl_grav_phase_alloc( ecl_grav , survey , ECL_GAS_PHASE , restart_file , calc_type);
    ecl_grav_survey_add_phase( survey , ECL_GAS_PHASE , gas_phase );
  }

  if (phases & ECL_WATER_PHASE) {
    ecl_grav_phase_type * water_phase = ecl_grav_phase_alloc( ecl_grav , survey , ECL_WATER_PHASE ,  restart_file , calc_type);
    ecl_grav_survey_add_phase( survey , ECL_WATER_PHASE , water_phase );
  }
}


static ecl_grav_survey_type * ecl_grav_survey_alloc_empty(const ecl_grav_type * ecl_grav ,
                                                          const char * name ,
                                                          grav_calc_type calc_type) {
  ecl_grav_survey_type * survey = (ecl_grav_survey_type*)util_malloc( sizeof * survey );
  UTIL_TYPE_ID_INIT( survey , ECL_GRAV_SURVEY_ID );
  survey->grid_cache   = ecl_grav->grid_cache;
  survey->aquifer_cell = ecl_grav->aquifer_cell;
  survey->name         = util_alloc_string_copy( name );
  survey->phase_list   = vector_alloc_new();
  survey->phase_map    = hash_alloc();

  if (calc_type & GRAV_CALC_USE_PORV)
    survey->porv       = (double*)util_calloc( ecl_grav->grid_cache->size() , sizeof * survey->porv );
  else
    survey->porv       = NULL;

  return survey;
}

static UTIL_SAFE_CAST_FUNCTION( ecl_grav_survey , ECL_GRAV_SURVEY_ID )


/**
   Check that the rporv values are in the right ballpark.  For ECLIPSE
   version 2008.2 they are way fucking off. Check PORV versus RPORV
   for some random locations in the grid.
*/

static void ecl_grav_survey_assert_RPORV( const ecl_grav_survey_type * survey , const ecl_file_type * init_file ) {
  const ecl::ecl_grid_cache& grid_cache  = *(survey->grid_cache);
  int   active_size                      = grid_cache.size();
  const ecl_kw_type * init_porv_kw       = ecl_file_iget_named_kw( init_file , PORV_KW , 0);
  int check_points                       = 100;
  int check_nr                           = 0;
  const std::vector<int>& global_index   = grid_cache.global_index();

  while (check_nr < check_points) {
    int active_index = rand() % active_size;

    double init_porv    = ecl_kw_iget_as_double( init_porv_kw , global_index[active_index] );    /* NB - this uses global indexing. */
    if (init_porv > 0) {
      double rporv      = survey->porv[ active_index ];
      double log_pormod = log10( rporv / init_porv );

      if (fabs( log_pormod ) > 1) {
        /* Detected as error if the effective pore volume multiplier
           is greater than 10 or less than 0.10. */
        fprintf(stderr,"-----------------------------------------------------------------\n");
        fprintf(stderr,"INIT PORV : %g \n",init_porv);
        fprintf(stderr,"RPORV     : %g \n",rporv);
        fprintf(stderr,"Hmmm - the RPORV values extracted from the restart file seem to be \n");
        fprintf(stderr,"veeery different from the initial porv value. This might indicate \n");
        fprintf(stderr,"an ECLIPSE bug in the RPORV handling. Try using another ECLIPSE version,\n");
        fprintf(stderr,"or alternatively the PORMOD approach instead\n");
        fprintf(stderr,"-----------------------------------------------------------------\n");
        exit(1);
      }
      check_nr++;
    }
  }
}



/**
   There are currently two main methods to add a survey; differentiated by
   how the mass of various phases in each cell is calculated:

    1. We can calculate the mass of each phase from the relation:

          mass = saturation * pore_volume * mass_density.

      This method requires access to the instantaneous pore volume. This
      can be accessed in two different ways, based either on the RPORV
      keyword or the PORV_MOD keyword. This functionality is available
      through the ecl_grav_survey_alloc_RPORV() and
      ecl_grav_survey_alloc_PORMOD() functions.


   2. The mass of each phase can be calculated based on the fluid in place
      values (volume of phase when the matter is brought to standard
      conditions), i.e. the FIPGAS, FIPWAT and FIPOIL keywords, and the
      corresponding densities at surface conditions. This functionality is
      implemented with the ecl_grav_survey_alloc_FIP() function.

      Observe that use of the FIP based method requires densities entered
      with ecl_grav_new_std_density()/ecl_grav_add_std_density() prior to
      adding the actual survey.
*/



/**
   Allocate one survey based on using the RPORV keyword from the
   restart file to calculate the instantaneous pore volume in each
   cell.

   Unfortunately different versions of ECLIPSE have showed a wide
   range of bugs related to the RPORV keyword, including:

    - Using the pressure values instead of pore volumes - this will be
      cached by the ecl_grav_survey_assert_RPORV() function.

    - Ignoring the dynamic pore volume changes, and just using
      RPORV  == INIT PORV.
*/


static ecl_grav_survey_type * ecl_grav_survey_alloc_RPORV(ecl_grav_type * ecl_grav ,
                                                          const ecl_file_view_type * restart_file ,
                                                          const char * name ) {
  ecl_grav_survey_type * survey = ecl_grav_survey_alloc_empty( ecl_grav , name , GRAV_CALC_RPORV);
  if (ecl_file_view_has_kw( restart_file , RPORV_KW)) {
    ecl_kw_type * rporv_kw = ecl_file_view_iget_named_kw( restart_file , RPORV_KW , 0);
    int iactive;
    for (iactive = 0; iactive < ecl_kw_get_size( rporv_kw ); iactive++)
      survey->porv[ iactive ] = ecl_kw_iget_as_double( rporv_kw , iactive );
  } else
    util_abort("%s: restart file did not contain %s keyword??\n",__func__ , RPORV_KW);

  {
    const ecl_file_type * init_file = ecl_grav->init_file;
    ecl_grav_survey_assert_RPORV( survey , init_file );
    ecl_grav_survey_add_phases( ecl_grav , survey ,  restart_file , GRAV_CALC_RPORV);
  }
  return survey;
}



static ecl_grav_survey_type * ecl_grav_survey_alloc_PORMOD(ecl_grav_type * ecl_grav ,
                                                           const ecl_file_view_type * restart_file ,
                                                           const char * name ) {
  ecl::ecl_grid_cache& grid_cache = *(ecl_grav->grid_cache);
  ecl_grav_survey_type * survey = ecl_grav_survey_alloc_empty( ecl_grav , name , GRAV_CALC_PORMOD);
  ecl_kw_type * init_porv_kw    = ecl_file_iget_named_kw( ecl_grav->init_file    , PORV_KW   , 0 );  /* Global indexing */
  ecl_kw_type * pormod_kw       = ecl_file_view_iget_named_kw( restart_file , PORMOD_KW , 0 );            /* Active indexing */
  const int size                = grid_cache.size();
  const auto& global_index      = grid_cache.global_index();
  int active_index;

  for (active_index = 0; active_index < size; active_index++)
    survey->porv[ active_index ] = ecl_kw_iget_float( pormod_kw , active_index ) * ecl_kw_iget_float( init_porv_kw , global_index[active_index] );

  ecl_grav_survey_add_phases( ecl_grav , survey , restart_file , GRAV_CALC_PORMOD);

  return survey;
}



/**
   Use of the ecl_grav_survey_alloc_FIP() function requires that the densities
   have been added for all phases with the ecl_grav_new_std_density() and
   possibly also the ecl_grav_add_std_density() functions.
*/

static ecl_grav_survey_type * ecl_grav_survey_alloc_FIP(ecl_grav_type * ecl_grav ,
                                                        const ecl_file_view_type * restart_file ,
                                                        const char * name ) {

  ecl_grav_survey_type * survey = ecl_grav_survey_alloc_empty( ecl_grav , name , GRAV_CALC_FIP);
  ecl_grav_survey_add_phases( ecl_grav , survey , restart_file , GRAV_CALC_FIP);

  return survey;
}



static ecl_grav_survey_type * ecl_grav_survey_alloc_RFIP(ecl_grav_type * ecl_grav ,
                                                         const ecl_file_view_type * restart_file ,
                                                         const char * name ) {

  ecl_grav_survey_type * survey = ecl_grav_survey_alloc_empty( ecl_grav , name , GRAV_CALC_RFIP);
  ecl_grav_survey_add_phases( ecl_grav , survey , restart_file , GRAV_CALC_RFIP);

  return survey;
}



static void ecl_grav_survey_free( ecl_grav_survey_type * grav_survey ) {
  free( grav_survey->name );
  free( grav_survey->porv );
  vector_free( grav_survey->phase_list );
  hash_free( grav_survey->phase_map );
  free( grav_survey );
}

static void ecl_grav_survey_free__( void * __grav_survey ) {
  ecl_grav_survey_type * grav_survey = ecl_grav_survey_safe_cast( __grav_survey );
  ecl_grav_survey_free( grav_survey );
}



static double ecl_grav_survey_eval( const ecl_grav_survey_type * base_survey,
                                    const ecl_grav_survey_type * monitor_survey ,
                                    ecl_region_type * region ,
                                    double utm_x , double utm_y , double depth, int phase_mask) {
  int phase_nr;
  double deltag = 0;
  for (phase_nr = 0; phase_nr < vector_get_size( base_survey->phase_list ); phase_nr++) {
    ecl_grav_phase_type * base_phase    = (ecl_grav_phase_type*)vector_iget( base_survey->phase_list , phase_nr );
    if (base_phase->phase & phase_mask) {
      if (monitor_survey != NULL) {
        const ecl_grav_phase_type * monitor_phase = (const ecl_grav_phase_type*)vector_iget_const( monitor_survey->phase_list , phase_nr );
        deltag += ecl_grav_phase_eval( base_phase , monitor_phase , region , utm_x , utm_y , depth );
      } else
        deltag += ecl_grav_phase_eval( base_phase , NULL , region , utm_x , utm_y , depth );
    }
  }
  return deltag;
}

/*****************************************************************/
/**
   The grid instance is only used during the construction phase. The
   @init_file object is used by the ecl_grav_add_survey_XXX()
   functions; and calling scope must NOT destroy this object before
   all surveys have been added.
*/

ecl_grav_type * ecl_grav_alloc( const ecl_grid_type * ecl_grid, const ecl_file_type * init_file) {
  ecl_grav_type * ecl_grav = (ecl_grav_type*)util_malloc( sizeof * ecl_grav );
  ecl_grav->init_file      = init_file;
  ecl_grav->grid_cache     = new ecl::ecl_grid_cache(ecl_grid);
  ecl_grav->aquifer_cell   = ecl_grav_common_alloc_aquifer_cell( *(ecl_grav->grid_cache) , ecl_grav->init_file );

  ecl_grav->surveys        = hash_alloc();
  ecl_grav->std_density    = hash_alloc();
  return ecl_grav;
}



static void ecl_grav_add_survey__( ecl_grav_type * grav , const char * name , ecl_grav_survey_type * survey) {
  hash_insert_hash_owned_ref( grav->surveys , name , survey , ecl_grav_survey_free__ );
}


ecl_grav_survey_type * ecl_grav_add_survey_RPORV( ecl_grav_type * grav , const char * name , const ecl_file_view_type * restart_file ) {
  ecl_grav_survey_type * survey = ecl_grav_survey_alloc_RPORV( grav , restart_file , name );
  ecl_grav_add_survey__( grav , name , survey );
  return survey;
}


ecl_grav_survey_type * ecl_grav_add_survey_FIP( ecl_grav_type * grav , const char * name , const ecl_file_view_type * restart_file ) {
  ecl_grav_survey_type * survey = ecl_grav_survey_alloc_FIP( grav , restart_file , name );
  ecl_grav_add_survey__( grav , name , survey );
  return survey;
}

ecl_grav_survey_type * ecl_grav_add_survey_RFIP( ecl_grav_type * grav , const char * name , const ecl_file_view_type * restart_file ) {
  ecl_grav_survey_type * survey = ecl_grav_survey_alloc_RFIP( grav , restart_file , name );
  ecl_grav_add_survey__( grav , name , survey );
  return survey;
}


ecl_grav_survey_type * ecl_grav_add_survey_PORMOD( ecl_grav_type * grav , const char * name , const ecl_file_view_type * restart_file ) {
  ecl_grav_survey_type * survey = ecl_grav_survey_alloc_PORMOD( grav , restart_file , name );
  ecl_grav_add_survey__( grav , name , survey );
  return survey;
}


static ecl_grav_survey_type * ecl_grav_get_survey( const ecl_grav_type * grav , const char * name) {
  if (name == NULL)
    return NULL;  // Calling scope must determine if this is OK?
  else {
    if (hash_has_key( grav->surveys , name))
      return (ecl_grav_survey_type*)hash_get( grav->surveys , name );
    else {
      hash_iter_type * survey_iter = hash_iter_alloc( grav->surveys );
      fprintf(stderr,"Survey name:%s not registered. Available surveys are: \n\n     " , name);
      while (!hash_iter_is_complete( survey_iter )) {
        const char * survey = hash_iter_get_next_key( survey_iter );
        fprintf(stderr,"%s ",survey);
      }
      fprintf(stderr,"\n\n");
      hash_iter_free( survey_iter );
      exit(1);
    }
  }
}




double ecl_grav_eval( const ecl_grav_type * grav , const char * base, const char * monitor , ecl_region_type * region , double utm_x, double utm_y , double depth, int phase_mask) {
  ecl_grav_survey_type * base_survey    = ecl_grav_get_survey( grav , base );
  ecl_grav_survey_type * monitor_survey = ecl_grav_get_survey( grav , monitor );

  return ecl_grav_survey_eval( base_survey , monitor_survey , region , utm_x , utm_y , depth , phase_mask);
}


/******************************************************************/
/* The functions ecl_grav_new_std_density() and ecl_grav_add_std_density() are
   used to "install" standard conditions densities for the various phases
   involved. These functions must be called prior to calling
   ecl_grav_add_survey_FIP() - failure to do so will lead to hard failure.
*/


/**
   The function ecl_grav_new_std_density() is used to add a default density for
   a new phase.
*/

void ecl_grav_new_std_density( ecl_grav_type * grav , ecl_phase_enum phase , double default_density) {
  const char * phase_key = ecl_util_get_phase_name( phase );
  if (!hash_has_key( grav->std_density , phase_key ))
    hash_insert_hash_owned_ref( grav->std_density , phase_key , double_vector_alloc( 0 , default_density ) , double_vector_free__ );
}

/**
   In cases with many PVT regions it is possible to install per PVT
   region densities. The ecl_grav_new_std_density() must be called
   first to install a default density for the phase, and then this
   function can be called afterwards to install density for a
   particular PVT region.  In the example below we set the default gas
   density to 0.75, but in PVT regions 2 and 7 the density is
   different:

      ecl_grav_new_std_density( grav , ECL_GAS_PHASE , 0.75 );
      ecl_grav_add_std_density( grav , ECL_GAS_PHASE , 2 , 0.70 );
      ecl_grav_add_std_density( grav , ECL_GAS_PHASE , 7 , 0.80 );
*/


void ecl_grav_add_std_density( ecl_grav_type * grav , ecl_phase_enum phase , int pvtnum , double density) {
  double_vector_type * std_density = (double_vector_type*)hash_get( grav->std_density , ecl_util_get_phase_name( phase ));
  double_vector_iset( std_density , pvtnum , density );
}



void ecl_grav_free( ecl_grav_type * ecl_grav ) {
  delete ecl_grav->grid_cache;
  free( ecl_grav->aquifer_cell );
  hash_free( ecl_grav->surveys );
  hash_free( ecl_grav->std_density );
  free( ecl_grav );
}
