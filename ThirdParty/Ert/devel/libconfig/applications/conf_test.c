/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'conf_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <conf.h>

int main()
{
  const char * enkf_conf_help = "The main enkf conf shall contain neccessary infomation to run the enkf.";
  conf_class_type * enkf_conf_class = conf_class_alloc_empty("ENKF_conf", true, false, enkf_conf_help);
  conf_class_set_help(enkf_conf_class, enkf_conf_help);
  
  /** Create and insert HISTORY_OBSERVATION class. */
  {
    const char * help_class_history_observation = "The class HISTORY_OBSERVATION is used to condition on a time series from the production history. The name of the an instance is used to define the item to condition on, and should be in summary.x syntax. E.g., creating a HISTORY_OBSERVATION instance with name GOPR:P4 conditions on GOPR for group P4.";
    conf_class_type * history_observation_class = conf_class_alloc_empty("HISTORY_OBSERVATION", false, false, help_class_history_observation);
    conf_class_set_help(history_observation_class, help_class_history_observation);

    
    const char * help_item_spec_error_mode = "The string ERROR_MODE gives the error mode for the observation.";
    conf_item_spec_type * item_spec_error_mode = conf_item_spec_alloc("ERROR_MODE", true, DT_STR, help_item_spec_error_mode);
    conf_item_spec_set_help(item_spec_error_mode, help_item_spec_error_mode);
    
    conf_item_spec_add_restriction(item_spec_error_mode, "rel");
    conf_item_spec_add_restriction(item_spec_error_mode, "abs");
    conf_item_spec_add_restriction(item_spec_error_mode, "relmin");
    
    conf_item_spec_set_default_value(item_spec_error_mode, "rel");
    

    const char * help_item_spec_error = "The positive floating number ERROR gives the standard deviation (abs) or the relative uncertainty (rel/relmin) of the observations.";
    conf_item_spec_type * item_spec_error     = conf_item_spec_alloc("ERROR", true, DT_POSFLOAT, help_item_spec_error);
    conf_item_spec_set_default_value(item_spec_error, "0.10");
    conf_item_spec_set_help(item_spec_error, help_item_spec_error);

    const char * help_item_spec_error_min = "The positive floating point number ERROR_MIN gives the minimum value for the standard deviation of the observation when relmin is used.";
    conf_item_spec_type * item_spec_error_min = conf_item_spec_alloc("ERROR_MIN", true, DT_POSFLOAT, help_item_spec_error_min);
    conf_item_spec_set_default_value(item_spec_error_min, "0.10");
    conf_item_spec_set_help(item_spec_error_min, help_item_spec_error_min);
    
    
    conf_class_insert_owned_item_spec(history_observation_class, item_spec_error_mode);
    conf_class_insert_owned_item_spec(history_observation_class, item_spec_error);
    conf_class_insert_owned_item_spec(history_observation_class, item_spec_error_min);

    conf_class_insert_owned_sub_class(enkf_conf_class, history_observation_class);
  }



  /** Create and insert SUMMARY_OBSERVATION class. */
  {
    const char * help_class_summary_observation = "The class SUMMARY_OBSERVATION can be used to condition on any observation whos simulated value is written to the summary file.";
    conf_class_type * summary_observation_class = conf_class_alloc_empty("SUMMARY_OBSERVATION", false, false, help_class_summary_observation);
    conf_class_set_help(summary_observation_class, help_class_summary_observation);

    const char * help_item_spec_value = "The floating point number VALUE gives the observed value.";
    conf_item_spec_type * item_spec_value = conf_item_spec_alloc("VALUE", true, DT_FLOAT, help_item_spec_value);
    conf_item_spec_set_help(item_spec_value, help_item_spec_value);
  

    const char * help_item_spec_error = "The positive floating point number ERROR is the standard deviation of the observed value.";
    conf_item_spec_type * item_spec_error = conf_item_spec_alloc("ERROR", true, DT_POSFLOAT, help_item_spec_error);
    conf_item_spec_set_help(item_spec_error, help_item_spec_error);

    const char * help_item_spec_date = "The DATE item gives the date of the observation. Format is dd/mm/yyyy.";
    conf_item_spec_type * item_spec_date = conf_item_spec_alloc("DATE", false, DT_DATE, help_item_spec_date);
    conf_item_spec_set_help(item_spec_date, help_item_spec_date);

    const char * help_item_spec_days = "The DAYS item gives the observation time as days after simulation start.";
    conf_item_spec_type * item_spec_days = conf_item_spec_alloc("DAYS", false, DT_POSFLOAT, help_item_spec_days);
    conf_item_spec_set_help(item_spec_days, help_item_spec_days);

    const char * help_item_spec_restart = "The RESTART item gives the observation time as the ECLIPSE restart nr.";
    conf_item_spec_type * item_spec_restart = conf_item_spec_alloc("RESTART", false, DT_POSINT, help_item_spec_restart);
    conf_item_spec_set_help(item_spec_restart, help_item_spec_restart);

    const char * help_item_spec_sumkey = "The string SUMMARY_KEY is used to look up the simulated value in the summary file. It has the same format as the summary.x program, e.g. WOPR:P4";
    conf_item_spec_type * item_spec_sumkey = conf_item_spec_alloc("KEY", true, DT_STR, help_item_spec_sumkey);
    conf_item_spec_set_help(item_spec_sumkey, help_item_spec_sumkey);

    conf_class_insert_owned_item_spec(summary_observation_class, item_spec_value);
    conf_class_insert_owned_item_spec(summary_observation_class, item_spec_error);
    conf_class_insert_owned_item_spec(summary_observation_class, item_spec_date);
    conf_class_insert_owned_item_spec(summary_observation_class, item_spec_days);
    conf_class_insert_owned_item_spec(summary_observation_class, item_spec_restart);
    conf_class_insert_owned_item_spec(summary_observation_class, item_spec_sumkey);

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
    conf_class_type * block_observation_class = conf_class_alloc_empty("BLOCK_OBSERVATION", false, false, help_class_block_observation);
    conf_class_set_help(block_observation_class, help_class_block_observation);

    const char * help_item_spec_field = "The item FIELD gives the observed field. E.g., ECLIPSE fields such as PRESSURE, SGAS or any user defined fields such as PORO or PERMX.";
    conf_item_spec_type * item_spec_field = conf_item_spec_alloc("FIELD", true, DT_STR, help_item_spec_field);
    conf_item_spec_set_help(item_spec_field, help_item_spec_field);

    const char * help_item_spec_date = "The DATE item gives the date of the observation. Format is dd/mm/yyyy.";
    conf_item_spec_type * item_spec_date = conf_item_spec_alloc("DATE", true, DT_DATE, help_item_spec_date);
    conf_item_spec_set_help(item_spec_date, help_item_spec_date);

    conf_class_insert_owned_item_spec(block_observation_class, item_spec_field);
    conf_class_insert_owned_item_spec(block_observation_class, item_spec_date);

    /** Create and insert the sub class OBS. */
    {
      const char * help_class_obs = "The class OBS is used to specify a single observed point.";
      conf_class_type * obs_class = conf_class_alloc_empty("OBS", true, true , help_class_obs);
      conf_class_set_help(obs_class, help_class_obs);

      const char * help_item_i = "The item I gives the I index of the block observation.";
      conf_item_spec_type * item_spec_i = conf_item_spec_alloc("I", true, DT_POSINT, help_item_i);
      conf_item_spec_set_help(item_spec_i, help_item_i);

      const char * help_item_j = "The item J gives the J index of the block observation.";
      conf_item_spec_type * item_spec_j = conf_item_spec_alloc("J", true, DT_POSINT, help_item_j);
      conf_item_spec_set_help(item_spec_j, help_item_j);

      const char * help_item_k = "The item K gives the K index of the block observation.";
      conf_item_spec_type * item_spec_k = conf_item_spec_alloc("K", true, DT_POSINT, help_item_k);
      conf_item_spec_set_help(item_spec_k, help_item_k);

      const char * help_item_spec_value = "The floating point number VALUE gives the observed value.";
      conf_item_spec_type * item_spec_value = conf_item_spec_alloc("VALUE", true, DT_FLOAT, help_item_spec_value);
      conf_item_spec_set_help(item_spec_value, help_item_spec_value);
    
      const char * help_item_spec_error = "The positive floating point number ERROR is the standard deviation of the observed value.";
      conf_item_spec_type * item_spec_error = conf_item_spec_alloc("ERROR", true, DT_POSFLOAT, help_item_spec_error);
      conf_item_spec_set_help(item_spec_error, help_item_spec_error);

      conf_class_insert_owned_item_spec(obs_class, item_spec_i);
      conf_class_insert_owned_item_spec(obs_class, item_spec_j);
      conf_class_insert_owned_item_spec(obs_class, item_spec_k);
      conf_class_insert_owned_item_spec(obs_class, item_spec_value);
      conf_class_insert_owned_item_spec(obs_class, item_spec_error);

      conf_class_insert_owned_sub_class(block_observation_class, obs_class);
    }

    conf_class_insert_owned_sub_class(enkf_conf_class, block_observation_class);
  }

  

  /** Try to create an instance of the enkf_conf_class. */

  conf_instance_type * enkf_conf  = conf_instance_alloc_from_file(enkf_conf_class, "enkf_conf",  "testcase/test.txt");
  conf_instance_type * enkf_conf2 = conf_instance_alloc_from_file(enkf_conf_class, "enkf_conf2", "testcase/test2.txt");

  /** Validate enkf_conf_class. */

  conf_instance_validate(enkf_conf);
  conf_instance_validate(enkf_conf2);

  /** Overload. */
  conf_instance_overload(enkf_conf, enkf_conf2);

  
  /** Print the name of the HISTORY_OBSERVATION instances. */
  {
    stringlist_type * history_observations = conf_instance_alloc_list_of_sub_instances_of_class_by_name(enkf_conf, "HISTORY_OBSERVATION");

    int num_history_observations = stringlist_get_size(history_observations);
    
    for(int obs_nr = 0; obs_nr < num_history_observations ; obs_nr++)
    {
      printf("%2i. %s\n", obs_nr, stringlist_iget(history_observations, obs_nr));
      const conf_instance_type * sched_obs = conf_instance_get_sub_instance_ref(enkf_conf, stringlist_iget(history_observations, obs_nr));
      double error = conf_instance_get_item_value_double(sched_obs, "ERROR");
      printf("    std.dev :  %f\n", error);
    }

    stringlist_free(history_observations);
  }

  
  /** Clean up. */

  conf_instance_free(enkf_conf);
  conf_instance_free(enkf_conf2);
  conf_class_free(enkf_conf_class);

}
