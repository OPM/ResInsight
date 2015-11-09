/*
   Copyright (C) 2015  Statoil ASA, Norway.

   The file 'job_status_test.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef __JOB_QUEUE_STATUS_H__
#define __JOB_QUEUE_STATUS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/type_macros.h>
#include <ert/job_queue/queue_driver.h>

  typedef struct job_queue_status_struct job_queue_status_type;

  job_queue_status_type * job_queue_status_alloc();
  void job_queue_status_free( job_queue_status_type * status );
  int  job_queue_status_get_count( job_queue_status_type * status , job_status_type status_type);
  void job_queue_status_clear( job_queue_status_type * status );
  void job_queue_status_inc( job_queue_status_type * status_count , job_status_type status_type);
  bool job_queue_status_transition( job_queue_status_type * status_count , job_status_type src_status , job_status_type target_status);
  int job_queue_status_get_total_count( const job_queue_status_type * status );
  const char * job_queue_status_name( job_status_type status );

  UTIL_IS_INSTANCE_HEADER( job_queue_status );
  UTIL_SAFE_CAST_HEADER( job_queue_status );

#ifdef __cplusplus
}
#endif
#endif
