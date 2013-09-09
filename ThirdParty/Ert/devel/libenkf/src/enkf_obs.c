/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_obs.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <ert/util/hash.h>
#include <ert/util/util.h>
#include <ert/util/msg.h>

#include <ert/config/conf.h>

#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/ecl_sum.h>

#include <ert/enkf/summary_obs.h>
#include <ert/enkf/block_obs.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/obs_vector.h>
#include <ert/enkf/enkf_state.h>
#include <ert/enkf/local_ministep.h>
#include <ert/enkf/local_config.h>
#include <ert/enkf/meas_data.h>
#include <ert/enkf/enkf_obs.h>
#include <ert/enkf/enkf_defaults.h>
#include <ert/enkf/config_keys.h>

/*

The observation system
----------------------

The observation system in the EnKF code is a three layer system. At
the the top is the enkf_obs_type. The enkf_main object contains one
enkf_obs instance which has internalized ALL the observation data. In
enkf_obs the the data is internalized in a hash table, where the keys
in the table are the keys used the observation file.

The next level is the obs_vector type which is a vector of length
num_report_steps. Each element in this vector can either point a
spesific observation instance (which actually contains the data), or
be NULL, if the observation is not active at this report step. In
addition the obs_vector contains function pointers to manipulate the
observation data at the lowest level.

At the lowest level we have specific observation instances,
field_obs, summary_obs and gen_obs. These instances contain the actual
data.

To summarize we can say:

  1. enkf_obs has ALL the observation data.

  2. obs_vector has the full time series for one observation key,
     i.e. all the watercuts in well P2.

  3. field_obs/gen_obs/summary_obs instances contain the actual
     observed data for one (logical) observation and one report step.


In the following example we have two observations

 WWCT:OP1 The water cut in well OP1. This is an observation which is
    active for many report steps, at the lowest level it is
    implemented as summary_obs.

 RFT_P2 This is an RFT test for one well. Only active at one report
    step, implemented at the lowest level as a field_obs instance.


 In the example below there are in total five report steps, hence all
 the obs_vector instances have five 'slots'. If there is no active
 observation for a particular report step, the corresponding pointer
 in the obs_vector instance is NULL.



      _____________________           _____________________
     /                       enkf_obs                      \
     |                                                     |
     |                                                     |
     | obs_hash: {"WWCT:OP1" , "RFT_P2"}                   |
     |                |           |                        |
     |                |           |                        |
     \________________|___________|________________________/
                      |           |
                      |           |
                      |           |
                      |           \--------------------------------------------------------------\
                      |                                                                          |
                      |                                                                          |
                     \|/                                                                         |
 |--- obs_vector: WWCT:OP1 -----------------------------------------------------|                |
 | Function pointers:       --------  --------  --------  --------  --------    |                |
 | Pointing to the          |      |  |      |  |      |  |      |  |      |    |                |
 | underlying               | NULL |  |  X   |  |  X   |  | NULL |  |  X   |    |                |
 | implementation in the    |      |  |  |   |  |  |   |  |      |  |  |   |    |                |
 | summary_obs object.      --------  ---|----  ---|----  --------  ---|----    |                |
 |---------------------------------------|---------|-------------------|--------|                |
                                         |         |                   |                         |
                                        \|/        |                   |                         |
                                |-- summary_obs -| |                  \|/                        |
                                | Value: 0.56..  | |           |-- summary_obs -|                |
                                | std  : 0.15..  | |           | Value: 0.70..  |                |
                                |----------------| |           | std  : 0.25..  |                |
                                                   |           |----------------|                |
                                                  \|/                                            |
                                          |-- summary_obs -|                                     |
                                          | Value: 0.62..  |                                     |
                                          | std  : 0.12..  |                                     |
                                          |----------------|                                     |
                                                                                                 |
                                                                                                 |
                                                                                                 |
  The observation WWCT:OP1 is an observation of summary type, and the                            |
  obs_vector contains pointers to summary_obs instances; along with                              |
  function pointers to manipulate the summary_obs instances. The                                 |
  observation is not active for report steps 0 and 3, so for these                               |
  report steps the obse vector has a NULL pointer.                                               |
                                                                                                 |
                                                                                                 |
                                                                                                 |
                                                                                                 |
                                                                                                 |
                                                                                                 |
 |--- obs_vector: RFT_P2 -------------------------------------------------------|                |
 | Function pointers:       --------  --------  --------  --------  --------    |                |
 | Pointing to the          |      |  |      |  |      |  |      |  |      |    |<---------------/
 | underlying               | NULL |  | NULL |  | NULL |  |  X   |  | NULL |    |
 | implementation in the    |      |  |      |  |      |  |  |   |  |      |    |
 | field_obs object.        --------  --------  --------  ---|----  --------    |
 |-----------------------------------------------------------|------------------|
                                                             |
                                                             |                                       
                                                            \|/
                                        |-- field_obs -----------------------------------|
                                        | i = 25 , j = 16, k = 10, value = 278, std = 10 |
                                        | i = 25 , j = 16, k = 11, value = 279, std = 10 |
                                        | i = 25 , j = 16, k = 12, value = 279, std = 10 |
                                        | i = 25 , j = 17, k = 12, value = 281, std = 10 |
                                        | i = 25 , j = 18, k = 12, value = 282, std = 10 |
                                        |------------------------------------------------|


 The observation RFT_P2 is an RFT observation which is only active at
 one report step, i.e. 4/5 pointers in the obs_vector are just NULL
 pointers. The actual observation(s) are stored in a field_obs
 instance.

 */


/**
TODO

    This static function header shall be removed when the
    configuration is unified....
*/
static conf_class_type * enkf_obs_get_obs_conf_class();



//////////////////////////////////////////////////////////////////////////////////////



struct enkf_obs_struct {
  /** A hash of obs_vector_types indexed by user provided keys. */
  bool                  have_obs;
  char                * config_file;  /* The name of the config file which has been loaded. */ 
  hash_type           * obs_hash;
  time_t_vector_type  * obs_time;     /* For fast lookup of report_step -> obs_time */
  const history_type  * history;      /* A shared (not owned by enkf_obs) reference to the history object - used when
                                         adding HISTORY observations. */
};



//////////////////////////////////////////////////////////////////////////////////////



enkf_obs_type * enkf_obs_alloc(  )
{
  enkf_obs_type * enkf_obs = util_malloc(sizeof * enkf_obs);
  enkf_obs->have_obs       = false;
  enkf_obs->obs_hash       = hash_alloc();
  enkf_obs->obs_time       = time_t_vector_alloc(0  , -1 );

  enkf_obs->history        = NULL;
  enkf_obs->config_file    = NULL; 
  return enkf_obs;
}



bool enkf_obs_have_obs( const enkf_obs_type * enkf_obs ) {
  return enkf_obs->have_obs;
}


void enkf_obs_free(enkf_obs_type * enkf_obs) {
  hash_free(enkf_obs->obs_hash);
  time_t_vector_free( enkf_obs->obs_time );
  util_safe_free( enkf_obs->config_file );
  free(enkf_obs);
}




time_t enkf_obs_iget_obs_time(enkf_obs_type * enkf_obs , int report_step) {
  time_t obs_time     = time_t_vector_safe_iget( enkf_obs->obs_time , report_step );
  time_t default_time = time_t_vector_get_default( enkf_obs->obs_time );
  if (obs_time == default_time) {
    obs_time = history_get_time_t_from_restart_nr( enkf_obs->history , report_step );
    time_t_vector_iset( enkf_obs->obs_time , report_step , obs_time );
  }
  return obs_time;
}


/**
   Observe that the obs_vector can be NULL - in which it is of course not added.
*/
void enkf_obs_add_obs_vector(enkf_obs_type       * enkf_obs,
                             const char          * key ,
                             const obs_vector_type * vector)
{
  if (vector != NULL) {
    if (hash_has_key(enkf_obs->obs_hash , key))
      util_abort("%s: Observation with key:%s already added.\n",__func__ , key);
    hash_insert_hash_owned_ref(enkf_obs->obs_hash , key , vector , obs_vector_free__);
  }
}


bool enkf_obs_has_key(const enkf_obs_type * obs , const char * key) {
  return hash_has_key(obs->obs_hash , key);
}

obs_vector_type * enkf_obs_get_vector(const enkf_obs_type * obs, const char * key) {
  return hash_get(obs->obs_hash , key);
}



static void enkf_obs_get_obs_and_measure_summary(const enkf_obs_type      * enkf_obs,
                                                 obs_vector_type          * obs_vector , 
                                                 enkf_fs_type             * fs,
                                                 const int_vector_type    * step_list , 
                                                 state_enum                 state,
                                                 const int_vector_type     * ens_active_list , 
                                                 const enkf_state_type     **   ensemble ,
                                                 meas_data_type             *   meas_data,
                                                 obs_data_type              * obs_data,
                                                 const local_obsset_type    * obsset , 
                                                 double_vector_type         * obs_value , 
                                                 double_vector_type         * obs_std) {
  
  const active_list_type * active_list = local_obsset_get_obs_active_list( obsset , obs_vector_get_obs_key( obs_vector ));
  matrix_type * error_covar = NULL;
  int active_count          = 0;
  int last_step = -1;
  int step;

  /*1: Determine which report_steps have active observations; and collect the observed values. */
  double_vector_reset( obs_std );
  double_vector_reset( obs_value );
  
  for (int i = 0; i < int_vector_size( step_list ); i++) {
    step = int_vector_iget( step_list , i );
    if (obs_vector_iget_active( obs_vector , step ) && active_list_iget( active_list , 0 /* Index into the scalar summary observation */)) {
      {
        const summary_obs_type * summary_obs = obs_vector_iget_node( obs_vector , step );
        double_vector_iset( obs_std   , active_count , summary_obs_get_std( summary_obs ));
        double_vector_iset( obs_value , active_count , summary_obs_get_value( summary_obs ));
        last_step = step;
      }
      active_count++;
    }
  }
  
  
  if (active_count > 0) {
    /*2: Estimate a covariance matrix. */
    auto_corrf_ftype * auto_corrf;
    double auto_corrf_param;
    {
      const summary_obs_type * summary_obs = obs_vector_iget_node( obs_vector , last_step );
      auto_corrf       = summary_obs_get_auto_corrf( summary_obs );
      auto_corrf_param = summary_obs_get_auto_corrf_param( summary_obs );
    }
    
    if ((active_count > 1) && (auto_corrf != NULL)) {
      int i,j;
      error_covar = matrix_alloc( active_count , active_count ); /* Will be freed by the obs_block instance. */
      for (i = 0; i < active_count; i++) {
        for (j=0; j <= i; j++) {
          double covar = sqrt( double_vector_iget( obs_std , i ) * double_vector_iget( obs_std , j ));
          double corr  = auto_corrf( (time_t_vector_iget( enkf_obs->obs_time , i ) - time_t_vector_iget( enkf_obs->obs_time , j )) / (24.00 * 3600) , auto_corrf_param );
          
          matrix_iset(error_covar , i , j  , covar * corr );
          if (i != j)
            matrix_iset(error_covar , j , i  , covar * corr );
        }
      }
      {
        char * filename = util_alloc_sprintf( "/tmp/covar/%s_%04d-%04d" , obs_vector_get_obs_key( obs_vector ), 
                                              int_vector_iget( step_list , 0 ),
                                              int_vector_get_last( step_list ));
        FILE * stream = util_mkdir_fopen( filename , "w");

        matrix_fprintf(error_covar , "%7.3f " , stream );
        fclose( stream );

        free( filename );
      }
    }
    
    
    /*
      3: Fill up the obs_block and meas_block structures with this
      time-aggregated summary observation.  Passing in the error_covar
      matrix (which can be NULL) to the obs_block instance.
    */

    {
      obs_block_type  * obs_block  = obs_data_add_block( obs_data , obs_vector_get_obs_key( obs_vector ) , active_count , error_covar , true);
      meas_block_type * meas_block = meas_data_add_block( meas_data, obs_vector_get_obs_key( obs_vector ) , int_vector_get_last( step_list ) , active_count );
      
      for (int i=0; i < active_count; i++) 
        obs_block_iset( obs_block , i , double_vector_iget( obs_value , i) , double_vector_iget( obs_std , i ));
      
      active_count = 0;
      for (int i = 0; i < int_vector_size( step_list ); i++) {
        int step = int_vector_iget( step_list , i );
        if (obs_vector_iget_active( obs_vector , step ) && active_list_iget( active_list , 0 /* Index into the scalar summary observation */)) {
          
          for (int iens_index = 0; iens_index < int_vector_size( ens_active_list ); iens_index++) {
            const int iens = int_vector_iget( ens_active_list , iens_index );
            const char * state_key = obs_vector_get_state_kw(obs_vector);
            enkf_node_type * enkf_node = enkf_state_get_node(ensemble[iens] , state_key);
            node_id_type node_id = {.report_step = step, 
                                    .iens        = iens , 
                                    .state       = state };
            
            enkf_node_load( enkf_node , fs , node_id );
            
            meas_block_iset(meas_block , 
                            iens_index , active_count , 
                            summary_get( enkf_node_value_ptr( enkf_node ) , node_id.report_step , node_id.state ));
            
          }
          active_count++;
        } 
      }
    }
  }
}





/*
  This will append observations and simulated responses from
  report_step to obs_data and meas_data.  
  Call obs_data_reset and meas_data_reset on obs_data and meas_data
  if you want to use fresh instances.
*/
void enkf_obs_get_obs_and_measure(const enkf_obs_type    * enkf_obs,
                                  enkf_fs_type           * fs,
                                  const int_vector_type  * step_list , 
                                  state_enum               state,
                                  const int_vector_type * ens_active_list , 
                                  const enkf_state_type    ** ensemble ,
                                  meas_data_type           * meas_data,
                                  obs_data_type            * obs_data,
                                  const local_obsset_type  * obsset) {

  double_vector_type * work_value  = double_vector_alloc( 0 , -1 );
  double_vector_type * work_std    = double_vector_alloc( 0 , -1 );
  
  hash_iter_type * iter = local_obsset_alloc_obs_iter( obsset );
  while ( !hash_iter_is_complete(iter) ) {
    const char * obs_key         = hash_iter_get_next_key( iter );
    obs_vector_type * obs_vector = hash_get( enkf_obs->obs_hash , obs_key );
    obs_impl_type obs_type       = obs_vector_get_impl_type( obs_vector );
    if ((obs_type == SUMMARY_OBS)) //&& ((end_step - start_step) > 1))
      /* The measure_summary() function fails for normal non-merged EnKF updates. */
      enkf_obs_get_obs_and_measure_summary( enkf_obs , 
                                            obs_vector , 
                                            fs , 
                                            step_list , 
                                            state , 
                                            ens_active_list , 
                                            ensemble , 
                                            meas_data , 
                                            obs_data , 
                                            obsset , 
                                            work_value, 
                                            work_std);

    else {
      for (int i=0; i < int_vector_size( step_list ); i++) {
        int report_step = int_vector_iget( step_list , i );
        if (obs_vector_iget_active(obs_vector , report_step)) {                             /* The observation is active for this report step.     */
          const active_list_type * active_list = local_obsset_get_obs_active_list( obsset , obs_key );
          obs_vector_iget_observations(obs_vector , report_step , obs_data , active_list);  /* Collect the observed data in the obs_data instance. */
          /* Could be multithreaded */
          for (int iens_index = 0; iens_index < int_vector_size( ens_active_list ); iens_index++) {
            const int iens = int_vector_iget( ens_active_list , iens_index );
            obs_vector_measure(obs_vector , fs , state , report_step , iens_index , ensemble[iens] , meas_data , active_list);
          }
        } 
      }
    }
  }
  hash_iter_free(iter);
  double_vector_free( work_value );
  double_vector_free( work_std   );
}



void enkf_obs_reload( enkf_obs_type * enkf_obs , 
                      const history_type * history , 
                      const ecl_grid_type * grid , 
                      const ecl_sum_type * refcase , 
                      double std_cutoff , 
                      ensemble_config_type * ensemble_config ) {
  enkf_obs_load( enkf_obs , history , enkf_obs->config_file , grid , refcase , std_cutoff , ensemble_config );
}



/**
   This function will load an observation configuration from the
   observation file @config_file. 

   If called several times during one invocation the function will
   start by clearing the current content.
*/



void enkf_obs_load(enkf_obs_type * enkf_obs , 
                   const history_type * history , 
                   const char * config_file,  
                   const ecl_grid_type * grid , 
                   const ecl_sum_type * refcase , 
                   double std_cutoff , ensemble_config_type * ensemble_config) {
  
  if (config_file == NULL) {
    hash_clear( enkf_obs->obs_hash );
    enkf_obs->have_obs = false;
  }  else {
    enkf_obs->history = history;
    if ( enkf_obs->history == NULL) {
      fprintf(stderr,"** ERROR: When loading obervations you must provide either REFCASE or a SCHEDULE file.\n");
      fprintf(stderr,"**        The observations in obs file:%s will be ignored \n",config_file);
      enkf_obs->have_obs = false;
      return;
    }  else {
      int last_report                      = history_get_last_restart( enkf_obs->history );
      conf_class_type    * enkf_conf_class = enkf_obs_get_obs_conf_class();
      conf_instance_type * enkf_conf       = conf_instance_alloc_from_file(enkf_conf_class, "enkf_conf", config_file);
      
      if(conf_instance_validate(enkf_conf) == false) 
        util_abort("Can not proceed with this configuration.\n");
      
      if (enkf_obs->config_file != NULL)      /* Clear current instance, observe that this function   */
        hash_clear( enkf_obs->obs_hash );     /* will reload even if it is called repeatedly with the */
                                              /* same config_file.                                    */ 
      
      /** Handle HISTORY_OBSERVATION instances. */
      {
        stringlist_type * hist_obs_keys = conf_instance_alloc_list_of_sub_instances_of_class_by_name(enkf_conf, "HISTORY_OBSERVATION");
        int               num_hist_obs  = stringlist_get_size(hist_obs_keys);
        
        for(int hist_obs_nr = 0; hist_obs_nr < num_hist_obs; hist_obs_nr++) {
          const char               * obs_key       = stringlist_iget(hist_obs_keys, hist_obs_nr);
          const conf_instance_type * hist_obs_conf = conf_instance_get_sub_instance_ref(enkf_conf, obs_key);
          obs_vector_type * obs_vector;
          enkf_config_node_type * config_node;
          
          config_node = ensemble_config_add_summary( ensemble_config , obs_key , LOAD_FAIL_WARN );
          if (config_node != NULL) {
            obs_vector = obs_vector_alloc( SUMMARY_OBS , obs_key , ensemble_config_get_node( ensemble_config , obs_key ), last_report);
            if (obs_vector != NULL) {
              if (obs_vector_load_from_HISTORY_OBSERVATION(obs_vector , 
                                                           hist_obs_conf , 
                                                           enkf_obs->history ,
                                                           ensemble_config,
                                                           std_cutoff ))
                enkf_obs_add_obs_vector(enkf_obs, obs_key, obs_vector);
              else {
                fprintf(stderr,"** Could not load historical data for observation:%s - ignored\n",obs_key);
                obs_vector_free( obs_vector );
              }
            }
          } else 
            fprintf(stderr,"** Warning: summary:%s does not exist - observation:%s not added. \n", obs_key , obs_key);
        }
        
        stringlist_free(hist_obs_keys);
      }
      
      
      
      /** Handle SUMMARY_OBSERVATION instances. */
      {
        stringlist_type * sum_obs_keys = conf_instance_alloc_list_of_sub_instances_of_class_by_name(enkf_conf, "SUMMARY_OBSERVATION");
        int               num_sum_obs  = stringlist_get_size(sum_obs_keys);
        
        
        for(int sum_obs_nr = 0; sum_obs_nr < num_sum_obs; sum_obs_nr++) {
          const char               * obs_key      = stringlist_iget(sum_obs_keys, sum_obs_nr);
          const conf_instance_type * sum_obs_conf = conf_instance_get_sub_instance_ref(enkf_conf, obs_key);
          const char               * sum_key      = conf_instance_get_item_value_ref(   sum_obs_conf , "KEY"   );
          obs_vector_type * obs_vector;
          enkf_config_node_type * config_node;
          
          config_node = ensemble_config_add_summary( ensemble_config , sum_key , LOAD_FAIL_WARN );
          if (config_node != NULL) {
            obs_vector = obs_vector_alloc( SUMMARY_OBS , obs_key , ensemble_config_get_node( ensemble_config , sum_key ), last_report);
            if (obs_vector != NULL) {
              obs_vector_load_from_SUMMARY_OBSERVATION(obs_vector , sum_obs_conf , enkf_obs->history , ensemble_config);
              enkf_obs_add_obs_vector(enkf_obs, obs_key, obs_vector);
            }
          } else 
            fprintf(stderr,"** Warning: summary key:%s does not exist - observation key:%s not added.\n", sum_key , obs_key);
        }
        stringlist_free(sum_obs_keys);
      }
      
    
      /** Handle BLOCK_OBSERVATION instances. */
      {
        stringlist_type * block_obs_keys = conf_instance_alloc_list_of_sub_instances_of_class_by_name(enkf_conf, "BLOCK_OBSERVATION");
        int               num_block_obs  = stringlist_get_size(block_obs_keys);
        
        for(int block_obs_nr = 0; block_obs_nr < num_block_obs; block_obs_nr++)
          {
            const char               * obs_key        = stringlist_iget(block_obs_keys, block_obs_nr);
            const conf_instance_type * block_obs_conf = conf_instance_get_sub_instance_ref(enkf_conf, obs_key);
            obs_vector_type * obs_vector = obs_vector_alloc_from_BLOCK_OBSERVATION(block_obs_conf , grid , refcase , enkf_obs->history ,ensemble_config);
            if (obs_vector != NULL)
              enkf_obs_add_obs_vector(enkf_obs, obs_key, obs_vector);
          }
        stringlist_free(block_obs_keys);
      }
      
      
      /** Handle GENERAL_OBSERVATION instances. */
      {
        stringlist_type * block_obs_keys = conf_instance_alloc_list_of_sub_instances_of_class_by_name(enkf_conf, "GENERAL_OBSERVATION");
        int               num_block_obs  = stringlist_get_size(block_obs_keys);
        
        for(int block_obs_nr = 0; block_obs_nr < num_block_obs; block_obs_nr++)
          {
            const char               * obs_key        = stringlist_iget(block_obs_keys, block_obs_nr);
            const conf_instance_type * gen_obs_conf   = conf_instance_get_sub_instance_ref(enkf_conf, obs_key);
            
            obs_vector_type * obs_vector = obs_vector_alloc_from_GENERAL_OBSERVATION(gen_obs_conf , enkf_obs->history  , ensemble_config);
            if (obs_vector != NULL) 
              enkf_obs_add_obs_vector(enkf_obs, obs_key, obs_vector);
          }
        stringlist_free(block_obs_keys);
      }
      
      /* Initializing obs_time */
      {
        int step;
        for (step =0; step <= last_report; step++) {
          time_t obs_time = history_get_time_t_from_restart_nr( enkf_obs->history , step );
          time_t_vector_iset( enkf_obs->obs_time , step , obs_time );
        }
      }
      
      conf_instance_free(enkf_conf      );
      conf_class_free(   enkf_conf_class);
      enkf_obs->config_file = util_realloc_string_copy( enkf_obs->config_file , config_file );
      enkf_obs->have_obs = true;
    }
  }
}






 static conf_class_type * enkf_obs_get_obs_conf_class( void ) {
  const char * enkf_conf_help = "An instance of the class ENKF_CONFIG shall contain neccessary infomation to run the enkf.";
  conf_class_type * enkf_conf_class = conf_class_alloc_empty("ENKF_CONFIG", true , false , enkf_conf_help);
  conf_class_set_help(enkf_conf_class, enkf_conf_help);



  /** Create and insert HISTORY_OBSERVATION class. */
  {
    const char * help_class_history_observation = "The class HISTORY_OBSERVATION is used to condition on a time series from the production history. The name of the an instance is used to define the item to condition on, and should be in summary.x syntax. E.g., creating a HISTORY_OBSERVATION instance with name GOPR:P4 conditions on GOPR for group P4.";

    conf_class_type * history_observation_class = conf_class_alloc_empty("HISTORY_OBSERVATION", false , false, help_class_history_observation);

    conf_item_spec_type * item_spec_error_mode = conf_item_spec_alloc("ERROR_MODE", true, DT_STR , "The string ERROR_MODE gives the error mode for the observation.");
    conf_item_spec_type * item_spec_auto_corrf_param = conf_item_spec_alloc("AUTO_CORRF_PARAM", false, DT_FLOAT , "A parameter passed to the auto correlation function.");
    const char * help_item_spec_auto_corrf = "The name of the auto correlation function used for smoother updates.";
    conf_item_spec_type * item_spec_auto_corrf = conf_item_spec_alloc("AUTO_CORRF", false , DT_STR , help_item_spec_auto_corrf);
    conf_item_spec_add_restriction( item_spec_auto_corrf , AUTO_CORRF_EXP);
    conf_item_spec_add_restriction( item_spec_auto_corrf , AUTO_CORRF_GAUSS);


    conf_item_spec_add_restriction(item_spec_error_mode, "REL");
    conf_item_spec_add_restriction(item_spec_error_mode, "ABS");
    conf_item_spec_add_restriction(item_spec_error_mode, "RELMIN");
    conf_item_spec_set_default_value(item_spec_error_mode, "RELMIN");

    conf_item_spec_type * item_spec_error     = conf_item_spec_alloc("ERROR", true, DT_POSFLOAT , "The positive floating number ERROR gives the standard deviation (ABS) or the relative uncertainty (REL/RELMIN) of the observations.");
    conf_item_spec_set_default_value(item_spec_error, "0.10");

    conf_item_spec_type * item_spec_error_min = conf_item_spec_alloc("ERROR_MIN", true, DT_POSFLOAT , "The positive floating point number ERROR_MIN gives the minimum value for the standard deviation of the observation when RELMIN is used.");
    conf_item_spec_set_default_value(item_spec_error_min, "0.10");

    conf_class_insert_owned_item_spec(history_observation_class, item_spec_error_mode);
    conf_class_insert_owned_item_spec(history_observation_class, item_spec_error);
    conf_class_insert_owned_item_spec(history_observation_class, item_spec_error_min);
    conf_class_insert_owned_item_spec(history_observation_class, item_spec_auto_corrf);
    conf_class_insert_owned_item_spec(history_observation_class, item_spec_auto_corrf_param);

    /** Sub class segment. */
    {
      const char * help_class_segment = "The class SEGMENT is used to fine tune the error model.";
      conf_class_type * segment_class = conf_class_alloc_empty("SEGMENT", false , false, help_class_segment);

      conf_item_spec_type * item_spec_start_segment = conf_item_spec_alloc("START", true, DT_INT, "The first restart in the segment.");
      conf_item_spec_type * item_spec_stop_segment  = conf_item_spec_alloc("STOP", true, DT_INT, "The last restart in the segment.");

      conf_item_spec_type * item_spec_error_mode_segment = conf_item_spec_alloc("ERROR_MODE", true, DT_STR , "The string ERROR_MODE gives the error mode for the observation.");
      conf_item_spec_add_restriction(item_spec_error_mode_segment, "REL");
      conf_item_spec_add_restriction(item_spec_error_mode_segment, "ABS");
      conf_item_spec_add_restriction(item_spec_error_mode_segment, "RELMIN");
      conf_item_spec_set_default_value(item_spec_error_mode_segment, "RELMIN");

      conf_item_spec_type * item_spec_error_segment     = conf_item_spec_alloc("ERROR", true, DT_POSFLOAT , "The positive floating number ERROR gives the standard deviation (ABS) or the relative uncertainty (REL/RELMIN) of the observations.");
      conf_item_spec_set_default_value(item_spec_error_segment, "0.10");

      conf_item_spec_type * item_spec_error_min_segment = conf_item_spec_alloc("ERROR_MIN", true, DT_POSFLOAT , "The positive floating point number ERROR_MIN gives the minimum value for the standard deviation of the observation when RELMIN is used.");
      conf_item_spec_set_default_value(item_spec_error_min_segment, "0.10");

  
      conf_class_insert_owned_item_spec(segment_class, item_spec_start_segment);
      conf_class_insert_owned_item_spec(segment_class, item_spec_stop_segment);
      conf_class_insert_owned_item_spec(segment_class, item_spec_error_mode_segment);
      conf_class_insert_owned_item_spec(segment_class, item_spec_error_segment);
      conf_class_insert_owned_item_spec(segment_class, item_spec_error_min_segment);

      conf_class_insert_owned_sub_class(history_observation_class, segment_class);
    }
    
    conf_class_insert_owned_sub_class(enkf_conf_class, history_observation_class);
  }



  /** Create and insert SUMMARY_OBSERVATION class. */
  {
    const char * help_class_summary_observation = "The class SUMMARY_OBSERVATION can be used to condition on any observation whos simulated value is written to the summary file.";
    conf_class_type * summary_observation_class = conf_class_alloc_empty("SUMMARY_OBSERVATION", false , false, help_class_summary_observation);

    const char * help_item_spec_value = "The floating point number VALUE gives the observed value.";
    conf_item_spec_type * item_spec_value = conf_item_spec_alloc("VALUE", true, DT_FLOAT , help_item_spec_value);

    const char * help_item_spec_error = "The positive floating point number ERROR is the standard deviation of the observed value.";
    conf_item_spec_type * item_spec_error = conf_item_spec_alloc("ERROR", true, DT_POSFLOAT ,help_item_spec_error );

    const char * help_item_spec_date = "The DATE item gives the observation time as the date date it occured. Format is dd/mm/yyyy.";
    conf_item_spec_type * item_spec_date = conf_item_spec_alloc("DATE", false, DT_DATE , help_item_spec_date);

    const char * help_item_spec_days = "The DAYS item gives the observation time as days after simulation start.";
    conf_item_spec_type * item_spec_days = conf_item_spec_alloc("DAYS", false, DT_POSFLOAT , help_item_spec_days);

    const char * help_item_spec_restart = "The RESTART item gives the observation time as the ECLIPSE restart nr.";
    conf_item_spec_type * item_spec_restart = conf_item_spec_alloc("RESTART", false, DT_POSINT , help_item_spec_restart);

    const char * help_item_spec_sumkey = "The string SUMMARY_KEY is used to look up the simulated value in the summary file. It has the same format as the summary.x program, e.g. WOPR:P4";
    conf_item_spec_type * item_spec_sumkey = conf_item_spec_alloc("KEY", true, DT_STR , help_item_spec_sumkey);

    conf_item_spec_type * item_spec_error_min = conf_item_spec_alloc("ERROR_MIN", true, DT_POSFLOAT , 
                                                                     "The positive floating point number ERROR_MIN gives the minimum value for the standard deviation of the observation when RELMIN is used.");
    conf_item_spec_type * item_spec_error_mode = conf_item_spec_alloc("ERROR_MODE", true, DT_STR , "The string ERROR_MODE gives the error mode for the observation.");
    
    conf_item_spec_add_restriction(item_spec_error_mode, "REL");
    conf_item_spec_add_restriction(item_spec_error_mode, "ABS");
    conf_item_spec_add_restriction(item_spec_error_mode, "RELMIN");
    conf_item_spec_set_default_value(item_spec_error_mode, "ABS");
    conf_item_spec_set_default_value(item_spec_error_min, "0.10");

    conf_class_insert_owned_item_spec(summary_observation_class, item_spec_value);
    conf_class_insert_owned_item_spec(summary_observation_class, item_spec_error);
    conf_class_insert_owned_item_spec(summary_observation_class, item_spec_date);
    conf_class_insert_owned_item_spec(summary_observation_class, item_spec_days);
    conf_class_insert_owned_item_spec(summary_observation_class, item_spec_restart);
    conf_class_insert_owned_item_spec(summary_observation_class, item_spec_sumkey);
    conf_class_insert_owned_item_spec(summary_observation_class, item_spec_error_mode);
    conf_class_insert_owned_item_spec(summary_observation_class, item_spec_error_min);

    /** Create a mutex on DATE, DAYS and RESTART. */
    conf_item_mutex_type * time_mutex = conf_class_new_item_mutex(summary_observation_class , true , false);

    conf_item_mutex_add_item_spec(time_mutex, item_spec_date);
    conf_item_mutex_add_item_spec(time_mutex, item_spec_days);
    conf_item_mutex_add_item_spec(time_mutex, item_spec_restart);

    conf_class_insert_owned_sub_class(enkf_conf_class, summary_observation_class);
  }



  /** Create and insert BLOCK_OBSERVATION class. */
  {
    const char * help_class_block_observation = "The class BLOCK_OBSERVATION can be used to condition on an observation whos simulated values are block/cell values of a field, e.g. RFT tests.";
    conf_class_type * block_observation_class = conf_class_alloc_empty("BLOCK_OBSERVATION", false , false, help_class_block_observation);

    const char * help_item_spec_field = "The item FIELD gives the observed field. E.g., ECLIPSE fields such as PRESSURE, SGAS or any user defined fields such as PORO or PERMX.";
    conf_item_spec_type * item_spec_field = conf_item_spec_alloc("FIELD", true , DT_STR , help_item_spec_field);

    const char * help_item_spec_date = "The DATE item gives the observation time as the date date it occured. Format is dd/mm/yyyy.";
    conf_item_spec_type * item_spec_date = conf_item_spec_alloc("DATE", false, DT_DATE , help_item_spec_date);

    const char * help_item_spec_days = "The DAYS item gives the observation time as days after simulation start.";
    conf_item_spec_type * item_spec_days = conf_item_spec_alloc("DAYS", false, DT_POSFLOAT , help_item_spec_days);

    const char * help_item_spec_restart = "The RESTART item gives the observation time as the ECLIPSE restart nr.";
    conf_item_spec_type * item_spec_restart = conf_item_spec_alloc("RESTART", false, DT_POSINT , help_item_spec_restart);

    conf_item_spec_type * item_spec_source = conf_item_spec_alloc("SOURCE", false, DT_STR , "The simulated data can be taken from the field or summary keys.");
    
    
    conf_item_spec_add_restriction(item_spec_source, "FIELD");
    conf_item_spec_add_restriction(item_spec_source, "SUMMARY");
    conf_item_spec_set_default_value(item_spec_source, "FIELD");

    conf_class_insert_owned_item_spec(block_observation_class, item_spec_source);
    conf_class_insert_owned_item_spec(block_observation_class, item_spec_field);
    conf_class_insert_owned_item_spec(block_observation_class, item_spec_date);
    conf_class_insert_owned_item_spec(block_observation_class, item_spec_days);
    conf_class_insert_owned_item_spec(block_observation_class, item_spec_restart);

    /** Create a mutex on DATE, DAYS and RESTART. */
    {
      conf_item_mutex_type * time_mutex = conf_class_new_item_mutex(block_observation_class , true , false);
      conf_item_mutex_add_item_spec(time_mutex, item_spec_date);
      conf_item_mutex_add_item_spec(time_mutex, item_spec_days);
      conf_item_mutex_add_item_spec(time_mutex, item_spec_restart);
    }

    /** Create and insert the sub class OBS. */
    {
      const char * help_class_obs = "The class OBS is used to specify a single observed point.";
      conf_class_type * obs_class = conf_class_alloc_empty("OBS", true , false, help_class_obs);

      const char * help_item_i = "The item I gives the I index of the block observation.";
      conf_item_spec_type * item_spec_i = conf_item_spec_alloc("I", true, DT_POSINT , help_item_i);

      const char * help_item_j = "The item J gives the J index of the block observation.";
      conf_item_spec_type * item_spec_j = conf_item_spec_alloc("J", true,  DT_POSINT, help_item_j);

      const char * help_item_k = "The item K gives the K index of the block observation.";
      conf_item_spec_type * item_spec_k = conf_item_spec_alloc("K", true, DT_POSINT, help_item_k);

      const char * help_item_spec_value = "The floating point number VALUE gives the observed value.";
      conf_item_spec_type * item_spec_value = conf_item_spec_alloc("VALUE", true, DT_FLOAT , help_item_spec_value);

      const char * help_item_spec_error = "The positive floating point number ERROR is the standard deviation of the observed value.";
      conf_item_spec_type * item_spec_error = conf_item_spec_alloc("ERROR",  true, DT_POSFLOAT , help_item_spec_error);

      conf_item_spec_type * item_spec_error_mode = conf_item_spec_alloc("ERROR_MODE", true, DT_STR , "The string ERROR_MODE gives the error mode for the observation.");

      conf_item_spec_type * item_spec_error_min = conf_item_spec_alloc("ERROR_MIN", true, DT_POSFLOAT , 
                                                                       "The positive floating point number ERROR_MIN gives the minimum value for the standard deviation of the observation when RELMIN is used.");
      
      conf_item_spec_add_restriction(item_spec_error_mode, "REL");
      conf_item_spec_add_restriction(item_spec_error_mode, "ABS");
      conf_item_spec_add_restriction(item_spec_error_mode, "RELMIN");
      conf_item_spec_set_default_value(item_spec_error_mode, "ABS");
      conf_item_spec_set_default_value(item_spec_error_min, "0.10" );

      conf_class_insert_owned_item_spec(obs_class, item_spec_i);
      conf_class_insert_owned_item_spec(obs_class, item_spec_j);
      conf_class_insert_owned_item_spec(obs_class, item_spec_k);
      conf_class_insert_owned_item_spec(obs_class, item_spec_value);
      conf_class_insert_owned_item_spec(obs_class, item_spec_error);
      conf_class_insert_owned_item_spec(obs_class, item_spec_error_mode);
      conf_class_insert_owned_item_spec(obs_class, item_spec_error_min);
      
      conf_class_insert_owned_sub_class(block_observation_class, obs_class);
    }

    conf_class_insert_owned_sub_class(enkf_conf_class, block_observation_class);
  }

  /** Create and insert class for general observations. */
  {
    const char * help_item_spec_restart = "The RESTART item gives the observation time as the ECLIPSE restart nr.";
    const char * help_item_spec_field = "The item DATA gives the observed GEN_DATA instance.";
    const char * help_item_spec_date = "The DATE item gives the observation time as the date date it occured. Format is dd/mm/yyyy.";
    const char * help_item_spec_days = "The DAYS item gives the observation time as days after simulation start.";
  
    conf_class_type * gen_obs_class = conf_class_alloc_empty("GENERAL_OBSERVATION" , false , false, "The class general_observation is used for general observations");

    conf_item_spec_type * item_spec_field       = conf_item_spec_alloc("DATA", true,  DT_STR , help_item_spec_field);
    conf_item_spec_type * item_spec_date        = conf_item_spec_alloc("DATE", false,  DT_DATE , help_item_spec_date);
    conf_item_spec_type * item_spec_days        = conf_item_spec_alloc("DAYS", false, DT_POSFLOAT , help_item_spec_days);
    conf_item_spec_type * item_spec_restart     = conf_item_spec_alloc("RESTART", false, DT_POSINT , help_item_spec_restart);
    conf_item_spec_type * item_spec_error_covar = conf_item_spec_alloc("ERROR_COVAR", false, DT_FILE , "Name of file containing error-covariance as formatted matrix - no header");

    conf_class_insert_owned_item_spec(gen_obs_class, item_spec_error_covar);
    conf_class_insert_owned_item_spec(gen_obs_class, item_spec_field);
    conf_class_insert_owned_item_spec(gen_obs_class, item_spec_date);
    conf_class_insert_owned_item_spec(gen_obs_class, item_spec_days);
    conf_class_insert_owned_item_spec(gen_obs_class, item_spec_restart);
    /** Create a mutex on DATE, DAYS and RESTART. */
    {
      conf_item_mutex_type * time_mutex = conf_class_new_item_mutex(gen_obs_class , true , false);
      
      conf_item_mutex_add_item_spec(time_mutex, item_spec_date);
      conf_item_mutex_add_item_spec(time_mutex, item_spec_days);
      conf_item_mutex_add_item_spec(time_mutex, item_spec_restart);
    }
    
    {
      conf_item_spec_type * item_spec_obs_file = conf_item_spec_alloc("OBS_FILE" , false , DT_FILE , "The name of an (ascii) file with observation values.");
      conf_item_spec_type * item_spec_value    = conf_item_spec_alloc("VALUE" , false , DT_FLOAT , "One scalar observation value.");
      conf_item_spec_type * item_spec_error    = conf_item_spec_alloc("ERROR" , false , DT_FLOAT , "One scalar observation error.");
      conf_item_mutex_type * value_mutex       = conf_class_new_item_mutex( gen_obs_class , true  , false);
      conf_item_mutex_type * value_error_mutex = conf_class_new_item_mutex( gen_obs_class , false , true);

      conf_class_insert_owned_item_spec(gen_obs_class , item_spec_obs_file);
      conf_class_insert_owned_item_spec(gen_obs_class , item_spec_value);
      conf_class_insert_owned_item_spec(gen_obs_class , item_spec_error);


      /* If the observation is in terms of VALUE - we must also have ERROR.
         The conf system does not (currently ??) enforce this dependency. */
         
      conf_item_mutex_add_item_spec( value_mutex , item_spec_value);
      conf_item_mutex_add_item_spec( value_mutex , item_spec_obs_file);

      conf_item_mutex_add_item_spec( value_error_mutex , item_spec_value);
      conf_item_mutex_add_item_spec( value_error_mutex , item_spec_error);
    }
  
  
    /* 
       The default is that all the elements in DATA are observed, but
       we can restrict ourselves to a list of indices, with either the
       INDEX_LIST or INDEX_FILE keywords.
    */
    {
      conf_item_spec_type * item_spec_index_list = conf_item_spec_alloc("INDEX_LIST"  ,  false  ,  DT_STR   , "A list of indicies - possibly with ranges which should be observed in the target field.");
      conf_item_spec_type * item_spec_index_file = conf_item_spec_alloc("INDEX_FILE"  ,  false  ,  DT_FILE  , "An ASCII file containing a list of indices which should be observed in the target field.");
      conf_item_mutex_type * index_mutex         = conf_class_new_item_mutex( gen_obs_class , false , false);
      
      conf_class_insert_owned_item_spec(gen_obs_class, item_spec_index_list);
      conf_class_insert_owned_item_spec(gen_obs_class, item_spec_index_file);
      conf_item_mutex_add_item_spec(index_mutex , item_spec_index_list);
      conf_item_mutex_add_item_spec(index_mutex , item_spec_index_file);
    }
    
    conf_class_insert_owned_sub_class(enkf_conf_class, gen_obs_class);
  }


  return enkf_conf_class;
}



/**
   Allocates a stringlist of obs target keys which correspond to
   summary observations, these are then added to the state vector in
   enkf_main.
*/
stringlist_type * enkf_obs_alloc_typed_keylist(enkf_obs_type * enkf_obs , obs_impl_type obs_type) {
  stringlist_type * vars = stringlist_alloc_new();
  hash_iter_type  * iter = hash_iter_alloc(enkf_obs->obs_hash); 
  const char * key = hash_iter_get_next_key(iter);
  while ( key != NULL) {
    obs_vector_type * obs_vector = hash_get( enkf_obs->obs_hash , key);
    if (obs_vector_get_impl_type(obs_vector) == obs_type)
      stringlist_append_copy(vars , key);
    key = hash_iter_get_next_key(iter);
  }
  hash_iter_free(iter);
  return vars;
}


/**
 */

stringlist_type * enkf_obs_alloc_matching_keylist(const enkf_obs_type * enkf_obs , const char * input_string) {
  stringlist_type  *  matching_keys = stringlist_alloc_new();
  stringlist_type  *  obs_keys      = hash_alloc_stringlist( enkf_obs->obs_hash );
  char             ** input_keys;
  int                 num_keys;
  
  
  util_split_string( input_string , " " , &num_keys , &input_keys);
  for (int i=0; i < num_keys; i++) {
    bool key_found = false;

    const char * input_key = input_keys[i];
    for (int j = 0; j < stringlist_get_size( obs_keys ); j++) {
      const char * obs_key = stringlist_iget( obs_keys , j);
      
      if (util_string_match( obs_key , input_key)) {
        if (!stringlist_contains( matching_keys , obs_key ))
          stringlist_append_copy( matching_keys , obs_key);
        key_found = true;
      }
    }
  }
  
  
  util_free_stringlist( input_keys , num_keys );
  stringlist_free( obs_keys );
  return matching_keys;
}



/**
   This function allocates a hash table which looks like this:

     {"OBS_KEY1": "STATE_KEY1", "OBS_KEY2": "STATE_KEY2", "OBS_KEY3": "STATE_KEY3", ....}

   where "OBS_KEY" represents the keys in the enkf_obs hash, and the
   value they are pointing at are the enkf_state keywords they are
   measuring. For instance if we have an observation with key "RFT_1A"
   the entry in the table will be:  ... "RFT_1A":  "PRESSURE", ..
   since an RFT observation observes the pressure.

   Let us consider the watercut in a well. Then the state_kw will
   typically be WWCT:P1 for a well named 'P1'. Let us assume that this
   well is observed both as a normal HISTORY observation from
   SCHEDULE, and from two separator tests, called S1 and S2. Then the
   hash table will look like this:

       "WWCT:P1": "WWCT:P1",
       "S1"     : "WWCT:P1",
       "S2"     : "WWCT:P1"


   I.e. there are three different observations keys, all observing the
   same state_kw.
*/



hash_type * enkf_obs_alloc_data_map(enkf_obs_type * enkf_obs)
{
  hash_type      * map  = hash_alloc();
  hash_iter_type * iter = hash_iter_alloc(enkf_obs->obs_hash); 
  const char * key = hash_iter_get_next_key(iter);
  while ( key != NULL) {
    obs_vector_type * obs_vector = hash_get( enkf_obs->obs_hash , key);
    hash_insert_ref( map , key , obs_vector_get_state_kw(obs_vector));
    key = hash_iter_get_next_key(iter);
  }
  hash_iter_free( iter );
  return map;
}


hash_iter_type * enkf_obs_alloc_iter( const enkf_obs_type * enkf_obs ) {
  return hash_iter_alloc(enkf_obs->obs_hash);
}


const char * enkf_obs_get_config_file( const enkf_obs_type * enkf_obs) {
  return enkf_obs->config_file;
}

void enkf_obs_fprintf_config( const enkf_obs_type * enkf_obs , FILE * stream) {
  if (enkf_obs->config_file != NULL) {
    fprintf( stream , CONFIG_COMMENTLINE_FORMAT );
    fprintf( stream , CONFIG_COMMENT_FORMAT , "The observations are stored in a separate config file.");
    fprintf( stream , CONFIG_COMMENT_FORMAT , "Unfortunately there is separate config language for the observations ... :-(");
    fprintf(stream , CONFIG_KEY_FORMAT      , OBS_CONFIG_KEY);
    fprintf(stream , CONFIG_ENDVALUE_FORMAT , enkf_obs->config_file );
    fprintf(stream , "\n\n");
  }
}


/**
   This function takes a string like this: "PRESSURE:1,4,7" - it
   splits the string on ":" and tries to lookup a config object with
   that key. For the general string A:B:C:D it will try consecutively
   the keys: A, A:B, A:B:C, A:B:C:D. If a config object is found it is
   returned, otherwise NULL is returned.

   The last argument is the pointer to a string which will be updated
   with the node-spesific part of the full key. So for instance with
   the example "PRESSURE:1,4,7", the index_key will contain
   "1,4,7". If the full full_key is used to find an object index_key
   will be NULL, that also applies if no object is found.
*/



const obs_vector_type * enkf_obs_user_get_vector(const enkf_obs_type * obs , const char  * full_key, char ** index_key ) {
  const obs_vector_type * vector = NULL;
  char ** key_list;
  int     keys;
  int     key_length = 1;
  int offset;

  *index_key = NULL;
  util_split_string(full_key , ":" , &keys , &key_list);
  while (vector == NULL && key_length <= keys) {
    char * current_key = util_alloc_joined_string( (const char **) key_list , key_length , ":");
    if (enkf_obs_has_key(obs , current_key))
      vector = enkf_obs_get_vector(obs , current_key);
    else
      key_length++;
    offset = strlen( current_key );
    free( current_key );
  }
  if (vector != NULL) {
    if (offset < strlen( full_key ))
      *index_key = util_alloc_string_copy(&full_key[offset+1]);
  }

  util_free_stringlist(key_list , keys);
  return vector;
}

/*****************************************************************/

void enkf_obs_scale_std(enkf_obs_type * enkf_obs, double scale_factor) {

  hash_iter_type * observation_vector_iterator = enkf_obs_alloc_iter(enkf_obs);

  while (!hash_iter_is_complete(observation_vector_iterator)) {
    obs_vector_type * current_obs_vector = hash_iter_get_next_value(observation_vector_iterator);
    obs_vector_scale_std(current_obs_vector, scale_factor);
  }
  
  hash_iter_free(observation_vector_iterator);
}

