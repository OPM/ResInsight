/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'esummary.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <string.h>
#include <signal.h>

#include <ert/util/util.h>
#include <ert/util/vector.h>
#include <ert/util/time_interval.h>

#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_sum.h>


void install_SIGNALS(void) {
  signal(SIGSEGV , util_abort_signal);    /* Segmentation violation, i.e. overwriting memory ... */
  signal(SIGINT  , util_abort_signal);    /* Control C */
  signal(SIGTERM , util_abort_signal);    /* If killing the enkf program with SIGTERM (the default kill signal) you will get a backtrace. Killing with SIGKILL (-9) will not give a backtrace.*/
}

void usage() {
  fprintf(stderr," The esummary.x program can be used to extract summary vectors from \n");
  fprintf(stderr," an ensemble of summary files: \n\n");
  fprintf(stderr,"    bash%% esummary.x  ECLIPSE1.DATA ECLIPSE2.DATA  KEY1  KEY2  ... \n\n"); 
  exit(1);
}


#define MISSING_STRING "       0.000"



int main(int argc , char ** argv) {
  install_SIGNALS();
  {
    if (argc < 3) usage(); 

    {
      ecl_sum_type * first_ecl_sum = NULL;   /* This governs the timing */
      vector_type  * ecl_sum_list = vector_alloc_new();
      time_interval_type * time_union = NULL;
      time_interval_type * time_intersect = NULL;
      time_interval_type * time = NULL;
      bool use_time_union = true;
      
      int load_count = 0;
      int nvars;
      char ** var_list;
      bool *  has_var;

      /** Loading the data */
      {
        int iarg     = 1;
        /*  Some sorting here???  */
        while (iarg < argc && util_file_exists( argv[iarg] )) {
          char * path , * basename;
          ecl_sum_type * ecl_sum;
          util_alloc_file_components( argv[iarg] , &path , &basename  , NULL); 
          fprintf(stderr,"Loading case: %s/%s" , path , basename); fflush(stderr);
          ecl_sum = ecl_sum_fread_alloc_case( argv[iarg] , ":");
          if (ecl_sum) {
            if (first_ecl_sum == NULL) {
              first_ecl_sum = ecl_sum;  /* Keep track of this  - might sort the vector */
              time_union = time_interval_alloc_copy( ecl_sum_get_sim_time( ecl_sum ));
              time_intersect = time_interval_alloc_copy( ecl_sum_get_sim_time( ecl_sum ));
              
              if (use_time_union)
                time = time_union;
              else
                time = time_intersect;

            vector_append_owned_ref( ecl_sum_list , ecl_sum , ecl_sum_free__ );
            load_count++;
            } else {
              const time_interval_type * ti = ecl_sum_get_sim_time( ecl_sum );
              if (time_interval_has_overlap(time , ti)) {
                time_interval_intersect( time_intersect , ti );
                time_interval_extend( time_union , ti );
                
                vector_append_owned_ref( ecl_sum_list , ecl_sum , ecl_sum_free__ );
                load_count++;
              } else {
                fprintf(stderr,"** Warning case:%s has no time overlap - discarded \n",ecl_sum_get_case( ecl_sum ));
                ecl_sum_free( ecl_sum );
              }
            }
          } else 
            fprintf(stderr," --- no data found?!");
          
          iarg++;
          fprintf(stderr,"\n");
          util_safe_free( path );
          free( basename );
        }
      }
      if (load_count == 0)
        usage();

      nvars    =  argc - load_count - 1;
      if (nvars == 0) util_exit(" --- No variables \n");
      var_list = &argv[load_count + 1];
      has_var = util_calloc( nvars , sizeof * has_var );
      
      /* 
         Checking that the summary files have the various variables -
         if a variable is missing from one of the cases it is
         completely discarded.
      */
      {
        int iens,ivar;
        
        /* Checking the variables */
        for (ivar = 0; ivar < nvars; ivar++) 
          has_var[ivar] = true;
        
        for (iens = 0; iens < vector_get_size( ecl_sum_list ); iens++) {
          const ecl_sum_type * ecl_sum = vector_iget_const( ecl_sum_list , iens );
          for (ivar = 0; ivar < nvars; ivar++) {
            if (has_var[ivar]) {
              if (!ecl_sum_has_general_var( ecl_sum , var_list[ivar] )) {
                fprintf(stderr,"** Warning: could not find variable: \'%s\' in case:%s - completely discarded.\n", var_list[ivar] , ecl_sum_get_case(ecl_sum));
                has_var[ivar] = false;
              }
            }
          }
        }
      }
      
      if (!time_interval_equal(time_union , time_intersect )) {
        fprintf(stderr,"** Warning: not all simulations have the same length. ");
        if (use_time_union)
          fprintf(stderr,"Using %s for missing values.\n" , MISSING_STRING);
        else
          fprintf(stderr,"Only showing common time period.\n");
      }
      
      /** The actual summary lookup. */
      {
        time_t_vector_type * date_list = time_t_vector_alloc(0,0);
        time_t start_time = time_interval_get_start( time );
        FILE * stream = stdout;
        int iens,ivar,itime;
        
        ecl_util_init_month_range( date_list , time_interval_get_start( time ) , time_interval_get_end( time ));
        for (itime = 0; itime < time_t_vector_size( date_list ); itime++) {
          time_t current_time = time_t_vector_iget( date_list , itime );
          for (ivar = 0; ivar < nvars; ivar++) {                                          /* Iterating over the variables */
            if (has_var[ivar]) { 
              for (iens = 0; iens < vector_get_size( ecl_sum_list ); iens++) {            /* Iterating over the ensemble members */
                const ecl_sum_type * ecl_sum = vector_iget_const( ecl_sum_list , iens );  
                
                if (ivar == 0 && iens == 0) {                                             /* Display time info in the first columns */
                  int day,month,year;
                  util_set_date_values_utc( current_time , &day , &month, &year);
                  fprintf(stream , "%7.2f   %02d/%02d/%04d   " , util_difftime_days( start_time , current_time ) , day , month , year);
                }
                
                {
                  const time_interval_type * sim_time = ecl_sum_get_sim_time( ecl_sum );

                  if (time_interval_arg_before( sim_time , current_time)) 
                    fprintf(stream , " %s " , MISSING_STRING); // We are before this case has data.
                  else if (time_interval_arg_after( sim_time , current_time)) 
                    fprintf(stream , " %s " , MISSING_STRING); // We are after this case has data.
                  else {
                    double value = ecl_sum_get_general_var_from_sim_time(ecl_sum , current_time , var_list[ivar]);
                    fprintf(stream , " %12.3f " , value);
                  } 

                }
              }
            }
          }
          fprintf(stream , "\n");
        }
        time_t_vector_free( date_list );
      }
      vector_free( ecl_sum_list );
      free( has_var );
    }
  }
}
