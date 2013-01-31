/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_sched.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __ENKF_SCHED_H__
#define __ENKF_SCHED_H__
#include <stdio.h>
#include <stdlib.h>

#include <ert/util/stringlist.h>

#include <ert/sched/sched_file.h>

#include <ert/enkf/enkf_types.h>


typedef struct enkf_sched_struct      enkf_sched_type;
typedef struct enkf_sched_node_struct enkf_sched_node_type;



void                           enkf_sched_fprintf(const enkf_sched_type *  , FILE * );
enkf_sched_type *              enkf_sched_fscanf_alloc(const char * , int , run_mode_type);
void                           enkf_sched_free(enkf_sched_type *);
int                            enkf_sched_get_num_nodes(const enkf_sched_type *);
int                            enkf_sched_get_last_report(const enkf_sched_type * enkf_sched);
int                            enkf_sched_get_node_index(const enkf_sched_type * , int );
const enkf_sched_node_type *   enkf_sched_iget_node(const enkf_sched_type * , int);
void                           enkf_sched_node_get_data(const enkf_sched_node_type * , int * , int * , bool * );
int                            enkf_sched_node_get_last_step(const enkf_sched_node_type * );
#endif
