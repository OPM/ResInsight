/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_sched.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include <ert/util/util.h>
#include <ert/util/stringlist.h>
#include <ert/util/vector.h>
#include <ert/util/parser.h>

#include <ert/enkf/enkf_types.h>
#include <ert/enkf/enkf_sched.h>
#include <ert/enkf/enkf_defaults.h>

/**
   How long is the simulation?
   ===========================
   
   There are some many configuration variables and usage patterns
   control the length of the simulation that you can seriously get
   dizzy - they all come with their own potential for +/- 1
   misunderstandings. 

   User options:
   -------------

     SCHEDULE_FILE: This is basically the schedule file given by the
        user which controls the historical period. This schedule file
        is guranteed to provide all members with the same DATES /
        TSTEP keywords.

     PREDICTION_SCHEDULE_FILE: This is optional file which the user
        can supply for predictions. It will be parsed and appended to
        the internal sched_file instance. Observe that the
        PREDICTION_SCHEDULE_FILE can be per. member, hence for the
        prediction part there is no longer a guarantee that all
        member simulations are equally long.

     ENKF_SCHED_FILE: With this configuration option the user can have
        reasonably fine-grained control on the time steps in the
        simulation. The simulation will be controlled by an enkf_sched
        instance (implemented in this file), to supply a
        ENKF_SCHED_FILE is optional, if no file is supplied by the
        user a default enkf_sched instance is allocated.


   Usage patterns:
   ---------------
   




   
   Offset +/- 1 convention:
   ------------------------
   
   The main 'master' of this information is the sched_file instance,
   that provides the function sched_file_get_num_restart_files() which
   returns the total number of restart files from a full simulation:


      schedule_file.inc
      -----------------
      --- Simulation START 1. JAN 2000  

      DATES 
        1  'FEB' 2000  /
      /

      
      DATES
        1  'MAR' 2000 /
      /


      DATES
        1  'APR' 2000 /
      /
      END

   This schedule file produce the following restart files when a full
   simulation is run:

      Restart_file   |    Corresponding date
      --------------------------------------
      BASE.X0000     |       1. JAN  2000
      BASE.X0001     |       1. FEB  2000
      BASE.X0002     |       1. MAR  2000
      BASE.X0003     |       1. APR  2000
      --------------------------------------

   So in this case the function sched_file_get_num_restarts() will
   return 4, but the last valid restart file has number '0003'. The
   fundamental query in this functionality is to query the sched_file
   instance, and that will return 4 in the example shown above. On the
   other hand most of the API and user interface in this application
   is based on an inclusive upper limit, i.e. we translate

      "Total number of restart files: 4" ==> "Number of the last restart file: 3"

   This translation is done 'immediately' the sched_file routine
   returns 4, and that should be immediately converted to three.



   Observe that the functions in this file are run - not at system
   bootstrap, but when starting a simulation.
*/



struct enkf_sched_node_struct {
  int                  report_step1;
  int                  report_step2;
  bool                 enkf_active;
};





struct enkf_sched_struct {
  vector_type    *nodes;                  /* Vector consisting of enkf_sched_node_type instances. */
  int             last_report;            /* The last report_step in this enkf_sched instance - internal convenience variable. */
};




static enkf_sched_node_type * enkf_sched_node_alloc(int report_step1 , int report_step2 , bool enkf_active) {
  enkf_sched_node_type * node = util_malloc(sizeof * node );
  node->report_step1  = report_step1;
  node->report_step2  = report_step2;
  node->enkf_active   = enkf_active;
  return node;
}



void enkf_sched_node_get_data(const enkf_sched_node_type * node , int * report_step1 , int * report_step2 , bool * enkf_on) {
  *report_step1    = node->report_step1;
  *report_step2    = node->report_step2;
  *enkf_on         = node->enkf_active;
}

int enkf_sched_node_get_last_step(const enkf_sched_node_type * node) {
  return node->report_step2;
}



static void enkf_sched_node_free(enkf_sched_node_type * node) {
  free(node);
}

static void enkf_sched_node_free__(void * arg) {
  enkf_sched_node_free( (enkf_sched_node_type *) arg );
}




static void enkf_sched_node_fprintf(const enkf_sched_node_type * node , FILE * stream) {
  if (node->enkf_active)
    fprintf(stream , "%4d   %4d   %s     ",node->report_step1 , node->report_step2 , "ON ");
  else
    fprintf(stream , "%4d   %4d   %s     ",node->report_step1 , node->report_step2 , "OFF");
  
  fprintf(stream , "\n");
}




/*****************************************************************/

static void enkf_sched_append_node(enkf_sched_type * enkf_sched , enkf_sched_node_type * new_node) {
  vector_append_owned_ref(enkf_sched->nodes , new_node , enkf_sched_node_free__);
  enkf_sched->last_report = new_node->report_step2;
}


/**
   This function scans a stream for the info it needs to allocate a
   enkf_sched_node_type instance. The format expected on the stream is as follows:

   REPORT_STEP1   REPORT_STEP2   ON|OFF   <STRIDE>   

   Observe the following:

   * If the list contains four or more items, we try to interpret item
     number four as a stride. 

   * If no stride is found, a default stride is used - the default
     stride is report_step2 - report_step1 (i.e. the whole thing in
     one go.)

   * If the stream is positioned at an empty line NULL is returned. No
     comments are supported.
*/


static void  enkf_sched_fscanf_alloc_nodes(enkf_sched_type * enkf_sched , FILE * stream , bool * at_eof) {
  enkf_sched_node_type * sched_node  = NULL;
  char ** token_list;
  bool enkf_active = false; /* Compiler shut up */
  int report_step1 , report_step2, report_stride;
  int tokens;
  
  char * line = util_fscanf_alloc_line(stream , at_eof);
  if (line != NULL) {
    util_split_string(line , " \t" , &tokens , &token_list);
    if (tokens >= 3) {
      if (util_sscanf_int(token_list[0] , &report_step1) && util_sscanf_int(token_list[1] , &report_step2)) {
        util_strupr(token_list[2]);
                  
        report_stride = report_step2 - report_step1;
        if (strcmp(token_list[2] , "ON") == 0) 
          enkf_active = true;
        else if (strcmp(token_list[2] , "OFF") == 0) 
          enkf_active = false;
        else 
          util_abort("%s: failed to interpret %s as ON || OFF \n",__func__ , token_list[2]);
        
        if (tokens > 3) 
          if (!util_sscanf_int(token_list[3] , &report_stride))
            util_abort("%s: failed to interpret:%s as an integer stride \n",__func__ , token_list[3]);
      } else
        util_abort("%s: failed to parse %s and %s as integers\n",__func__ , token_list[0] , token_list[1]);

      {
        /* Adding node(s): */
        int step1 = report_step1;
        int step2;
        
        do {
          step2 = util_int_min(step1 + report_stride , report_step2);
          sched_node = enkf_sched_node_alloc(step1 , step2 , enkf_active);
          step1 = step2;
          enkf_sched_append_node( enkf_sched , sched_node);
        } while (step2 < report_step2);
      }
    }
    util_free_stringlist(token_list , tokens);
    free(line);
  } 
}


/*****************************************************************/



static void enkf_sched_verify__(const enkf_sched_type * enkf_sched) {
  if (vector_get_size( enkf_sched->nodes ) == 0)
    /*
      In the case configurations with no dynamics at all, e.g. a pure
      RMS forward model we allow for a enkf_sched instance with no
      nodes.
    */
    return;
  else {
    int index;
    const enkf_sched_node_type * first_node = vector_iget_const( enkf_sched->nodes , 0);
    if (first_node->report_step1 != 0)
      util_abort("%s: must start at report-step 0 \n",__func__);
    
    for (index = 0; index < (vector_get_size(enkf_sched->nodes) - 1); index++) {
      const enkf_sched_node_type * node1 = vector_iget_const( enkf_sched->nodes , index );
      const enkf_sched_node_type * node2 = vector_iget_const( enkf_sched->nodes , index + 1);
      int report1      = node1->report_step1;
      int report2      = node1->report_step2;
      int next_report1 = node2->report_step1;
      
      if (report1 >= report2) {
        enkf_sched_fprintf(enkf_sched , stdout);
        util_abort("%s: enkf_sched step of zero/negative length:%d - %d - that is not allowed \n",__func__ , report1 , report2);
      }
      
      if (report2 != next_report1) {
        enkf_sched_fprintf(enkf_sched , stdout);
        util_abort("%s report steps must be continous - there is a gap.\n",__func__);
      }
    }
    
    /* Verify that report_step2 > report_step1 also for the last node. */
    {
      index = vector_get_size(enkf_sched->nodes) - 1;
      const enkf_sched_node_type * node1 = vector_iget_const( enkf_sched->nodes , index );
      int report1      = node1->report_step1;
      int report2      = node1->report_step2;
      
      if (report2 <= report1)
        util_abort("%s: enkf_sched step of zero/negative length:%d - %d - that is not allowed \n",__func__ , report1 , report2);
      
    }
  }
}





void enkf_sched_free( enkf_sched_type * enkf_sched) {
  vector_free( enkf_sched->nodes );
  free( enkf_sched );
}





static enkf_sched_type * enkf_sched_alloc_empty( ) {
  enkf_sched_type * enkf_sched      = util_malloc(sizeof * enkf_sched );
  enkf_sched->nodes                 = vector_alloc_new();  
  enkf_sched->last_report           = 0;
  return enkf_sched;
}



static void  enkf_sched_set_default(enkf_sched_type * enkf_sched , int last_history_restart , run_mode_type run_mode) {
  enkf_sched_node_type * node;

  if (run_mode == ENKF_ASSIMILATION) {
    /* Default enkf: stride one - active at all report steps. */
    /* Have to explicitly add all these nodes. */
    int report_step;
    for (report_step = 0; report_step < last_history_restart; report_step++) {   
      node = enkf_sched_node_alloc(report_step , report_step + 1, true );
      enkf_sched_append_node(enkf_sched , node);
    }
    /* Okay we are doing assimilation and prediction in one go - fair enough. */
  } else {
    /* 
       experiment: Do the whole thing in two steps, first the whole
       history, and then subsequently the prediction part (if there is
       any).
    */
    printf("Last: %d \n",last_history_restart);
    node = enkf_sched_node_alloc(0 , last_history_restart , false ); 
    enkf_sched_append_node(enkf_sched , node);
  }
}




/**
   This functions parses a config file, and builds a enkf_sched_type *
   instance from it. If the filename argument is NULL a default
   enkf_sched_type instance is allocated.
*/

enkf_sched_type * enkf_sched_fscanf_alloc(const char * enkf_sched_file , int last_history_restart , run_mode_type run_mode) {
  enkf_sched_type * enkf_sched = enkf_sched_alloc_empty( );
  if (enkf_sched_file == NULL)
    enkf_sched_set_default(enkf_sched , last_history_restart , run_mode);
  else {
    FILE * stream = util_fopen(enkf_sched_file , "r");
    bool at_eof;
    do { 
      enkf_sched_fscanf_alloc_nodes(enkf_sched , stream , &at_eof);
    } while (!at_eof);
    
    fclose( stream );
  }
  enkf_sched_verify__(enkf_sched);
  return enkf_sched;
}



void enkf_sched_fprintf(const enkf_sched_type * enkf_sched , FILE * stream) {
  int i;
  for (i=0; i < vector_get_size( enkf_sched->nodes ); i++) 
    enkf_sched_node_fprintf(vector_iget_const( enkf_sched->nodes , i) , stream );
  
}



int enkf_sched_get_last_report(const enkf_sched_type * enkf_sched) {
  return enkf_sched->last_report;
}

int enkf_sched_get_num_nodes(const enkf_sched_type * enkf_sched) {
  return vector_get_size( enkf_sched->nodes );
}



/**
   This function takes a report number, and returns the index of
   enkf_sched_node which contains (in the half-open interval: [...>)
   this report number. The function will abort if the node can be found.
*/
int enkf_sched_get_node_index(const enkf_sched_type * enkf_sched , int report_step) {
  if (report_step < 0 || report_step >= enkf_sched->last_report) {
    printf("Looking for report_step:%d \n", report_step);
    enkf_sched_fprintf(enkf_sched , stdout);
    util_abort("%s: could not find it ... \n",__func__);
    return -1;
  } else {
    int index = 0;
    while (1) {
      const enkf_sched_node_type * node = vector_iget_const(enkf_sched->nodes , index);
      if (node->report_step1 <= report_step && node->report_step2 > report_step)
        break;
      index++;
    }
    return index;
  }
}


const enkf_sched_node_type * enkf_sched_iget_node(const enkf_sched_type * enkf_sched , int index) {
  return vector_iget_const( enkf_sched->nodes , index );
}
