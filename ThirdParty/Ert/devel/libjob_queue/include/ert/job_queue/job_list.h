/*
   Copyright (C) 2015  Statoil ASA, Norway.

   The file 'job_node.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef __JOB_LIST_H__
#define __JOB_LIST_H__


#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/type_macros.h>

#include <ert/job_queue/queue_driver.h>
#include <ert/job_queue/job_node.h>


typedef struct job_list_struct job_list_type;

  job_list_type * job_list_alloc();
  void job_list_free( job_list_type * job_list );
  int job_list_get_size( const job_list_type * job_list );
  void job_list_add_job( job_list_type * job_list , job_queue_node_type * job_node );
  job_queue_node_type * job_list_iget_job( const job_list_type * job_list , int queue_index);
  void job_list_reset( job_list_type * job_list );
  void job_list_get_wrlock( job_list_type * list);
  void job_list_get_rdlock( job_list_type * list);
  void job_list_reader_wait( job_list_type * list, int usleep_time1, int usleep_time2);
  void job_list_unlock( job_list_type * list);

  UTIL_SAFE_CAST_HEADER( job_list );
  UTIL_IS_INSTANCE_HEADER( job_list );

#ifdef __cplusplus
}
#endif
#endif

